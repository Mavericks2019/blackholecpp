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
    QOpenGLShaderProgram *m_circleProgram = nullptr;
    QOpenGLShaderProgram *m_compositeProgram = nullptr;
    QOpenGLFramebufferObject *m_fbo = nullptr;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
};

#endif // GLMULTIPASSWIDGET_H