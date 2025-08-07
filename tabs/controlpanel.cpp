#include "controlpanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFrame>

ControlPanel::ControlPanel(QWidget* parent) : QFrame(parent) {
    setFrameShape(QFrame::StyledPanel);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(15, 15, 15, 15);
    
    // Title
    QLabel* titleLabel = new QLabel("OpenGL Black Hole Controls");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    
    // Separator
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    layout->addWidget(separator);
    
    // Background type group
    QGroupBox* bgGroup = new QGroupBox("Background Type");
    QVBoxLayout* bgLayout = new QVBoxLayout(bgGroup);
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    bgLayout->addLayout(btnLayout);
    
    bgChessBtn = createBgButton("Chess", 0);
    bgBlackBtn = createBgButton("Black", 1);
    bgStarsBtn = createBgButton("Stars", 2);
    bgTextureBtn = createBgButton("Texture", 3);
    
    btnLayout->addWidget(bgChessBtn);
    btnLayout->addWidget(bgBlackBtn);
    btnLayout->addWidget(bgStarsBtn);
    btnLayout->addWidget(bgTextureBtn);
    
    layout->addWidget(bgGroup);
    
    // Stretch
    layout->addStretch(1);
    
    // Aspect ratio group
    QGroupBox* ratioGroup = new QGroupBox("Aspect Ratio");
    QVBoxLayout* ratioLayout = new QVBoxLayout(ratioGroup);
    
    ratioLabel = new QLabel("Current: 1.00");
    ratioLabel->setAlignment(Qt::AlignCenter);
    ratioLayout->addWidget(ratioLabel);
    
    QLabel* ratioInfo = new QLabel("To maintain circle shape:\nWidth : Height = 1 : 1");
    ratioInfo->setAlignment(Qt::AlignCenter);
    ratioLayout->addWidget(ratioInfo);
    
    layout->addWidget(ratioGroup);
    
    // Footer
    QLabel* footer = new QLabel("© 2023 OpenGL Qt5 Demo | Dark Theme | MSAA Enabled");
    footer->setAlignment(Qt::AlignCenter);
    layout->addWidget(footer);
}

QPushButton* ControlPanel::createBgButton(const QString& text, int type) {
    QPushButton* btn = new QPushButton(text);
    btn->setCheckable(true);
    btn->setFixedHeight(30);
    connect(btn, &QPushButton::clicked, this, [this, type]() {
        setBackgroundType(type);
    });
    return btn;
}

void ControlPanel::setBackgroundType(int type) {
    bgChessBtn->setChecked(type == 0);
    bgBlackBtn->setChecked(type == 1);
    bgStarsBtn->setChecked(type == 2);
    bgTextureBtn->setChecked(type == 3);
    emit backgroundTypeChanged(type);
}

void ControlPanel::setAspectRatio(const QString& ratio) {
    ratioLabel->setText(ratio);
}