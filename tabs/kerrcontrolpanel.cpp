#include "kerrcontrolpanel.h"
#include <QGroupBox>

KerrControlPanel::KerrControlPanel(QWidget* parent) : QFrame(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Title
    titleLabel = new QLabel("Kerr Black Hole");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #d0d0ff; padding: 15px 0;");
    mainLayout->addWidget(titleLabel);

    // Info
    infoLabel = new QLabel(
        "Kerr-Newman Black Hole Renderer\n\n"
        "Controls:\n"
        "  W/S: Forward/Back\n"
        "  A/D: Left/Right\n"
        "  R/F: Up/Down\n"
        "  Q/E: Roll\n"
        "  Mouse: Look\n\n"
        "Physics:\n"
        "  Mass: 10^7 M_sun\n"
        "  Spin: a* = 0.997\n"
        "  Charge: Q* = 0\n\n"
        "Features:\n"
        "  - Accretion disk + jets\n"
        "  - Heat haze refraction\n"
        "  - TAA temporal anti-aliasing\n"
        "  - HDR bloom\n"
        "  - Topology map overlay"
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #b0b0c0; font-size: 11px; padding: 5px;");
    mainLayout->addWidget(infoLabel);

    // Bloom toggle
    bloomCheck = new QCheckBox("Bloom");
    bloomCheck->setChecked(true);
    mainLayout->addWidget(bloomCheck);

    // TAA toggle
    taaCheck = new QCheckBox("Temporal AA (TAA)");
    taaCheck->setChecked(true);
    mainLayout->addWidget(taaCheck);

    // Reset camera button
    resetBtn = new QPushButton("Reset Camera");
    resetBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #5a5a7a; color: #e0e0ff; border: 1px solid #787898;"
        "   border-radius: 5px; padding: 8px; font-weight: bold; font-size: 12px; min-height: 30px;"
        "}"
        "QPushButton:hover { background-color: #6a6a8a; border: 1px solid #a0a0c0; }"
        "QPushButton:pressed { background-color: #4a4a6a; }"
    );
    mainLayout->addWidget(resetBtn);

    // FPS display
    fpsLabel = new QLabel("FPS: --");
    fpsLabel->setStyleSheet("color: #80ff80; font-size: 14px; font-weight: bold; padding: 5px;");
    mainLayout->addWidget(fpsLabel);

    mainLayout->addStretch();

    // Connect signals
    connect(bloomCheck, &QCheckBox::toggled, this, &KerrControlPanel::bloomChanged);
    connect(taaCheck, &QCheckBox::toggled, this, &KerrControlPanel::taaChanged);
    connect(resetBtn, &QPushButton::clicked, this, &KerrControlPanel::resetCameraClicked);
}

void KerrControlPanel::setFPS(float fps) {
    fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
}
