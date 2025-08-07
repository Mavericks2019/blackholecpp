#include "glbasicwidget.h"
#include <QDebug>
#include <cmath>
#include <QDateTime>
#include <QOpenGLFunctions_4_3_Core>
#include <QFileInfo>  // 添加文件信息头文件

GLBasicWidget::GLBasicWidget(QWidget* parent) : QOpenGLWidget(parent) {
    setMinimumSize(600, 600);
    
    QSurfaceFormat fmt;
    fmt.setSamples(4); // 4x MSAA
    fmt.setVersion(4, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(fmt);
    
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() { update(); });
    timer->start(16); // ~60 FPS
    
    startTime = QDateTime::currentMSecsSinceEpoch() / 1000.0f;
}

GLBasicWidget::~GLBasicWidget() {
    makeCurrent();
    delete program;
    vao.destroy();
    vbo.destroy();
    doneCurrent();
}

void GLBasicWidget::initializeGL() {
    initializeOpenGLFunctions();
    
    // 添加上下文有效性检查
    if (!context()->isValid()) {
        qCritical() << "OpenGL context is invalid!";
        return;
    }
    
    // 打印OpenGL版本信息
    qDebug() << "OpenGL Version:" << QString::fromLatin1((const char*)glGetString(GL_VERSION));
    qDebug() << "GLSL Version:" << QString::fromLatin1((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    // 创建着色器程序
    program = new QOpenGLShaderProgram(this);
    
    // 检查着色器文件是否存在
    QString vertPath = "../shaders/basic.vert";
    QString fragPath = "../shaders/basic.frag";
    
    qDebug() << "Vertex shader path:" << QFileInfo(vertPath).absoluteFilePath();
    qDebug() << "Fragment shader path:" << QFileInfo(fragPath).absoluteFilePath();
    
    if (!QFile::exists(vertPath)) {
        qCritical() << "Vertex shader file not found:" << vertPath;
    }
    if (!QFile::exists(fragPath)) {
        qCritical() << "Fragment shader file not found:" << fragPath;
    }
    
    // 加载着色器
    if (!loadShader(QOpenGLShader::Vertex, vertPath)) {
        qCritical() << "Vertex shader error:" << program->log();
    }
    if (!loadShader(QOpenGLShader::Fragment, fragPath)) {
        qCritical() << "Fragment shader error:" << program->log();
    }
    if (!program->link()) {
        qCritical() << "Shader link error:" << program->log();
    }
    
    // 检查着色器是否成功链接
    if (!program->isLinked()) {
        qCritical() << "Shader program failed to link!";
        return;
    }
    
    // 创建VAO和VBO
    vao.create();
    vao.bind();
    
    vbo.create();
    vbo.bind();
    
    // 全屏矩形顶点数据 (位置 + 纹理坐标)
    const float vertices[] = {
        // 位置          // 纹理坐标
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
         
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    };
    vbo.allocate(vertices, sizeof(vertices));
    
    // 配置顶点属性
    program->bind();
    
    // 位置属性 (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    // 纹理坐标属性 (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    program->release();
    vbo.release();
    vao.release();
    
    // 检查OpenGL错误
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        qDebug() << "OpenGL error after initialization:" << err;
    }
    
    qDebug() << "OpenGL initialization complete";
}

void GLBasicWidget::paintGL() {
    // 清除缓冲区
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (!program || !program->isLinked()) {
        qDebug() << "Shader program not ready for painting";
        return;
    }
    
    program->bind();
    vao.bind();
    
    // 设置统一变量
    float elapsedTime = QDateTime::currentMSecsSinceEpoch() / 1000.0f - startTime;
    program->setUniformValue("iTime", elapsedTime);
    program->setUniformValue("iResolution", QVector2D(width(), height()));
    
    // 打印调试信息
    static int frameCount = 0;
    if (frameCount++ % 60 == 0) {
        qDebug() << "Painting at time:" << elapsedTime << "Resolution:" << width() << "x" << height();
    }
    
    // 绘制全屏矩形
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    vao.release();
    program->release();
    
    // 检查OpenGL错误
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        qDebug() << "OpenGL error after paintGL:" << err;
    }
}

void GLBasicWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    qDebug() << "Resized to:" << w << "x" << h;
}

bool GLBasicWidget::loadShader(QOpenGLShader::ShaderType type, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open shader file:" << filePath;
        return false;
    }
    
    QTextStream stream(&file);
    QString shaderSource = stream.readAll();
    file.close();
    
    // 添加调试信息
    qDebug() << "Loading shader:" << QFileInfo(filePath).absoluteFilePath();
    if (shaderSource.isEmpty()) {
        qWarning() << "Shader source is empty for file:" << filePath;
        return false;
    }
    
    bool success = program->addShaderFromSourceCode(type, shaderSource);
    if (!success) {
        qDebug() << "Shader compilation error (" << filePath << "):" << program->log();
    }
    return success;
}