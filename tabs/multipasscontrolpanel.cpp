#include "multipasscontrolpanel.h"

MultiPassControlPanel::MultiPassControlPanel(QWidget* parent)
    : QFrame(parent) {
    setFrameShape(QFrame::StyledPanel);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // 标题
    QLabel* titleLabel = new QLabel("Multi-Pass Rendering");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    
    // 分隔线
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedHeight(2);
    layout->addWidget(separator);
    
    // 信息标签
    infoLabel = new QLabel(
        "This demo shows multi-pass rendering using FBO.\n\n"
        "<b>First Pass:</b> Render a circle to a texture using FBO.\n"
        "<b>Second Pass:</b> Render a rectangle and composite with the circle texture.\n\n"
        "The text is drawn using QPainter in the paintGL function."
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("margin-top: 15px;");
    layout->addWidget(infoLabel);
    
    // 添加拉伸因子
    layout->addStretch(1);
    
    // 页脚
    QLabel* footer = new QLabel("© 2023 OpenGL Qt5 Demo | Multi-Pass Rendering");
    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet("color: #9090a0; font-size: 10px; margin-top: 20px;");
    layout->addWidget(footer);
}