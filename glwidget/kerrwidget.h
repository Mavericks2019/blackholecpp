#ifndef KERRWIDGET_H
#define KERRWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLFramebufferObject>
#include <QSurfaceFormat>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QPoint>
#include <QSet>

class KerrWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core {
    Q_OBJECT
public:
    explicit KerrWidget(QWidget* parent = nullptr);
    ~KerrWidget();

    // Public controls
    void setShowBloom(bool show);
    void setShowTAA(bool show);
    void resetCamera();
    void setBlackHoleMass(double mass);
    void setSpin(double spin);

signals:
    void fpsUpdated(float fps);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    void setupQuad();
    void createFBOs();
    void updateKeyStateTexture();
    void renderPass(QOpenGLShaderProgram* prog, GLuint targetTexture,
                    GLuint tex0, GLuint tex1, GLuint tex2, GLuint tex3);

    // Quad geometry
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;

    // Shader programs (5 passes)
    QOpenGLShaderProgram* progA = nullptr;     // Kerr ray tracer
    QOpenGLShaderProgram* progB = nullptr;     // Bloom pyramid + camera
    QOpenGLShaderProgram* progC = nullptr;     // Horizontal blur
    QOpenGLShaderProgram* progD = nullptr;     // Vertical blur
    QOpenGLShaderProgram* progImage = nullptr; // Final composite

    // FBO textures (ping-pong for A)
    QOpenGLFramebufferObject* fboA0 = nullptr;
    QOpenGLFramebufferObject* fboA1 = nullptr;
    QOpenGLFramebufferObject* fboB0 = nullptr;
    QOpenGLFramebufferObject* fboB1 = nullptr;
    QOpenGLFramebufferObject* fboC = nullptr;
    QOpenGLFramebufferObject* fboD = nullptr;

    // Key state texture
    QOpenGLTexture* keyStateTexture = nullptr;
    QSet<int> pressedKeys;

    // Timing
    QElapsedTimer frameTimer;
    QElapsedTimer fpsTimer;
    float lastFrameDelta = 0.016f;
    float totalTime = 0.0f;
    int frameCount = 0;
    int fpsFrameCount = 0;
    float fps = 0.0f;

    // Mouse state
    QVector4D iMouse = QVector4D(0, 0, 0, 0);
    bool mouseDown = false;
    QPoint lastMousePos;

    // Settings
    bool showBloom = true;
    bool showTAA = true;
    int currentA = 0;  // ping-pong index for FBO A
    int currentB = 0;  // ping-pong index for FBO B

    int viewW = 800, viewH = 600;

    // Frame capture (set to true for debugging)
    bool captureFrames = false;
    int captureFrameCount = 0;
    int maxCaptureFrames = 60;
    QString captureDir;
};

#endif // KERRWIDGET_H
