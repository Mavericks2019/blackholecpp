#ifndef GLCIRCLEWIDGET_H
#define GLCIRCLEWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions_4_3_Core>
#include <QSurfaceFormat>
#include <QMouseEvent>
#include <QTimer>
#include <QVector3D>
#include <QVector2D>
#include <QVector4D>
#include <QPoint>
#include <QOpenGLFramebufferObject>
#include <QElapsedTimer>

class GLCircleWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {
    Q_OBJECT
public:
    explicit GLCircleWidget(QWidget* parent = nullptr);
    void setBackgroundType(int type);
    void setShowMipmap(bool show);

signals:
    void aspectRatioChanged(const QString& ratio);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

public:
    void createChessTexture();
    void updateAspectRatio();

private:
    // OpenGL resources
    QOpenGLShaderProgram* program = nullptr;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
    QOpenGLTexture* chessTexture = nullptr;
    
    // FBO and textures
    QOpenGLFramebufferObject* fbo = nullptr;
    QOpenGLTexture* prevFrameTexture = nullptr;
    QOpenGLShaderProgram* screenProgram = nullptr;
    
    // Mipmap resources
    bool showMipmap = false;
    QOpenGLShaderProgram* mipmapProgram = nullptr;
    QOpenGLFramebufferObject* mipmapFBO = nullptr;
    
    // hotizontal resources
    bool horizontal = false;
    QOpenGLShaderProgram* horizontalProgram = nullptr;
    QOpenGLFramebufferObject* horizontalFBO = nullptr;

    // Mipmap resources
    bool vertical = false;
    QOpenGLShaderProgram* verticalProgram = nullptr;
    QOpenGLFramebufferObject* verticalFBO = nullptr;
    QElapsedTimer frameTimer;

    float lastFrameTime = 0.0f;

    // Uniform values
    QVector3D circleColor{1.0f, 0.0f, 0.0f};
    QVector2D offset{0.2f, 0.2f};
    float radius = 0.2f;
    float blackHoleMass = 1.49e7f;
    int backgroundType = 0;
    QVector3D chessTextureResolution{64.0f, 64.0f, 0.0f};
    
    // Shadertoy-like variables
    float iTime = 0.0f;
    int iFrame = 0;
    QVector4D iMouse;
    bool mousePressed = false;
    QPoint lastMousePos;

    // 帧率计算成员
    QElapsedTimer fpsTimer;
    int frameCount = 0;
    float fps = 0.0f;

    public slots:
    void setHorizontalBlurEnabled(bool enabled);
    void setVerticalBlurEnabled(bool enabled);
};

#endif // GLCIRCLEWIDGET_H