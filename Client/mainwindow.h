#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QTime>
#include <QTimer>
#include <QDateTime>
#include <QTcpServer>
#include <QTcpSocket>

#define IP "120.78.66.220"
#define PORT 8800
#define BUF_SIZE 1024*4

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void logOutput(QString log);
    void sendMessage(QString sender, QString reciever, QString message);
    void sendFile();
    void userRegister(QString username, QString password, QString question, QString answer);
    void userLogin(QString username, QString password);
    QString handledString(QString str);
    QString readString(QString str);
    void exit();
private slots:
    void registerEnabled();

    void on_loginBtn_clicked();

    void on_toRegisterBtn_clicked();

    void on_cancelBtn_clicked();

    void on_registerBtn_clicked();

    void on_usernameEdit_textChanged(const QString &arg1);

    void on_passwordEdit_textChanged(const QString &arg1);

    void on_forgotBtn_clicked();

    void on_f_send_clicked();

    void on_f_cancel_clicked();

private:
    Ui::MainWindow *ui;
    QTcpServer* tcpServer;
    QTcpSocket* tcpSocket;
    QTcpSocket* tcpSocket_client;
    QTimer* timer;
};

#endif // MAINWINDOW_H
