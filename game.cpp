#include <QApplication>
#include "interface.h"

GameConfig globalConfig;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    MainWindow w;
    w.setWindowTitle("🎸 Guitar Hero Qt Project");
    w.show();

    return a.exec();
}