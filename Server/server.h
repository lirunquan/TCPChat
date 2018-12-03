#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QTimer>
#include <QtNetwork>
#include <QUdpSocket>

const int M = 20;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    ~Server();
    void init();
    void userLoaded();
    void handleMessage(QString m_name, QTcpSocket* socket);
    void start(QTcpServer* tcp, int port);
    void userStateUpdate();
    void saveToFile();
    void logOutput(QString log);
    QString readString(QString str);
signals:

public slots:

private:
    QTcpServer* tcpServer;
    QTcpSocket* tcpSocket[M];
    QUdpSocket* udpSocket;
    QTimer timer;
};

#endif // SERVER_H
