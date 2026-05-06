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
#include <QRandomGenerator>
#include <QUrl>
#include <QDir>
#include <QKeyEvent>

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
    painter->drawRoundedRect(0, 0, 100, 100, 10, 10);
    painter->setBrush(ledColor);
    painter->drawEllipse(75, 10, 15, 15);
}
void Amplifier::flash() { ledColor = Qt::green; update(); flashTimer->start(200); }
void Amplifier::resetColor() { ledColor = Qt::red; update(); }

MusicNote::MusicNote(qreal x, qreal y) {
    setPos(x, y);
    setZValue(3);
}
QRectF MusicNote::boundingRect() const { return QRectF(0, 0, 20, 30); }
void MusicNote::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setBrush(Qt::yellow);
    painter->drawEllipse(0, 15, 12, 12);
    painter->drawRect(10, 0, 3, 20);
}

GameNoteSprite::GameNoteSprite(int stringIndex, const QColor& color) : strIdx(stringIndex), noteColor(color) {
    setZValue(10);
}
QRectF GameNoteSprite::boundingRect() const { return QRectF(-25, -25, 50, 50); }
void GameNoteSprite::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(noteColor);
    painter->setPen(QPen(Qt::white, 2));
    painter->drawEllipse(-20, -20, 40, 40);
}

GameWidget::GameWidget(QWidget *parent) : QWidget(parent) {
    setFixedSize(VIEW_WIDTH, VIEW_HEIGHT);
    scene = new QGraphicsScene(0, 0, VIEW_WIDTH, VIEW_HEIGHT, this);
    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setStyleSheet("background: transparent; border: none;");
    
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
    
    scoreLabel = new QLabel("Очки: 0", this);
    scoreLabel->setGeometry(VIEW_WIDTH - 160, 20, 140, 40);
    scoreLabel->setStyleSheet("color: #00FF00; font-size: 24px; font-weight: bold; background: rgba(0,0,0,150); padding: 5px;");

    QPushButton *stopBtn = new QPushButton("Меню", this);
    stopBtn->setGeometry(10, 10, 80, 30);
    connect(stopBtn, &QPushButton::clicked, this, &GameWidget::stopGame);
}

void GameWidget::startGame() {
    scene->clear(); 
    notes.clear(); 
    score = 0;
    scoreLabel->setText("Очки: 0");

    QPixmap bgPix("3ae8bceb83287ebf54c46b101eafd361.jpg");
    if (!bgPix.isNull()) {
        scene->setBackgroundBrush(bgPix.scaled(VIEW_WIDTH, VIEW_HEIGHT, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }

    drawFretboard(); 
    this->setFocus(); 

    QString songPath = QDir::current().absoluteFilePath(globalConfig.selectedSong);
    mediaPlayer->setSource(QUrl::fromLocalFile(songPath));
    audioOutput->setVolume(1.0);

    countdownValue = 3;
    countdownLabel->show();
    QTimer *ctTimer = new QTimer(this);
    connect(ctTimer, &QTimer::timeout, [this, ctTimer]() {
        if (--countdownValue > 0) countdownLabel->setText(QString::number(countdownValue));
        else if (countdownValue == 0) countdownLabel->setText("GO!");
        else {
            countdownLabel->hide(); ctTimer->stop(); ctTimer->deleteLater();
            mediaPlayer->play(); gameTimer->start(16); spawnTimer->start(800);
        }
    });
    ctTimer->start(1000);
}

void GameWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_1) checkHit(0);
    else if (event->key() == Qt::Key_2) checkHit(1);
    else if (event->key() == Qt::Key_3) checkHit(2);
    else if (event->key() == Qt::Key_4) checkHit(3);
}

void GameWidget::checkHit(int stringIdx) {
    for (int i = 0; i < notes.size(); ++i) {
        if (notes[i]->strIdx == stringIdx && notes[i]->progress > 0.85 && notes[i]->progress < 1.1) {
            score += 10;
            scoreLabel->setText(QString("Очки: %1").arg(score));
            scene->removeItem(notes[i]);
            delete notes.takeAt(i);
            return;
        }
    }
}

void GameWidget::drawFretboard() {
    QPen pen(QColor(255, 255, 255, 80), 4);
    for (int i = 0; i < globalConfig.numStrings; ++i) {
        qreal x = VIEW_WIDTH/2.0 + (i-(globalConfig.numStrings-1)/2.0)*80.0;
        scene->addLine(x, 0, x, VIEW_HEIGHT, pen);
    }
    scene->addLine(0, VIEW_HEIGHT-60, VIEW_WIDTH, VIEW_HEIGHT-60, QPen(Qt::red, 5));
}

void GameWidget::spawnNote() {
    int s = QRandomGenerator::global()->bounded(globalConfig.numStrings);
    QList<QColor> c = {Qt::green, Qt::red, Qt::yellow, Qt::blue};
    GameNoteSprite *n = new GameNoteSprite(s, c[s % c.size()]);
    scene->addItem(n); notes.append(n);
}

void GameWidget::updatePhysics() {
    for (int i = notes.size()-1; i >= 0; --i) {
        notes[i]->progress += 0.008;
        qreal x = VIEW_WIDTH/2.0 + (notes[i]->strIdx-(globalConfig.numStrings-1)/2.0)*80.0;
        notes[i]->setPos(x, 50 + (VIEW_HEIGHT-100)*notes[i]->progress);
        if (notes[i]->progress >= 1.2) { scene->removeItem(notes[i]); delete notes.takeAt(i); }
    }
}

void GameWidget::stopGame() {
    gameTimer->stop(); spawnTimer->stop(); mediaPlayer->stop();
    emit returnToMenu();
}
