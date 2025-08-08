#include "glmultipasswidget.h"
#include <QDebug>
#include <QFile>
#include <QPainter>
#include <QDateTime>
#include <iostream>

GLMultiPassWidget::GLMultiPassWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    setMinimumSize(600, 400);
}

GLMultiPassWidget::~GLMultiPassWidget()
{
    // 清理资源
    makeCurrent();
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);
    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);
    delete m_circleProgram;
    delete m_compositeProgram;
    delete m_fbo;
    doneCurrent();
}

void GLMultiPassWidget::initializeGL()
{
    // 初始化OpenGL函数
    initializeOpenGLFunctions();
    
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f); // 深蓝色背景

    // 创建圆形着色器程序
    m_circleProgram = new QOpenGLShaderProgram(this);
    m_circleProgram->addShaderFromSourceCode(QOpenGLShader::Vertex,
        "attribute vec2 aPos;\n"
        "void main() {\n"
        "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "}");
    m_circleProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"
        "uniform vec2 center;\n"
        "uniform float radius;\n"
        "uniform vec3 circleColor;\n"
        "void main() {\n"
        "    float d = distance(gl_FragCoord.xy, center);\n"
        "    float alpha = smoothstep(radius, radius - 1.0, d);\n"
        "    gl_FragColor = vec4(circleColor, alpha);\n"
        "}");
    m_circleProgram->link();

    // 创建合成着色器程序
    m_compositeProgram = new QOpenGLShaderProgram(this);
    m_compositeProgram->addShaderFromSourceCode(QOpenGLShader::Vertex,
        "attribute vec2 aPos;\n"
        "attribute vec2 aTexCoord;\n"
        "varying vec2 TexCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "    TexCoord = aTexCoord;\n"
        "}");
    m_compositeProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"
        "varying vec2 TexCoord;\n"
        "uniform sampler2D circleTexture;\n"
        "uniform vec3 rectColor;\n"
        "uniform vec2 rectCenter;\n"
        "uniform vec2 rectSize;\n"
        "void main() {\n"
        "    vec2 rectHalf = rectSize * 0.5;\n"
        "    bool inRect = abs(TexCoord.x - rectCenter.x) < rectHalf.x && \n"
        "                  abs(TexCoord.y - rectCenter.y) < rectHalf.y;\n"
        "    vec4 rect = vec4(0.0);\n"
        "    if (inRect) {\n"
        "        rect = vec4(rectColor, 1.0);\n"
        "    }\n"
        "    vec4 circle = texture2D(circleTexture, TexCoord);\n"
        "    gl_FragColor = mix(rect, circle, circle.a);\n"
        "}");
    m_compositeProgram->link();

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

    // 创建VAO和VBO
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    // 位置属性
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    // 纹理坐标属性
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindVertexArray(0);
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
    // 第一通道: 渲染圆形到纹理
    m_fbo->bind();
    glViewport(0, 0, m_fbo->width(), m_fbo->height());
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 透明背景
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_circleProgram->bind();
    glBindVertexArray(m_quadVAO);
    
    // 设置圆形参数
    m_circleProgram->setUniformValue("center", QVector2D(width()/2.0f, height()/2.0f));
    m_circleProgram->setUniformValue("radius", 150.0f);
    m_circleProgram->setUniformValue("circleColor", QVector3D(0.8f, 0.2f, 0.2f)); // 红色
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // 第二通道: 渲染到屏幕并合成
    m_fbo->release();
    glViewport(0, 0, width(), height());
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f); // 深蓝色背景
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_compositeProgram->bind();
    glBindVertexArray(m_quadVAO);
    
    // 绑定圆形纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo->texture());
    m_compositeProgram->setUniformValue("circleTexture", 0);
    
    // 设置矩形参数
    m_compositeProgram->setUniformValue("rectColor", QVector3D(0.2f, 0.7f, 0.3f)); // 绿色
    m_compositeProgram->setUniformValue("rectCenter", QVector2D(0.4f, 0.6f));
    m_compositeProgram->setUniformValue("rectSize", QVector2D(0.3f, 0.3f));
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // 使用QPainter绘制文本
    QPainter painter(this);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12));
    painter.drawText(10, 30, "OpenGL多通道渲染示例 - 圆形与矩形合成");
    painter.setFont(QFont("Arial", 10));
    painter.drawText(10, 50, "第一通道: 圆形渲染到纹理");
    painter.drawText(10, 70, "第二通道: 矩形渲染并合成圆形纹理");
    painter.end();
}