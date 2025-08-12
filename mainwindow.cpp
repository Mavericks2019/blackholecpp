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
#include <QSplitter>
#include <QCheckBox>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("OpenGL Demo - Dark Theme");
    resize(2700, 1800);
    
    // Set dark theme
    setDarkPalette();
    
    // Central widget
    QWidget* centralWidget = new QWidget();
    setCentralWidget(centralWidget);
    
    // Main layout for central widget
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    
    // Create main splitter (horizontal)
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->setHandleWidth(8);
    mainSplitter->setChildrenCollapsible(false);
    mainSplitter->setStyleSheet(
        "QSplitter::handle {"
        "   background-color: #4a4a5a;"
        "   border: 1px solid #3a3a4a;"
        "   border-radius: 4px;"
        "}"
        "QSplitter::handle:hover {"
        "   background-color: #5a5a6a;"
        "}"
    );
    
    // Left panel (contains tabs)
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create tab widget
    tabWidget = new QTabWidget();
    tabWidget->setTabPosition(QTabWidget::North);
    tabWidget->setDocumentMode(false);
    
    // Apply tab styles
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: none; margin-top: 4px; }"
        "QTabBar::tab { background: #3a3a4a; color: #c0c0d0; padding: 8px 16px;"
        "margin-right: 4px; border-top-left-radius: 8px; border-top-right-radius: 8px;"
        "border: 1px solid #3a3a4a; border-bottom: none; font-weight: bold; font-size: 12px; }"
        "QTabBar::tab:selected { background: #505060; color: #ffffff; border-color: #5a5a6a; }"
        "QTabBar::tab:hover { background: #454555; }"
        "QTabBar::tab:!selected { margin-top: 4px; }"
    );
    
    // Create tab content
    createTabs();
    
    leftLayout->addWidget(tabWidget);
    
    // Create control panels
    controlStack = new QStackedWidget();
    createControlPanels();
    
    // Set control panel width
    controlStack->setMinimumWidth(300);
    controlStack->setMaximumWidth(600);
    
    // Add panels to splitter
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(controlStack);
    
    // Set initial size proportions (80% left, 20% right)
    QList<int> sizes;
    sizes << width() * 0.80 << width() * 0.20;
    mainSplitter->setSizes(sizes);
    
    // Add splitter to main layout
    mainLayout->addWidget(mainSplitter);
    
    // Connect signals
    connectSignals();
    
    // Apply styles
    applyStyles();
    
    // Set initial active tab
    tabWidget->setCurrentIndex(0);
    controlStack->setCurrentIndex(0);
}

MainWindow::~MainWindow() {
    // Clean up resources
    delete basicCanvas;
    delete basicControl;
    delete circleCanvas;
    delete circleControl;
    delete multiPassCanvas;
    delete multiPassControl;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // Clean up OpenGL resources
    if (basicCanvas) {
        basicCanvas->makeCurrent();
        basicCanvas->doneCurrent();
    }
    if (circleCanvas) {
        circleCanvas->makeCurrent();
        circleCanvas->doneCurrent();
    }
    if (multiPassCanvas) {
        multiPassCanvas->makeCurrent();
        multiPassCanvas->doneCurrent();
    }
    QMainWindow::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    // Update aspect ratio for all canvases
    if (circleCanvas) {
        circleCanvas->updateAspectRatio();
    }
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    // Force repaint of all canvases
    if (basicCanvas) {
        basicCanvas->update();
    }
    if (circleCanvas) {
        circleCanvas->update();
    }
    if (multiPassCanvas) {
        multiPassCanvas->update();
    }
}

void MainWindow::onTabChanged(int index) {
    controlStack->setCurrentIndex(index);
    // Force repaint when switching tabs
    switch(index) {
        case 0: // Basic
            if (basicCanvas) basicCanvas->update();
            break;
        case 1: // Black Hole
            if (circleCanvas) circleCanvas->update();
            break;
        case 2: // Multi-Pass
            if (multiPassCanvas) multiPassCanvas->update();
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
    // Basic Demo tab (index 0)
    QWidget* basicTab = new QWidget();
    QVBoxLayout* basicLayout = new QVBoxLayout(basicTab);
    basicCanvas = new GLBasicWidget();
    basicLayout->addWidget(basicCanvas);
    tabWidget->addTab(basicTab, "Basic Demo");
    
    // Black Hole tab (index 1)
    QWidget* circleTab = new QWidget();
    QVBoxLayout* circleLayout = new QVBoxLayout(circleTab);
    circleCanvas = new GLCircleWidget();
    circleLayout->addWidget(circleCanvas);
    tabWidget->addTab(circleTab, "Black Hole Demo");
    
    // Multi-Pass Demo tab (index 2)
    QWidget* multiPassTab = new QWidget();
    QVBoxLayout* multiPassLayout = new QVBoxLayout(multiPassTab);
    multiPassCanvas = new GLMultiPassWidget();
    multiPassLayout->addWidget(multiPassCanvas);
    tabWidget->addTab(multiPassTab, "Multi-Pass Demo");
}

void MainWindow::createControlPanels() {
    // Basic control panel (index 0)
    basicControl = new BasicControlPanel();
    controlStack->addWidget(basicControl);
    
    // Black Hole control panel (index 1)
    circleControl = new ControlPanel();
    controlStack->addWidget(circleControl);
    
    // Multi-Pass control panel (index 2)
    multiPassControl = new MultiPassControlPanel();
    controlStack->addWidget(multiPassControl);
}

void MainWindow::connectSignals() {
    // Tab change signal
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    
    // Basic control signals
    connect(basicControl, &BasicControlPanel::rotateRequested, []() {
        // Rotation logic for triangle
    });
    
    // Black Hole control signals
    connect(circleControl, &ControlPanel::backgroundTypeChanged,
            circleCanvas, &GLCircleWidget::setBackgroundType);
    
    connect(circleCanvas, &GLCircleWidget::aspectRatioChanged,
            circleControl, &ControlPanel::setAspectRatio);
    
    // Connect mipmap signal
    connect(circleControl, &ControlPanel::showMipmapChanged,
            circleCanvas, &GLCircleWidget::setShowMipmap);

    // 连接模糊方向信号
    connect(circleControl, &ControlPanel::horizontalBlurChanged,
            circleCanvas, &GLCircleWidget::setHorizontalBlurEnabled);
    
    connect(circleControl, &ControlPanel::verticalBlurChanged,
            circleCanvas, &GLCircleWidget::setVerticalBlurEnabled);
    
    // Initial aspect ratio update
    if (circleCanvas) {
        circleCanvas->updateAspectRatio();
    }
}

void MainWindow::applyStyles() {
    // Apply basic control panel style
    basicControl->setStyleSheet(
        "background-color: #2d2d3a; border-radius: 8px; border: 1px solid #3a3a4a;"
    );
    
    // Apply Black Hole control panel style
    circleControl->setStyleSheet(
        "background-color: #2d2d3a; border-radius: 8px; border: 1px solid #3a3a4a;"
    );
    
    // Apply Multi-Pass control panel style
    multiPassControl->setStyleSheet(
        "background-color: #2d2d3a; border-radius: 8px; border: 1px solid #3a3a4a;"
    );
    
    // Title styles
    QLabel* basicTitle = basicControl->findChild<QLabel*>("titleLabel");
    if (basicTitle) {
        basicTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #d0d0ff; padding: 15px 0;");
    }
    
    QLabel* circleTitle = circleControl->findChild<QLabel*>("titleLabel");
    if (circleTitle) {
        circleTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #d0d0ff; padding: 15px 0;");
    }
    
    QLabel* multiPassTitle = multiPassControl->findChild<QLabel*>("titleLabel");
    if (multiPassTitle) {
        multiPassTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #d0d0ff; padding: 15px 0;");
    }
    
    // Button styles
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
    
    // Black Hole button styles
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
    
    // Mipmap radio button style - 改为圆形单选按钮
    QRadioButton* mipmapRadio = circleControl->findChild<QRadioButton*>("mipmapRadioButton");
    QRadioButton* horizontalBlurRadio = circleControl->findChild<QRadioButton*>("horizontalBlurRadio");
    QRadioButton* verticalBlurRadio = circleControl->findChild<QRadioButton*>("verticalBlurRadio");
    // Footer styles
    QLabel* basicFooter = basicControl->findChild<QLabel*>();
    if (basicFooter && basicFooter->text().contains("©")) {
        basicFooter->setStyleSheet("color: #9090a0; font-size: 10px; margin-top: 20px;");
    }
    
    QLabel* circleFooter = circleControl->findChild<QLabel*>();
    if (circleFooter && circleFooter->text().contains("©")) {
        circleFooter->setStyleSheet("color: #9090a0; font-size: 10px; margin-top: 20px;");
    }

    QLabel* multiPassFooter = multiPassControl->findChild<QLabel*>();
    if (multiPassFooter && multiPassFooter->text().contains("©")) {
        multiPassFooter->setStyleSheet("color: #9090a0; font-size: 10px; margin-top: 20px;");
    }
}