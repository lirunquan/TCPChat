#ifndef FILETRANSMIT_H
#define FILETRANSMIT_H

#include <QDialog>
#include <QFileDialog>
#include <QDialog>
#include <QUdpSocket>
#include <QTimer>

namespace Ui {
class FileTransmit;
}

class FileTransmit : public QDialog
{
    Q_OBJECT

public:
    explicit FileTransmit(QWidget *parent = 0);
    ~FileTransmit();
private slots:
    void readData_send();
    void readData_recv();
    void sendData();
    void on_exit_clicked();

    void on_choose_clicked();

    void on_send_clicked();

private:
    Ui::FileTransmit *ui;
    QUdpSocket* udpSocketServer;
    QUdpSocket* udpSocketClient;
    QFile fileToSend;
    QString fileInfo;
    QFile fileRecv;
    QString nameOfSend;
    QString nameOfRecv;
    qint64 sizeOfSend;
    qint64 sizeOfRecv;
    qint64 recvedSize;
    qint64 sendedSize;
    QTimer* timer;
    QHostAddress recvHost;
    quint16 recvPort = 12580;
    bool isFirst;
};

#endif // FILETRANSMIT_H
