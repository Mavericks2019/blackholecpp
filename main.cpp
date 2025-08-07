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

#include "glwidget/glcirclewidget.h"
#include "tabs/controlpanel.h"

// ====================== MainWindow ======================
class MainWindow : public QMainWindow {
public:
    MainWindow() {
        setWindowTitle("OpenGL Demo - Dark Theme");
        resize(1200, 900);
        
        // Apply dark palette
        setDarkPalette();
        
        // Central widget
        QWidget* centralWidget = new QWidget();
        setCentralWidget(centralWidget);
        
        QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
        mainLayout->setSpacing(15);
        mainLayout->setContentsMargins(15, 15, 15, 15);
        
        // Left panel (tabs and canvas)
        QWidget* leftPanel = new QWidget();
        QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
        
        tabWidget = new QTabWidget();
        tabWidget->setTabPosition(QTabWidget::North);
        
        // Black hole demo tab
        QWidget* circleTab = new QWidget();
        QVBoxLayout* circleLayout = new QVBoxLayout(circleTab);
        circleCanvas = new GLCircleWidget();
        circleLayout->addWidget(circleCanvas);
        tabWidget->addTab(circleTab, "Black Hole Demo");
        
        // Add other tabs (stubs)
        tabWidget->addTab(new QWidget(), "Basic Demo");
        tabWidget->addTab(new QWidget(), "Multi-Pass Demo");
        
        leftLayout->addWidget(tabWidget);
        mainLayout->addWidget(leftPanel, 3);
        
        // Control stack
        controlStack = new QStackedWidget();
        
        // Black hole controls
        circleControl = new ControlPanel();
        controlStack->addWidget(circleControl);
        
        // Add other controls (stubs)
        controlStack->addWidget(new QWidget());
        controlStack->addWidget(new QWidget());
        
        mainLayout->addWidget(controlStack, 1);
        
        // Connections
        connect(circleControl, &ControlPanel::backgroundTypeChanged,
                circleCanvas, &GLCircleWidget::setBackgroundType);
        
        connect(circleCanvas, &GLCircleWidget::aspectRatioChanged,
                circleControl, &ControlPanel::setAspectRatio);
        
        connect(tabWidget, &QTabWidget::currentChanged, 
                controlStack, &QStackedWidget::setCurrentIndex);
        
        // Initial aspect ratio
        circleCanvas->updateAspectRatio();
    }

protected:
    void closeEvent(QCloseEvent* event) override {
        // Cleanup OpenGL resources
        if (circleCanvas) {
            circleCanvas->makeCurrent();
            // Additional cleanup if needed
            circleCanvas->doneCurrent();
        }
        QMainWindow::closeEvent(event);
    }

public:
    void setDarkPalette() {
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(45, 45, 55));
        darkPalette.setColor(QPalette::WindowText, QColor(220, 220, 220));
        darkPalette.setColor(QPalette::Base, QColor(35, 35, 45));
        darkPalette.setColor(QPalette::AlternateBase, QColor(45, 45, 55));
        darkPalette.setColor(QPalette::ToolTipBase, QColor(220, 220, 220));
        darkPalette.setColor(QPalette::ToolTipText, QColor(220, 220, 220));
        darkPalette.setColor(QPalette::Text, QColor(220, 220, 220));
        darkPalette.setColor(QPalette::Button, QColor(65, 65, 75));
        darkPalette.setColor(QPalette::ButtonText, QColor(220, 220, 220));
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Highlight, QColor(110, 110, 170));
        darkPalette.setColor(QPalette::HighlightedText, Qt::white);
        darkPalette.setColor(QPalette::Link, QColor(100, 150, 200));
        
        qApp->setPalette(darkPalette);
        qApp->setStyle("Fusion");
    }

    // UI elements
    QTabWidget* tabWidget;
    QStackedWidget* controlStack;
    GLCircleWidget* circleCanvas;
    ControlPanel* circleControl;
};

// ====================== Entry Point ======================
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Set application font
    QFont font("Segoe UI", 10);
    app.setFont(font);
    
    // Check shader files
    QStringList shaderFiles = {
        "../shaders/circle.frag",
        "../shaders/circle.vert"
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
    
    MainWindow window;
    window.show();
    return app.exec();
}