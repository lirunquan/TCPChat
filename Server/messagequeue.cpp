#include "messagequeue.h"

MessageQueue::MessageQueue(QString m_message, QString m_name, MessageQueue* m_next, bool m_sended)
{
    this->message = m_message;
    this->reciever = m_name;
    this->next = m_next;
    this->isSended = m_sended;
}
