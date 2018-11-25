#include "filetransmit.h"
#include "ui_filetransmit.h"
#include <QMessageBox>
#include <QTime>

extern QString ip_recv;//the IP of reciever
extern bool sendOrReceiver;//send:true;recv:false;

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
    if(sendOrReceiver){
        udpSocketServer = new QUdpSocket(this);
        udpSocketServer->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024*1024*100);
        connect(udpSocketServer, SIGNAL(readyRead()), this, SLOT(readData_send()));
    }
    else{
        udpSocketClient = new QUdpSocket(this);
        udpSocketClient->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024*1024*100);
        udpSocketClient->bind(recvPort);
        connect(udpSocketClient, SIGNAL(readyRead()), this, SLOT(readData_recv()));
    }
    if(ip_recv == ""){
        QMessageBox::information(NULL, "Error", "IP is empty");
        close();
    }
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=](){
        sendData();
    });
}

FileTransmit::~FileTransmit()
{
    if(udpSocketClient){
        udpSocketClient->close();
        delete udpSocketClient;
        udpSocketClient = NULL;
    }
    if(udpSocketServer){
        udpSocketServer->close();
        delete udpSocketServer;
        udpSocketServer = NULL;
    }
    delete ui;
}
 void FileTransmit::sendData()
 {
     for(int i=0; i<numOfPackage; i++){
         if(!isConfirmed[i]){
             udpSocketServer->writeDatagram(send_packages[i], QHostAddress(ip_recv), recvPort);
         }
     }
 }
void FileTransmit::readData_recv()
{
    while(udpSocketClient->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(udpSocketClient->pendingDatagramSize());
        QHostAddress senderHost;
        quint16 senderPort;
        udpSocketClient->readDatagram(datagram.data(), datagram.size(), &senderHost, &senderPort);
        if("FileHead" == QString(datagram).section("##",0,0)){//FileHead##filename##size
            QString path = QFileDialog::getExistingDirectory(NULL, "Save File", "../");
            nameOfRecv = path.append("/").append(QString(datagram).section("##",1,1));
            sizeOfRecv = QString(datagram).section("##",2,2).toInt();
            //dialog show filename and size
            fileRecv.setFileName(nameOfRecv);
            count_recv = 0;
            isTail = false;
            for(int i=0;i<N;i++){
                isRecved[i] = false;
                isWriten[i] = false;
                recv_packages[i].clear();
            }
            isFirst = false;
            count_recver.start();
            totTime.start();
            sec_recver = 0;
            //progressbar->maxmum,minimum,value
            if(!fileRecv.open(QIODevice::WriteOnly)){
                udpSocketClient->writeDatagram("##Failed to open file", senderHost, senderPort);
                return;
            }
            else{
                udpSocketClient->writeDatagram("##Head recieved", senderHost, senderPort);
            }
        }
        else{
            if(!isTail){
                QString buffer = QString(datagram);
                int num_big = buffer.section("&#&#",0,0).toInt();
                int num_small = buffer.section("&#&#",1,1).toInt();
                if(num_big == count_recv){
                    if(!isRecved[num_small]){
                        int x = buffer.section("&#&#",0,0).size();
                        int y = buffer.section("&#&#",1,1).size();
                        recv_packages[num_small] = datagram.mid(x+y+8, package_size);
                        isRecved[num_small] = true;
                        udpSocketClient->writeDatagram(QString("%1").arg(num_small).toUtf8(), senderHost, senderPort);
                        int m=0;
                        while(isWriten[m] && m<N){
                            m++;
                        }
                        for(int i=m; i<N; i++){
                            if(isRecved[i]){
                                fileRecv.write(recv_packages[i], recv_packages[i].size());
                                isWriten[i] = true;
                                if(recv_packages[i].size() < package_size){
                                    isTail = true;
                                }
                                if(i == N-1){
                                    count_recv++;
                                    //progressbar->setvalue(count_recv*N*package_size)
                                    int ctimer = count_recver.elapsed();
                                    if(ctimer > interval){
                                        count_recver.start();
                                        double speed = ((sec_recver+1)*N*package_size*1000/ctimer/1024);
                                        //text speed
                                        sec_recver = 0;
                                    }
                                    else{
                                        sec_recver++;
                                    }
                                    for(int i=0;i<N;i++){
                                        isRecved[i] = false;
                                        isWriten[i] = false;
                                        recv_packages[i].clear();
                                    }
                                }
                            }
                            else {
                                break;
                            }
                        }
                        if(isTail){
                            udpSocketClient->writeDatagram("##File all recieved", senderHost, senderPort);
                            //progressbar setvalue(sizerecv)
                            //button enable
                            isFirst = true;
                            int sum = totTime.msecsTo(QTime::currentTime());
                            int sum_s = sum/1000;
                            long int speed_s = (sizeOfRecv*1000/sum)/1024;
                            if(sum_sec >= 60){
                                int sum_min = sum_sec/60;
                                sum_sec = sum_sec%60;
                                QMessageBox::warning(this,
                                                     "Transmission completed",
                                                     QString("Takes %1 (min) %2 (sec). \nAverage Speed: %3 kb/s").arg(sum_m).arg(sum_s).arg(speed_s),
                                                     QMessageBox::Yes);
                //                close();
                            }
                            else{
                                QMessageBox::warning(this,
                                                     "Transmission completed",
                                                     QString("Take %1 (sec). \nAverage Speed: %2 kb/s").arg(sum_s).arg(speed_s),
                                                     QMessageBox::Yes);
                            }
                            close();
                        }
                    }
                    else{
                        udpSocketClient->writeDatagram(QString("%1").arg(num_small).toUtf8(), senderHost, senderPort);
                    }
                }
            }
            else{
                if(isFirst){
                    isFirst = true;
                }
                qDebug() << "file all saved"<<endl;
            }
        }
    }
}
void FileTransmit::readData_send()
{
    while(udpSocketServer->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(udpSocketServer->pendingDatagramSize());
        recvHost.clear();
        udpSocketServer->readDatagram(datagram.data(), datagram.size(), &recvHost, &recvPort);
        if("##Head recieved" == datagram){
            qDebug("Head recieved");
            count_send = 0;
            isOver = false;
            isAllConfirmed = false;
            count_sender.start();
            totTime.start();
            sec_sender = 0;
            for(int i=0; i<N; i++){
                isConfirmed[i] = false;
            }
            int n=0;
            while(!fileToSend.atEnd() && n<N){
                QByteArray line(fileToSend.read(package_size));
                send_packages[n].clear();
                send_packages[n].append(QString("%1&#&#%2&#&#").arg(count_send).arg(n));
                send_packages[n].append(line);
                isRemade[n] = false;
                udpSocketServer->writeDatagram(send_packages[n], QHostAddress(ip_recv), recvPort);
                n++;
                qDebug()<<"send over"<<count_send<<n<<line.size();
                if(line.size() < package_size){
                    fileToSend.close();
                    break;
                }
            }
            numOfPackage = n;
            timer->start(out_time);
        }
        else if("##Failed to open file" == datagram){
            fileToSend.close();
            //text
            return ;
        }
        else if("##File all recieved" == datagram){
            isAllConfirmed = true;
            //progressbar,button,text
            timer->stop();
            delete timer;
            QDebug()<<"All transmitted";
            int sum = totTime.elapsed();
            int sum_sec = sum/1000;
            long int speed = (sizeOfSend*1000/sum)/1024;
            if(sum_sec >= 60){
                int sum_min = sum_sec/60;
                sum_sec = sum_sec%60;
                QMessageBox::warning(this,
                                     "Transmission completed",
                                     QString("Takes %1 (min) %2 (sec). \nAverage Speed: %3 kb/s").arg(sum_min).arg(sum_sec).arg(speed),
                                     QMessageBox::Yes);
//                close();
            }
            else{
                QMessageBox::warning(this,
                                     "Transmission completed",
                                     QString("Take %1 (sec). \nAverage Speed: %2 kb/s").arg(sum_sec).arg(speed),
                                     QMessageBox::Yes);
            }
            close();
        }
        else if(!isAllConfirmed){
            int numConfirm = QString(datagram).toInt();
            if(!isConfirmed[numConfirm]){
                isConfirmed[numConfirm] = true;
                int make = 0;
                while(isRemade[make] && make<N){
                    make ++;
                }
                while(!isRemade[make] && isConfirmed[make] && make<N && !isOver){
                    if(!fileToSend.atEnd()){
                        QByteArray line(fileToSend.read(package_size));
                        send_packages[make].clear();
                        send_packages[make].append(QString("%1&#&#%2&#&#").arg(count_send+1).arg(make));
                        send_packages[make].append(line);
                        isRemade[make] = true;
                        make ++;
                        if(line.size() < package_size){
                            isOver = true;
                            fileToSend.close();
                            break;
                        }
                    }
                }
            }
            else{
                return ;
            }
            bool isPass = true;
            for(int i=0; i<numOfPackage; i++){
                if(!isConfirmed[i]){
                    isPass = false;
                    break;
                }
            }
            if(isPass){
                count_send ++ ;
                //progressbar
                int ctimes = count_sender.elapsed();
                if(ctimes > interval){
                    count_sender.start();
                    double speed = ((sec_sender+1)*N*package_size*1000/ctimes/1024);
                    //text
                    sec_sender = 0;
                }
                else{
                    sec_sender++;
                }
                int m_num = 0;
                while(isRemade[m_num] && m_num<N){
                    isConfirmed[m_num] = false;
                    m_num ++;
                }
                numOfPackage = m_num;
                for(int i=0;i<m_num;i++){
                    isRemade[i] = false;
                    udpSocketServer->writeDatagram(send_packages[i], QHostAddress(ip_recv), recvPort);
                }
                timer->start(out_time);
            }
            else{
                //out of time
                timer->start(out_time);
            }
        }
    }
}
