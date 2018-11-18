#include "user.h"

User::User(QString myName, QString myPassword, QString myQues, QString myAnswer,  int ison,  QString myIP,  int myPort)
{
    this->username = myName;
    this->password = myPassword;
    this->question = myQues;
    this->answer = myAnswer;
    this->online_state = ison;
    this->ipAdd = myIP;
    this->port = myPort;
}

bool User::setIpPort(QString m_name, QString m_ip, int m_port)
{
    if(this->username == m_name)
    {
        this->ipAdd = m_ip;
        this->port = m_port;
        return true;
    }
    else return false;
}

bool User::setState(QString m_name)
{
    if(this->username == m_name){
        this->online_state = 1;
        return true;
    }
    else return false;
}
QString User::toString()
{
    return QString("%1 %2 %3 %4 ").arg(username).arg(online_state).arg(ipAdd).arg(port);
}