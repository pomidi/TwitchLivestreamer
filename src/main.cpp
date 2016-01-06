#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    QApplication::setApplicationVersion("0.0.1");
    MainWindow w;
    w.setWindowTitle("TwitchStreamer");
    w.show();

    return a.exec();
}
