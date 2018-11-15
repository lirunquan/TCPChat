#include "tcpserver.h"
#include "mysql.h"

TCPServer::TCPServer()
{

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
