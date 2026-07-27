#include "kerrwidget.h"
#include <QDebug>
#include <QOpenGLShader>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QImage>

KerrWidget::KerrWidget(QWidget* parent) : QOpenGLWidget(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    QSurfaceFormat fmt;
    // fmt.setSamples(4); // DISABLED: MSAA default FBO doesn't accept rendering
    fmt.setVersion(4, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(fmt);

    frameTimer.start();
    fpsTimer.start();

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        update();
    });
    timer->start(16); // ~60 FPS
}

KerrWidget::~KerrWidget() {
    makeCurrent();
    delete progA; delete progB; delete progC; delete progD; delete progImage;
    delete fboA0; delete fboA1; delete fboB0; delete fboB1; delete fboC; delete fboD;
    delete keyStateTexture;
    vbo.destroy();
    vao.destroy();
    doneCurrent();
}

void KerrWidget::setupQuad() {
    float quadVertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };

    vao.create();
    vao.bind();
    vbo.create();
    vbo.bind();
    vbo.allocate(quadVertices, sizeof(quadVertices));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    vbo.release();
    vao.release();
}

void KerrWidget::createFBOs() {
    int w = viewW, h = viewH;
    delete fboA0; delete fboA1; delete fboB0; delete fboB1; delete fboC; delete fboD;

    QOpenGLFramebufferObjectFormat fmt;
    fmt.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    fmt.setInternalTextureFormat(GL_RGBA32F);

    fboA0 = new QOpenGLFramebufferObject(w, h, fmt);
    fboA1 = new QOpenGLFramebufferObject(w, h, fmt);
    fboB0 = new QOpenGLFramebufferObject(w, h, fmt);
    fboB1 = new QOpenGLFramebufferObject(w, h, fmt);
    fboC  = new QOpenGLFramebufferObject(w, h, fmt);
    fboD  = new QOpenGLFramebufferObject(w, h, fmt);

    // Set LINEAR filtering on all FBO textures to prevent banding artifacts
    GLuint fboTextures[] = {
        fboA0->texture(), fboA1->texture(),
        fboB0->texture(), fboB1->texture(),
        fboC->texture(), fboD->texture()
    };
    for (GLuint tex : fboTextures) {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void KerrWidget::updateKeyStateTexture() {
    if (!keyStateTexture) {
        keyStateTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        keyStateTexture->setSize(256, 1);
        keyStateTexture->setFormat(QOpenGLTexture::RGBA32F);
        keyStateTexture->allocateStorage();
        keyStateTexture->setMinificationFilter(QOpenGLTexture::Nearest);
        keyStateTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
    }

    // Create key state data
    QVector<float> keyData(256 * 4, 0.0f);
    for (int key : pressedKeys) {
        if (key >= 0 && key < 256) {
            keyData[key * 4] = 1.0f;  // .x = pressed
        }
    }

    keyStateTexture->bind();
    keyStateTexture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32,
                             keyData.constData());
    keyStateTexture->release();
}

void KerrWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    setupQuad();
    viewW = width(); viewH = height();
    createFBOs();

    auto loadShader = [](const QString& name) -> QOpenGLShaderProgram* {
        auto* p = new QOpenGLShaderProgram();
        if (!p->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/kerr/screen.vert")) {
            qWarning() << "Failed to load vertex shader:" << p->log();
            delete p; return nullptr;
        }
        if (!p->addShaderFromSourceFile(QOpenGLShader::Fragment, name)) {
            qWarning() << "Failed to load fragment shader" << name << ":" << p->log();
            delete p; return nullptr;
        }
        if (!p->link()) {
            qWarning() << "Failed to link shader" << name << ":" << p->log();
            delete p; return nullptr;
        }
        return p;
    };

    progA = loadShader(":/shaders/kerr/kerr_a.frag");
    progB = loadShader(":/shaders/kerr/kerr_b.frag");
    progC = loadShader(":/shaders/kerr/kerr_c.frag");
    progD = loadShader(":/shaders/kerr/kerr_d.frag");
    progImage = loadShader(":/shaders/kerr/kerr_image.frag");

    updateKeyStateTexture();
}

void KerrWidget::resizeGL(int w, int h) {
    if (viewW == w && viewH == h) return;  // avoid unnecessary FBO recreation
    viewW = w; viewH = h;
    createFBOs();
}

void KerrWidget::renderPass(QOpenGLShaderProgram* prog, GLuint targetTexture,
                             GLuint tex0, GLuint tex1, GLuint tex2, GLuint tex3) {
    if (!prog) return;
    prog->bind();

    // Common uniforms
    prog->setUniformValue("iTime", totalTime);
    prog->setUniformValue("iTimeDelta", lastFrameDelta);
    prog->setUniformValue("iFrame", frameCount);
    prog->setUniformValue("iMouse", iMouse);
    prog->setUniformValue("iResolution", QVector2D(float(viewW), float(viewH)));

    // Channel resolutions
    QVector3D chanRes[4] = {
        QVector3D(float(viewW), float(viewH), 0),
        QVector3D(float(viewW), float(viewH), 0),
        QVector3D(float(viewW), float(viewH), 0),
        QVector3D(float(viewW), float(viewH), 0)
    };
    prog->setUniformValueArray("iChannelResolution", chanRes, 4);

    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);
    prog->setUniformValue("iChannel0", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex1);
    prog->setUniformValue("iChannel1", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex2);
    prog->setUniformValue("iChannel2", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, tex3);
    prog->setUniformValue("iChannel3", 3);

    // Draw full-screen quad
    vao.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    vao.release();

    prog->release();
}

void KerrWidget::paintGL() {
    // Calculate timing
    float elapsed = frameTimer.elapsed() / 1000.0f;
    lastFrameDelta = elapsed;
    totalTime += elapsed;
    frameTimer.restart();
    frameCount++;

    // FPS calculation (uses separate counter — never reset frameCount which is iFrame)
    fpsFrameCount++;
    if (fpsTimer.elapsed() > 500) {
        fps = fpsFrameCount * 1000.0f / fpsTimer.elapsed();
        fpsFrameCount = 0;
        fpsTimer.restart();
        emit fpsUpdated(fps);
    }

    updateKeyStateTexture();

    GLuint texKeyState = keyStateTexture ? keyStateTexture->textureId() : 0;
    GLuint texA0 = fboA0 ? fboA0->texture() : 0;
    GLuint texA1 = fboA1 ? fboA1->texture() : 0;
    GLuint texB0 = fboB0 ? fboB0->texture() : 0;
    GLuint texB1 = fboB1 ? fboB1->texture() : 0;
    GLuint texC  = fboC  ? fboC->texture()  : 0;
    GLuint texD  = fboD  ? fboD->texture()  : 0;

    GLuint prevA = (currentA == 0) ? texA1 : texA0;
    GLuint curA  = (currentA == 0) ? texA0 : texA1;
    QOpenGLFramebufferObject* curFboA = (currentA == 0) ? fboA0 : fboA1;

    GLuint prevB = (currentB == 0) ? texB1 : texB0;
    GLuint curB  = (currentB == 0) ? texB0 : texB1;
    QOpenGLFramebufferObject* curFboB = (currentB == 0) ? fboB0 : fboB1;

    // Pass 1: Buffer A — Ray trace Kerr black hole into FBO A
    // iChannel0=keyState, iChannel1=prevB, iChannel2=prevB (camera), iChannel3=prevA (TAA)
    if (curFboA) curFboA->bind();
    glViewport(0, 0, viewW, viewH);
    glClear(GL_COLOR_BUFFER_BIT);
    renderPass(progA, curA, texKeyState, prevB, prevB, prevA);
    if (curFboA) curFboA->release();

    // Pass 2: Buffer B — Bloom pyramid + camera state into FBO B
    // iChannel0=curA, iChannel1=prevB (self prev frame), iChannel2=keyState, iChannel3=keyState
    if (curFboB) curFboB->bind();
    glViewport(0, 0, viewW, viewH);
    glClear(GL_COLOR_BUFFER_BIT);
    renderPass(progB, curB, curA, prevB, texKeyState, texKeyState);
    if (curFboB) curFboB->release();

    // Pass 3: Buffer C — Horizontal blur into FBO C
    if (fboC) fboC->bind();
    glViewport(0, 0, viewW, viewH);
    glClear(GL_COLOR_BUFFER_BIT);
    renderPass(progC, texC, curB, 0, 0, 0);
    if (fboC) fboC->release();

    // Pass 4: Buffer D — Vertical blur into FBO D
    if (fboD) fboD->bind();
    glViewport(0, 0, viewW, viewH);
    glClear(GL_COLOR_BUFFER_BIT);
    renderPass(progD, texD, texC, 0, 0, 0);
    if (fboD) fboD->release();

    // Pass 5: Final composite to screen (default FBO active after fboD->release())
    glViewport(0, 0, viewW, viewH);
    glClear(GL_COLOR_BUFFER_BIT);
    renderPass(progImage, 0, curA, curB, texC, texD);

    // === Fast frame sampling for flicker diagnosis ===
    // Sample 4 corner pixels per frame to detect black frames without PNG overhead
    if (captureFrames && captureFrameCount < maxCaptureFrames && viewW > 0 && viewH > 0) {
        if (captureDir.isEmpty()) {
            captureDir = QString("D:/blackholecpp/frame_capture_%1")
                .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
            QDir().mkpath(captureDir);
        }
        // Sample 5 pixels: center + 4 corners
        float pixels[5][4];
        glReadPixels(viewW/2, viewH/2, 1, 1, GL_RGBA, GL_FLOAT, pixels[0]);      // center
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, pixels[1]);                    // bottom-left
        glReadPixels(viewW-1, 0, 1, 1, GL_RGBA, GL_FLOAT, pixels[2]);              // bottom-right
        glReadPixels(0, viewH-1, 1, 1, GL_RGBA, GL_FLOAT, pixels[3]);              // top-left
        glReadPixels(viewW-1, viewH-1, 1, 1, GL_RGBA, GL_FLOAT, pixels[4]);        // top-right

        float avg = 0;
        for (int i = 0; i < 5; i++) avg += (pixels[i][0] + pixels[i][1] + pixels[i][2]) / 3.0f;
        avg /= 5.0f;

        // Log to file: frameCount, iFrame, avgBrightness, center RGB
        QFile log(captureDir + "/pixel_log.txt");
        if (log.open(QIODevice::Append)) {
            log.write(QString("%1\t%2\t%3\t%4\t%5\t%6\n")
                .arg(captureFrameCount)
                .arg(frameCount)
                .arg(avg, 0, 'f', 6)
                .arg(pixels[0][0], 0, 'f', 4)
                .arg(pixels[0][1], 0, 'f', 4)
                .arg(pixels[0][2], 0, 'f', 4).toUtf8());
        }

        captureFrameCount++;
        if (captureFrameCount >= maxCaptureFrames) {
            QFile doneLog(captureDir + "/DONE.txt");
            doneLog.open(QIODevice::WriteOnly); doneLog.close();
            captureFrames = false;
        }
    }
    // === End fast frame sampling ===

    // Ping-pong
    currentA = 1 - currentA;
    currentB = 1 - currentB;
}

// === Input handlers ===
void KerrWidget::mousePressEvent(QMouseEvent* event) {
    mouseDown = true;
    lastMousePos = event->pos();
    iMouse = QVector4D(event->pos().x(), viewH - event->pos().y(), 1, 0);
}

void KerrWidget::mouseReleaseEvent(QMouseEvent* event) {
    mouseDown = false;
    iMouse = QVector4D(event->pos().x(), viewH - event->pos().y(), -1, 0);
}

void KerrWidget::mouseMoveEvent(QMouseEvent* event) {
    lastMousePos = event->pos();
    if (mouseDown) {
        iMouse = QVector4D(event->pos().x(), viewH - event->pos().y(), 1, 0);
    } else {
        iMouse = QVector4D(event->pos().x(), viewH - event->pos().y(), -abs(iMouse.z()), 0);
    }
}

void KerrWidget::wheelEvent(QWheelEvent* event) {
    // Pass wheel events through (not used by default shader)
    QOpenGLWidget::wheelEvent(event);
}

void KerrWidget::keyPressEvent(QKeyEvent* event) {
    pressedKeys.insert(event->nativeVirtualKey());
    // Also handle Qt key codes mapped to ShaderToy key codes
    switch (event->key()) {
    case Qt::Key_W: pressedKeys.insert(87); break;
    case Qt::Key_A: pressedKeys.insert(65); break;
    case Qt::Key_S: pressedKeys.insert(83); break;
    case Qt::Key_D: pressedKeys.insert(68); break;
    case Qt::Key_Q: pressedKeys.insert(81); break;
    case Qt::Key_E: pressedKeys.insert(69); break;
    case Qt::Key_R: pressedKeys.insert(82); break;
    case Qt::Key_F: pressedKeys.insert(70); break;
    default: break;
    }
}

void KerrWidget::keyReleaseEvent(QKeyEvent* event) {
    pressedKeys.remove(event->nativeVirtualKey());
    switch (event->key()) {
    case Qt::Key_W: pressedKeys.remove(87); break;
    case Qt::Key_A: pressedKeys.remove(65); break;
    case Qt::Key_S: pressedKeys.remove(83); break;
    case Qt::Key_D: pressedKeys.remove(68); break;
    case Qt::Key_Q: pressedKeys.remove(81); break;
    case Qt::Key_E: pressedKeys.remove(69); break;
    case Qt::Key_R: pressedKeys.remove(82); break;
    case Qt::Key_F: pressedKeys.remove(70); break;
    default: break;
    }
}

void KerrWidget::focusOutEvent(QFocusEvent* event) {
    pressedKeys.clear();
    QOpenGLWidget::focusOutEvent(event);
}

// === Public controls ===
void KerrWidget::setShowBloom(bool show) { showBloom = show; }
void KerrWidget::setShowTAA(bool show) { showTAA = show; }

void KerrWidget::resetCamera() {
    // The shader handles camera reset automatically when iFrame <= 5
    // Force reset by triggering a re-init next frame
    frameCount = 0;
}

void KerrWidget::setBlackHoleMass(double mass) { /* Controlled via shader defines */ }
void KerrWidget::setSpin(double spin) { /* Controlled via shader defines */ }
