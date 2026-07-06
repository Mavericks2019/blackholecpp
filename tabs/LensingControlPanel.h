#ifndef LENSINGCONTROLPANEL_H
#define LENSINGCONTROLPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

class LensingControlPanel : public QWidget {
    Q_OBJECT

public:
    explicit LensingControlPanel(QWidget* parent = nullptr);

signals:
    void clearRaysRequested();
    void addSampleRaysRequested();

private:
    void setupUI();

private slots:
    void onClearRays();
    void onAddSampleRays();
};

#endif // LENSINGCONTROLPANEL_H