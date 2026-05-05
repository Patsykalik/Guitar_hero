#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <QGraphicsObject>
#include <QWidget>
#include <QTimer>
#include <QList>
#include <QColor>
#include <QPixmap>

// Константы и настройки (переносим сюда для удобства)
const int VIEW_WIDTH = 800;
const int VIEW_HEIGHT = 600;

struct GameConfig {
    int numStrings = 4;
    bool inclinedFretboard = false;
    QString selectedSong = "song1.mp3";
};
extern GameConfig globalConfig;

// Графические элементы
class GuitarPlayer : public QGraphicsObject {
    Q_OBJECT
public:
    GuitarPlayer();
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) override;
};

class Amplifier : public QGraphicsObject {
    Q_OBJECT
public:
    Amplifier();
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) override;
    void flash();
private slots:
    void resetColor();
private:
    QColor ledColor = Qt::red;
    QTimer *flashTimer;
};

class MusicNote : public QGraphicsObject {
    Q_OBJECT
public:
    MusicNote(qreal x, qreal y);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) override;
};

class GameNoteSprite : public QGraphicsObject {
    Q_OBJECT
public:
    GameNoteSprite(int stringIndex, const QColor& color);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) override;
    int strIdx;
    QColor noteColor;
    qreal progress = 0.0;
    QPixmap sprite;
};

// Сам игровой виджет
class GameWidget : public QWidget {
    Q_OBJECT
public:
    explicit GameWidget(QWidget *parent = nullptr);
    void startGame();
signals:
    void returnToMenu();
private slots:
    void updatePhysics();
    void spawnNote();
    void stopGame();
private:
    void drawFretboard();
    class QGraphicsScene *scene;
    class QGraphicsView *view;
    QTimer *gameTimer;
    QTimer *spawnTimer;
    class QLabel *countdownLabel;
    int countdownValue;
    class QMediaPlayer *mediaPlayer;
    class QAudioOutput *audioOutput;
    QList<GameNoteSprite*> notes;
};

#endif