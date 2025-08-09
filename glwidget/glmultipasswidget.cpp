#include "glmultipasswidget.h"
#include <QDebug>
#include <QPainter>
#include <QDateTime>
#include <iostream>

GLMultiPassWidget::GLMultiPassWidget(QWidget *parent) : QOpenGLWidget(parent),
    m_vbo(QOpenGLBuffer::VertexBuffer)
{
    setMinimumSize(600, 400);
    
    // 初始化计时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() { update(); });
    m_timer->start(16); // ~60 FPS
    
    // 记录开始时间
    m_startTime = std::chrono::high_resolution_clock::now();
}

GLMultiPassWidget::~GLMultiPassWidget()
{
    makeCurrent();
    delete m_basicProgram;  // 改为basic着色器
    delete m_compositeProgram;
    delete m_fbo;
    m_vao.destroy();
    m_vbo.destroy();
    delete m_timer;  // 清理计时器
    doneCurrent();
}

// 着色器加载辅助函数
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

    // 创建basic着色器程序 - 替换原来的圆形着色器
    m_basicProgram = new QOpenGLShaderProgram(this);
    if (!loadShaderSource(m_basicProgram, QOpenGLShader::Vertex, 
                         ":/shaders/basic.vert") ||
        !loadShaderSource(m_basicProgram, QOpenGLShader::Fragment, 
                         ":/shaders/basic.frag") ||
        !m_basicProgram->link()) 
    {
        qCritical() << "Basic shader program link failed:" << m_basicProgram->log();
    }

    // 创建合成着色器程序
    m_compositeProgram = new QOpenGLShaderProgram(this);
    if (!loadShaderSource(m_compositeProgram, QOpenGLShader::Vertex, 
                         ":/shaders/multipass.vert") ||
        !loadShaderSource(m_compositeProgram, QOpenGLShader::Fragment, 
                         ":/shaders/multipass_composite.frag") ||
        !m_compositeProgram->link()) 
    {
        qCritical() << "Composite shader program link failed:" << m_compositeProgram->log();
    }

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
    // 第一通道: 渲染分形效果到纹理
    m_fbo->bind();
    glViewport(0, 0, m_fbo->width(), m_fbo->height());
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 透明背景
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
    
    // 第二通道: 渲染到屏幕并合成
    m_fbo->release();
    glViewport(0, 0, width(), height());
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f); // 深蓝色背景
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_compositeProgram->bind();
    m_vao.bind();
    
    // 绑定分形纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo->texture());
    m_compositeProgram->setUniformValue("circleTexture", 0);
    
    // 设置矩形参数
    m_compositeProgram->setUniformValue("rectColor", QVector3D(0.2f, 0.7f, 0.3f)); // 绿色
    m_compositeProgram->setUniformValue("rectCenter", QVector2D(0.4f, 0.6f));
    m_compositeProgram->setUniformValue("rectSize", QVector2D(0.3f, 0.3f));
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    m_vao.release();
    m_compositeProgram->release();
}