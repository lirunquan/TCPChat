#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QtNetwork>
#include <memory>

class TCPServer : public QObject
{
    Q_OBJECT
public:
    TCPServer(QObject* parent = 0);
    virtual ~TCPServer();

private slots:
    bool start(uint16_t port);
    void stop();
    void acceptConnection();
private:
    QTcpServer* tcpServer;
    QTcpSocket* tcpsocket;
    QString id;

};

#endif // TCPSERVER_H
