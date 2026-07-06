#ifndef MULTIPASSCONTROLPANEL_H
#define MULTIPASSCONTROLPANEL_H

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>

class MultiPassControlPanel : public QFrame {
    Q_OBJECT
public:
    explicit MultiPassControlPanel(QWidget* parent = nullptr);

private:
    QLabel* infoLabel;
};

#endif // MULTIPASSCONTROLPANEL_H