#ifndef KERRCONTROLPANEL_H
#define KERRCONTROLPANEL_H

#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

class KerrControlPanel : public QFrame {
    Q_OBJECT
public:
    explicit KerrControlPanel(QWidget* parent = nullptr);

    void setFPS(float fps);

signals:
    void bloomChanged(bool enabled);
    void taaChanged(bool enabled);
    void resetCameraClicked();
    void blackHoleMassChanged(double mass);
    void spinChanged(double spin);

public:
    QLabel* fpsLabel;
    QCheckBox* bloomCheck;
    QCheckBox* taaCheck;
    QPushButton* resetBtn;
    QLabel* titleLabel;
    QLabel* infoLabel;
};

#endif // KERRCONTROLPANEL_H
