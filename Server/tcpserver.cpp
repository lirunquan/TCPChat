#include "tcpserver.h"
#include "mysql.h"
#include <QUuid>


TCPServer::TCPServer(QObject* parent):QTcpServer(parent)
{
    tcpServer = new QTcpServer(this);
    tcpsocket = new QTcpSocket(this);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    connect(tcpsocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
}
TCPServer::~TCPServer()
{}

bool TCPServer::start(uint16_t port)
{
    return tcpServer->listen(QHostAddress::AnyIPv4, port);
}
void TCPServer::stop()
{
    tcpServer->close();
}
void TCPServer::onError(QAbstractSocket::SocketError)
{
    qDebug()<<tcpsocket->errorString();
    stop();
}
void TCPServer::acceptConnection()
{

}
