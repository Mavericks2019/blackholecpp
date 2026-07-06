#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>

class ControlPanel : public QFrame {
    Q_OBJECT
public:
    explicit ControlPanel(QWidget* parent = nullptr);
    void setAspectRatio(const QString& ratio);

signals:
    void backgroundTypeChanged(int type);
    void showMipmapChanged(bool show);
    void horizontalBlurChanged(bool enabled);  // 改为bool类型信号
    void verticalBlurChanged(bool enabled);    // 改为bool类型信号
    void showRenderResultChanged(bool show);   // 新增渲染结果信号

public:
    QPushButton* createBgButton(const QString& text, int type);
    void setBackgroundType(int type);
    
    // UI elements
    QPushButton* bgChessBtn;
    QPushButton* bgBlackBtn;
    QPushButton* bgStarsBtn;
    QPushButton* bgTextureBtn;
    QLabel* ratioLabel;
    QCheckBox* mipmapRadioButton;
    QCheckBox* horizontalBlurRadio; 
    QCheckBox* verticalBlurRadio;
    QCheckBox* showRenderResultCheck; // 新增渲染结果复选框
};

#endif // CONTROLPANEL_H