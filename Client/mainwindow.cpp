#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "user.h"
#include <QMessageBox>
#include <QTextStream>

const int time_out = 1000;//longest waiting time;
const int M = 20;//max size of users;
enum{
    Login = 0,
    Send,
    Reciever,
    Chat,
    Client
};
User* user[M];

int m_size;
int mode[2];
bool sendOrReceiver;//send:true, reciever:false

bool isFind = false;
bool isQuestionReturn, isOffline, isSet;

quint16 port_num;

QString requestString = "";
QString m_name;
QString m_ques,m_answ;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    for(int i=0; i<M; i++){
        user[i] = NULL;
    }
    m_size = 0;
    mode[0] = Chat; mode[1] = Chat;
    isOffline = true;
    port_num = 7777;
    timer = new QTimer();
    connect(timer, &QTimer::timeout, [=](){
        QMessageBox::information(this, "Error", "Connect Failed!", QMessageBox::Yes);
        logOutput("Error: Connect Failed.");
        timer->stop();
    });
    tcpSocket = new QTcpSocket(this);
    tcpServer = new QTcpServer(this);
    tcpSocket_client = new QTcpSocket(this);
    tcpSocket->connectToHost(QHostAddress(IP), PORT);
    tcpSocket->write("##Request for login");//send tcp connect request for login
    timer->start(time_out);
    logOutput("Requset for login");
    connect(tcpServer, &QTcpServer::newConnection, [=](){

    });
    connect(tcpSocket_client, &QTcpSocket::connected, [=](){

    });
    connect(tcpSocket_client, &QTcpSocket::disconnected, [=](){

    });
    connect(tcpSocket_client, &QTcpSocket::readyRead, [=](){

    });
    connect(tcpSocket, &QTcpSocket::connected, [=](){
        //tips for connected success
        logOutput("connected success");
        isOffline = false;
        mode[0] = Chat;
    });
    connect(tcpSocket, &QTcpSocket::disconnected, [=](){
        //window operation for disconnection
        requestString.clear();
    });
    connect(tcpSocket, &QTcpSocket::readyRead, [=](){
        QByteArray buffer = tcpSocket->readAll();
        if(mode[0] == Login){
            if(!isFind){                
                if("login success" == QString(buffer).section("##",1,1)){
                    //login success, open the chat window
                    tcpSocket->write("##RequsetForUserInfo");
                    mode[0] = Chat;
                }
                else if("register success" == QString(buffer).section("##",1,1)){
                    //window hints the infomation
                    tcpSocket->write("##RequestForUserInfo");
                    mode[0] = Chat;
                }
                else{
                    //window hints the "login/register failed"
                }
            }
            else if("question" == QString(buffer).section("##",1,1)){
                m_ques = QString(buffer).section("##",2,2);
                //window change into finding password, change the m_answer
                //m_answ = QString("answer##%1##").arg(what user input);
                //m_answ.append(newpassword);
                tcpSocket->write(m_answ.toUtf8());
                isQuestionReturn = true;
            }
            else if("answer is right" == QString(buffer).section("##",1,1)){
                //hints the login infomation
                //user has login
                //password has changed into the new one
                tcpSocket->write("##RequestForUserInfo");
                mode[0] = Chat;
            }
            else{
                tcpSocket->write("wrong request");
                logOutput(QString(buffer));
                logOutput("something wrong");
            }
        }
        else if("RequestForContact" == QString(buffer).section("##",1,1)){//A##RequestForContact##B  A wants to contact B
            if(m_name != QString(buffer).section("##",2,2)){
                //show wrong request message in the window
            }
            else{
                QString c_sender = QString(buffer).section("##",0,0);
                //show contact sender in the window
                tcpSocket->write(QString("%1##AcceptContact##%2").arg(m_name).arg(c_sender).toUtf8());
            }
        }
        else if("load users' state" == QString(buffer).section("##",0,0)){
            for(int i=0; i<M; i++){
                user[i] = NULL;
            }
            m_size = 0;
            //window clears the list of users
            m_size = QString(buffer).section("##",1,1).toInt();
            if(m_size > 0){
                QString userInfo = QString(buffer).section("##",2,2);
                logOutput(userInfo);
                QTextStream info(&userInfo);
                int num_on = 0, num_off = 0;
                for(int i=0; i<m_size; i++){
                    QString u_name, u_ip;
                    int u_port, u_state;
                    info >> u_name;
                    info >> u_state;
                    info >> u_ip;
                    info >> u_port;
                    User* u_new = new User(u_name, u_state, u_ip, u_port);
                    user[i] = u_new;
                    if(!isSet && u_name == m_name){
                        setWindowTitle(QString("%1 %2:%3").arg(u_name).arg(u_ip).arg(u_port));
                        port_num = u_port;
                        tcpServer->listen(QHostAddress::Any, port_num);
                        isSet = true;
                    }
                    if(u_state == 1){
                        //users' list add an online user
                        num_on ++;
                    }
                    else{
                        //users' list add an offline user
                        num_off ++;
                    }
                }
            }
            else{
                //hints no other users;
                logOutput("no user in the server.");
            }
            if(!isOffline){
                isOffline = true;
                tcpSocket->write(QString("##Offline message##%1").arg(m_name).toUtf8());
                logOutput("send offline message.");
            }
        }
        else if("offline message" == QString(buffer).section("&&",0,0)){//handle offline message
            int num = QString(buffer).section("&&",1,1).toInt();//number of offline message
            for(int m=0; m<num+2; m++){
                QString single = QString(buffer).section("&&",m,m);
                QString sender = QString(single).section("##",0,0);//who sended
                QString message_off = QString(single).section("##",1,1);//content of message
                QString reciever = QString(single).section("##",2,2);//who will recieve
                //show the message in the window
            }
        }
        else if("##Permission for login" == QString(buffer)){
            timer->stop();
            //window change into login
            if(requestString != ""){
                tcpSocket->write(requestString.toUtf8());
                mode[0] = Login;
            }
            else{
                logOutput("request string is empty.");
            }
        }
        else{
            //handle common message, sending "A##message##B" means A sends message to B
            QString sender = QString(buffer).section("##",0,0);
            QString m_common = QString(buffer).section("##",1,1);
            QString reciever = QString(buffer).section("##",2,2);
            //show message in the window
        }
    });
}

MainWindow::~MainWindow()
{
    for(int i=0; i<M; i++){
        if(user[i]){
            delete user[i];
            user[i] = NULL;
        }
    }
    tcpSocket->close();
    tcpServer->close();
    tcpSocket_client->close();
    if(tcpSocket){
        delete tcpSocket;
        tcpSocket = NULL;
    }
    if(tcpServer){
        delete tcpServer;
        tcpServer = NULL;
    }
    if(tcpSocket_client){
        delete tcpSocket_client;
        tcpSocket_client = NULL;
    }
    delete ui;
    logOutput("Close Client");
}

void MainWindow::logOutput(QString log)
{
    QDateTime date(QDateTime::currentDateTime());
    QTime time(QTime::currentTime());
    QString current = QString("%1 %2 : ").arg(date.toString("dd.MM.yyyy")).arg(time.toString("hh:mm:ss"));
    QFile logFile("log.txt");
    if(logFile.open(QIODevice::WriteOnly|QIODevice::Append)){
        QTextStream output(&logFile);
        qDebug() << current << log << "\n";
        output << current << log << "\n";
    }
    logFile.close();
}
void MainWindow::sendMessage(QString sender, QString reciever, QString message)
{
    QString sending = sender.append(QString("##%1##").arg(message)).append(reciever);
    tcpSocket->write(sending.toUtf8());
    logOutput(QString("Sending common message to %1").arg(reciever));
}
void MainWindow::userLogin(QString username, QString password)
{
    requestString = QString("login##%1##%2").arg(username).arg(password);
    logOutput(requestString);
}
void MainWindow::userRegister(QString username, QString password, QString question, QString answer)
{
    requestString = QString("register##%1##%2##%3##%4").arg(username).arg(password).arg(question).arg(answer);
    logOutput(requestString);
}
void MainWindow::exit()
{
    for(int i=0; i<m_size; i++){
        delete user[i];
        user[i] = NULL;
    }//clear all the users
    m_size = 0;
    if(tcpSocket->isOpen()){
        tcpSocket->disconnectFromHost();//disconnect the connection to server
    }
}
