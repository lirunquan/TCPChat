#include "senddialog.h"
#include "ui_senddialog.h"

extern QString ip_recv;
extern QString matchCode;
extern bool sendOrReceiver;

const int N = 8;
const int package_size = 1024;
const int out_time = 1000;
const int interval = 1000;

bool isConfirm[N];
bool isOver;
bool isAllConfirm;

QString recvHost;

QByteArray send_packages[N];

int count_sent;
int numOfPackeages;
int sec_sender;
int recv_port;

QTime sum_time;
QTime count_time_sender;

SendDialog::SendDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SendDialog)
{
    ui->setupUi(this);
    udpSender = new QUdpSocket(this);
    timer = new QTimer(this);
    ui->send->setEnabled(false);
    ui->nameLine->setReadOnly(true);
    ui->sizeLine->setReadOnly(true);
    udpSender->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024*1024*100);
    udpSender->bind(7755);
    match = false;
    udpSender->writeDatagram(QString("FileSender##%1").arg(matchCode).toUtf8(), QHostAddress("120.78.66.220"), 7755);
    connect(udpSender, &QUdpSocket::readyRead, this, [=](){
        readDatagrams();
    });
    connect(timer, &QTimer::timeout, this, [=](){
        if(!match){
            udpSender->writeDatagram(QString("FileSender##%1").arg(matchCode).toUtf8(), QHostAddress("120.78.66.220"), 7755);
        }
        else{
            sendData();
        }
    });
    ui->textBrowser->setText("");
    qDebug() << ip_recv;
    numOfPackeages = 0;
}

SendDialog::~SendDialog()
{
    udpSender->close();
    delete ui;
}
void SendDialog::sendData()
{
    for(int i=0; i<numOfPackeages; i++){
        if(!isConfirm[i]){
            udpSender->writeDatagram(send_packages[i], QHostAddress(recvHost), recv_port);
        }
    }
}
void SendDialog::readDatagrams()
{
    while(udpSender->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(udpSender->pendingDatagramSize());
        recvHost.clear();
        QHostAddress host;
        quint16 port;
        udpSender->readDatagram(datagram.data(), datagram.size(), &host, &port);
        qDebug() << datagram;
        if(count_sent != 0)
            qDebug() << count_sent;
        if("GotReceiver" == QString(datagram).section("##",0,0)){
            recvHost = QString(datagram).section("##",1,1);
            recv_port = QString(datagram).section("##",2,2).toInt();
            match = true;
            ui->send->setEnabled(true);
//            timer->stop();
        }
        else if("NoReceiver" == QString(datagram)){
            match = false;
//            timer->start(out_time*10);
        }
        else if("##FileHeadRecved" == QString(datagram)){
            ui->textBrowser->append("File head message arrived.");
            count_sent = 0;
            isOver = false;
            isAllConfirm = false;
            count_time_sender.start();
            sum_time.start();
            sec_sender = 0;
            int num = 0;
            while(!fileSend.atEnd() && num<N){
                QByteArray line(fileSend.read(package_size));
                send_packages[num].clear();
                send_packages[num].append(QString("%1&#&#%2&#&#").arg(count_sent).arg(num));
                send_packages[num].append(line);
                send_packages[num].append("&&");
                isConfirm[num] = false;
                num++;
                if(line.size() < package_size){
                    fileSend.close();
                    isOver = true;
                    break;
                }
            }
            numOfPackeages = num;
            sendData();
            timer->start(out_time);
        }
        else if("##FileRecved" == QString(datagram)){
            isAllConfirm = true;
            ui->progressBar->setValue(sendedSize);
            int sum = sum_time.elapsed();
            int sum_s = sum/1000;
            if(sum_s>=60){
                int sum_m = sum_s/60;
                sum_s = sum_s%60;
                ui->textBrowser->append("File transmission completed.");
                ui->textBrowser->append(QString("Totally cost %1 min %2 sec").arg(sum_m).arg(sum_s));
            }
            else{
                ui->textBrowser->append("File transmission completed.");
                ui->textBrowser->append(QString("Totally cost %1 sec").arg(sum_s));
            }
            timer->stop();
            delete timer;
        }
        else if("##FileSaveFailed" == QString(datagram)){
            fileSend.close();
            ui->textBrowser->append("File message did not match.");
            return ;
        }
        else if(!isAllConfirm){
            int numConfirmed = QString(datagram).toInt();
            if(!isConfirm[numConfirmed]){
                isConfirm[numConfirmed] = true;
            }
            else{
                return;
            }
            bool isPass = true;
            for(int i=0; i<numOfPackeages; i++){
                if(!isConfirm[i]){
                    isPass = false;
                    break;
                }
            }
            if(isPass){
                if(!isOver){
                    count_sent++;
                    int c_time = count_time_sender.elapsed();
                    if(c_time > interval){
                        count_time_sender.start();
                        double speed = (sec_sender+1)*N*package_size*1000/c_time/1024;
                        ui->speed->setText(QString("%1 KB/s").arg(speed));
                        sec_sender = 0;
                    }
                    else{
                        sec_sender++;
                    }
                    int num = 0;
                    while(!fileSend.atEnd() && num<N){
                        QByteArray line(fileSend.read(package_size));
                        send_packages[num].clear();
                        send_packages[num].append(QString("%1&#&#%2&#&#").arg(count_sent).arg(num));
                        send_packages[num].append(line);
                        send_packages[num].append("&&");
                        isConfirm[num] = false;
                        num++;
                        if(line.size()<package_size){
                            fileSend.close();
                            isOver = true;
                            break;
                        }
                    }
                    numOfPackeages = num;
                    sendData();
                    timer->start(out_time);
                }
            }
            else{
                timer->start(out_time);
            }
        }
    }
}
void SendDialog::on_choose_clicked()
{
    filename.clear();
    ui->textBrowser->setText("");
    QString path = QFileDialog::getOpenFileName(this, "Choose", "../");
    if(!path.isEmpty()){
//        ui->send->setEnabled(true);
        QFileInfo info(path);
        filename = info.fileName();
        ui->nameLine->setText(filename);
        fileSize = info.size();
        if(fileSize>=1024){
            ui->sizeLine->setText(QString("%1 KB").arg(fileSize/1024));
        }
        else{
            ui->sizeLine->setText(QString("%1 B").arg(fileSize));
        }
        sendedSize = 0;
        fileSend.setFileName(path);
        if(!fileSend.open(QIODevice::ReadOnly)){
            ui->send->setEnabled(false);
            ui->textBrowser->append(QString("%1 opened failed, please confirm.").arg(path));
        }
        else{
            ui->send->setEnabled(true);
            ui->textBrowser->append("Ready to send file.");
            ui->textBrowser->append(QString("File: %1").arg(ui->nameLine->text()));
            ui->textBrowser->append(QString("Total: %1").arg(ui->sizeLine->text()));
            ui->progressBar->setMaximum(fileSize);
            ui->progressBar->setMinimum(0);
            ui->progressBar->setValue(0);
        }
    }
    else{
        ui->textBrowser->append("Invalid path.");
    }
}

void SendDialog::on_send_clicked()
{
    ui->send->setEnabled(false);
    ui->choose->setEnabled(false);
    udpSender->connectToHost(QHostAddress(ip_recv), recv_port);
    udpSender->writeDatagram(QString("FileHead##%1##%2").arg(filename).arg(fileSize).toUtf8(), QHostAddress(recvHost), recv_port);
}
