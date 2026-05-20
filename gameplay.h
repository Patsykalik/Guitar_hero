#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QResizeEvent>
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWidget>
#include <QTimer>
#include <QList>
#include <QColor>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QElapsedTimer>
#include <QLabel>
#include <QLineEdit>

const int VIEW_WIDTH = 1280;
const int VIEW_HEIGHT = 720;

struct GameConfig {

    int numStrings = 4;

    bool inclinedFretboard = true;

    QString selectedSong = "Zhit_v_kayf.mp3";
};

extern GameConfig globalConfig;

struct BeatNote {

    qint64 time;

    int stringIndex;
};

class GuitarPlayer : public QGraphicsObject {

    Q_OBJECT

public:
    GuitarPlayer();

    QRectF boundingRect() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override;
};

class Amplifier : public QGraphicsObject {

    Q_OBJECT

public:
    Amplifier();

    QRectF boundingRect() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override;

    void flash();

private slots:
    void resetColor();

private:
    QColor ledColor = Qt::red;

    QTimer *flashTimer;
};

class GameNoteSprite : public QGraphicsObject {

    Q_OBJECT

public:
    GameNoteSprite(int stringIndex,
                   const QColor &color);

    QRectF boundingRect() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override;

    int strIdx;

    QColor noteColor;

    qreal speed = 600.0;
};

class GameWidget : public QWidget {

    Q_OBJECT

    void saveScoreToDatabase();

public:
    explicit GameWidget(QWidget *parent = nullptr);

    void startGame();

signals:
    void returnToMenu();

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updatePhysics();

    void stopGame();

private:
    void drawFretboard();

    void checkHit(int stringIdx);

    void updateHUD();

    void gameOver();

    void generateBeatMap();

    void showHitEffect(const QPointF &pos,
                       const QString &text,
                       const QColor &color);

    void showMissEffect(const QPointF &pos);

    QPointF getHitLinePos(int stringIdx) const;

    void togglePause();

    QGraphicsScene *scene;

    QGraphicsView *view;

    QTimer *gameTimer;

    QAudioOutput *audioOutput;

    QMediaPlayer *mediaPlayer;

    QLabel *scoreLabel;

    QLabel *gameOverLabel;

    QLabel *pauseLabel;

    QLineEdit *nameInput;

    QList<GameNoteSprite*> notes;

    QList<BeatNote> beatMap;

    int nextBeatIndex = 0;

    int score = 0;

    int combo = 0;

    int lives = 10;

    bool isGameOver = false;

    bool paused = false;

    QElapsedTimer deltaTimer;

    qint64 fakeTime = 0;
};

#endif
