#include "mainwindow.h"
#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QPalette>
#include <QFont>
#include <QFile>
#include <QStyleFactory>
#include <QTimer>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("OpenGL Demo - Dark Theme");
    resize(2400, 1800);
    
    // 设置深色主题
    setDarkPalette();
    
    // 中央部件
    QWidget* centralWidget = new QWidget();
    setCentralWidget(centralWidget);
    
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 左侧面板
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    
    // 创建标签页
    tabWidget = new QTabWidget();
    tabWidget->setTabPosition(QTabWidget::North);
    tabWidget->setDocumentMode(false);
    
    // 应用标签页样式
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: none; margin-top: 4px; }"
        "QTabBar::tab { background: #3a3a4a; color: #c0c0d0; padding: 8px 16px;"
        "margin-right: 4px; border-top-left-radius: 8px; border-top-right-radius: 8px;"
        "border: 1px solid #3a3a4a; border-bottom: none; font-weight: bold; font-size: 12px; }"
        "QTabBar::tab:selected { background: #505060; color: #ffffff; border-color: #5a5a6a; }"
        "QTabBar::tab:hover { background: #454555; }"
        "QTabBar::tab:!selected { margin-top: 4px; }"
    );
    
    // 创建标签页内容
    createTabs();
    
    leftLayout->addWidget(tabWidget);
    mainLayout->addWidget(leftPanel, 3);
    
    // 创建控制面板
    controlStack = new QStackedWidget();
    createControlPanels();
    mainLayout->addWidget(controlStack, 1);
    
    // 连接信号
    connectSignals();
    
    // 应用样式
    applyStyles();
    
    // 设置初始激活标签页为Basic Demo
    tabWidget->setCurrentIndex(0);
    controlStack->setCurrentIndex(0);
}

MainWindow::~MainWindow() {
    // 清理资源
    delete basicCanvas;
    delete basicControl;
    delete circleCanvas;
    delete circleControl;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // 清理OpenGL资源
    if (basicCanvas) {
        basicCanvas->makeCurrent();
        basicCanvas->doneCurrent();
    }
    if (circleCanvas) {
        circleCanvas->makeCurrent();
        circleCanvas->doneCurrent();
    }
    QMainWindow::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    // 更新所有画布的宽高比
    if (circleCanvas) {
        circleCanvas->updateAspectRatio();
    }
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    // 强制重绘所有画布
    if (basicCanvas) {
        basicCanvas->update();
    }
    if (circleCanvas) {
        circleCanvas->update();
    }
}

void MainWindow::onTabChanged(int index) {
    controlStack->setCurrentIndex(index);
    // 切换标签页时强制重绘
    switch(index) {
        case 0: // Basic
            if (basicCanvas) basicCanvas->update();
            break;
        case 1: // Black Hole
            if (circleCanvas) circleCanvas->update();
            break;
        case 2: // Multi-Pass
            // 未来扩展
            break;
    }
}

void MainWindow::setDarkPalette() {
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
    qApp->setStyle(QStyleFactory::create("Fusion"));
}

void MainWindow::createTabs() {
    // 基本功能标签页 (索引0)
    QWidget* basicTab = new QWidget();
    QVBoxLayout* basicLayout = new QVBoxLayout(basicTab);
    basicCanvas = new GLBasicWidget();
    basicLayout->addWidget(basicCanvas);
    tabWidget->addTab(basicTab, "Basic Demo");
    
    // Black Hole标签页 (索引1)
    QWidget* circleTab = new QWidget();
    QVBoxLayout* circleLayout = new QVBoxLayout(circleTab);
    circleCanvas = new GLCircleWidget();
    circleLayout->addWidget(circleCanvas);
    tabWidget->addTab(circleTab, "Black Hole Demo");
    
    // Multi-Pass Demo (索引2)
    tabWidget->addTab(new QWidget(), "Multi-Pass Demo");
}

void MainWindow::createControlPanels() {
    // 基本功能控制面板 (索引0)
    basicControl = new BasicControlPanel();
    controlStack->addWidget(basicControl);
    
    // Black Hole控制面板 (索引1)
    circleControl = new ControlPanel();
    controlStack->addWidget(circleControl);
    
    // Multi-Pass控制面板 (索引2)
    controlStack->addWidget(new QWidget());
}

void MainWindow::connectSignals() {
    // 标签页切换
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    
    // Basic控制信号
    connect(basicControl, &BasicControlPanel::rotateRequested, []() {
        // 这里可以添加旋转三角形的逻辑
    });
    
    // Black Hole控制信号
    connect(circleControl, &ControlPanel::backgroundTypeChanged,
            circleCanvas, &GLCircleWidget::setBackgroundType);
    
    connect(circleCanvas, &GLCircleWidget::aspectRatioChanged,
            circleControl, &ControlPanel::setAspectRatio);
    
    // 初始宽高比更新
    if (circleCanvas) {
        circleCanvas->updateAspectRatio();
    }
}

void MainWindow::applyStyles() {
    // 应用基本控制面板样式
    basicControl->setStyleSheet(
        "background-color: #2d2d3a; border-radius: 8px; border: 1px solid #3a3a4a;"
    );
    
    // 应用Black Hole控制面板样式
    circleControl->setStyleSheet(
        "background-color: #2d2d3a; border-radius: 8px; border: 1px solid #3a3a4a;"
    );
    
    // 标题样式
    QLabel* basicTitle = basicControl->findChild<QLabel*>("titleLabel");
    if (basicTitle) {
        basicTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #d0d0ff; padding: 15px 0;");
    }
    
    // Black Hole标题样式
    QLabel* circleTitle = circleControl->findChild<QLabel*>("titleLabel");
    if (circleTitle) {
        circleTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #d0d0ff; padding: 15px 0;");
    }
    
    // 按钮样式
    QPushButton* rotateBtn = basicControl->findChild<QPushButton*>("rotateBtn");
    if (rotateBtn) {
        rotateBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #6a6a8a; color: #e0e0ff; border: 1px solid #8888aa;"
            "   border-radius: 5px; padding: 12px; font-weight: bold; font-size: 14px; min-height: 40px;"
            "   transition: background-color 0.2s, border 0.2s;"
            "}"
            "QPushButton:hover { background-color: #7a7a9a; border: 2px solid #a0a0c0; }"
            "QPushButton:pressed { background-color: #5a5a7a; transform: translateY(1px); }"
            "QPushButton:checked { background-color: #505070; border: 2px solid #a0a0c0; }"
        );
    }
    
    // Black Hole按钮样式
    QList<QPushButton*> bgButtons = circleControl->findChildren<QPushButton*>();
    for (QPushButton* btn : bgButtons) {
        btn->setStyleSheet(
            "QPushButton {"
            "   background-color: #5a5a7a; color: #e0e0ff; border: 1px solid #787898;"
            "   border-radius: 5px; padding: 8px; font-weight: bold; font-size: 12px; min-height: 30px;"
            "   transition: background-color 0.2s, border 0.2s;"
            "}"
            "QPushButton:hover { background-color: #6a6a8a; border: 1px solid #a0a0c0; }"
            "QPushButton:pressed { background-color: #4a4a6a; transform: translateY(1px); }"
            "QPushButton:checked { background-color: #505070; border: 2px solid #a0a0c0; }"
        );
    }
    
    // 页脚样式
    QLabel* basicFooter = basicControl->findChild<QLabel*>();
    if (basicFooter && basicFooter->text().contains("©")) {
        basicFooter->setStyleSheet("color: #9090a0; font-size: 10px; margin-top: 20px;");
    }
    
    QLabel* circleFooter = circleControl->findChild<QLabel*>();
    if (circleFooter && circleFooter->text().contains("©")) {
        circleFooter->setStyleSheet("color: #9090a0; font-size: 10px; margin-top: 20px;");
    }
}