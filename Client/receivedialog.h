#ifndef RECEIVEDIALOG_H
#define RECEIVEDIALOG_H

#include <QDialog>
#include <QUdpSocket>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QTime>
#include <QTimer>

namespace Ui {
class ReceiveDialog;
}

class ReceiveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReceiveDialog(QWidget *parent = 0);
    ~ReceiveDialog();
private slots:
    void readDatagrams();

private:
    Ui::ReceiveDialog *ui;
    QUdpSocket* udpRecver;
    QFile fileRecv;
    QString filename;
    quint64 fileSize;
    quint64 recvedSize;
    const quint16 recv_port = 7755;
    bool isFirst;
    QTimer* timer;
    bool match;
};

#endif // RECEIVEDIALOG_H
