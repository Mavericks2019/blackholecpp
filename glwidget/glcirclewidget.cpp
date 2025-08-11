#include "glcirclewidget.h"
#include <QDebug>
#include <QFile>
#include <QImage>
#include <cmath>

GLCircleWidget::GLCircleWidget(QWidget* parent) : QOpenGLWidget(parent) {
    setMinimumSize(600, 600);
    
    QSurfaceFormat fmt;
    fmt.setSamples(4); // 4x MSAA
    fmt.setVersion(4, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(fmt);
    
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        update();
    });
    timer->start(16); // ~60 FPS
    
    // Initialize pointers
    fbo = nullptr;
    prevFrameTexture = nullptr;
    screenProgram = nullptr;
    mipmapProgram = nullptr;
    mipmapFBO = nullptr;
}

void GLCircleWidget::initializeGL() {
    initializeOpenGLFunctions();

    frameTimer.start();
    lastFrameTime = frameTimer.elapsed() / 1000.0f;
    
    // Create main shader program
    program = new QOpenGLShaderProgram(this);
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, "../shaders/circle.vert")) {
        qDebug() << "Vertex shader error:" << program->log();
    }
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, "../shaders/circle.frag")) {
        qDebug() << "Fragment shader error:" << program->log();
    }
    if (!program->link()) {
        qDebug() << "Shader link error:" << program->log();
    }
    
    // Create screen shader program
    screenProgram = new QOpenGLShaderProgram(this);
    if (!screenProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "../shaders/screen.vert")) {
        qDebug() << "Screen vertex shader error:" << screenProgram->log();
    }
    if (!screenProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "../shaders/screen.frag")) {
        qDebug() << "Screen fragment shader error:" << screenProgram->log();
    }
    if (!screenProgram->link()) {
        qDebug() << "Screen shader link error:" << screenProgram->log();
    }
    
    // Create mipmap shader program
    mipmapProgram = new QOpenGLShaderProgram(this);
    if (!mipmapProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "../shaders/screen.vert")) {
        qDebug() << "Mipmap vertex shader error:" << mipmapProgram->log();
    }
    if (!mipmapProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "../shaders/mipmap.frag")) {
        qDebug() << "Mipmap fragment shader error:" << mipmapProgram->log();
    }
    if (!mipmapProgram->link()) {
        qDebug() << "Mipmap shader link error:" << mipmapProgram->log();
    }
    
    // Create VAO and VBO
    vao.create();
    vao.bind();
    
    vbo.create();
    vbo.bind();
    
    // Fullscreen quad vertices
    const float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
         
        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };
    vbo.allocate(vertices, sizeof(vertices));
    
    // Configure attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    // Create chess texture
    createChessTexture();
    
    vao.release();
}

void GLCircleWidget::paintGL() {
    // Calculate real time delta
    float currentTime = frameTimer.elapsed() / 1000.0f;
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;
    iTime += deltaTime;
    iFrame++;
    
    // Step 1: Render to FBO
    if (!fbo) {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setSamples(0);
        fbo = new QOpenGLFramebufferObject(width(), height(), format);
        
        prevFrameTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        prevFrameTexture->create();
        prevFrameTexture->bind();
        prevFrameTexture->setSize(width(), height());
        prevFrameTexture->setFormat(QOpenGLTexture::RGBA8_UNorm);
        prevFrameTexture->allocateStorage();
        prevFrameTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        prevFrameTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
        prevFrameTexture->release();
    }
    
    // Bind FBO for rendering
    fbo->bind();
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if (!program || !program->isLinked()) {
            fbo->release();
            return;
        }
        
        program->bind();
        vao.bind();
        
        // Set uniforms
        program->setUniformValue("circleColor", circleColor);
        program->setUniformValue("iResolution", width(), height());
        program->setUniformValue("offset", offset);
        program->setUniformValue("radius", radius);
        program->setUniformValue("MBlackHole", blackHoleMass);
        program->setUniformValue("backgroundType", backgroundType);
        program->setUniformValue("iFrame", iFrame);
        program->setUniformValue("iMouse", iMouse[0], iMouse[1], iMouse[2], iMouse[3]);
        program->setUniformValue("iTime", iTime);
        program->setUniformValue("iChannelResolution", 
            chessTextureResolution.x(), chessTextureResolution.y(), chessTextureResolution.z());
        program->setUniformValue("iTimeDelta", deltaTime);
        
        // Bind chess texture
        if (chessTexture) {
            glActiveTexture(GL_TEXTURE1);
            chessTexture->bind();
            program->setUniformValue("iChannel1", 1);
        }
        
        // Bind previous frame texture
        if (prevFrameTexture && iFrame > 0) {
            glActiveTexture(GL_TEXTURE3);
            prevFrameTexture->bind();
            program->setUniformValue("iChannel3", 3);
        }
        
        // Draw fullscreen quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Copy current frame to previous frame texture
        if (prevFrameTexture) {
            glBindTexture(GL_TEXTURE_2D, prevFrameTexture->textureId());
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width(), height());
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        vao.release();
        program->release();
    }
    fbo->release();
    
    // Apply mipmap effect if enabled
    GLuint textureToRender = fbo->texture();
    
    if (showMipmap) {
        // Create mipmap FBO if needed
        if (!mipmapFBO) {
            QOpenGLFramebufferObjectFormat format;
            format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
            format.setSamples(0);
            mipmapFBO = new QOpenGLFramebufferObject(width(), height(), format);
        }
        
        // Bind mipmap FBO
        mipmapFBO->bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        mipmapProgram->bind();
        vao.bind();
        
        // Bind FBO texture as input
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fbo->texture());
        mipmapProgram->setUniformValue("iChannel0", 0);
        mipmapProgram->setUniformValue("iResolution", QVector2D(width(), height()));
        
        // Draw fullscreen quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        vao.release();
        mipmapProgram->release();
        mipmapFBO->release();
        
        // Use mipmap processed texture
        textureToRender = mipmapFBO->texture();
    }
    
    // Step 3: Render to screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    screenProgram->bind();
    vao.bind();
    
    // Bind texture to render
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureToRender);
    screenProgram->setUniformValue("screenTexture", 0);
    
    // Draw fullscreen quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    vao.release();
    screenProgram->release();
}

void GLCircleWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    updateAspectRatio();
    
    // Delete old FBO and textures
    if (fbo) {
        delete fbo;
        fbo = nullptr;
    }
    if (prevFrameTexture) {
        delete prevFrameTexture;
        prevFrameTexture = nullptr;
    }
    if (mipmapFBO) {
        delete mipmapFBO;
        mipmapFBO = nullptr;
    }
    
    update();
}

void GLCircleWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        mousePressed = true;
        lastMousePos = event->pos();
        
        QPointF pos = event->pos();
        iMouse[2] = pos.x();
        iMouse[3] = height() - pos.y();
        
        iMouse[0] = pos.x();
        iMouse[1] = height() - pos.y();
        
        update();
    }
}

void GLCircleWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        mousePressed = false;
        iMouse[2] = 0.0f;
        iMouse[3] = 0.0f;
        
        QPointF pos = event->pos();
        iMouse[0] = pos.x();
        iMouse[1] = height() - pos.y();
        
        update();
    }
}

void GLCircleWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!mousePressed) return;
        
    QPointF pos = event->pos();
    iMouse[0] = pos.x();
    iMouse[1] = height() - pos.y();
    
    QPoint delta = event->pos() - lastMousePos;
    lastMousePos = event->pos();
    
    iMouse[2] += delta.x();
    iMouse[3] -= delta.y();
    
    update();
}

void GLCircleWidget::createChessTexture() {
    const int size = 64;
    QImage image(size, size, QImage::Format_RGBA8888);
    
    QColor color1(220, 220, 220);
    QColor color2(80, 80, 100);
    
    const int tileSize = size / 8;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int tileX = x / tileSize;
            int tileY = y / tileSize;
            image.setPixelColor(x, y, (tileX + tileY) % 2 == 0 ? color1 : color2);
        }
    }
    
    chessTexture = new QOpenGLTexture(image);
    chessTexture->setWrapMode(QOpenGLTexture::Repeat);
    chessTexture->setMinificationFilter(QOpenGLTexture::Linear);
    chessTexture->setMagnificationFilter(QOpenGLTexture::Linear);
}

void GLCircleWidget::updateAspectRatio() {
    float w = width();
    float h = height();
    
    if (w == 0 || h == 0) return;
    
    QString ratioText;
    if (w > h) {
        ratioText = QString("Current: %1 : 1").arg(w/h, 0, 'f', 2);
    } else {
        ratioText = QString("Current: 1 : %1").arg(h/w, 0, 'f', 2);
    }
    
    emit aspectRatioChanged(ratioText);
}

void GLCircleWidget::setBackgroundType(int type) {
    backgroundType = type;
    update();
}

void GLCircleWidget::setShowMipmap(bool show) {
    showMipmap = show;
    update();
}