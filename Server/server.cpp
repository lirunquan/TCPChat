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
    QTime t = QTime::currentTime();
    qsrand(t.msec() + t.second()*1000);
    QString localHostName = QHostInfo::localHostName();
    logOutput(QString("local host name: %1").arg(localHostName));
    QHostInfo info = QHostInfo::fromName(localHostName);
    foreach (QHostAddress address, info.addresses()) {
        if(address.protocol() == QAbstractSocket::IPv4Protocol){
            logOutput(address.toString());
        }
    }
    for(int i=0; i<M; i++){
        tcpSocket[i] = new QTcpSocket(this);
    }
    tcpServer = new QTcpServer(this);
    this->start(tcpServer, 8800);
    connect(tcpServer, &QTcpServer::newConnection, [=](){
        int index = cur;
        tcpSocket[index] = tcpServer->nextPendingConnection();
        cur ++;
        QString ip = tcpSocket[index]->peerAddress().toString().section(":", 3,3);
        quint16 port = tcpSocket[index]->peerPort();
        logOutput(QString("[%1-%2] is connected successfully.").arg(ip).arg(port));
        connect(tcpSocket[index], &QTcpSocket::connected, [=](){

        });
        connect(tcpSocket[index], &QTcpSocket::disconnected, [=](){
            logOutput(QString("[%1-%2] is disconnected.").arg(ip).arg(port));
            for(int i=0;i<User_data->size;i++){
                if(User_data->u[i]->ipAdd == ip){
                    logOutput(QString("%1 is disconnected.").arg(User_data->u[i]->username));
                    User_data->u[i]->online_state = 0;
                    break;
                }
            }
            if(index < cur-1){
                for(int i=index; i<cur-1; i++){
                    tcpSocket[i] = tcpSocket[i+1];
                }
            }
            cur --;
            userStateUpdate();
        });
        connect(tcpSocket[index], &QTcpSocket::readyRead, [=](){
            QByteArray buffer = tcpSocket[index]->readAll();
            if(mode[index] == AcceptLogin){
                if("login" == QString(buffer).section("##", 0 ,0)){
                    logOutput("login");
                    bool isPass = false;
                    for(int i=0; i<User_data->size; i++){
                        if(User_data->u[i]->username == QString(buffer).section("##", 1, 1) &&
                                User_data->u[i]->password == QString(buffer).section("##", 2,2)){
                            isPass = true;
                            logOutput(QString("%1 is connected successfully.").arg(User_data->u[i]->username));
                            User_data->u[i]->online_state = 1;
                            User_data->u[i]->ipAdd = ip;
                            User_data->u[i]->port = qrand()%10000+10000;
                            break;
                        }
                    }
                    if(isPass){
                        tcpSocket[index]->write(QString("##login success##%1").arg(QString(buffer).section("##",1,1)).toUtf8());
                        logOutput(QString("%1 login success").arg(QString(buffer).section("##",1,1)));
                    }
                    else{
                        tcpSocket[index]->write(QString("##login failed##%1").arg(QString(buffer).section("##",1,1)).toUtf8());
                        logOutput(QString("%1 login failed").arg(QString(buffer).section("##",1,1)));
                    }
                }
                else if("register" == QString(buffer).section("##",0,0)){
                    logOutput("register");
                    QString m_name = QString(buffer).section("##",1,1);
                    QString m_password = QString(buffer).section("##",2,2);
                    QString m_question = QString(buffer).section("##",3,3);
                    QString m_answer = QString(buffer).section("##",4,4);
                    bool isRegistered = false;
                    for(int i=0; i<User_data->size; i++){
                        if(User_data->u[i]->username == m_name){
                            isRegistered = true;
                            break;
                        }
                    }
                    if(!isRegistered){
                        int newPort = qrand()%10000+10000;
                        User_data->u[User_data->size] = new User(m_name, m_password, m_question, m_answer, 1, ip, newPort);
                        User_data->size ++ ;
                        tcpSocket[index]->write(QString("##register success##%1").arg(m_name).toUtf8());
                        saveToFile();
                        logOutput(QString("%1 register success").arg(m_name));
                    }
                    else{
                        tcpSocket[index]->write(QString("##register failed##%1 is already used").arg(m_name).toUtf8());
                        logOutput(QString("failed: %1 is already used").arg(m_name));
                    }
                }
                else if()
            }
        })
    });
}
void Server::start(QTcpServer tcp, uint16_t port)
{
    logOutput(QString("server port: %1").arg(port));
    tcp.listen(QHostAddress::Any, port);
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