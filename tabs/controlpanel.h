#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

class ControlPanel : public QFrame {
    Q_OBJECT
public:
    explicit ControlPanel(QWidget* parent = nullptr);
    void setAspectRatio(const QString& ratio);

signals:
    void backgroundTypeChanged(int type);
    void showMipmapChanged(bool show);

public:
    QPushButton* createBgButton(const QString& text, int type);
    void setBackgroundType(int type);
    
    // UI elements
    QPushButton* bgChessBtn;
    QPushButton* bgBlackBtn;
    QPushButton* bgStarsBtn;
    QPushButton* bgTextureBtn;
    QLabel* ratioLabel;
    QCheckBox* mipmapCheckBox;
};

#endif // CONTROLPANEL_H