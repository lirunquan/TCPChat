#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "user.h"
#include "filetransmit.h"
#include <QMessageBox>

#include <QTextStream>
#include <QtNetwork>

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
QString ip_send, ip_recv;
QString requestString = "";
QString m_name;
QString recv_name;
int recv_state;//1:online,0:offline
QString m_ques,m_answ;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(100,100,400,300);
    ui->fontComboBox->setVisible(false);
    ui->boldButton->setVisible(false);
    ui->italicButton->setVisible(false);
    ui->underlineButton->setVisible(false);
    ui->comboBox->setVisible(false);
    ui->chatWidget->setVisible(false);
    ui->c_message_send->setEnabled(false);
    ui->c_file_send->setEnabled(false);
    ui->f_cancel->setVisible(false);
    ui->stackedWidget->setCurrentIndex(0);
    connect(ui->r_username, SIGNAL(textChanged(QString)), this, SLOT(registerEnabled()));
    connect(ui->r_password, SIGNAL(textChanged(QString)), this, SLOT(registerEnabled()));
    connect(ui->r_confirm, SIGNAL(textChanged(QString)), this, SLOT(registerEnabled()));
    connect(ui->r_answer, SIGNAL(textChanged(QString)), this, SLOT(registerEnabled()));
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
    QString localHostName = QHostInfo::localHostName();
    logOutput(localHostName);
    QHostInfo info = QHostInfo::fromName(localHostName);
    foreach (QHostAddress address, info.addresses()) {
        if(address.protocol() == QAbstractSocket::IPv4Protocol){
            logOutput(address.toString());
        }
    }
    isSet = false;
    tcpSocket = new QTcpSocket(this);
    tcpServer = new QTcpServer(this);
    tcpSocket_client = new QTcpSocket(this);
    tcpSocket->connectToHost(QHostAddress(IP), PORT);
    tcpSocket->write("##ChatMode");//send tcp connect request for login
//    timer->start(time_out);
    ui->label->setText("Connecting to server...");
//    logOutput("Requset for login");
    connect(tcpServer, &QTcpServer::newConnection, [=](){
        tcpSocket_client = tcpServer->nextPendingConnection();
        QString c_ip = tcpSocket_client->peerAddress().toString().section(":",3,3);
        mode[1] = Chat;
        logOutput(QString("%1 is connected, number of users: %2").arg(c_ip).arg(m_size));
        logOutput(m_name);
        for(int i=0; i<m_size; i++){
            if(user[i]->ip == c_ip){
                ip_recv = c_ip;
                logOutput(QString("%1 is connected to localhost").arg(user[i]->username));
                break;
            }
        }
        connect(tcpSocket_client, &QTcpSocket::connected, [=](){

        });
        connect(tcpSocket_client, &QTcpSocket::disconnected, [=](){
            ip_recv.clear();
        });
        connect(tcpSocket_client, &QTcpSocket::readyRead, [=](){
            QString buffer = tcpSocket_client->readAll();
            logOutput(buffer);
            if(mode[1] == Chat){
                if("##RequestForSendingFile" == QString(buffer)){
                    if(QMessageBox::Yes == QMessageBox::information(
                                this, "Request for file transmission", "Do you accept it?", QMessageBox::Yes, QMessageBox::No)){
                        tcpSocket_client->write("##AcceptSending");
                        logOutput("accept sending");
                        sendOrReceiver = false;
                        //show dialog of sending file
                        FileTransmit *dialogRecv = new FileTransmit(this);
                        dialogRecv->show();
                    }
                    else{
                        tcpSocket_client->write("##RefuseSending");
                        logOutput("refuse sending");
                    }
                }
                else if("##AcceptSending" == QString(buffer)){
                    sendOrReceiver = true;
                    FileTransmit* dia = new FileTransmit(this);
                    dia->show();
                }
                else if("##RefuseSending" == QString(buffer)){
                    QMessageBox::information(this, "Sorry", "He is inconvenient to recieve the file.");
                }
            }
        });
    });
    connect(tcpSocket_client, &QTcpSocket::connected, [=](){

    });
    connect(tcpSocket_client, &QTcpSocket::disconnected, [=](){
        ip_recv.clear();
    });
    connect(tcpSocket_client, &QTcpSocket::readyRead, [=](){
        QString buffer = tcpSocket_client->readAll();
        logOutput(buffer);
        if(mode[1] == Chat){
            if("##RequestForSendingFile" == QString(buffer)){
                if(QMessageBox::Yes == QMessageBox::information(this, "Request for file transmission", "Do you accept it?", QMessageBox::Yes, QMessageBox::No)){
                    tcpSocket_client->write("##AccptSending");
                    logOutput("accept sending");
                    sendOrReceiver = false;
                    FileTransmit* dialog = new FileTransmit(this);
                    dialog->show();
                }
                else{
                    tcpSocket_client->write("##RefuseSending");
                    logOutput("refuse sending");
                }
            }
            else if("##AcceptSending" == QString(buffer)){
                sendOrReceiver = true;
                FileTransmit* dial = new FileTransmit(this);
                dial->show();
            }
            else if("##RefuseSending" == QString(buffer)){
                QMessageBox::information(this, "Sorry", "He is inconvenient to recieve the file.");
            }
        }
    });
    connect(tcpSocket, &QTcpSocket::connected, [=](){
        //tips for connected success
        logOutput("connected success");
        ui->label->setText("Connecting to server  ...  Done.");
//        isOffline = false;
        mode[0] = Chat;
    });
    connect(tcpSocket, &QTcpSocket::disconnected, [=](){
        //window operation for disconnection
        requestString.clear();
    });
    connect(tcpSocket, &QTcpSocket::readyRead, [=](){
        QByteArray buffer = tcpSocket->readAll();
        logOutput(QString(buffer));
        if(mode[0] == Login){
            if(!isFind){                
                if("login success" == QString(buffer).section("##",1,1)){
                    logOutput("login success");
                    this->onSuccess();
                    m_name = readString(QString(buffer).section("##",2,2));
                    mode[0] = Chat;
                }
                else if("register success" == QString(buffer).section("##",1,1)){
                    onSuccess();
                    m_name = readString(QString(buffer).section("##",2,2));
                    mode[0] = Chat;
                }
                else{
                    //window hints the "login/register failed"
                    ui->stackedWidget->setCurrentIndex(1);
                    ui->usernameEdit->setText("");
                    ui->passwordEdit->setText("");
                    mode[0] = Login;
                }
            }
            else if("question" == QString(buffer).section("##",1,1)){
                m_ques = QString(buffer).section("##",2,2);
                ui->stackedWidget->setCurrentIndex(3);
                ui->f_username->setText(ui->usernameEdit->text());
                ui->f_question->setText(m_ques.replace("_", " "));
                isQuestionReturn = true;
            }
            else if("answer is right" == QString(buffer).section("##",1,1)){
                onSuccess();
                m_name = readString(QString(buffer).section("##",2,2));
                mode[0] = Chat;
            }
            else if("##answer is wrong" == QString(buffer)){
                QMessageBox::warning(this, "Error", "Answer is wrong.");
                ui->f_answer->setText("");
                ui->f_newpw->setText("");
                ui->f_confirm->setText("");
            }
            else if(QString("user %1 does not exist").arg(handledString(ui->usernameEdit->text())) == QString(buffer)){
                QMessageBox::warning(this, "Error", QString("user %1 does not exist").arg(ui->usernameEdit->text()));
                ui->passwordEdit->setText("");
            }
            else{
                tcpSocket->write("wrong request");
                logOutput(QString(buffer));
                logOutput("something wrong");
            }
        }
        else if(mode[0] == Client){
            if("AcceptContact" == QString(buffer).section("##",1,1)){
                if(m_name == readString(QString(buffer).section("##", 2,2))){
                    QString connect_name = readString(QString(buffer).section("##",0,0));
                    QString myIP = "";
                    qint16 myPort = 0;
                    for(int i=0;i<m_size;i++){
                        if(user[i]->username == connect_name){
                            myIP = user[i]->ip;
                            myPort = user[i]->port;
                        }
                    }
                    if(!myIP.isEmpty() && myPort!=0){
                        tcpSocket_client->connectToHost(QHostAddress(myIP), myPort);
                        if(tcpSocket_client->isOpen()){
                            ip_recv = myIP;
                        }
                        mode[1] = Chat;
                    }
                }
                else{
                    QMessageBox::information(NULL, "Warning", "Received wrong connection request.");
                }
            }
            mode[0] = Chat;
        }
        else if("RequestForContact" == QString(buffer).section("##",1,1)){//A##RequestForContact##B  A wants to contact B
            if(m_name != readString(QString(buffer).section("##",2,2))){
                //show wrong request message in the window
            }
            else{
                QString c_sender = readString(QString(buffer).section("##",0,0));
                if(QMessageBox::Yes == QMessageBox::information(this, "Contact", QString("%1 wants to send a file to you,\nDo you accept?"), QMessageBox::Yes, QMessageBox::No)){
                    tcpSocket->write(QString("%1##AcceptContact##%2").arg(handledString(m_name)).arg(handledString(c_sender)).toUtf8());
                }
            }
        }
        else if("load users' states" == QString(buffer).section("##",0,0)){
            for(int i=0; i<M; i++){
                user[i] = NULL;
            }
            m_size = 0;
            QStandardItemModel* on_model = new QStandardItemModel(this);
            QStandardItemModel* off_model = new QStandardItemModel(this);
            ui->c_on_list->setModel(on_model);
            ui->c_off_list->setModel(off_model);
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
                    User* u_new = new User(readString(u_name), u_state, u_ip, u_port);
                    user[i] = u_new;
                    if(!isSet && readString(u_name) == m_name){
                        setWindowTitle(QString("%1 %2:%3").arg(readString(u_name)).arg(u_ip).arg(u_port));
                        port_num = u_port;
                        tcpServer->listen(QHostAddress::Any, port_num);
                        isSet = true;
                    }
                    if(u_state == 1){
                        //users' list add an online user
                        if(u_name != m_name){
                            QStandardItem* item_on = new QStandardItem(u_name);
                            on_model->appendRow(item_on);
                            num_on ++;
                        }
                    }
                    else{
                        //users' list add an offline user
                        QStandardItem* item_off = new QStandardItem(u_name);
                        off_model->appendRow(item_off);
                        num_off ++;
                    }
                }
                ui->c_on_list->setModel(on_model);
                ui->c_off_list->setModel(off_model);
                ui->c_on_label->setText(QString("Online(%1)").arg(num_on));
                ui->c_off_label->setText(QString("Offline(%1)").arg(num_off));
                connect(ui->c_on_list, SIGNAL(clicked(QModelIndex)), this, SLOT(online_click(QModelIndex)));
                connect(ui->c_off_list, SIGNAL(clicked(QModelIndex)), this, SLOT(offline_click(QModelIndex)));
            }
            else{
                logOutput("no user in the server.");
            }
            if(!isOffline){
                isOffline = true;
                tcpSocket->write(QString("Offline message##%1").arg(handledString(m_name)).toUtf8());
                logOutput("send offline message.");
            }
        }
        else if("offline message" == QString(buffer).section("&&",0,0)){//handle offline message
            int num = QString(buffer).section("&&",1,1).toInt();//number of offline message
            qDebug()<<num;
            if(num>0){
                QString str = "";
                for(int m=0; m<num+2; m++){
                    QString single = QString(buffer).section("&&",m,m);
                    QString sender = readString(QString(single).section("##",0,0));//who sended
                    QString message_off = readString(QString(single).section("##",1,1)) ;//content of message
                    QString reciever = readString(QString(single).section("##",2,2)) ;//who will recieve
                    QString time_sent = QString(single).section("##",3,3);
                    str += QString("%1\n%2: %3\n").arg(time_sent).arg(sender).arg(message_off);
                    qDebug()<<str;
                }
                QMessageBox::about(NULL, "Offline message", str);
            }
        }
        else if("##Permission for login" == QString(buffer)){
            mode[0] = Login;
            ui->label->setText("Login");
            ui->label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
            ui->frame_2->setVisible(true);
            ui->stackedWidget->setCurrentIndex(1);
            setWindowTitle("Login");
            ui->loginBtn->setEnabled(false);
            ui->forgotBtn->setEnabled(false);
        }
        else if("##ChatMode, waiting request" == QString(buffer)){
            tcpSocket->write("##Request for login");
//            timer->start(time_out);
        }
        else if("##Logout" == QString(buffer)){
            m_name = "";
            mode[0] = Login;
            tcpSocket->write("##Request for login");
            this->setGeometry(100,100,400,300);
            ui->menuBar->clear();
            ui->msgBrowser->setText("");
            ui->chatWidget->setVisible(false);
            ui->frame->setVisible(true);
            ui->stackedWidget->setVisible(true);
            ui->usernameEdit->setText("");
            ui->passwordEdit->setText("");
        }
        else{
            //handle common message, sending "A##message##B##time" means A sends message to B
            QString sender = readString(QString(buffer).section("##",0,0));
            QString m_common = readString(QString(buffer).section("##",1,1));
            QString reciever = readString(QString(buffer).section("##",2,2));
            //show message in the window
            ui->msgBrowser->append(QString("%1 %2: \n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(sender));
            ui->msgBrowser->append(QString(m_common));
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
void MainWindow::sendMessage(QString sender, QString message, QString reciever)
{
    QString sending = handledString(sender).append(
                QString("##%1##%2##%3").arg(handledString(message))).arg(handledString(reciever)).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    tcpSocket->write(sending.toUtf8());
    logOutput(QString("Sending message to %1").arg(reciever));
}
void MainWindow::sendFile()
{
    tcpSocket_client->write("##RequestForSendingFile");
    mode[0] = Client;
}
void MainWindow::userLogin(QString username, QString password)
{
    requestString = QString("login##%1##%2").arg(handledString(username)).arg(handledString(password));
    tcpSocket->write(requestString.toUtf8());
    logOutput(requestString);
}
void MainWindow::userRegister(QString username, QString password, QString question, QString answer)
{
    requestString = QString("register##%1##%2##%3##%4").arg(handledString(username)).arg(handledString(password)).arg(handledString(question)).arg(handledString(answer));
    tcpSocket->write(requestString.toUtf8());
    logOutput(requestString);
}
QString MainWindow::handledString(QString str)
{
    QString string = str;
    string.replace(QString("#"), QString("/#"));
    string.replace(QString("&"), QString("/&"));
    return string;
}
QString MainWindow::readString(QString str)
{
    QString string = str;
    string.replace(QString("/#"), QString("#"));
    string.replace(QString("/&"), QString("&"));
    return string;
}
void MainWindow::onSuccess()
{
    isOffline = false;
    this->setGeometry (100,100,710,520);
//    ui->menuBar->clear();
    QMenu* menu = ui->menuBar->addMenu("Menu");
    QAction* logout = menu->addAction("Logout");
    QAction* exit = menu->addAction("Exit");
    connect(logout, &QAction::triggered, [=](){
        if(QMessageBox::Yes == QMessageBox::information(this, "Logout", "Are you sure to logout?", QMessageBox::Yes, QMessageBox::No)){
            tcpSocket->write(QString("logout##%1").arg(handledString(m_name)).toUtf8());
        }
    });
    connect(exit, &QAction::triggered, [=](){
        if(QMessageBox::Yes == QMessageBox::information(this, "Exit", "Are you sure to exit?", QMessageBox::Yes, QMessageBox::No)){
            this->exit();
            this->close();
        }
    });
    ui->frame->setVisible(false);
    ui->stackedWidget->setVisible(false);
    ui->chatWidget->setVisible(true);
    ui->c_file_send->setEnabled(false);
    ui->recver_label->setText("");
    recv_name = "";recv_state = 0;
    tcpSocket->write("##RequestForUserInfo");
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

void MainWindow::on_loginBtn_clicked()
{
    userLogin(ui->usernameEdit->text(), ui->passwordEdit->text());
//    QMessageBox::warning(this, "Error", "Please enter your username.");
}


void MainWindow::on_toRegisterBtn_clicked()
{
    setWindowTitle("Register");
    ui->label->setText("Register");
    ui->usernameEdit->setText("");
    ui->passwordEdit->setText("");
    ui->stackedWidget->setCurrentIndex(2);
    ui->r_answer->setText("");
    ui->r_username->setText("");
    ui->r_password->setText("");
    ui->r_confirm->setText("");
    ui->r_question->setCurrentText("");
    registerEnabled();
}

void MainWindow::on_cancelBtn_clicked()
{
    ui->r_username->setText("");
    ui->r_password->setText("");
    ui->r_confirm->setText("");
    ui->r_answer->setText("");
    ui->r_question->setCurrentIndex(0);
    ui->stackedWidget->setCurrentIndex(1);

}

void MainWindow::on_registerBtn_clicked()
{
//    qDebug()<<ui->r_question->currentText();
    QChar c = ui->r_username->text().at(0);
    if(ui->r_username->text().isEmpty()){
        QMessageBox::warning(this, "Error", "Username is required.");
    }
    else if(!c.isLetterOrNumber()){
        QMessageBox::warning(this, "Error", "Invalid username.");
        ui->r_username->setText("");
        ui->r_password->setText("");
    }
    else if(ui->r_password->text().length() <= 5){
        QMessageBox::warning(this, "Error", "Password is required to be not shorter than 6.");
        ui->r_password->setText("");
        ui->r_confirm->setText("");
    }
    else if(ui->r_confirm->text().isEmpty() || ui->r_confirm->text() != ui->r_password->text()){
        QMessageBox::warning(this, "Error", "Please confirm your password.");
        ui->r_password->setText("");
        ui->r_confirm->setText("");
    }
    else if(ui->r_answer->text().isEmpty()){
        QMessageBox::warning(this, "Error", "Please enter your answer.");
    }
    else{
        userRegister(ui->r_username->text(),
                     ui->r_password->text(),
                     ui->r_question->currentText().replace(" ","_"),
                     ui->r_answer->text());
    }
}

void MainWindow::on_usernameEdit_textChanged(const QString &arg1)
{
    if(ui->usernameEdit->text().isEmpty()){
        ui->forgotBtn->setEnabled(false);
    }
    else{
        ui->forgotBtn->setEnabled(true);
    }
}

void MainWindow::on_passwordEdit_textChanged(const QString &arg1)
{
    if(ui->usernameEdit->text().isEmpty()||ui->passwordEdit->text().length()<6){
        ui->loginBtn->setEnabled(false);
    }
    else{
        ui->loginBtn->setEnabled(true);
    }
}
void MainWindow::registerEnabled()
{
    if(ui->r_username->text().isEmpty() || ui->r_password->text().isEmpty() || ui->r_confirm->text().isEmpty() ||
            ui->r_question->currentText().isEmpty() || ui->r_answer->text().isEmpty()){
        ui->registerBtn->setEnabled(false);
    }
    else {
        ui->registerBtn->setEnabled(true);
    }
}

void MainWindow::on_forgotBtn_clicked()
{
    ui->label->setText("");
    if(!ui->usernameEdit->text().isEmpty()){
//        ui->stackedWidget->setCurrentIndex(3);
        tcpSocket->write(QString("find##%1").arg(handledString(ui->usernameEdit->text())).toUtf8());
        isFind = true;
    }

}

void MainWindow::on_f_send_clicked()
{
    if(ui->f_answer->text().isEmpty()){
        QMessageBox::warning(this, "Error", "Please enter your answer.");
    }
    else if(ui->f_newpw->text().isEmpty()){
        QMessageBox::warning(this, "Error", "Please enter your new password.");
    }
    else if(ui->f_confirm->text().isEmpty()){
        QMessageBox::warning(this, "Error", "Please confirm your password.");
    }
    else if(ui->f_confirm->text() != ui->f_newpw->text()){
        QMessageBox::warning(this, "Error", "The two passwords don't agree.");
    }
    else{
        m_answ = QString("answer##%1##").arg(handledString(ui->f_answer->text())).append(handledString(ui->f_newpw->text()));
        tcpSocket->write(m_answ.toUtf8());
    }
}

void MainWindow::on_f_cancel_clicked()
{
    ui->f_username->setText("");
    ui->f_question->setText("");
    ui->f_answer->setText("");
    ui->f_newpw->setText("");
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_c_file_send_clicked()
{
    sendFile();
}

void MainWindow::on_c_message_send_clicked()
{
    if(!ui->msgEdit->toPlainText().isEmpty()){
        ui->msgBrowser->append(QString("%1 I:").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
        ui->msgBrowser->append(QString("%1").arg(ui->msgEdit->toPlainText()));
        sendMessage(m_name, QString(ui->msgEdit->toPlainText()), recv_name);
//        logOutput(handledString(ui->msgEdit->toPlainText()));
//        logOutput(ui->msgEdit->toPlainText());
        qDebug()<<QString("%1").arg(ui->msgEdit->toPlainText());
//        qDebug()<<handledString(ui->msgEdit->toHtml());
        ui->msgEdit->setText("");
    }
    else{
        QMessageBox::warning(NULL, "", "Can not send empty message.");
    }
}
void MainWindow::online_click(QModelIndex index)
{
    recv_name = index.data().toString();
    recv_state = 1;
    ui->recver_label->setText(recv_name);
    ui->msgBrowser->append(QString("To %1").arg(recv_name));
    ui->c_file_send->setEnabled(true);
    ui->c_message_send->setEnabled(true);
}
void MainWindow::offline_click(QModelIndex index)
{
    recv_name = index.data().toString();
    recv_state = 0;
    ui->recver_label->setText(recv_name);
    ui->msgBrowser->append(QString("--To %1--").arg(recv_name));
    ui->c_file_send->setEnabled(false);
    ui->c_message_send->setEnabled(true);
}

void MainWindow::on_msgBrowser_textChanged()
{
    ui->msgBrowser->moveCursor(QTextCursor::End);
}
