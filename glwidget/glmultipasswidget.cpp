#include "glmultipasswidget.h"
#include <QDebug>
#include <QPainter>
#include <QDateTime>
#include <QMouseEvent>
#include <iostream>
#include <cmath>

GLMultiPassWidget::GLMultiPassWidget(QWidget *parent) : QOpenGLWidget(parent),
    m_vbo(QOpenGLBuffer::VertexBuffer)
{
    setMinimumSize(600, 400);
    
    // 初始化计时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() { 
        m_iTime += 0.016f;
        m_iFrame++;
        update(); 
    });
    m_timer->start(16); // ~60 FPS
    
    // 记录开始时间
    m_startTime = std::chrono::high_resolution_clock::now();
}

GLMultiPassWidget::~GLMultiPassWidget()
{
    makeCurrent();
    delete m_basicProgram;
    delete m_circleProgram;
    delete m_fbo;
    m_vao.destroy();
    m_vbo.destroy();
    delete m_chessTexture; // 清理棋盘纹理
    delete m_timer;
    doneCurrent();
}

bool GLMultiPassWidget::loadShaderSource(QOpenGLShaderProgram* program, 
                                         QOpenGLShader::ShaderType type, 
                                         const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open shader file:" << filePath;
        return false;
    }
    
    QByteArray source = file.readAll();
    file.close();
    
    if (!program->addShaderFromSourceCode(type, source)) {
        qWarning() << "Failed to compile shader:" << filePath;
        qWarning() << program->log();
        return false;
    }
    
    return true;
}

void GLMultiPassWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    // 创建第一通道着色器程序
    m_basicProgram = new QOpenGLShaderProgram(this);
    if (!loadShaderSource(m_basicProgram, QOpenGLShader::Vertex, 
                         ":/shaders/basic.vert") ||
        !loadShaderSource(m_basicProgram, QOpenGLShader::Fragment, 
                         ":/shaders/basic.frag") ||
        !m_basicProgram->link()) 
    {
        qCritical() << "Basic shader program link failed:" << m_basicProgram->log();
    }

    // 创建第二通道着色器程序（黑洞渲染）
    m_circleProgram = new QOpenGLShaderProgram(this);
    if (!loadShaderSource(m_circleProgram, QOpenGLShader::Vertex, 
                         ":/shaders/multipass.vert") ||
        !loadShaderSource(m_circleProgram, QOpenGLShader::Fragment, 
                         ":/shaders/multipass_circle.frag") || // 使用修改后的黑洞着色器
        !m_circleProgram->link()) 
    {
        qCritical() << "Circle shader program link failed:" << m_circleProgram->log();
    }

    // 创建棋盘纹理
    createChessTexture();

    // 创建全屏四边形
    float quadVertices[] = {
        // 位置         // 纹理坐标
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    // 创建并绑定VAO和VBO
    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(quadVertices, sizeof(quadVertices));
    
    // 位置属性
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    // 纹理坐标属性
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    m_vao.release();
    m_vbo.release();
}

void GLMultiPassWidget::resizeGL(int w, int h)
{
    // 创建帧缓冲对象和纹理
    if (m_fbo) {
        delete m_fbo;
        m_fbo = nullptr;
    }
    
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    m_fbo = new QOpenGLFramebufferObject(w, h, format);
}

void GLMultiPassWidget::paintGL()
{
    // === 帧率计算开始 ===
    static QElapsedTimer fpsTimer;
    static int frameCount = 0;
    static float fps = 0.0f;
    
    if (frameCount == 0) {
        fpsTimer.start();
    }
    frameCount++;
    
    // 每0.5秒更新一次帧率
    if (fpsTimer.elapsed() > 500) {
        fps = frameCount * 1000.0f / fpsTimer.elapsed();
        frameCount = 0;
        fpsTimer.restart();
    }
    // 第一通道: 渲染分形效果到纹理
    m_fbo->bind();
    glViewport(0, 0, m_fbo->width(), m_fbo->height());
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_basicProgram->bind();
    m_vao.bind();
    
    // 计算经过的时间
    auto now = std::chrono::high_resolution_clock::now();
    float elapsedTime = std::chrono::duration<float>(now - m_startTime).count();
    
    // 设置统一变量
    m_basicProgram->setUniformValue("iTime", elapsedTime);
    m_basicProgram->setUniformValue("iResolution", QVector2D(width(), height()));
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    m_vao.release();
    m_basicProgram->release();
    
    // 第二通道: 渲染黑洞效果
    m_fbo->release();
    glViewport(0, 0, width(), height());
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_circleProgram->bind();
    m_vao.bind();
    
    // 绑定第一通道的纹理作为背景
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo->texture());
    m_circleProgram->setUniformValue("backgroundTexture", 0);
    
    // 绑定棋盘纹理
    if (m_chessTexture) {
        glActiveTexture(GL_TEXTURE1);
        m_chessTexture->bind();
        m_circleProgram->setUniformValue("iChannel1", 1);
    }
    
    // 设置黑洞着色器参数
    m_circleProgram->setUniformValue("iResolution", QVector2D(width(), height()));
    m_circleProgram->setUniformValue("offset", m_offset);
    m_circleProgram->setUniformValue("radius", m_radius);
    m_circleProgram->setUniformValue("MBlackHole", m_blackHoleMass);
    m_circleProgram->setUniformValue("backgroundType", m_backgroundType);
    m_circleProgram->setUniformValue("iFrame", m_iFrame);
    m_circleProgram->setUniformValue("iMouse", m_iMouse[0], m_iMouse[1], m_iMouse[2], m_iMouse[3]);
    m_circleProgram->setUniformValue("iTime", m_iTime);
    m_circleProgram->setUniformValue("iChannelResolution", 
        m_chessTextureResolution.x(), m_chessTextureResolution.y(), m_chessTextureResolution.z());
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    m_vao.release();
    m_circleProgram->release();

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

void GLMultiPassWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_mousePressed = true;
        m_lastMousePos = event->pos();
        
        QPointF pos = event->pos();
        m_iMouse[2] = pos.x();
        m_iMouse[3] = height() - pos.y();
        
        m_iMouse[0] = pos.x();
        m_iMouse[1] = height() - pos.y();
        
        update();
    }
}

void GLMultiPassWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_mousePressed = false;
        m_iMouse[2] = 0.0f;
        m_iMouse[3] = 0.0f;
        
        QPointF pos = event->pos();
        m_iMouse[0] = pos.x();
        m_iMouse[1] = height() - pos.y();
        
        update();
    }
}

void GLMultiPassWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_mousePressed) return;
        
    QPointF pos = event->pos();
    m_iMouse[0] = pos.x();
    m_iMouse[1] = height() - pos.y();
    
    QPoint delta = event->pos() - m_lastMousePos;
    m_lastMousePos = event->pos();
    
    m_iMouse[2] += delta.x();
    m_iMouse[3] -= delta.y();
    
    update();
}

void GLMultiPassWidget::createChessTexture()
{
    const int size = 64;
    QImage image(size, size, QImage::Format_RGBA8888);
    
    QColor color1(220, 220, 220);  // 浅色格子
    QColor color2(80, 80, 100);    // 深色格子
    
    const int tileSize = size / 8;
    
    // 设置字母（国际象棋棋子表示）
    QString letters = "RNBQKBNR";  // 首行棋子：车、马、象、后、王、象、马、车
    
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    QFont font = painter.font();
    font.setPixelSize(tileSize - 4);
    font.setBold(true);
    painter.setFont(font);
    
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            // 计算格子颜色
            bool isLight = (x + y) % 2 == 0;
            QColor bgColor = isLight ? color1 : color2;
            QColor textColor = isLight ? color2 : color1;
            
            // 绘制格子背景
            painter.fillRect(x * tileSize, y * tileSize, tileSize, tileSize, bgColor);
            
            // 只在深色格子上添加字母
            if (!isLight) {
                painter.setPen(textColor);
                
                // 确定字母位置（棋盘顶部和底部）
                QString letter;
                if (y == 0 || y == 7) {
                    letter = letters.at(x);
                }
                // 在第二行和倒数第二行添加兵
                else if (y == 1 || y == 6) {
                    letter = "P";  // 兵(Pawn)
                }
                
                if (!letter.isEmpty()) {
                    // 居中绘制字母
                    QRect textRect(x * tileSize, y * tileSize, tileSize, tileSize);
                    painter.drawText(textRect, Qt::AlignCenter, letter);
                }
            }
        }
    }
    
    m_chessTexture = new QOpenGLTexture(image.mirrored());
    m_chessTexture->setWrapMode(QOpenGLTexture::Repeat);
    m_chessTexture->setMinificationFilter(QOpenGLTexture::Linear);
    m_chessTexture->setMagnificationFilter(QOpenGLTexture::Linear);
}