#include "LensingControlPanel.h"
#include <QApplication>

LensingControlPanel::LensingControlPanel(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void LensingControlPanel::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);

    // Title - 改为英文
    QLabel* titleLabel = new QLabel("2D Gravitational Lensing");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #d0d0ff; padding: 10px 0;");
    mainLayout->addWidget(titleLabel);

    // Control group - 改为英文
    QGroupBox* controlGroup = new QGroupBox("Ray Controls");
    controlGroup->setStyleSheet(
        "QGroupBox { color: #c0c0d0; font-weight: bold; border: 1px solid #5a5a6a; border-radius: 8px; margin-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 8px 0 8px; }"
    );
    
    QVBoxLayout* groupLayout = new QVBoxLayout(controlGroup);
    
    // Buttons - 改为英文
    QPushButton* clearBtn = new QPushButton("Clear All Rays");
    QPushButton* sampleBtn = new QPushButton("Add Sample Rays");
    
    // Button style
    QString buttonStyle = 
        "QPushButton {"
        "   background-color: #5a5a7a; color: #e0e0ff; border: 1px solid #787898;"
        "   border-radius: 5px; padding: 12px; font-weight: bold; font-size: 12px; min-height: 35px;"
        "   transition: background-color 0.2s, border 0.2s;"
        "}"
        "QPushButton:hover { background-color: #6a6a8a; border: 1px solid #a0a0c0; }"
        "QPushButton:pressed { background-color: #4a4a6a; transform: translateY(1px); }";
    
    clearBtn->setStyleSheet(buttonStyle);
    sampleBtn->setStyleSheet(buttonStyle);
    
    groupLayout->addWidget(clearBtn);
    groupLayout->addWidget(sampleBtn);
    
    mainLayout->addWidget(controlGroup);
    
    // Instructions - 改为英文
    QLabel* infoLabel = new QLabel(
        "<b>Instructions:</b><br>"
        "• Middle mouse drag: Pan view<br>"
        "• Mouse wheel: Zoom<br>"
        "• Left click: Emit new ray<br>"
        "• Rays automatically simulate gravitational lensing"
    );
    infoLabel->setStyleSheet("color: #a0a0b0; font-size: 11px; line-height: 1.4; background: #2a2a3a; padding: 12px; border-radius: 6px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);
    
    // Spacer
    mainLayout->addStretch();
    
    // Footer - 改为英文
    QLabel* footerLabel = new QLabel("© 2024 Black Hole Simulation - 2D Lensing");
    footerLabel->setStyleSheet("color: #9090a0; font-size: 10px; margin-top: 20px;");
    mainLayout->addWidget(footerLabel);
    
    // Connect signals
    connect(clearBtn, &QPushButton::clicked, this, &LensingControlPanel::onClearRays);
    connect(sampleBtn, &QPushButton::clicked, this, &LensingControlPanel::onAddSampleRays);
}

void LensingControlPanel::onClearRays() {
    emit clearRaysRequested();
}

void LensingControlPanel::onAddSampleRays() {
    emit addSampleRaysRequested();
}