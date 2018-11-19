#ifndef USER_H
#define USER_H
#include <QString>

class User
{
public:
    User(QString m_name, int m_state, QString m_ip, int m_port, bool m_hasConnected=false);
    QString username;
    int state;
    QString ip;
    int port;
    bool hasConnected;
    QString toString();
};

#endif // USER_H
