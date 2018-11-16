#include "tcpserver.h"
#include "mysql.h"
#include <QUuid>

TCPServer::TCPServer(QObject* parent):QTcpServer(parent)
{
    connect(this, SIGNAL(newConnection()), this, SLOT());
}
TCPServer::~TCPServer()
{}

bool TCPServer::start(uint16_t port)
{
    return listen(QHostAddress::AnyIPv4, port);
}
void TCPServer::stop()
{
    close();
}
