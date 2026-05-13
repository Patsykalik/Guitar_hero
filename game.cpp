#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QFontDatabase>

#include "interface.h"

GameConfig globalConfig;

int main(int argc, char *argv[]) {

    QApplication a(argc, argv);

    QFontDatabase::addApplicationFont(":/fonts/Roboto-Medium.ttf");
    a.setFont(QFont("Roboto", 10));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QCoreApplication::applicationDirPath() + "/leaderboard.db";
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        qDebug() << "SQLITE ERROR:" << db.lastError().text();
    } else {
        qDebug() << "SQLITE CONNECTED";
        QSqlQuery query;
       query.exec("CREATE TABLE IF NOT EXISTS leaderboard ("
           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "player_name TEXT,"
           "song_name TEXT,"
           "score INTEGER,"
           "played_at DATETIME DEFAULT CURRENT_TIMESTAMP"
           ")");
    }

    a.setStyleSheet(R"(
        * {
            outline: none;
        }
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                        stop:0 #0a0f1a, stop:1 #0f1522);
            color: #eef5ff;
            font-family: 'Segoe UI', 'Roboto', 'Helvetica Neue', sans-serif;
        }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                        stop:0 #2a2f3f, stop:1 #1a1f2f);
            color: #ffaa44;
            border: 1px solid rgba(255,170,68,0.3);
            border-radius: 32px;
            padding: 12px 24px;
            font-size: 16px;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                        stop:0 #3a3f55, stop:1 #2a2f45);
            border: 1px solid #ffaa44;
            color: #ffcc66;
        }
        QPushButton:pressed {
            background: #1a1f2f;
        }
        QComboBox, QSpinBox {
            background: rgba(30,35,50,220);
            border: 1px solid #ffaa44;
            border-radius: 24px;
            padding: 10px 18px;
            font-size: 15px;
            color: #ffdd99;
            selection-background-color: #ffaa44;
        }
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #ffaa44;
            margin-right: 12px;
        }
        QComboBox QAbstractItemView {
            background-color: #1e2335;
            selection-background-color: #ffaa44;
            border-radius: 18px;
            padding: 6px;
        }
        QLabel {
            color: #eef5ff;
        }
        QTableWidget {
            background: rgba(20,25,40,200);
            alternate-background-color: rgba(35,40,60,200);
            gridline-color: #ffaa44;
            border: 1px solid rgba(255,170,68,0.5);
            border-radius: 28px;
            padding: 8px;
        }
        QTableWidget::item {
            padding: 12px;
        }
        QHeaderView::section {
            background: #1e2335;
            color: #ffaa44;
            font-weight: bold;
            border: none;
            padding: 12px;
            font-size: 15px;
        }
        QScrollBar:vertical {
            background: #1a1f2f;
            width: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background: #ffaa44;
            border-radius: 6px;
            min-height: 40px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");

    MainWindow w;
    w.showMaximized();
    w.setWindowTitle("Guitar Hero");
    w.show();

    return a.exec();
}
