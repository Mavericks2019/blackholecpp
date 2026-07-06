#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QColorDialog>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QGroupBox>
#include <QTabWidget>
#include <QStackedWidget>
#include <QMessageBox>
#include <QOpenGLWidget>
#include <QSurfaceFormat>
#include <QMouseEvent>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <cmath>
#include <iostream>

// 使用新的MainWindow类
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // 设置应用程序字体
    QFont font("Segoe UI", 10);
    app.setFont(font);
    
    // 检查着色器文件
    QStringList shaderFiles = {
        ":/shaders/basic.frag",
        ":/shaders/basic.vert",
        ":/shaders/circle.frag",
        ":/shaders/circle.vert",
        ":/shaders/multipass.vert",
        ":/shaders/multipass_circle.frag",
        ":/shaders/multipass_composite.frag" 
    };
    
    QStringList missingFiles;
    for (const QString& file : shaderFiles) {
        if (!QFile::exists(file)) {
            missingFiles.append(file);
        }
    }
    
    if (!missingFiles.isEmpty()) {
        QMessageBox::critical(nullptr, "Shader Files Missing", 
            "Required shader files not found:\n" + missingFiles.join("\n"));
        return 1;
    }
    
    // 使用新的MainWindow类
    MainWindow window;
    window.show();
    return app.exec();
}