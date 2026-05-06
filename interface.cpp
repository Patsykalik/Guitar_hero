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
#include <QTimer>

RockScreenSaver::RockScreenSaver(QWidget *parent) : QWidget(parent) {
    setFixedSize(VIEW_WIDTH, VIEW_HEIGHT);
    scene = new QGraphicsScene(0, 0, VIEW_WIDTH, VIEW_HEIGHT, this);
    scene->setBackgroundBrush(Qt::black);

    amplifier = new Amplifier();
    amplifier->setPos(350, 250);
    scene->addItem(amplifier);

    player = new GuitarPlayer();
    scene->addItem(player);

    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("background: transparent; border: none;");

    selector = new QComboBox();
    selector->addItems({"🎸 Круг", "🎸 Синус", "🎸 Восьмёрка"});

    QPushButton *backBtn = new QPushButton("Назад в меню");
    connect(backBtn, &QPushButton::clicked, this, &RockScreenSaver::returnToMenu);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(view);
    layout->addWidget(selector);
    layout->addWidget(backBtn);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &RockScreenSaver::animate);
}

void RockScreenSaver::startAnimation() { timer->start(16); }
void RockScreenSaver::stopAnimation() { timer->stop(); }

void RockScreenSaver::animate() {
    angle += 0.05;
    qreal x = 400, y = 300;
    int mode = selector->currentIndex();

    if (mode == 0) { 
        x = 400 + 150 * qCos(angle);
        y = 300 + 150 * qSin(angle);
    } else if (mode == 1) { 
        x = 100 + fmod(angle * 50, 600);
        y = 300 + 100 * qSin(angle);
    } else { 
        x = 400 + 200 * qCos(angle);
        y = 300 + 100 * qSin(2 * angle);
    }

    player->setPos(x - 25, y - 25);
    player->setRotation(angle * 50);

    if (player->collidesWithItem(amplifier)) {
        if (!wasColliding) {
            amplifier->flash();
            scene->addItem(new MusicNote(amplifier->x() + 50, amplifier->y()));
            wasColliding = true;
        }
    } else {
        wasColliding = false;
    }
}

SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *title = new QLabel("Настройки гитары");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 24px; font-weight: bold;");

    QHBoxLayout *hBox = new QHBoxLayout();
    hBox->addWidget(new QLabel("Количество струн (1-4):"));
    stringsSpin = new QSpinBox();
    stringsSpin->setRange(1, 4);
    stringsSpin->setValue(globalConfig.numStrings);
    hBox->addWidget(stringsSpin);

    verticalRadio = new QRadioButton("Прямой гриф");
    inclinedRadio = new QRadioButton("Наклонный гриф");
    if (globalConfig.inclinedFretboard) inclinedRadio->setChecked(true);
    else verticalRadio->setChecked(true);

    QPushButton *saveBtn = new QPushButton("Сохранить и выйти");
    
    layout->addWidget(title);
    layout->addLayout(hBox);
    layout->addWidget(verticalRadio);
    layout->addWidget(inclinedRadio);
    layout->addWidget(saveBtn);

    connect(saveBtn, &QPushButton::clicked, [this]() {
        globalConfig.numStrings = stringsSpin->value(); 
        globalConfig.inclinedFretboard = inclinedRadio->isChecked();
        emit returnToMenu();
    });
}

MainMenuWidget::MainMenuWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *logo = new QLabel("GUITAR HERO");
    logo->setAlignment(Qt::AlignCenter);
    logo->setStyleSheet("font-size: 40px; color: red; font-weight: bold;");

    songSelector = new QComboBox();
    songSelector->addItems({"song1.mp3", "rock_track.mp3", "solo.mp3"});

    QPushButton *playBtn = new QPushButton("ИГРАТЬ");
    QPushButton *settingsBtn = new QPushButton("НАСТРОЙКИ");
    QPushButton *screensaverBtn = new QPushButton("ЗАСТАВКА");

    layout->addWidget(logo);
    layout->addWidget(new QLabel("Выберите трек:"));
    layout->addWidget(songSelector);
    layout->addWidget(playBtn);
    layout->addWidget(settingsBtn);
    layout->addWidget(screensaverBtn);

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

    stackedWidget->addWidget(mainMenu);    
    stackedWidget->addWidget(settings);    
    stackedWidget->addWidget(game);        
    stackedWidget->addWidget(screenSaver); 

    connect(mainMenu, &MainMenuWidget::openSettings, [this](){ stackedWidget->setCurrentIndex(1); });
    
    connect(mainMenu, &MainMenuWidget::startGame, [this](){ 
        stackedWidget->setCurrentIndex(2); 
        game->startGame(); 
        game->setFocus(); 
    });

    connect(mainMenu, &MainMenuWidget::openScreensaver, [this](){ 
        stackedWidget->setCurrentIndex(3); 
        screenSaver->startAnimation(); 
    });

    connect(settings, &SettingsWidget::returnToMenu, [this](){ stackedWidget->setCurrentIndex(0); });
    connect(game, &GameWidget::returnToMenu, [this](){ stackedWidget->setCurrentIndex(0); });
    
    connect(screenSaver, &RockScreenSaver::returnToMenu, [this](){ 
        screenSaver->stopAnimation(); 
        stackedWidget->setCurrentIndex(0); 
    });

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(stackedWidget);
    setLayout(mainLayout);
}
