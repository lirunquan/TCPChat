#include "receivedialog.h"
#include "ui_receivedialog.h"
extern QString ip_send;
extern QString matchCode;

const int N = 8;
const int package_size = 1024;
const int interval = 1000;

bool isRecved[N];
bool isWritten[N];
bool isTail;

QString sendHost;

QByteArray recv_packages[N];

QTime sumTime;
QTime count_time_recver;

int count_recv;
int sec_recv;
quint16 send_port;

ReceiveDialog::ReceiveDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReceiveDialog)
{
    ui->setupUi(this);
    udpRecver = new QUdpSocket(this);
    timer = new QTimer(this);
    udpRecver->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024*1024*100);
    udpRecver->bind(QHostAddress::LocalHost, recv_port);
    udpRecver->writeDatagram(QString("FileReceiver##%1").arg(matchCode).toUtf8(), QHostAddress("120.78.66.220"), 7755);
    connect(udpRecver, &QUdpSocket::readyRead, this, [=](){
        readDatagrams();
    });
    connect(timer, &QTimer::timeout, [=](){
        udpRecver->writeDatagram(QString("FileReceiver##%1").arg(matchCode).toUtf8(), QHostAddress("120.78.66.220"), 7755);
    });
    ui->nameLine->setText("");
    ui->sizeLine->setText("");
    ui->progressBar->setValue(0);
    ui->textBrowser->clear();
    ui->frame->setVisible(false);
    ui->frame_2->setVisible(false);
    ui->frame_3->setVisible(true);
    ui->frame_3->setGeometry(10,20,431,101);
    this->setGeometry(300,150,450,130);
//    setWindowFlags(Qt::WindowTitleHint);
}

ReceiveDialog::~ReceiveDialog()
{
    udpRecver->close();
    delete ui;
}
void ReceiveDialog::readDatagrams()
{
    while(udpRecver->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(udpRecver->pendingDatagramSize());
        QHostAddress host;
        quint16 port;
        udpRecver->readDatagram(datagram.data(), datagram.size(), &host, &port);
        qDebug() << datagram;
        if("GotSender" == QString(datagram).section("##",0,0)){
            sendHost = QString(datagram).section("##",1,1);
            send_port = QString(datagram).section("##",2,2).toInt();
            match = true;
        }
        else if("NoSender" == QString(datagram)){
            match = false;
            timer->start(1000);
        }
        else if("FileHead" == QString(datagram).section("##",0,0)){
            filename = QString(datagram).section("##",1,1);
            fileSize = QString(datagram).section("##",2,2).toInt();

            QString path = QFileDialog::getSaveFileName(this, "Save file", QString("../%1").arg(filename), "");
            if(!path.isEmpty()){
                fileRecv.setFileName(path);
                count_recv = 0;
                isTail = false;
                for(int i=0;i<N;i++){
                    isRecved[i] = false;
                    isWritten[i] = false;
                    recv_packages[i].clear();
                }
                isFirst = false;
                count_time_recver.start();
                sumTime.start();
                sec_recv = 0;
                if(!fileRecv.open(QIODevice::WriteOnly)){
                    udpRecver->writeDatagram("##FileSaveFailed", QHostAddress(sendHost), send_port);
                    return;
                }
                else{
                    udpRecver->writeDatagram("##FileHeadRecved", QHostAddress(sendHost), send_port);
                    this->setGeometry(300,150,450,300);
                    ui->frame->setVisible(true);
                    ui->frame_2->setVisible(true);
                    ui->nameLine->setText(path);
                    ui->sizeLine->setText(QString("%1 KB").arg(fileSize/1024));
                    ui->progressBar->setMaximum(fileSize);
                    ui->progressBar->setMinimum(0);
                    ui->progressBar->setValue(0);
                    ui->textBrowser->append("Start receiving file.");
                    ui->frame_3->setGeometry(10,290,431,101);
                }
            }
            else{
                udpRecver->writeDatagram("##FileSaveFailed", QHostAddress(sendHost), send_port);
                ui->label->setText("Saving file failed");
                return;
            }
        }
        else{
            if(!isTail){
                QList<QString> list = QString(datagram).split("&&");
                for(int i=0; i<list.size(); i++){
                    QString buffer = list[i];
                    int numOfBig = buffer.section("&#&#",0,0).toInt();
                    int numOfSmall = buffer.section("&#&#",1,1).toInt();
                    if(numOfBig == count_recv){
                        if(!isRecved[numOfSmall]){
                            int x = buffer.section("&#&#",0,0).size();
                            int y = buffer.section("&#&#",1,1).size();
                            recv_packages[numOfSmall] = datagram.mid(x+y+8, package_size);
                            isRecved[numOfSmall] = true;
                            udpRecver->writeDatagram(QString("%1").arg(numOfSmall).toUtf8(), QHostAddress(sendHost), send_port);
                            int m=0;
                            while(isWritten[m] && m<N){
                                m++;
                            }
                            for(int i=m; i<N; i++){
                                if(isRecved[i]){
                                    fileRecv.write(recv_packages[i], recv_packages[i].size());
                                    isWritten[i] = true;
                                    if(recv_packages[i].size() < package_size){
                                        isTail = true;
                                    }
                                    if(i == N-1){
                                        count_recv ++ ;
                                        ui->progressBar->setValue(count_recv*N*package_size);
                                        int c_time = count_time_recver.elapsed();
                                        if(c_time > interval){
                                            count_time_recver.start();
                                            double speed = (sec_recv+1)*N*package_size*1000/c_time/1024;
                                            ui->speed->setText(QString("%1 kB/s").arg(speed));
                                            sec_recv = 0;
                                        }
                                        else{
                                            sec_recv++;
                                        }
                                        for(int i=0; i<N; i++){
                                            isRecved[i] = false;
                                            isWritten[i] = false;
                                            recv_packages[i].clear();
                                        }
                                    }
                                }
                                else{
                                    break;
                                }
                            }
                            if(isTail){
                                udpRecver->writeDatagram("##FileRecved", QHostAddress(sendHost), send_port);
                                ui->progressBar->setValue(fileSize);
                                int sum = sumTime.elapsed();
                                int sum_s = sum/1000;
                                if(sum_s > 60){
                                    int sum_m = sum_s/60;
                                    sum_s = sum_s%60;
                                    ui->textBrowser->append("File transmission completed.");
                                    ui->textBrowser->append(QString("Totally cost %1 min %2 sec").arg(sum_m).arg(sum_s));
                                }
                                else{
                                    ui->textBrowser->append("File transmission completed.");
                                    ui->textBrowser->append(QString("Totally cost %1 sec").arg(sum_s));
                                }
                                fileRecv.close();
                                isFirst = true;
                            }
                        }
                        else{
                            udpRecver->writeDatagram(QString("%1").arg(numOfSmall).toUtf8(), QHostAddress(sendHost), send_port);
                        }
                    }
                }
            }
            else{
                if(isFirst){
                    ui->textBrowser->append("File written completed.");

                    isFirst = false;
                }
            }
        }
    }
}
