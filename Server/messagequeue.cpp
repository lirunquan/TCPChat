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
    for(int i=0;i<str.length();i++){
        if(str.at(i) == '#'){
            string.insert(i,"/");
        }
        if(str.at(i) == '&'){
            string.insert(i,"/");
        }
    }
    return string;
}
