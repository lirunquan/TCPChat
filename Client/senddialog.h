#ifndef SENDDIALOG_H
#define SENDDIALOG_H

#include <QDialog>
#include <QUdpSocket>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTime>
#include <QTimer>

namespace Ui {
class SendDialog;
}

class SendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SendDialog(QWidget *parent = 0);
    ~SendDialog();

private slots:
    void sendData();
    void readDatagrams();
    void on_choose_clicked();

    void on_send_clicked();

private:
    Ui::SendDialog *ui;
    QUdpSocket* udpSender;
    QString filename;
//    QFileInfo info;
    QFile fileSend;
    quint64 fileSize;
    quint64 sendedSize;
    QTimer* timer;
    bool match;
};

#endif // SENDDIALOG_H
