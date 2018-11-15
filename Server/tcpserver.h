#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <memory>

class TCPServer : public QTcpServer
{
    Q_OBJECT
public:
    TCPServer(QObject* parent = 0);
    virtual ~TCPServer();
    bool start(uint16_t port);
    void stop();
};

#endif // TCPSERVER_H
