#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QStackedWidget>
#include "glwidget/glbasicwidget.h"
#include "glwidget/glcirclewidget.h"  // 包含Black Hole的OpenGL控件
#include "glwidget/glmultipasswidget.h"
#include "tabs/basiccontrolpanel.h"
#include "tabs/controlpanel.h"  // 包含Black Hole的控制面板
#include "tabs/multipasscontrolpanel.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    
private slots:
    void onTabChanged(int index);
    
private:
    void setDarkPalette();
    void createTabs();
    void createControlPanels();
    void connectSignals();
    void applyStyles();
    
    // UI元素
    QTabWidget* tabWidget;
    QStackedWidget* controlStack;
    
    // Basic Demo
    GLBasicWidget* basicCanvas;
    BasicControlPanel* basicControl;
    
    // Black Hole Demo
    GLCircleWidget* circleCanvas;
    ControlPanel* circleControl;

    GLMultiPassWidget* multiPassCanvas = nullptr;
    MultiPassControlPanel* multiPassControl = nullptr;
};

#endif // MAINWINDOW_H