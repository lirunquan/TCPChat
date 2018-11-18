#include "server.h"
#include "messagequeue.h"
#include "user.h"

#include <QByteArray>
#include <QTime>
int cur;

struct user_data
{
    User* u[M];
    int size;
}*User_data;

MessageQueue* Message;
bool isFirst = false;
int mode[M];
enum{
    AcceptLogin = 0,
    Chat
};
int indexOf;

Server::Server(QObject *parent) : QObject(parent)
{
    User_data = new user_data();
    for(int i=0;i<M;i++){
        User_data->u[i] = NULL;
        mode[i] = Chat;
    }
    indexOf = 0;
    User_data->size = 0;
    cur = 0;
    Message = NULL;
    isFirst = true;
    userLoaded();
    userStateUpdate();

}
void Server::userLoaded()
{
    QFile userFile("Chat.txt");
    logOutput("load data from file.");
    if(userFile.open(QIODevice::ReadOnly|QIODevice::Text)){
        QByteArray a = QByteArray::fromBase64(userFile.readAll());
        QString str = QString(a);
        QTextStream input(&str);
        QString username = "";
        QString password = "";
        QString question = "";
        QString answer = "";
        User_data->size = 0;
        while(!input.atEnd()){
            input>>username;
            if(username != ""){
                QString s = QString("User: %1 ").arg(username);
                logOutput(s);
                input >> password;
                input >> question;
                input >> answer;
                User_data->u[User_data->size] = new User(username, password, question, answer);
                User_data->size ++;
            }
            else{
                break;
            }
        }
        userFile.close();
    }
    else{
        logOutput("Failed to open user data file");
        userFile.close();
    }
}
void Server::userStateUpdate()
{
    QString s = QString("Update user State, now the count of Users: %1").arg(User_data->size);
    logOutput(s);
    int num_on = 0;
    int num_off = 0;
    QString update = QString("load users states##%1##").arg(User_data->size);
    for(int i=0;i<User_data->size; i++){
        update = update.append(User_data->u[i]->toString());
        if(User_data->u[i]->online_state == 1){
            num_on ++;
        }
        else{
            num_off ++;
        }
    }
    s = QString("Online user: %1, Offline user: %2").arg(num_on).arg(num_off);
    logOutput(s);
    if(isFirst){
        isFirst = false;
    }
    else{
        for(int i=0; i<M; i++){
            if(tcpSocket[i]->isOpen()){
                s = QString("Send current index of user to current client: %1").arg(i);
                tcpSocket[i]->write(update.toUtf8());
            }
            else{
                break;
            }
        }
    }
}
void Server::logOutput(QString log)
{
    QTime current = QTime::currentTime();
    QString time = current.toString("yyyy.MM.dd hh:mm:ss.zzz ddd");
    QFile logFile("log.txt");
    if(logFile.open(QIODevice::WriteOnly | QIODevice::Append)){
        QTextStream output(&logFile);
        output << time << ":" << log << "\n";
    }
    logFile.close();
}
void Server::handleMessage(QString m_name, QTcpSocket *socket)
{
    MessageQueue* a = Message;
    QString sendMessage = "";
    int count = 0;
    while(a){
        if(a->reciever == m_name){
            sendMessage.append(QString("&&%1").arg(a->message));
            count++;
            a->isSended = true;
        }
        a = a->next;
    }
    if(count != 0){
        QString send2 = QString("offline message&&%1").arg(count).append(sendMessage);
        socket->write(send2.toUtf8());
    }
    logOutput("offline message sended");
    MessageQueue* head = Message;
    while (head!=NULL && head->isSended) {
        MessageQueue* pre = head;
        head = head->next;
        delete pre;
        pre = NULL;
    }
    if(head == NULL){
        Message = NULL;
    }
    else{
        Message = head;
        MessageQueue* p = Message;
        MessageQueue* q = Message->next;
        while (q != NULL) {
            if(q->isSended){
                while (q != NULL && q->isSended) {
                    MessageQueue* later = q;
                    q = q->next;
                    delete later;
                    later = NULL;
                }
                if(q == NULL){}
                else{
                    p = q;
                    q = q->next;
                }
            }
            else{
                p = q;
                q = q->next;
            }
        }
    }
}
void Server::saveToFile()
{
    QFile userFile("Chat.txt");
    if(userFile.open(QIODevice::WriteOnly)){
        QByteArray input ;
        for(int i=0; i<User_data->size; i++){
            input.append(User_data->u[i]->username.toUtf8());
            input.append(QString(" ").toUtf8());
            input.append(User_data->u[i]->password.toUtf8());
            input.append(QString(" ").toUtf8());
            input.append(User_data->u[i]->question.toUtf8());
            input.append(QString(" ").toUtf8());
            input.append(User_data->u[i]->answer.toUtf8());
            input.append(QString(" ").toUtf8());
        }
        userFile.write(input.toBase64());
        userFile.close();
        logOutput("Save to user file Success");
    }
    else{
        logOutput("Failed to open user data file");
        userFile.close();
    }
}
