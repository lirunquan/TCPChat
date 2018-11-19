#include "user.h"

User::User(QString m_name, int m_state, QString m_ip, int m_port, bool m_hasConnected)
{
    this->username = m_name;
    this->state = m_state;
    this->ip = m_ip;
    this->port = m_port;
    this->hasConnected = m_hasConnected;
}
QString User::toString()
{
    return QString("%1 %2 %3 %4 ").arg(username).arg(state).arg(ip).arg(port);
}
