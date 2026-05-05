#include "gameplay.h"
#include <QPainter>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QRandomGenerator>
#include <QUrl>

GuitarPlayer::GuitarPlayer() {
    setTransformOriginPoint(25, 25);
    setZValue(2);
}
QRectF GuitarPlayer::boundingRect() const { return QRectF(0, 0, 50, 50); }
void GuitarPlayer::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);
    QRadialGradient grad(25, 25, 25);
    grad.setColorAt(0, QColor(255, 80, 80));
    grad.setColorAt(1, QColor(120, 0, 0));
    painter->setBrush(grad);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(5, 10, 40, 40);
    painter->setBrush(Qt::darkGray);
    painter->drawRect(20, 0, 10, 15);
}

Amplifier::Amplifier() {
    flashTimer = new QTimer(this);
    flashTimer->setSingleShot(true);
    connect(flashTimer, &QTimer::timeout, this, &Amplifier::resetColor);
}
QRectF Amplifier::boundingRect() const { return QRectF(0, 0, 100, 100); }
void Amplifier::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);
    QLinearGradient grad(0, 0, 0, 100);
    grad.setColorAt(0, Qt::darkGray);
    grad.setColorAt(1, Qt::black);
    painter->setBrush(grad);
    painter->setPen(QPen(Qt::white, 2));
    painter->drawRoundedRect(0, 0, 100, 100, 10, 10);
    painter->setBrush(Qt::black);
    painter->drawEllipse(20, 20, 60, 60);
    painter->setBrush(ledColor);
    painter->drawEllipse(75, 10, 15, 15);
}
void Amplifier::flash() { ledColor = Qt::green; update(); flashTimer->start(200); }
void Amplifier::resetColor() { ledColor = Qt::red; update(); }

MusicNote::MusicNote(qreal x, qreal y) {
    setPos(x, y);
    setZValue(3);
    QPropertyAnimation *move = new QPropertyAnimation(this, "y");
    move->setDuration(800);
    move->setStartValue(y);
    move->setEndValue(y - 80);
    move->start(QAbstractAnimation::DeleteWhenStopped);
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect;
    setGraphicsEffect(effect);
    QPropertyAnimation *fade = new QPropertyAnimation(effect, "opacity");
    fade->setDuration(800);
    fade->setStartValue(1.0); fade->setEndValue(0.0);
    fade->start(QAbstractAnimation::DeleteWhenStopped);
    QTimer::singleShot(800, this, &QGraphicsObject::deleteLater);
}
QRectF MusicNote::boundingRect() const { return QRectF(0, 0, 20, 30); }
void MusicNote::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setBrush(Qt::yellow);
    painter->drawEllipse(0, 15, 12, 12);
    painter->drawRect(10, 0, 3, 20);
}

GameNoteSprite::GameNoteSprite(int stringIndex, const QColor& color) : strIdx(stringIndex), noteColor(color) {
    QPixmap atlas("atlas.png");
    if (!atlas.isNull()) sprite = atlas.copy(strIdx * 50, 0, 50, 50);
    setZValue(10);
}
QRectF GameNoteSprite::boundingRect() const { return QRectF(-25, -25, 50, 50); }
void GameNoteSprite::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) {
    if (!sprite.isNull()) painter->drawPixmap(-25, -25, sprite);
    else {
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(noteColor);
        painter->setPen(QPen(Qt::white, 2));
        painter->drawEllipse(-20, -20, 40, 40);
    }
}

GameWidget::GameWidget(QWidget *parent) : QWidget(parent) {
    setFixedSize(VIEW_WIDTH, VIEW_HEIGHT);
    scene = new QGraphicsScene(0, 0, VIEW_WIDTH, VIEW_HEIGHT, this);
    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("background: #111;");
    audioOutput = new QAudioOutput(this);
    mediaPlayer = new QMediaPlayer(this);
    mediaPlayer->setAudioOutput(audioOutput);
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &GameWidget::updatePhysics);
    spawnTimer = new QTimer(this);
    connect(spawnTimer, &QTimer::timeout, this, &GameWidget::spawnNote);
    countdownLabel = new QLabel(this);
    countdownLabel->setGeometry(VIEW_WIDTH/2-50, VIEW_HEIGHT/2-50, 100, 100);
    countdownLabel->setAlignment(Qt::AlignCenter);
    countdownLabel->setStyleSheet("color: white; font-size: 72px; font-weight: bold;");
    countdownLabel->hide();
    QPushButton *stopBtn = new QPushButton("Завершить", this);
    stopBtn->setGeometry(10, 10, 100, 30);
    connect(stopBtn, &QPushButton::clicked, this, &GameWidget::stopGame);
}

void GameWidget::startGame() {
    scene->clear(); notes.clear(); drawFretboard();
    mediaPlayer->setSource(QUrl::fromLocalFile(globalConfig.selectedSong));
    countdownValue = 3; countdownLabel->setText(QString::number(countdownValue));
    countdownLabel->show();
    QTimer *ctTimer = new QTimer(this);
    connect(ctTimer, &QTimer::timeout, [this, ctTimer]() {
        countdownValue--;
        if (countdownValue > 0) countdownLabel->setText(QString::number(countdownValue));
        else if (countdownValue == 0) countdownLabel->setText("GO!");
        else {
            countdownLabel->hide(); ctTimer->stop(); ctTimer->deleteLater();
            mediaPlayer->play(); gameTimer->start(16); spawnTimer->start(800);
        }
    });
    ctTimer->start(1000);
}

void GameWidget::drawFretboard() {
    QPen pen(Qt::lightGray, 4);
    int n = globalConfig.numStrings;
    for (int i = 0; i < n; ++i) {
        qreal topX, bottomX;
        if (globalConfig.inclinedFretboard) {
            topX = VIEW_WIDTH/2.0 + (i-(n-1)/2.0)*30.0;
            bottomX = VIEW_WIDTH/2.0 + (i-(n-1)/2.0)*100.0;
        } else {
            topX = VIEW_WIDTH/2.0 + (i-(n-1)/2.0)*80.0;
            bottomX = topX;
        }
        scene->addLine(topX, 50, bottomX, VIEW_HEIGHT-50, pen);
    }
    scene->addLine(0, VIEW_HEIGHT-60, VIEW_WIDTH, VIEW_HEIGHT-60, QPen(Qt::red, 6));
}

void GameWidget::spawnNote() {
    int stringIdx = QRandomGenerator::global()->bounded(globalConfig.numStrings);
    QList<QColor> colors = {Qt::green, Qt::red, Qt::yellow, Qt::blue, Qt::magenta};
    GameNoteSprite *note = new GameNoteSprite(stringIdx, colors[stringIdx % colors.size()]);
    scene->addItem(note); notes.append(note);
}

void GameWidget::updatePhysics() {
    int n = globalConfig.numStrings;
    for (int i = notes.size()-1; i >= 0; --i) {
        GameNoteSprite *note = notes[i];
        note->progress += 0.01;
        qreal topX, bottomX;
        if (globalConfig.inclinedFretboard) {
            topX = VIEW_WIDTH/2.0 + (note->strIdx-(n-1)/2.0)*30.0;
            bottomX = VIEW_WIDTH/2.0 + (note->strIdx-(n-1)/2.0)*100.0;
            note->setScale(0.5 + note->progress*0.7);
        } else {
            topX = VIEW_WIDTH/2.0 + (note->strIdx-(n-1)/2.0)*80.0;
            bottomX = topX;
        }
        note->setPos(topX + (bottomX-topX)*note->progress, 50 + (VIEW_HEIGHT-100)*note->progress);
        if (note->progress >= 1.1) { scene->removeItem(note); notes.removeAt(i); delete note; }
    }
}

void GameWidget::stopGame() {
    gameTimer->stop(); spawnTimer->stop(); mediaPlayer->stop();
    emit returnToMenu();
}