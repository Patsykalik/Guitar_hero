#include "gameplay.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QPainter>
#include <QPushButton>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QApplication>
#include <QFileInfo>
#include <QRadialGradient>
#include <QUrl>
#include <QGraphicsTextItem>
#include <QTimer>
#include <QGraphicsDropShadowEffect>

GuitarPlayer::GuitarPlayer()
{
    setZValue(2);
}

QRectF GuitarPlayer::boundingRect() const
{
    return QRectF(0,0,50,50);
}

void GuitarPlayer::paint(QPainter *painter,
                         const QStyleOptionGraphicsItem *,
                         QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(QColor(255,100,50));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(5,5,40,40);
}

Amplifier::Amplifier()
{
    flashTimer = new QTimer(this);
    flashTimer->setSingleShot(true);

    connect(flashTimer,
            &QTimer::timeout,
            this,
            &Amplifier::resetColor);
}

QRectF Amplifier::boundingRect() const
{
    return QRectF(0,0,50,50);
}

void Amplifier::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *,
                      QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(ledColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(0,0,50,50,15,15);
}

void Amplifier::flash()
{
    ledColor = Qt::yellow;
    update();
    flashTimer->start(120);
}

void Amplifier::resetColor()
{
    ledColor = Qt::red;
    update();
}

GameNoteSprite::GameNoteSprite(int stringIndex,
                               const QColor &color)
    : strIdx(stringIndex),
      noteColor(color)
{
    setZValue(10);
}

QRectF GameNoteSprite::boundingRect() const
{
    return QRectF(-40,-40,80,80);
}

void GameNoteSprite::paint(QPainter *painter,
                           const QStyleOptionGraphicsItem *,
                           QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);

    QColor glow = noteColor;
    glow.setAlpha(100);

    painter->setBrush(glow);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(-32,-32,64,64);

    QRadialGradient grad(0,0,28);

    grad.setColorAt(0.0, Qt::white);
    grad.setColorAt(0.3, noteColor.lighter(180));
    grad.setColorAt(0.8, noteColor);
    grad.setColorAt(1.0, noteColor.darker(250));

    painter->setBrush(grad);
    painter->setPen(QPen(Qt::white, 3));
    painter->drawEllipse(-24,-24,48,48);
}

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
{
    resize(1280,720);

    setMinimumSize(1000,600);

    setFocusPolicy(Qt::StrongFocus);

    scene = new QGraphicsScene(0, 0, width(), height(), this);

    view = new QGraphicsView(scene, this);

    view->setGeometry(0,0,width(),height());

    view->setRenderHints(
        QPainter::Antialiasing |
        QPainter::SmoothPixmapTransform
    );

    view->setStyleSheet(
        "background: black; border: none;"
    );

    audioOutput = new QAudioOutput(this);

    audioOutput->setVolume(1.0);

    mediaPlayer = new QMediaPlayer(this);

    mediaPlayer->setAudioOutput(audioOutput);

    gameTimer = new QTimer(this);

    connect(gameTimer,
            &QTimer::timeout,
            this,
            &GameWidget::updatePhysics);

    scoreLabel = new QLabel(this);

    scoreLabel->setGeometry(20,20,500,65);

    scoreLabel->setStyleSheet(
        "color: #ffcc55;"
        "font-size: 28px;"
        "font-weight: bold;"
        "background: rgba(0,0,0,180);"
        "border-radius: 35px;"
        "border: 2px solid #ffaa44;"
        "padding: 0 25px;"
    );

    gameOverLabel = new QLabel(this);

    gameOverLabel->hide();

    gameOverLabel->setAlignment(Qt::AlignCenter);

    gameOverLabel->setStyleSheet(
        "background: rgba(0,0,0,220);"
        "color: #ffaa44;"
        "font-size: 48px;"
        "font-weight: bold;"
        "border: 3px solid #ffaa44;"
        "border-radius: 50px;"
    );

    pauseLabel = new QLabel("ПАУЗА", this);

    pauseLabel->hide();

    pauseLabel->setAlignment(Qt::AlignCenter);

    pauseLabel->setStyleSheet(
        "background: rgba(0,0,0,220);"
        "color: #44ccff;"
        "font-size: 42px;"
        "font-weight: bold;"
        "border: 3px solid #44ccff;"
        "border-radius: 40px;"
    );

    nameInput = new QLineEdit(this);

    nameInput->hide();

    nameInput->setPlaceholderText("Введите имя и нажмите ENTER");

    nameInput->setStyleSheet(
        "font-size: 24px;"
        "padding: 10px;"
        "border-radius: 20px;"
        "background: rgba(0,0,0,220);"
        "color: white;"
        "border: 2px solid #ffaa44;"
    );

    connect(nameInput,
            &QLineEdit::returnPressed,
            [this]()
    {
        saveScoreToDatabase();

        nameInput->clear();

        nameInput->hide();

        emit returnToMenu();
    });

    QPushButton *btn = new QPushButton("МЕНЮ", this);

    btn->setGeometry(20,100,150,50);

    connect(btn,
            &QPushButton::clicked,
            this,
            &GameWidget::stopGame);
}

void GameWidget::generateBeatMap()
{
    beatMap.clear();

    if (globalConfig.selectedSong == "Zhit_v_kayf.mp3") {

        int beat = 900;
        int current = 5000;

        QList<int> pattern = {
            0,1,2,3,1,2,0,3,
            
            0,2,1,3,2,1,0,3
        };

        int idx=0;

        while(current<180000) {

            BeatNote n;

            n.time=current;

            n.stringIndex=
                pattern[idx%pattern.size()];

            beatMap.append(n);

            current+=beat;

            idx++;
        }

    } else if (globalConfig.selectedSong == "Freedom.mp3") {

        int beat=450;
        int current=2500;

        QList<int> pattern = {
            0,2,1,3,0,1,2,3,
            1,2,0,3,2,0,1,3
        };

        int idx=0;

        while(current<180000) {

            BeatNote n;

            n.time=current;

            n.stringIndex=
                pattern[idx%pattern.size()];

            beatMap.append(n);

            current+=beat;

            idx++;
        }

    } else if (globalConfig.selectedSong == "Don_Omar.mp3") {

        int beat=300;
        int current=1800;

        QList<int> pattern = {
            0,1,0,2,1,3,1,2,
            0,2,3,1,2,0,1,3
        };

        int idx=0;

        while(current<180000) {

            BeatNote n;

            n.time=current;

            n.stringIndex=
                pattern[idx%pattern.size()];

            beatMap.append(n);

            current+=beat;

            idx++;
        }
    }
}

void GameWidget::startGame()
{
    scene->clear();

    notes.clear();

    nextBeatIndex=0;

    fakeTime=-2000;

    score=0;

    combo=0;

    lives=10;

    paused=false;

    isGameOver=false;

    gameOverLabel->hide();

    pauseLabel->hide();

    nameInput->hide();

    nameInput->clear();

    updateHUD();

    drawFretboard();

    generateBeatMap();

    QString songPath =
        qApp->applicationDirPath()
        + "/"
        + globalConfig.selectedSong;

    if(QFileInfo::exists(songPath)) {

        mediaPlayer->stop();

        mediaPlayer->setSource(
            QUrl::fromLocalFile(songPath)
        );

        mediaPlayer->play();
    }

    deltaTimer.restart();

    gameTimer->start(16);

    activateWindow();

    setFocus(Qt::ActiveWindowFocusReason);
}

void GameWidget::drawFretboard()
{
    scene->clear();

    QString bgPath =
        qApp->applicationDirPath()
        + "/3ae8bceb83287ebf54c46b101eafd361.jpg";

    QPixmap bg(bgPath);

    if(!bg.isNull()) {

        bg = bg.scaled(width(),
                       height(),
                       Qt::IgnoreAspectRatio,
                       Qt::SmoothTransformation);

        scene->setBackgroundBrush(bg);
    }

    scene->addRect(0,0,width(),height(),
                   Qt::NoPen,
                   QColor(0,0,0,140));

    qreal topWidth = width()*0.25;

    qreal bottomWidth = width()*0.78;

    qreal centerX = width()/2;

    qreal topLeft = centerX - topWidth/2;

    qreal bottomLeft = centerX - bottomWidth/2;

    for(int i=0; i<globalConfig.numStrings; ++i) {

        qreal t =
            (i+1.0) /
            (globalConfig.numStrings+1.0);

        qreal xTop =
            topLeft + t*topWidth;

        qreal xBottom =
            bottomLeft + t*bottomWidth;

        scene->addLine(xTop,
                       0,
                       xBottom,
                       height(),
                       QPen(
                           QColor(255,170,68,200),
                           5
                       ));
    }

    scene->addLine(bottomLeft,
                   height()-100,
                   width()-bottomLeft,
                   height()-100,
                   QPen(QColor(255,100,50), 12));
}

void GameWidget::updatePhysics()
{
    if(isGameOver || paused)
        return;

    qreal dt =
        deltaTimer.restart()/1000.0;

    fakeTime += 16;

    if(nextBeatIndex < beatMap.size()) {

        BeatNote beat =
            beatMap[nextBeatIndex];

        if(fakeTime >= beat.time) {

            int s = beat.stringIndex;

            QList<QColor> colors = {
                QColor("#00ffaa"),
                QColor("#ffaa44"),
                QColor("#ff66cc"),
                QColor("#44ccff")
            };

            GameNoteSprite *note =
                new GameNoteSprite(
                    s,
                    colors[s%colors.size()]
                );

            qreal topWidth = width()*0.25;

            qreal centerX = width()/2;

            qreal topLeft =
                centerX - topWidth/2;

            qreal t =
                (s+1.0) /
                (globalConfig.numStrings+1.0);

            qreal x =
                topLeft + t*topWidth;

            note->setPos(x,-80);

            note->setScale(0.4);

            scene->addItem(note);

            notes.append(note);

            nextBeatIndex++;
        }
    }

    for(int i=notes.size()-1; i>=0; --i) {

        GameNoteSprite *note =
            notes[i];

        note->moveBy(0,
                     note->speed * dt);

        qreal progress =
            note->y()/height();

        note->setScale(
            0.4 + progress*1.8
        );

        qreal topWidth = width()*0.25;

        qreal bottomWidth = width()*0.78;

        qreal centerX = width()/2;

        qreal topLeft =
            centerX - topWidth/2;

        qreal bottomLeft =
            centerX - bottomWidth/2;

        qreal t =
            (note->strIdx+1.0) /
            (globalConfig.numStrings+1.0);

        qreal xTop =
            topLeft + t*topWidth;

        qreal xBottom =
            bottomLeft + t*bottomWidth;

        qreal interp =
            note->y()/height();

        note->setX(
            xTop + (xBottom-xTop)*interp
        );

        if(note->y() > height()-40) {

            combo=0;

            lives--;

            updateHUD();

            showMissEffect(note->pos());

            scene->removeItem(note);

            delete notes.takeAt(i);

            if(lives<=0) {

                gameOver();

                return;
            }
        }
    }
}

void GameWidget::checkHit(int stringIdx)
{
    if(isGameOver || paused)
        return;

    const qreal HIT_LINE =
        height()-100;

    GameNoteSprite* bestNote=nullptr;

    qreal bestDistance=999999;

    for(GameNoteSprite* note: notes) {

        if(note->strIdx!=stringIdx)
            continue;

        qreal dist =
            qAbs(note->y() - HIT_LINE);

        if(dist<bestDistance) {

            bestDistance=dist;

            bestNote=note;
        }
    }

    if(!bestNote) {

        combo=0;

        lives--;

        updateHUD();

        showMissEffect(getHitLinePos(stringIdx));

        if(lives<=0)
            gameOver();

        return;
    }

    if(bestDistance<=40) {

        score+=300;

        combo++;

        showHitEffect(
            bestNote->pos(),
            "PERFECT",
            QColor(255,215,0)
        );

    } else if(bestDistance<=80) {

        score+=100;

        combo++;

        showHitEffect(
            bestNote->pos(),
            "GOOD",
            QColor(100,255,100)
        );

    } else {

        combo=0;

        lives--;

        updateHUD();

        showMissEffect(bestNote->pos());

        if(lives<=0)
            gameOver();

        return;
    }

    updateHUD();

    scene->removeItem(bestNote);

    notes.removeOne(bestNote);

    delete bestNote;
}

void GameWidget::showHitEffect(const QPointF &pos,
                               const QString &text,
                               const QColor &color)
{
    QGraphicsTextItem *item =
        scene->addText(text);

    QFont f;

    f.setPointSize(28);

    f.setBold(true);

    item->setFont(f);

    item->setDefaultTextColor(color);

    item->setPos(pos.x()-50,
                 pos.y()-70);

    item->setZValue(100);

    QGraphicsDropShadowEffect *shadow =
        new QGraphicsDropShadowEffect;

    shadow->setBlurRadius(25);

    shadow->setOffset(0);

    shadow->setColor(color);

    item->setGraphicsEffect(shadow);

    QTimer *timer = new QTimer(this);

    connect(timer,
            &QTimer::timeout,
            [item, timer]()
    {
        item->moveBy(0,-4);

        item->setOpacity(
            item->opacity()-0.06
        );

        if(item->opacity() <= 0) {

            delete item;

            timer->stop();

            delete timer;
        }
    });

    timer->start(16);
}

void GameWidget::showMissEffect(const QPointF &pos)
{
    QGraphicsTextItem *item =
        scene->addText("MISS");

    QFont f;

    f.setPointSize(28);

    f.setBold(true);

    item->setFont(f);

    item->setDefaultTextColor(Qt::red);

    item->setPos(pos.x()-40,
                 pos.y()-60);

    item->setZValue(100);

    QGraphicsDropShadowEffect *shadow =
        new QGraphicsDropShadowEffect;

    shadow->setBlurRadius(20);

    shadow->setOffset(0);

    shadow->setColor(Qt::red);

    item->setGraphicsEffect(shadow);

    QTimer *timer = new QTimer(this);

    connect(timer,
            &QTimer::timeout,
            [item, timer]()
    {
        item->moveBy(0,-3);

        item->setOpacity(
            item->opacity()-0.05
        );

        if(item->opacity() <= 0) {

            delete item;

            timer->stop();

            delete timer;
        }
    });

    timer->start(16);
}

QPointF GameWidget::getHitLinePos(int stringIdx) const
{
    qreal topWidth=width()*0.25;

    qreal bottomWidth=width()*0.78;

    qreal centerX=width()/2;

    qreal topLeft=centerX-topWidth/2;

    qreal bottomLeft=centerX-bottomWidth/2;

    qreal t =
        (stringIdx+1.0) /
        (globalConfig.numStrings+1.0);

    qreal xTop =
        topLeft + t*topWidth;

    qreal xBottom =
        bottomLeft + t*bottomWidth;

    qreal hitY = height()-100;

    qreal interp = hitY/height();

    return QPointF(
        xTop + (xBottom-xTop)*interp,
        hitY
    );
}

void GameWidget::updateHUD()
{
    scoreLabel->setText(
        QString("⚡ %1  🔥 x%2  ❤️ %3/10")
        .arg(score)
        .arg(combo)
        .arg(lives)
    );
}

void GameWidget::togglePause()
{
    if(isGameOver)
        return;

    paused = !paused;

    if(paused) {

        gameTimer->stop();

        mediaPlayer->pause();

        pauseLabel->show();

    } else {

        deltaTimer.restart();

        gameTimer->start(16);

        mediaPlayer->play();

        pauseLabel->hide();
    }
}

void GameWidget::gameOver()
{
    isGameOver=true;

    gameTimer->stop();

    mediaPlayer->stop();

    gameOverLabel->setGeometry(
        width()*0.2,
        height()*0.28,
        width()*0.6,
        170
    );

    gameOverLabel->setText(
        QString("GAME OVER\nSCORE: %1")
        .arg(score)
    );

    gameOverLabel->show();

    nameInput->setGeometry(
        width()*0.35,
        height()*0.58,
        width()*0.3,
        60
    );

    nameInput->show();

    nameInput->setFocus();
}

void GameWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape) {

        togglePause();

        return;
    }

    switch(event->key()) {

    case Qt::Key_1:
        checkHit(0);
        break;

    case Qt::Key_2:
        checkHit(1);
        break;

    case Qt::Key_3:
        checkHit(2);
        break;

    case Qt::Key_4:
        checkHit(3);
        break;

    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void GameWidget::stopGame()
{
    gameTimer->stop();

    mediaPlayer->stop();

    emit returnToMenu();
}

void GameWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    view->setGeometry(0,0,width(),height());

    scene->setSceneRect(0,0,width(),height());
}

void GameWidget::saveScoreToDatabase()
{
    QSqlDatabase db =
        QSqlDatabase::database();

    if(!db.isOpen())
        return;

    QString playerName =
        nameInput->text().trimmed();

    if(playerName.isEmpty())
        playerName = "Player";

    QSqlQuery query;

    query.prepare(
        "INSERT INTO leaderboard("
        "player_name,"
        "song_name,"
        "score"
        ") "
        "VALUES("
        ":player,"
        ":song,"
        ":score)"
    );

    query.bindValue(
        ":player",
        playerName
    );

    query.bindValue(
        ":song",
        globalConfig.selectedSong
    );

    query.bindValue(
        ":score",
        score
    );

    query.exec();
}
