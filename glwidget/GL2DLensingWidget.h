#ifndef GL2DLENSINGWIDGET_H
#define GL2DLENSINGWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QTimer>
#include <QVector2D>
#include <QVector3D>
#include <vector>

struct Ray2D {
    double x, y;           // Cartesian coordinates
    double r, phi;         // Polar coordinates
    double dr, dphi;       // Velocities
    double E, L;           // Conserved quantities
    std::vector<QVector2D> trail; // Trail points
    
    Ray2D(QVector2D pos, QVector2D dir);
    void step(double dlambda, double rs);
    void geodesicRHS(const Ray2D& ray, double rhs[4], double rs);
};

class GL2DLensingWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit GL2DLensingWidget(QWidget* parent = nullptr);
    ~GL2DLensingWidget();

    void addRay(QVector2D pos, QVector2D dir);
    void clearRays();
    void initializeRays();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    // OpenGL resources
    QOpenGLShaderProgram* program = nullptr;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
    
    // Physics parameters
    double blackHoleMass;
    double schwarzschildRadius;
    
    // Ray data
    std::vector<Ray2D> rays;
    
    // View controls
    QVector2D offset;
    double zoom;
    bool middleMousePressed;
    QPoint lastMousePos;
    
    // Animation timer
    QTimer* animationTimer = nullptr;

private slots:
    void animate();
};

#endif // GL2DLENSINGWIDGET_H