#include "messagequeue.h"

MessageQueue::MessageQueue(QString m_message, QString m_name, MessageQueue* m_next, bool m_sended)
{
    this->message = preString(m_message);
    this->reciever = m_name;
    this->next = m_next;
    this->isSended = m_sended;
}
QString MessageQueue::preString(QString str)
{
    QString string = str;
    string.replace(QString("#"), QString("/#"));
    string.replace(QString("&"), QString("/&"));
    return string;
}
