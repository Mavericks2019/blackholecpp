#include "glcirclewidget.h"
#include <QDebug>
#include <QFile>
#include <QImage>
#include <cmath>
#include <QPainter>

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
    horizontalFBO = nullptr;
    verticalFBO = nullptr;
    
    // 初始化帧率计数器
    frameCount = 0;
    fps = 0.0f;
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

    // Create horizontal blur shader program
    horizontalProgram = new QOpenGLShaderProgram(this);
    if (!horizontalProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "../shaders/screen.vert")) {
        qDebug() << "Horizontal vertex shader error:" << horizontalProgram->log();
    }
    if (!horizontalProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "../shaders/horizontal.frag")) {
        qDebug() << "Horizontal fragment shader error:" << horizontalProgram->log();
    }
    if (!horizontalProgram->link()) {
        qDebug() << "Horizontal shader link error:" << horizontalProgram->log();
    }

    // Create vertical blur shader program
    verticalProgram = new QOpenGLShaderProgram(this);
    if (!verticalProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "../shaders/screen.vert")) {
        qDebug() << "Vertical vertex shader error:" << verticalProgram->log();
    }
    if (!verticalProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "../shaders/vertical.frag")) {
        qDebug() << "Vertical fragment shader error:" << verticalProgram->log();
    }
    if (!verticalProgram->link()) {
        qDebug() << "Vertical shader link error:" << verticalProgram->log();
    }

    // Create vertical blur shader program
    resultProgram = new QOpenGLShaderProgram(this);
    if (!resultProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "../shaders/screen.vert")) {
        qDebug() << "Vertical vertex shader error:" << resultProgram->log();
    }
    if (!resultProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "../shaders/screen_result.frag")) {
        qDebug() << "Vertical fragment shader error:" << resultProgram->log();
    }
    if (!resultProgram->link()) {
        qDebug() << "Vertical shader link error:" << resultProgram->log();
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
    // === 帧率计算开始 ===
    frameCount++;
    
    // 每0.5秒更新一次帧率
    if (fpsTimer.isValid() && fpsTimer.elapsed() > 500) {
        fps = frameCount * 1000.0f / fpsTimer.elapsed();
        frameCount = 0;
        fpsTimer.restart();
    } else if (!fpsTimer.isValid()) {
        fpsTimer.start();
    }
    // === 帧率计算结束 ===
    
    // 计算真实的时间增量
    float currentTime = frameTimer.elapsed() / 1000.0f;
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;
    iTime += deltaTime;
    iFrame++;
    
    // 第一步：渲染到帧缓冲
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
        
        GLuint texId = fbo->texture();
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    // 绑定FBO进行渲染
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
    // 保存原始渲染纹理
    GLuint originalTexture = fbo->texture();
    fbo->release();
    // 初始化处理后的纹理为原始纹理
    GLuint processedTexture = originalTexture;
    
    // 应用 mipmap 效果
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
        
        // Bind input texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, processedTexture);  // 使用当前处理后的纹理
        mipmapProgram->setUniformValue("iChannel0", 0);
        mipmapProgram->setUniformValue("iResolution", width(), height());
        
        // Draw fullscreen quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        vao.release();
        mipmapProgram->release();
        mipmapFBO->release();
        
        // Update processed texture
        processedTexture = mipmapFBO->texture();
    }
    
    // 应用水平模糊
    if (horizontal) {
        // 创建水平模糊FBO（如果需要）
        if (!horizontalFBO) {
            QOpenGLFramebufferObjectFormat format;
            format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
            format.setSamples(0);
            horizontalFBO = new QOpenGLFramebufferObject(width(), height(), format);

            // 设置纹理过滤和环绕模式
            GLuint texId = horizontalFBO->texture();
            glBindTexture(GL_TEXTURE_2D, texId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        
        // 绑定水平模糊FBO
        horizontalFBO->bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        horizontalProgram->bind();
        vao.bind();
        
        // 绑定输入纹理（使用当前处理后的纹理）
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, processedTexture);
        horizontalProgram->setUniformValue("iChannel0", 0);
        horizontalProgram->setUniformValue("iResolution", width(), height());
        
        // 绘制全屏四边形
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        vao.release();
        horizontalProgram->release();
        horizontalFBO->release();
        
        // 更新处理后的纹理
        processedTexture = horizontalFBO->texture();
    }
    
    // 应用垂直模糊
    if (vertical) {
        // 创建垂直模糊FBO（如果需要）
        if (!verticalFBO) {
            QOpenGLFramebufferObjectFormat format;
            format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
            format.setSamples(0);
            verticalFBO = new QOpenGLFramebufferObject(width(), height(), format);
            // 设置纹理过滤和环绕模式
            GLuint texId = verticalFBO->texture();
            glBindTexture(GL_TEXTURE_2D, texId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        
        // 绑定垂直模糊FBO
        verticalFBO->bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        verticalProgram->bind();
        vao.bind();
        
        // 绑定输入纹理（使用当前处理后的纹理）
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, processedTexture);
        verticalProgram->setUniformValue("iChannel0", 0);
        verticalProgram->setUniformValue("iResolution", width(), height());
        
        // 绘制全屏四边形
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        vao.release();
        verticalProgram->release();
        verticalFBO->release();
        
        // 更新处理后的纹理
        processedTexture = verticalFBO->texture();
    }
    
    // 保存Bloom纹理（处理后的纹理）
    GLuint bloomTexture = processedTexture;
    
    // Step 3: Render to screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (result) {
        // 使用resultProgram（screen_result.frag）
        resultProgram->bind();
        vao.bind();
        
        // // 绑定原始纹理到iChannel0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, originalTexture);
        resultProgram->setUniformValue("iChannel0", 0);
        
        // 绑定Bloom纹理到iChannel3
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, bloomTexture);
        resultProgram->setUniformValue("iChannel3", 3);
        
        // 设置分辨率uniform
        resultProgram->setUniformValue("iResolution", QVector2D(width(), height()));
        
        // 绘制全屏四边形
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        vao.release();
        resultProgram->release();
    } else {
        // 使用原来的screenProgram
        screenProgram->bind();
        vao.bind();
        
        // 绑定要渲染的纹理（可能是原始纹理或处理后的纹理）
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, processedTexture);
        screenProgram->setUniformValue("screenTexture", 0);
        
        // 绘制全屏四边形
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        vao.release();
        screenProgram->release();
    }
    
    // === 在右下角绘制帧率 ===
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    
    // 创建半透明背景
    QRect textRect(width() - 120, height() - 40, 110, 30);
    painter.fillRect(textRect, QColor(0, 0, 0, 150));
    
    // 绘制帧率文本
    QString fpsText = QString("FPS: %1").arg(fps, 0, 'f', 1);
    painter.drawText(textRect, Qt::AlignCenter, fpsText);
    // === 帧率绘制结束 ===
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
    if (horizontalFBO) {
        delete horizontalFBO;
        horizontalFBO = nullptr;
    }
    if (verticalFBO) {
        delete verticalFBO;
        verticalFBO = nullptr;
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

void GLCircleWidget::setHorizontalBlurEnabled(bool enabled) {
    horizontal = enabled;
    update();
}

void GLCircleWidget::setVerticalBlurEnabled(bool enabled) {
    vertical = enabled;
    update();
}

void GLCircleWidget::setShowRenderResult(bool show) {
    // 当需要显示渲染结果时，启用所有效果
    if (show) {
        result = show;
        showMipmap = show;
        horizontal = show;
        vertical = show;
    }
    update();
}