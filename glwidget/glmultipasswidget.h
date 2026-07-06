#ifndef GLMULTIPASSWIDGET_H
#define GLMULTIPASSWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Core> // 升级到4.3核心
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture> // 添加纹理支持
#include <QFile>
#include <QCoreApplication>
#include <QTimer>
#include <QMouseEvent> // 添加鼠标事件支持
#include <chrono>
#include <QElapsedTimer>
#include <QPainter>

class GLMultiPassWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
{
    Q_OBJECT

public:
    explicit GLMultiPassWidget(QWidget *parent = nullptr);
    ~GLMultiPassWidget() override;

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    
    // 添加着色器加载辅助函数
    bool loadShaderSource(QOpenGLShaderProgram* program, 
                          QOpenGLShader::ShaderType type, 
                          const QString& filePath);

private:
    void createChessTexture(); // 添加棋盘纹理创建函数
    
    QOpenGLShaderProgram *m_basicProgram = nullptr;
    QOpenGLShaderProgram *m_circleProgram = nullptr; // 添加黑洞着色器程序
    QOpenGLFramebufferObject *m_fbo = nullptr;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QTimer* m_timer = nullptr;
    QOpenGLTexture* m_chessTexture = nullptr; // 棋盘纹理
    
    // 黑洞渲染参数
    QVector2D m_offset{0.2f, 0.2f};
    float m_radius = 0.2f;
    float m_blackHoleMass = 1.49e7f;
    int m_backgroundType = 3;
    QVector3D m_chessTextureResolution{64.0f, 64.0f, 0.0f};
    float m_iTime = 0.0f;
    int m_iFrame = 0;
    QVector4D m_iMouse;
    bool m_mousePressed = false;
    QPoint m_lastMousePos;

    // 使用高精度时间点
    std::chrono::high_resolution_clock::time_point m_startTime;
};

#endif // GLMULTIPASSWIDGET_H