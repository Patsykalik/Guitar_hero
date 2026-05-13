#ifndef INTERFACE_H
#define INTERFACE_H

#include <QWidget>
#include <QTableWidget>

#include "gameplay.h"

class RockScreenSaver : public QWidget {

    Q_OBJECT

public:
    explicit RockScreenSaver(QWidget *parent = nullptr);

    void startAnimation();

    void stopAnimation();

signals:
    void returnToMenu();

private slots:
    void animate();

private:
    class QGraphicsScene *scene;

    class QGraphicsView *view;

    GuitarPlayer *player;

    Amplifier *amplifier;

    class QComboBox *selector;

    QTimer *timer;

    qreal angle = 0;

    bool wasColliding = false;
};



class SettingsWidget : public QWidget {

    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);

signals:
    void returnToMenu();

private:
    class QSpinBox *stringsSpin;

    class QRadioButton *verticalRadio;

    class QRadioButton *inclinedRadio;
};



class LeaderboardWidget : public QWidget {

    Q_OBJECT

public:
    explicit LeaderboardWidget(QWidget *parent = nullptr);

    void loadScores();

signals:
    void returnToMenu();

private:
    QTableWidget *table;
};



class MainMenuWidget : public QWidget {

    Q_OBJECT

public:
    explicit MainMenuWidget(QWidget *parent = nullptr);

signals:
    void startGame();

    void openSettings();

    void openScreensaver();

    void openLeaderboard();

private:
    class QComboBox *songSelector;
};



class MainWindow : public QWidget {

    Q_OBJECT

public:
    MainWindow();

private:
    class QStackedWidget *stackedWidget;

    MainMenuWidget *mainMenu;

    SettingsWidget *settings;

    GameWidget *game;

    RockScreenSaver *screenSaver;

    LeaderboardWidget *leaderboard;
};

#endif
