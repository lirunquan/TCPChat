#include "mainwindow.h"
#include "filetransmit.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
//    FileTransmit f;
//    f.show();

    return a.exec();
}
