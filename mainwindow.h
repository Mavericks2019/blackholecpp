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

// 新增2D引力透镜组件的头文件
#include "glwidget/GL2DLensingWidget.h"
#include "tabs/LensingControlPanel.h"

// Kerr Black Hole
#include "glwidget/kerrwidget.h"
#include "tabs/kerrcontrolpanel.h"

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
    
    // Black Hole Demo (放在最前面)
    GLCircleWidget* circleCanvas;
    ControlPanel* circleControl;
    
    // 新增：2D引力透镜Demo
    GL2DLensingWidget* lensingCanvas;
    LensingControlPanel* lensingControl;
    
    // Basic Demo
    GLBasicWidget* basicCanvas;
    BasicControlPanel* basicControl;
    
    // Multi-Pass Demo
    GLMultiPassWidget* multiPassCanvas = nullptr;
    MultiPassControlPanel* multiPassControl = nullptr;

    // Kerr Black Hole Demo
    KerrWidget* kerrCanvas = nullptr;
    KerrControlPanel* kerrControl = nullptr;
};

#endif // MAINWINDOW_H