#include "GL2DLensingWidget.h"
#include <QDebug>
#include <cmath>

// 物理常量
const double c = 299792458.0;
const double G = 6.67430e-11;
const double PI_VALUE = 3.14159265358979323846;

// Ray2D 实现
Ray2D::Ray2D(QVector2D pos, QVector2D dir) : x(pos.x()), y(pos.y()) {
    // 转换为极坐标
    r = sqrt(x*x + y*y);
    phi = atan2(y, x);
    
    // 计算极坐标下的速度分量
    double vx = dir.x(), vy = dir.y();
    dr = vx * cos(phi) + vy * sin(phi);
    dphi = (-vx * sin(phi) + vy * cos(phi)) / r;
    
    // 计算守恒量
    double rs = 2.0 * G * 8.54e36 / (c*c);
    L = r * r * dphi;
    
    double f = 1.0 - rs / r;
    // 对于类光测地线，使用正确的能量计算
    E = sqrt(f * f * dr * dr / (f * f) + f * L * L / (r * r));
    
    // 初始轨迹点
    trail.push_back(QVector2D(x, y));
    
    qDebug() << "New ray at (" << x << "," << y << ") with v=(" << vx << "," << vy << ")";
    qDebug() << "Polar: r=" << r << "phi=" << phi << "dr=" << dr << "dphi=" << dphi;
    qDebug() << "Conserved: E=" << E << "L=" << L;
}

void Ray2D::step(double dlambda, double rs) {
    if (r <= rs) {
        qDebug() << "Ray inside event horizon at r=" << r;
        return; // 停止如果在事件视界内
    }
    
    // 使用RK4积分
    double y0[4] = { r, phi, dr, dphi };
    double k1[4], k2[4], k3[4], k4[4], temp[4];

    // k1
    geodesicRHS(*this, k1, rs);
    
    // k2  
    Ray2D r2(QVector2D(0,0), QVector2D(0,0));
    for (int i = 0; i < 4; i++) temp[i] = y0[i] + k1[i] * dlambda/2.0;
    r2.r = temp[0]; r2.phi = temp[1]; r2.dr = temp[2]; r2.dphi = temp[3];
    r2.E = E; r2.L = L;
    geodesicRHS(r2, k2, rs);

    // k3
    Ray2D r3(QVector2D(0,0), QVector2D(0,0));
    for (int i = 0; i < 4; i++) temp[i] = y0[i] + k2[i] * dlambda/2.0;
    r3.r = temp[0]; r3.phi = temp[1]; r3.dr = temp[2]; r3.dphi = temp[3];
    r3.E = E; r3.L = L;
    geodesicRHS(r3, k3, rs);

    // k4
    Ray2D r4(QVector2D(0,0), QVector2D(0,0));
    for (int i = 0; i < 4; i++) temp[i] = y0[i] + k3[i] * dlambda;
    r4.r = temp[0]; r4.phi = temp[1]; r4.dr = temp[2]; r4.dphi = temp[3];
    r4.E = E; r4.L = L;
    geodesicRHS(r4, k4, rs);

    // 更新状态
    r    += (dlambda/6.0) * (k1[0] + 2*k2[0] + 2*k3[0] + k4[0]);
    phi  += (dlambda/6.0) * (k1[1] + 2*k2[1] + 2*k3[1] + k4[1]);
    dr   += (dlambda/6.0) * (k1[2] + 2*k2[2] + 2*k3[2] + k4[2]);
    dphi += (dlambda/6.0) * (k1[3] + 2*k2[3] + 2*k3[3] + k4[3]);

    // 转换回笛卡尔坐标
    x = r * cos(phi);
    y = r * sin(phi);
    
    // 记录轨迹
    trail.push_back(QVector2D(x, y));
    
    // 调试输出
    if (trail.size() % 100 == 0) {
        qDebug() << "Ray step: r=" << r << "phi=" << phi << "x=" << x << "y=" << y;
    }
}

void Ray2D::geodesicRHS(const Ray2D& ray, double rhs[4], double rs) {
    double r = ray.r;
    double dr = ray.dr;
    double dphi = ray.dphi;
    double E = ray.E;
    double L = ray.L;

    double f = 1.0 - rs / r;

    // dr/dlambda = dr (已经定义)
    rhs[0] = dr;
    
    // dφ/dlambda = dphi (已经定义)  
    rhs[1] = dphi;
    
    // d²r/dlambda² - 修正的测地线方程
    // 使用史瓦西度规下的类光测地线方程
    rhs[2] = - (rs/(2.0*r*r)) * (1.0 - 3.0*L*L/(r*r*f)) * (E*E) 
              + (L*L)/(r*r*r) * (1.0 - rs/r);
    
    // d²φ/dlambda²
    rhs[3] = -2.0 * dr * dphi / r;
}

// GL2DLensingWidget 实现
GL2DLensingWidget::GL2DLensingWidget(QWidget* parent) 
    : QOpenGLWidget(parent), 
      blackHoleMass(8.54e36),
      zoom(1.0),
      middleMousePressed(false),
      offset(0.0f, 0.0f) {
    
    setMinimumSize(600, 600);
    
    // 计算史瓦西半径
    schwarzschildRadius = 2.0 * G * blackHoleMass / (c*c);
    qDebug() << "Schwarzschild Radius:" << schwarzschildRadius;
    
    // 设置动画定时器
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &GL2DLensingWidget::animate);
    animationTimer->start(16); // ~60 FPS
    
    // 初始化一些示例光线
    initializeRays();
}

GL2DLensingWidget::~GL2DLensingWidget() {
    makeCurrent();
    vao.destroy();
    vbo.destroy();
    delete program;
    doneCurrent();
}

void GL2DLensingWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // 创建着色器程序
    program = new QOpenGLShaderProgram(this);
    
    // 顶点着色器
    const char* vsrc =
        "#version 330 core\n"
        "layout(location = 0) in vec2 position;\n"
        "uniform mat4 projection;\n"
        "void main() {\n"
        "    gl_Position = projection * vec4(position, 0.0, 1.0);\n"
        "}\n";
    
    // 片段着色器
    const char* fsrc =
        "#version 330 core\n"
        "out vec4 fragColor;\n"
        "uniform vec3 color;\n"
        "void main() {\n"
        "    fragColor = vec4(color, 1.0);\n"
        "}\n";
    
    if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vsrc)) {
        qDebug() << "Vertex shader error:" << program->log();
    }
    if (!program->addShaderFromSourceCode(QOpenGLShader::Fragment, fsrc)) {
        qDebug() << "Fragment shader error:" << program->log();
    }
    if (!program->link()) {
        qDebug() << "Shader link error:" << program->log();
    }
    
    // 创建VAO和VBO
    vao.create();
    vbo.create();
    
    vao.bind();
    vbo.bind();
    
    // 配置顶点属性
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    vao.release();
    vbo.release();
}

void GL2DLensingWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (!program->bind()) {
        qDebug() << "Failed to bind shader program";
        return;
    }
    
    // 设置投影矩阵（正交投影）
    QMatrix4x4 projection;
    float aspect = float(width()) / float(height());
    float viewWidth = 2e11f * zoom;
    float viewHeight = viewWidth / aspect;
    
    projection.ortho(-viewWidth/2 + offset.x(), viewWidth/2 + offset.x(),
                     -viewHeight/2 + offset.y(), viewHeight/2 + offset.y(),
                     -1.0f, 1.0f);
    
    program->setUniformValue("projection", projection);
    
    vao.bind();
    
    // 绘制黑洞（事件视界）
    program->setUniformValue("color", QVector3D(1.0f, 0.0f, 0.0f));
    
    // 绘制黑洞圆盘
    const int segments = 100;
    std::vector<float> circleVertices;
    
    // 中心点
    circleVertices.push_back(0.0f);
    circleVertices.push_back(0.0f);
    
    // 圆周边
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * PI_VALUE * i / segments;
        circleVertices.push_back(schwarzschildRadius * cos(angle));
        circleVertices.push_back(schwarzschildRadius * sin(angle));
    }
    
    vbo.bind();
    vbo.allocate(circleVertices.data(), circleVertices.size() * sizeof(float));
    glDrawArrays(GL_TRIANGLE_FAN, 0, circleVertices.size() / 2);
    
    // 绘制光线轨迹
    program->setUniformValue("color", QVector3D(1.0f, 1.0f, 1.0f));
    
    for (const auto& ray : rays) {
        if (ray.trail.size() < 2) continue;
        
        // 绘制轨迹线
        std::vector<float> trailVertices;
        for (const auto& point : ray.trail) {
            trailVertices.push_back(point.x());
            trailVertices.push_back(point.y());
        }
        
        vbo.allocate(trailVertices.data(), trailVertices.size() * sizeof(float));
        glDrawArrays(GL_LINE_STRIP, 0, trailVertices.size() / 2);
        
        // 绘制当前光线位置（点）
        if (!ray.trail.empty()) {
            const auto& currentPos = ray.trail.back();
            std::vector<float> pointVertices = { currentPos.x(), currentPos.y() };
            vbo.allocate(pointVertices.data(), pointVertices.size() * sizeof(float));
            glPointSize(3.0f);
            glDrawArrays(GL_POINTS, 0, 1);
        }
    }
    
    vao.release();
    vbo.release();
    program->release();
}

void GL2DLensingWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void GL2DLensingWidget::initializeRays() {
    rays.clear();
    // 添加一些示例光线 - 从不同位置发射以展示引力透镜效应
    addRay(QVector2D(-1e11, 2e10), QVector2D(c, 0.0f));
    addRay(QVector2D(-1e11, 5e10), QVector2D(c, 0.0f));
    addRay(QVector2D(-1e11, 1e11), QVector2D(c, 0.0f));
    addRay(QVector2D(-1e11, -2e10), QVector2D(c, 0.0f));
    addRay(QVector2D(-1e11, -5e10), QVector2D(c, 0.0f));
    addRay(QVector2D(-1e11, -1e11), QVector2D(c, 0.0f));
}

void GL2DLensingWidget::addRay(QVector2D pos, QVector2D dir) {
    rays.emplace_back(pos, dir);
    qDebug() << "Added ray at (" << pos.x() << "," << pos.y() << ")";
}

void GL2DLensingWidget::clearRays() {
    rays.clear();
    update();
}

void GL2DLensingWidget::animate() {
    // 更新所有光线的位置
    for (auto& ray : rays) {
        ray.step(1e3, schwarzschildRadius); // 使用合适的步长
    }
    update();
}

void GL2DLensingWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        middleMousePressed = true;
        lastMousePos = event->pos();
    } else if (event->button() == Qt::LeftButton) {
        // 鼠标左键添加新光线 - 这是点击发射光线的逻辑
        float aspect = float(width()) / float(height());
        float viewWidth = 2e11f * zoom;
        float viewHeight = viewWidth / aspect;
        
        QPointF pos = event->pos();
        
        // 将屏幕坐标转换为世界坐标
        float worldX = (pos.x() / width() - 0.5f) * viewWidth + offset.x();
        float worldY = (0.5f - pos.y() / height()) * viewHeight + offset.y();
        
        qDebug() << "Mouse click at screen (" << pos.x() << "," << pos.y() 
                 << ") -> world (" << worldX << "," << worldY << ")";
        
        // 从点击位置发射光线，方向向右（朝向黑洞）
        addRay(QVector2D(worldX, worldY), QVector2D(c, 0.0f));
        
        update();
    }
}

void GL2DLensingWidget::mouseMoveEvent(QMouseEvent* event) {
    if (middleMousePressed) {
        QPoint delta = event->pos() - lastMousePos;
        float aspect = float(width()) / float(height());
        float viewWidth = 2e11f * zoom;
        
        offset.setX(offset.x() - delta.x() * viewWidth / width());
        offset.setY(offset.y() + delta.y() * viewWidth / width() / aspect);
        
        lastMousePos = event->pos();
        update();
    }
}

void GL2DLensingWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        middleMousePressed = false;
    }
}

void GL2DLensingWidget::wheelEvent(QWheelEvent* event) {
    float delta = event->angleDelta().y() > 0 ? 0.9f : 1.1f;
    zoom *= delta;
    
    zoom = qMax(zoom, 1e-6);
    zoom = qMin(zoom, 1e6);
    
    update();
}