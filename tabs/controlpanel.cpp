#include "controlpanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFrame>
#include <QRadioButton> // 改为包含QRadioButton

ControlPanel::ControlPanel(QWidget* parent) : QFrame(parent) {
    setFrameShape(QFrame::StyledPanel);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // Title
    QLabel* titleLabel = new QLabel("OpenGL Black Hole Controls");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    
    // Separator
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedHeight(2);
    layout->addWidget(separator);
    
    // Background type group
    QGroupBox* bgGroup = new QGroupBox("Background Type");
    QVBoxLayout* bgLayout = new QVBoxLayout(bgGroup);
    bgLayout->setContentsMargins(10, 15, 10, 15);
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
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
    
    // 添加间距
    layout->addSpacing(20);
    
    // Mipmap debug option - 改为QRadioButton
    QGroupBox* mipmapGroup = new QGroupBox("Debug Options");
    QVBoxLayout* mipmapLayout = new QVBoxLayout(mipmapGroup);
    mipmapLayout->setContentsMargins(10, 15, 10, 15);
    
    // 将QCheckBox改为QRadioButton
    mipmapRadioButton = new QRadioButton("Show Mipmap Effect");
    mipmapRadioButton->setObjectName("mipmapRadioButton"); // 更新对象名
    mipmapLayout->addWidget(mipmapRadioButton);
    
    layout->addWidget(mipmapGroup);
    
    // 添加间距
    layout->addSpacing(20);
    
    // Aspect ratio group
    QGroupBox* ratioGroup = new QGroupBox("Aspect Ratio");
    QVBoxLayout* ratioLayout = new QVBoxLayout(ratioGroup);
    ratioLayout->setContentsMargins(10, 15, 10, 15);
    
    ratioLabel = new QLabel("Current: 1.00");
    ratioLabel->setAlignment(Qt::AlignCenter);
    ratioLayout->addWidget(ratioLabel);
    
    QLabel* ratioInfo = new QLabel("To maintain circle shape:\nWidth : Height = 1 : 1");
    ratioInfo->setAlignment(Qt::AlignCenter);
    ratioInfo->setMargin(10);
    ratioLayout->addWidget(ratioInfo);
    
    layout->addWidget(ratioGroup);
    
    // 添加拉伸因子
    layout->addStretch(1);
    
    // Footer
    QLabel* footer = new QLabel("© 2023 OpenGL Qt5 Demo | Dark Theme | MSAA Enabled");
    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet("color: #9090a0; font-size: 10px; margin-top: 20px;");
    layout->addWidget(footer);
    
    // 连接mipmap单选按钮信号
    connect(mipmapRadioButton, &QRadioButton::toggled, this, [this](bool checked) {
        emit showMipmapChanged(checked);
    });
}

QPushButton* ControlPanel::createBgButton(const QString& text, int type) {
    QPushButton* btn = new QPushButton(text);
    btn->setObjectName(QString("bgBtn_%1").arg(type));
    btn->setCheckable(true);
    btn->setFixedHeight(30);
    connect(btn, &QPushButton::clicked, this, [this, type]() {
        setBackgroundType(type);
    });
    return btn;
}

void ControlPanel::setBackgroundType(int type) {
    // 更新所有按钮文本 - 移除可能存在的对号
    bgChessBtn->setText(bgChessBtn->text().replace("✓ ", ""));
    bgBlackBtn->setText(bgBlackBtn->text().replace("✓ ", ""));
    bgStarsBtn->setText(bgStarsBtn->text().replace("✓ ", ""));
    bgTextureBtn->setText(bgTextureBtn->text().replace("✓ ", ""));
    
    // 为选中的按钮添加对号
    if (type == 0) bgChessBtn->setText("✓ " + bgChessBtn->text());
    if (type == 1) bgBlackBtn->setText("✓ " + bgBlackBtn->text());
    if (type == 2) bgStarsBtn->setText("✓ " + bgStarsBtn->text());
    if (type == 3) bgTextureBtn->setText("✓ " + bgTextureBtn->text());
    
    // 设置选中状态
    bgChessBtn->setChecked(type == 0);
    bgBlackBtn->setChecked(type == 1);
    bgStarsBtn->setChecked(type == 2);
    bgTextureBtn->setChecked(type == 3);
    
    emit backgroundTypeChanged(type);
}

void ControlPanel::setAspectRatio(const QString& ratio) {
    ratioLabel->setText(ratio);
}