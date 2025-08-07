#ifndef BASICCONTROLPANEL_H
#define BASICCONTROLPANEL_H

#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

class BasicControlPanel : public QFrame {
    Q_OBJECT
public:
    explicit BasicControlPanel(QWidget* parent = nullptr);
    
signals:
    void rotateRequested();

private:
    QPushButton* rotateBtn;
    QLabel* infoLabel;
};

#endif // BASICCONTROLPANEL_H