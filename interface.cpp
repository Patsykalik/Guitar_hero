#include "interface.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QStackedWidget>
#include <QtMath>
#include <QApplication>

RockScreenSaver::RockScreenSaver(QWidget *parent) : QWidget(parent) {
    setFixedSize(VIEW_WIDTH, VIEW_HEIGHT);
    scene = new QGraphicsScene(0, 0, VIEW_WIDTH, VIEW_HEIGHT);
    scene->setBackgroundBrush(Qt::black);
    amplifier = new Amplifier(); amplifier->setPos(350, 250); scene->addItem(amplifier);
    player = new GuitarPlayer(); scene->addItem(player);
    view = new QGraphicsView(scene);
    selector = new QComboBox(); selector->addItems({"🎸 Круг", "🎸 Синус", "🎸 Восьмёрка"});
    QPushButton *backBtn = new QPushButton("Назад в меню");
    connect(backBtn, &QPushButton::clicked, this, &RockScreenSaver::returnToMenu);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(view); layout->addWidget(selector); layout->addWidget(backBtn);
    timer = new QTimer(this); connect(timer, &QTimer::timeout, this, &RockScreenSaver::animate);
}
void RockScreenSaver::startAnimation() { timer->start(16); }
void RockScreenSaver::stopAnimation() { timer->stop(); }
void RockScreenSaver::animate() {
    angle += 0.05; qreal x = 400, y = 300;
    if (selector->currentIndex() == 0) { x = 400+180*qCos(angle); y = 300+130*qSin(angle); }
    else if (selector->currentIndex() == 1) { x = 100+fmod(angle*100, 600); y = 300+120*qSin(angle*2); }
    else { x = 400+200*qSin(angle); y = 300+120*qSin(angle)*qCos(angle); }
    player->setPos(x, y); player->setRotation(qSin(angle*3)*30);
    if (player->collidesWithItem(amplifier)) {
        if (!wasColliding) { amplifier->flash(); scene->addItem(new MusicNote(x+25, y+25)); }
        wasColliding = true;
    } else wasColliding = false;
    scene->setBackgroundBrush(QColor::fromHsl((int)(angle*40)%360, 150, 40));
}

SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Количество струн:"));
    stringsSpin = new QSpinBox(); stringsSpin->setRange(3, 5); stringsSpin->setValue(globalConfig.numStrings);
    layout->addWidget(stringsSpin);
    inclinedRadio = new QRadioButton("Наклонный гриф");
    verticalRadio = new QRadioButton("Вертикальный");
    if (globalConfig.inclinedFretboard) inclinedRadio->setChecked(true); else verticalRadio->setChecked(true);
    layout->addWidget(verticalRadio); layout->addWidget(inclinedRadio);
    QPushButton *saveBtn = new QPushButton("Сохранить");
    connect(saveBtn, &QPushButton::clicked, [this]() {
        globalConfig.numStrings = stringsSpin->value();
        globalConfig.inclinedFretboard = inclinedRadio->isChecked();
        emit returnToMenu();
    });
    layout->addWidget(saveBtn);
}

MainMenuWidget::MainMenuWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    songSelector = new QComboBox(); songSelector->addItems({"song1.mp3", "track_rock.wav"});
    QPushButton *playBtn = new QPushButton("▶ Играть");
    QPushButton *settingsBtn = new QPushButton("⚙ Настройки");
    QPushButton *screensaverBtn = new QPushButton("📺 Скринсейвер");
    layout->addWidget(new QLabel("<h1>Guitar Hero</h1>"));
    layout->addWidget(songSelector); layout->addWidget(playBtn);
    layout->addWidget(settingsBtn); layout->addWidget(screensaverBtn);
    connect(playBtn, &QPushButton::clicked, [this]() {
        globalConfig.selectedSong = songSelector->currentText();
        emit startGame();
    });
    connect(settingsBtn, &QPushButton::clicked, this, &MainMenuWidget::openSettings);
    connect(screensaverBtn, &QPushButton::clicked, this, &MainMenuWidget::openScreensaver);
}

MainWindow::MainWindow() {
    setFixedSize(VIEW_WIDTH, VIEW_HEIGHT);
    stackedWidget = new QStackedWidget(this);
    mainMenu = new MainMenuWidget();
    settings = new SettingsWidget();
    game = new GameWidget();
    screenSaver = new RockScreenSaver();
    stackedWidget->addWidget(mainMenu); stackedWidget->addWidget(settings);
    stackedWidget->addWidget(game); stackedWidget->addWidget(screenSaver);
    connect(mainMenu, &MainMenuWidget::openSettings, [this](){ stackedWidget->setCurrentIndex(1); });
    connect(mainMenu, &MainMenuWidget::startGame, [this](){ stackedWidget->setCurrentIndex(2); game->startGame(); });
    connect(mainMenu, &MainMenuWidget::openScreensaver, [this](){ stackedWidget->setCurrentIndex(3); screenSaver->startAnimation(); });
    connect(settings, &SettingsWidget::returnToMenu, [this](){ stackedWidget->setCurrentIndex(0); });
    connect(game, &GameWidget::returnToMenu, [this](){ stackedWidget->setCurrentIndex(0); });
    connect(screenSaver, &RockScreenSaver::returnToMenu, [this](){ screenSaver->stopAnimation(); stackedWidget->setCurrentIndex(0); });
}