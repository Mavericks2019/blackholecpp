#ifndef GLMULTIPASSWIDGET_H
#define GLMULTIPASSWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QFile>
#include <QCoreApplication>
#include <QTimer>
#include <chrono>

class GLMultiPassWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit GLMultiPassWidget(QWidget *parent = nullptr);
    ~GLMultiPassWidget() override;

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
    // 添加着色器加载辅助函数
    bool loadShaderSource(QOpenGLShaderProgram* program, 
                          QOpenGLShader::ShaderType type, 
                          const QString& filePath);

private:
    QOpenGLShaderProgram *m_basicProgram = nullptr;  // 改为basic着色器
    QOpenGLShaderProgram *m_compositeProgram = nullptr;
    QOpenGLFramebufferObject *m_fbo = nullptr;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QTimer* m_timer = nullptr;  // 添加计时器
    
    // 使用高精度时间点
    std::chrono::high_resolution_clock::time_point m_startTime;
};

#endif // GLMULTIPASSWIDGET_H