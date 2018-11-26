#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <QString>

class MessageQueue
{
public:
    MessageQueue(QString m_message, QString m_name, MessageQueue* m_next=NULL, bool m_sended=false);
    QString preString(QString str);
    QString message;
    QString reciever;
    MessageQueue* next;
    bool isSended;
};

#endif // MESSAGEQUEUE_H
