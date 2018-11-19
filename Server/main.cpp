#include <QCoreApplication>
#include <QTcpServer>
#include <QDir>
#include "server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
#ifdef _DEBUG
    auto dir = QDir(QCoreApplication::applicationDirPath());
#else
    auto dir = QDir(QDir::currentPath());
#endif
    Server server(&a);
    server.init();
    return a.exec();
}
