#include "filetransmit.h"
#include "ui_filetransmit.h"
#include <QMessageBox>
#include <QTime>

extern QString ip_recv;//the IP of reciever
extern bool sendOrReciever;//send:true;recv:false;

const int N = 8;
bool isRecved[N];
bool isWriten[N];
bool isConfirmed[N];
bool isRemade[N];
QByteArray send_packages[N];
QByteArray recv_packages[N];
int count_send;
int count_recv;
int numOfPackage;
bool isOver;
bool isAllConfirmed;
bool isTail;

const int out_time = 1000;
const int package_size = 1024;
const int interval = 1000;
QTime totTime;
QTime count_sender, count_recver;
int sec_sender, sec_recver;

FileTransmit::FileTransmit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileTransmit)
{
    ui->setupUi(this);
    if(sendOrReciever){
        udpSocketServer = new QUdpSocket(this);
        udpSocketServer->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024*1024*100);
    }
}

FileTransmit::~FileTransmit()
{
    delete ui;
}
