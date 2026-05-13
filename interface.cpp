#include "interface.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QStackedWidget>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtMath>
#include <QTableWidget>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QCoreApplication>


RockScreenSaver::RockScreenSaver(QWidget *parent)
    : QWidget(parent)
{
    scene = new QGraphicsScene(0, 0, VIEW_WIDTH, VIEW_HEIGHT, this);

    scene->setBackgroundBrush(Qt::black);

    view = new QGraphicsView(scene, this);

    view->setStyleSheet("background: transparent; border: none;");

    amplifier = new Amplifier();
    amplifier->setPos(375, 275);

    scene->addItem(amplifier);

    player = new GuitarPlayer();

    scene->addItem(player);

    selector = new QComboBox();

    selector->addItems({"Круг", "Синус"});

    QPushButton *btn = new QPushButton("НАЗАД");

    connect(btn,
            &QPushButton::clicked,
            this,
            &RockScreenSaver::returnToMenu);

    QVBoxLayout *l = new QVBoxLayout(this);

    l->addWidget(view);
    l->addWidget(selector);
    l->addWidget(btn);

    timer = new QTimer(this);

    connect(timer,
            &QTimer::timeout,
            this,
            &RockScreenSaver::animate);
}

void RockScreenSaver::startAnimation()
{
    timer->start(16);
}

void RockScreenSaver::stopAnimation()
{
    timer->stop();
}

void RockScreenSaver::animate()
{
    angle += 0.05;

    qreal x;
    qreal y;

    if (selector->currentIndex() == 0) {

        x = 400 + 200 * qCos(angle);
        y = 300 + 200 * qSin(angle);

    } else {

        x = 400 + 200 * qCos(angle);
        y = 300 + 100 * qSin(angle * 2);
    }

    player->setPos(x - 25, y - 25);

    if (player->collidesWithItem(amplifier)) {

        if (!wasColliding) {

            amplifier->flash();

            wasColliding = true;
        }

    } else {

        wasColliding = false;
    }
}


SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *l = new QVBoxLayout(this);

    stringsSpin = new QSpinBox();

    stringsSpin->setRange(2, 4);

    verticalRadio = new QRadioButton("Вертикально");

    inclinedRadio = new QRadioButton("Наклон");

    QFile loadFile(QCoreApplication::applicationDirPath() + "/settings.json");

    if (loadFile.open(QIODevice::ReadOnly)) {

        QByteArray data = loadFile.readAll();

        QJsonDocument doc = QJsonDocument::fromJson(data);

        QJsonObject obj = doc.object();

        globalConfig.numStrings =
            obj["numStrings"].toInt(4);

        globalConfig.inclinedFretboard =
            obj["inclinedFretboard"].toBool(true);

        loadFile.close();
    }

    stringsSpin->setValue(globalConfig.numStrings);

    if (globalConfig.inclinedFretboard)
        inclinedRadio->setChecked(true);
    else
        verticalRadio->setChecked(true);

    QPushButton *b = new QPushButton("ПРИМЕНИТЬ");

    l->addWidget(new QLabel("КОЛИЧЕСТВО СТРУН:"));
    l->addWidget(stringsSpin);
    l->addWidget(verticalRadio);
    l->addWidget(inclinedRadio);
    l->addWidget(b);

    connect(b,
            &QPushButton::clicked,
            [this]() {

        globalConfig.numStrings =
            stringsSpin->value();

        globalConfig.inclinedFretboard =
            inclinedRadio->isChecked();

        QJsonObject obj;

        obj["numStrings"] =
            globalConfig.numStrings;

        obj["inclinedFretboard"] =
            globalConfig.inclinedFretboard;

        QJsonDocument doc(obj);

        QFile saveFile(
            QCoreApplication::applicationDirPath()
            + "/settings.json"
        );

        if (saveFile.open(QIODevice::WriteOnly)) {

            saveFile.write(doc.toJson());

            saveFile.close();
        }

        emit returnToMenu();
    });
}


LeaderboardWidget::LeaderboardWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title =
        new QLabel("ТАБЛИЦА ЛИДЕРОВ");

    title->setStyleSheet(
        "font-size: 42px;"
        "font-weight: bold;"
        "color: #ffaa44;"
        "border-bottom: 3px solid #ffaa44;"
        "padding-bottom: 15px;"
    );

    title->setAlignment(Qt::AlignCenter);

    table = new QTableWidget();

    table->setColumnCount(3);

   table->setColumnCount(4);

    table->setHorizontalHeaderLabels({
    "ИМЯ",
    "ПЕСНЯ",
    "ОЧКИ",
    "ДАТА"
 });

    table->horizontalHeader()->
            setSectionResizeMode(
                QHeaderView::Stretch
            );

    QPushButton *back =
        new QPushButton("НАЗАД");

    connect(back,
            &QPushButton::clicked,
            this,
            &LeaderboardWidget::returnToMenu);

    layout->addWidget(title);
    layout->addWidget(table);
    layout->addWidget(back);
}

void LeaderboardWidget::loadScores()
{
    table->setRowCount(0);

    QSqlQuery query(
        "SELECT player_name, song_name, score, played_at "
        "FROM leaderboard "
        "ORDER BY score DESC"
    );

    int row = 0;

    while (query.next()) {

        table->insertRow(row);

        table->setItem(
            row,
            0,
            new QTableWidgetItem(
                query.value(0).toString()
            )
        );

        table->setItem(
            row,
            1,
            new QTableWidgetItem(
                query.value(1).toString()
            )
        );

        table->setItem(
            row,
            2,
            new QTableWidgetItem(
                query.value(2).toString()
            )
        );

        table->setItem(
            row,
            3,
            new QTableWidgetItem(
                query.value(3).toString()
            )
        );

        row++;
    }
}

MainMenuWidget::MainMenuWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *l = new QVBoxLayout(this);

    l->setSpacing(25);

    QLabel *logo =
        new QLabel("🎸GUITAR HERO");

    logo->setStyleSheet(
        "font-size: 52px;"
        "font-weight: 900;"
        "color: #ffaa44;"
        "letter-spacing: 3px;"
    );

    logo->setAlignment(Qt::AlignCenter);

    songSelector = new QComboBox();

    songSelector->addItems({
        "Zhit_v_kayf.mp3",
        "Freedom.mp3",
        "Don_Omar.mp3"
    });

    QPushButton *b1 =
        new QPushButton("▶ ИГРАТЬ");

    QPushButton *b2 =
        new QPushButton("⚙ НАСТРОЙКИ");

    QPushButton *b3 =
        new QPushButton("🎬 ЗАСТАВКА");

    QPushButton *b4 =
        new QPushButton("🏆 ТАБЛИЦА ЛИДЕРОВ");

    QPushButton *b5 =
        new QPushButton("✖ ВЫХОД");

    l->addWidget(logo);
    l->addWidget(songSelector);
    l->addWidget(b1);
    l->addWidget(b2);
    l->addWidget(b3);
    l->addWidget(b4);
    l->addWidget(b5);

    connect(b1,
            &QPushButton::clicked,
            [this]() {

        globalConfig.selectedSong =
            songSelector->currentText();

        emit startGame();
    });

    connect(b2,
            &QPushButton::clicked,
            this,
            &MainMenuWidget::openSettings);

    connect(b3,
            &QPushButton::clicked,
            this,
            &MainMenuWidget::openScreensaver);

    connect(b4,
            &QPushButton::clicked,
            this,
            &MainMenuWidget::openLeaderboard);

    connect(b5,
            &QPushButton::clicked,
            qApp,
            &QApplication::quit);
}


MainWindow::MainWindow()
{
    stackedWidget =
        new QStackedWidget(this);

    mainMenu =
        new MainMenuWidget();

    settings =
        new SettingsWidget();

    game =
        new GameWidget();

    screenSaver =
        new RockScreenSaver();

    leaderboard =
        new LeaderboardWidget();

    stackedWidget->addWidget(mainMenu);
    stackedWidget->addWidget(settings);
    stackedWidget->addWidget(game);
    stackedWidget->addWidget(screenSaver);
    stackedWidget->addWidget(leaderboard);

    connect(mainMenu,
            &MainMenuWidget::openSettings,
            [this]() {

        stackedWidget->setCurrentIndex(1);
    });

    connect(mainMenu,
            &MainMenuWidget::startGame,
            [this]() {

        stackedWidget->setCurrentIndex(2);

        game->startGame();

        game->setFocus();
    });

    connect(mainMenu,
            &MainMenuWidget::openScreensaver,
            [this]() {

        stackedWidget->setCurrentIndex(3);

        screenSaver->startAnimation();
    });

    connect(mainMenu,
            &MainMenuWidget::openLeaderboard,
            [this]() {

        leaderboard->loadScores();

        stackedWidget->setCurrentWidget(
            leaderboard
        );
    });

    connect(settings,
            &SettingsWidget::returnToMenu,
            [this]() {

        stackedWidget->setCurrentIndex(0);
    });

    connect(game,
            &GameWidget::returnToMenu,
            [this]() {

        stackedWidget->setCurrentIndex(0);
    });

    connect(screenSaver,
            &RockScreenSaver::returnToMenu,
            [this]() {

        screenSaver->stopAnimation();

        stackedWidget->setCurrentIndex(0);
    });

    connect(leaderboard,
            &LeaderboardWidget::returnToMenu,
            [this]() {

        stackedWidget->setCurrentIndex(0);
    });

    QVBoxLayout *l =
        new QVBoxLayout(this);

    l->setContentsMargins(0,0,0,0);

    l->addWidget(stackedWidget);

    resize(VIEW_WIDTH, VIEW_HEIGHT);

    setMinimumSize(900, 600);
}
