#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "user.h"

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
int sendOrReceiver;//send:1,reciever:-1

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
        output << current << log << "\n";
    }
    logFile.close();
}
