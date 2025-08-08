#ifndef GLMULTIPASSWIDGET_H
#define GLMULTIPASSWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QTimer>

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

private:
    QOpenGLShaderProgram *m_circleProgram = nullptr;
    QOpenGLShaderProgram *m_compositeProgram = nullptr;
    QOpenGLFramebufferObject *m_fbo = nullptr;
    GLuint m_quadVAO = 0;
    GLuint m_quadVBO = 0;
};

#endif // GLMULTIPASSWIDGET_H