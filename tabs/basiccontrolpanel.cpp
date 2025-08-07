#include "basiccontrolpanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QGroupBox>

BasicControlPanel::BasicControlPanel(QWidget* parent) : QFrame(parent) {
    setFrameShape(QFrame::StyledPanel);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20); // 增加边距
    
    // 标题
    QLabel* titleLabel = new QLabel("OpenGL Basic Controls");
    titleLabel->setObjectName("titleLabel"); // 添加对象名称
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    
    // 分隔线
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setFixedHeight(2);
    layout->addWidget(separator);
    
    // 控制按钮
    rotateBtn = new QPushButton("Rotate Triangle");
    rotateBtn->setObjectName("rotateBtn"); // 添加对象名称
    connect(rotateBtn, &QPushButton::clicked, this, &BasicControlPanel::rotateRequested);
    layout->addWidget(rotateBtn);
    
    // 添加间距
    layout->addSpacing(20);
    
    // 信息面板
    QGroupBox* infoGroup = new QGroupBox("Information");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    infoLayout->setContentsMargins(10, 15, 10, 15); // 增加内边距
    
    infoLabel = new QLabel(
        "<b>OpenGL Basic Demo</b><br><br>"
        "This demo shows a simple OpenGL 4.3 core profile triangle.<br><br>"
        "<b>Features:</b><br>"
        "- Vertex Array Object (VAO)<br>"
        "- Vertex Buffer Object (VBO)<br>"
        "- Attribute pointers<br>"
        "- Color interpolation"
    );
    infoLabel->setWordWrap(true);
    infoLabel->setMargin(10); // 增加文本边距
    infoLayout->addWidget(infoLabel);
    
    layout->addWidget(infoGroup);
    
    // 添加拉伸因子
    layout->addStretch(1);
    
    // 添加页脚
    QLabel* footer = new QLabel("© 2023 OpenGL Qt5 Demo | Dark Theme");
    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet("color: #9090a0; font-size: 10px; margin-top: 20px;");
    layout->addWidget(footer);
}