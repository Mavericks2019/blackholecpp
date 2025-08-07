#ifndef GLBASICWIDGET_H
#define GLBASICWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions_4_3_Core>
#include <QTimer>
#include <QFile>
#include <chrono> // 添加高精度时间库

class GLBasicWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {
    Q_OBJECT
public:
    explicit GLBasicWidget(QWidget* parent = nullptr);
    ~GLBasicWidget();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    QOpenGLShaderProgram* program = nullptr;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
    QTimer* timer = nullptr;
    
    // 使用高精度时间点
    std::chrono::high_resolution_clock::time_point startTime;
    
    bool loadShader(QOpenGLShader::ShaderType type, const QString& filePath);
};
#endif // GLBASICWIDGET_H