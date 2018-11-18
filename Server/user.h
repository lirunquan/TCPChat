#ifndef USER_H
#define USER_H
#include <QString>

class User
{
public:
    User(QString myName,
         QString myPassword,
         QString myQues,
         QString myAnswer,
         int ison=0,
         QString myIP="ip",
         int myPort=0);
    bool setIpPort(QString m_name, QString m_ip, int m_port);
    bool setState(QString m_name);
    QString toString();
    QString username;
    QString password;
    QString question;
    QString answer;
    int online_state;//0:offline;1:online
    QString ipAdd;
    int port;

};

#endif // USER_H
