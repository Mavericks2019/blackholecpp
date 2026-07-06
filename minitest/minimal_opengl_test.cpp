#include <QOpenGLWidget>
#include <QApplication>
#include <QDebug>

class MinimalGLWidget : public QOpenGLWidget {
protected:
    void initializeGL() override {
        qDebug() << "OpenGL Version:" << (const char*)glGetString(GL_VERSION);
        qDebug() << "OpenGL Vendor:" << (const char*)glGetString(GL_VENDOR);
        qDebug() << "OpenGL Renderer:" << (const char*)glGetString(GL_RENDERER);
    }
    
    void paintGL() override {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    MinimalGLWidget widget;
    widget.show();
    
    return app.exec();
}