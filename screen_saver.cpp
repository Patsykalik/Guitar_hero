#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QTimer>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QtMath>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

const int VIEW_WIDTH = 800;
const int VIEW_HEIGHT = 600;

class GuitarPlayer : public QGraphicsObject {
    Q_OBJECT
public:
    GuitarPlayer() {
        setTransformOriginPoint(25, 25);
        setZValue(2);
    }

    QRectF boundingRect() const override {
        return QRectF(0, 0, 50, 50);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setRenderHint(QPainter::Antialiasing);

        QRadialGradient grad(25, 25, 25);
        grad.setColorAt(0, QColor(255, 80, 80));
        grad.setColorAt(1, QColor(120, 0, 0));

        painter->setBrush(grad);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(5, 10, 40, 40);

        painter->setBrush(Qt::darkGray);
        painter->drawRect(20, 0, 10, 15);

        painter->setBrush(Qt::white);
        painter->drawEllipse(18, 2, 4, 4);
        painter->drawEllipse(28, 2, 4, 4);

        painter->setPen(QPen(Qt::white, 1));
        painter->drawLine(22, 15, 22, 50);
        painter->drawLine(28, 15, 28, 50);
    }
};

class Amplifier : public QGraphicsObject {
    Q_OBJECT
public:
    Amplifier() {
        flashTimer = new QTimer(this);
        flashTimer->setSingleShot(true);
        connect(flashTimer, &QTimer::timeout, this, &Amplifier::resetColor);
    }

    QRectF boundingRect() const override {
        return QRectF(0, 0, 100, 100);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) override {
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

    void flash() {
        ledColor = Qt::green;
        update();
        flashTimer->start(200);
    }

private slots:
    void resetColor() {
        ledColor = Qt::red;
        update();
    }

private:
    QColor ledColor = Qt::red;
    QTimer *flashTimer;
};

class MusicNote : public QGraphicsObject {
    Q_OBJECT
public:
    MusicNote(qreal x, qreal y) {
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
        fade->setStartValue(1.0);
        fade->setEndValue(0.0);
        fade->start(QAbstractAnimation::DeleteWhenStopped);

        QTimer::singleShot(800, this, &QGraphicsObject::deleteLater);
    }

    QRectF boundingRect() const override {
        return QRectF(0, 0, 20, 30);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setBrush(Qt::yellow);
        painter->drawEllipse(0, 15, 12, 12);
        painter->drawRect(10, 0, 3, 20);
    }
};

class RockScreenSaver : public QWidget {
    Q_OBJECT
public:
    RockScreenSaver() {
        setFixedSize(VIEW_WIDTH, VIEW_HEIGHT);

        scene = new QGraphicsScene(0, 0, VIEW_WIDTH, VIEW_HEIGHT);

        QPixmap bg("concert_hall.jpg");
        if (!bg.isNull())
            scene->setBackgroundBrush(bg.scaled(VIEW_WIDTH, VIEW_HEIGHT));
        else
            scene->setBackgroundBrush(Qt::black);

        amplifier = new Amplifier();
        amplifier->setPos(350, 250);
        scene->addItem(amplifier);

        player = new GuitarPlayer();
        scene->addItem(player);

        view = new QGraphicsView(scene);
        view->setRenderHint(QPainter::Antialiasing);
        view->setFrameShape(QFrame::NoFrame);

        selector = new QComboBox();
        selector->addItems({"🎸 Круг", "🎸 Синус", "🎸 Восьмёрка"});
        selector->setStyleSheet(
            "QComboBox { background:black; color:lime; padding:6px; border-radius:6px; }"
        );

        QLabel *label = new QLabel("Выбери траекторию:");
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet(
            "color:white; background:rgba(0,0,0,180); padding:6px; border-radius:8px;"
        );

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(view);
        layout->addWidget(label);
        layout->addWidget(selector);

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &RockScreenSaver::animate);
        timer->start(16);
    }

private:
    void animate() {
        angle += 0.05;

        qreal x = 400, y = 300;

        switch (selector->currentIndex()) {
        case 0:
            x = 400 + 180 * qCos(angle);
            y = 300 + 130 * qSin(angle);
            break;

        case 1:
            x = 100 + fmod(angle * 100, 600);
            y = 300 + 120 * qSin(angle * 2);
            break;

        case 2:
            x = 400 + 200 * qSin(angle);
            y = 300 + 120 * qSin(angle) * qCos(angle);
            break;
        }

        player->setPos(x, y);
        player->setRotation(qSin(angle * 3) * 30);
        player->setScale(1.0 + 0.1 * qSin(angle * 4));

        if (player->collidesWithItem(amplifier)) {
            if (!wasColliding) {
                amplifier->flash();
                scene->addItem(new MusicNote(x + 25, y + 25));
            }
            wasColliding = true;
        } else {
            wasColliding = false;
        }

        scene->setBackgroundBrush(QColor::fromHsl(
            (int)(angle * 40) % 360, 150, 40
        ));
    }

    QGraphicsScene *scene;
    QGraphicsView *view;
    GuitarPlayer *player;
    Amplifier *amplifier;
    QComboBox *selector;
    QTimer *timer;

    qreal angle = 0;
    bool wasColliding = false;
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    RockScreenSaver w;
    w.setWindowTitle("🎸 Guitar Hero Screen Saver");
    w.show();

    return a.exec();
}

#include "main.moc"