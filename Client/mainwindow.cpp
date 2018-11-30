#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "user.h"
#include "filetransmit.h"
#include "senddialog.h"
#include "receivedialog.h"
#include <QMessageBox>
#include <QKeyEvent>
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
    ui->fontComboBox->setVisible(true);
    ui->boldButton->setVisible(true);
    ui->italicButton->setVisible(true);
    ui->underlineButton->setVisible(true);
    ui->comboBox->setVisible(true);
    ui->chatWidget->setVisible(false);
    ui->c_message_send->setEnabled(false);
    ui->c_file_send->setEnabled(false);
    ui->f_cancel->setVisible(false);
    ui->stackedWidget->setCurrentIndex(0);
    ui->msgEdit->setFocusPolicy(Qt::StrongFocus);
    ui->msgEdit->setFocus();
    ui->msgEdit->installEventFilter(this);
    connect(ui->c_on_list, SIGNAL(clicked(QModelIndex)), this, SLOT(online_click(QModelIndex)));
    connect(ui->c_off_list, SIGNAL(clicked(QModelIndex)), this, SLOT(offline_click(QModelIndex)));
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
    tcpSocket->connectToHost(QHostAddress(IP), PORT);
    tcpSocket->write("##ChatMode");//send tcp connect request for login
    ui->label->setText("Connecting to server...");
    connect(tcpSocket, &QTcpSocket::connected, [=](){
        //tips for connected success
        logOutput("connected success");
        ui->label->setText("正在连接服务器....连接成功！");
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
                else if("login failed" == QString(buffer).section("##",1,1)){
                    //window hints the "login/register failed"
                    ui->statusBar->showMessage("Login failed");
                    ui->stackedWidget->setCurrentIndex(1);
                    ui->usernameEdit->setText("");
                    ui->passwordEdit->setText("");
                    tcpSocket->write("##Request for login");
                    mode[0] = Chat;
                }
                else if("register failed" == QString(buffer).section("##",1,1)){
                    //window hints the "login/register failed"
                    ui->statusBar->showMessage("Login failed");
                    ui->stackedWidget->setCurrentIndex(1);
                    ui->usernameEdit->setText("");
                    ui->passwordEdit->setText("");
                    tcpSocket->write("##Request for login");
                    mode[0] = Chat;
                }
                else{
                    logOutput("wrong message from server");
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
                qDebug() << "accept contact" <<endl;
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
                        qDebug() << myIP << " " << myPort << endl;
                        ip_recv = myIP;
                        sendOrReceiver = true;
                        SendDialog* dialog = new SendDialog(this);
                        dialog->show();
                    }
                }
                else{
                    QMessageBox::information(NULL, "Warning", "Received wrong connection request.");
                }
            }
            else if("RefuseContact" == QString(buffer).section("##",1,1)){
                QMessageBox::information(this, "Refuse", "He refused contacting.");
            }
            mode[0] = Chat;
        }
        else if("RequestForContact" == QString(buffer).section("##",1,1)){//A##RequestForContact##B  A wants to contact B
            if(m_name != readString(QString(buffer).section("##",2,2))){
                //show wrong request message in the window
                mode[0] = Chat;
            }
            else{
                QString c_sender = readString(QString(buffer).section("##",0,0));
                if(QMessageBox::Yes == QMessageBox::information(this, "Contact", QString("%1 wants to send a file to you,\nDo you accept?").arg(c_sender), QMessageBox::Yes, QMessageBox::No)){
                    tcpSocket->write(QString("%1##AcceptContact##%2").arg(handledString(m_name)).arg(handledString(c_sender)).toUtf8());
                    sendOrReceiver = false;
                    ReceiveDialog* dialog = new ReceiveDialog(this);
                    dialog->show();
                    mode[0] = Chat;
                }
                else{
                    tcpSocket->write(QString("%1##RefuseContact##%2").arg(handledString(m_name)).arg(handledString(c_sender)).toUtf8());
                    mode[0] = Chat;
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
                        ip_recv = u_ip;
                        port_num = u_port;
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
                for(int m=2; m<num+2; m++){
                    QString single = QString(buffer).section("&&",m,m);
                    QString sender = readString(QString(single).section("##",0,0));//who sended
                    QString message_off = readString(QString(single).section("##",1,1)) ;//content of message
                    QString reciever = readString(QString(single).section("##",2,2)) ;//who will recieve
                    QString time_sent = QString(single).section("##",3,3);
                    str += QString("%1\n%2: \n%3\n").arg(time_sent).arg(sender).arg(message_off);
                    qDebug()<<str;
                }
                QMessageBox::about(NULL, QString("%1 offline message").arg(num), str);
            }
        }
        else if("##Permission for login" == QString(buffer)){
            mode[0] = Login;
            ui->label->setText("登录");
            ui->label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
            ui->frame_2->setVisible(true);
            ui->stackedWidget->setCurrentIndex(1);
            setWindowTitle("TCPChat--Login");
            ui->loginBtn->setEnabled(false);
            ui->forgotBtn->setEnabled(false);
        }
        else if("##ChatMode, waiting request" == QString(buffer)){
            tcpSocket->write("##Request for login");
        }
        else if("##Logout" == QString(buffer)){
            m_name = "";
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
            ui->msgBrowser->append(QString("%1 %2:").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(sender));
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
    if(tcpSocket){
        delete tcpSocket;
        tcpSocket = NULL;
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
    tcpSocket->write(QString("%1##RequestForContact##%2").arg(m_name).arg(recv_name).toUtf8());
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
    this->setGeometry (100,100,950,640);
    QMenu* menu = ui->menuBar->addMenu("菜单");
    QAction* logout = menu->addAction("账号下线");
    QAction* exit = menu->addAction("退出");
    connect(logout, &QAction::triggered, [=](){
        if(QMessageBox::Yes == QMessageBox::information(this, "Logout", "是否确定下线？", QMessageBox::Yes, QMessageBox::No)){
            tcpSocket->write(QString("logout##%1").arg(handledString(m_name)).toUtf8());
        }
    });
    connect(exit, &QAction::triggered, [=](){
        if(QMessageBox::Yes == QMessageBox::information(this, "Exit", "是否确定退出？", QMessageBox::Yes, QMessageBox::No)){
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
}


void MainWindow::on_toRegisterBtn_clicked()
{
    setWindowTitle("TCPChat--Register");
    ui->label->setText("注册");
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
    QChar c = ui->r_username->text().at(0);
    if(ui->r_username->text().isEmpty()){
        QMessageBox::warning(this, "错误", "用户名为空！");
    }
    else if(!c.isLetterOrNumber()){
        QMessageBox::warning(this, "错误", "无效用户名！");
        ui->r_username->setText("");
        ui->r_password->setText("");
    }
    else if(ui->r_password->text().length() <= 5){
        QMessageBox::warning(this, "错误", "密码要求六位以上！");
        ui->r_password->setText("");
        ui->r_confirm->setText("");
    }
    else if(ui->r_confirm->text().isEmpty() || ui->r_confirm->text() != ui->r_password->text()){
        QMessageBox::warning(this, "错误", "请确认你的密码！");
        ui->r_password->setText("");
        ui->r_confirm->setText("");
    }
    else if(ui->r_answer->text().isEmpty()){
        QMessageBox::warning(this, "错误", "请输入你的问题答案！");
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
        tcpSocket->write(QString("find##%1").arg(handledString(ui->usernameEdit->text())).toUtf8());
        isFind = true;
    }

}

void MainWindow::on_f_send_clicked()
{
    if(ui->f_answer->text().isEmpty()){
        QMessageBox::warning(this, "错误", "请输入你的问题答案！");
    }
    else if(ui->f_newpw->text().isEmpty()){
        QMessageBox::warning(this, "错误", "请输入你的新密码！");
    }
    else if(ui->f_confirm->text().isEmpty()){
        QMessageBox::warning(this, "错误", "请确认你的密码！");
    }
    else if(ui->f_confirm->text() != ui->f_newpw->text()){
        QMessageBox::warning(this, "错误", "两次密码输入不一致");
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
bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    if(target == ui->msgEdit)
    {
        if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *k = static_cast<QKeyEvent*>(event);
            if(k->key()==Qt::Key_Return||k->key()==Qt::Key_Enter){
                on_c_message_send_clicked();
                return true;
            }
        }
    }
    return QWidget::eventFilter(target,event);
}

void MainWindow::on_c_message_send_clicked()
{
    if(!ui->msgEdit->toPlainText().isEmpty()){
        ui->msgBrowser->append(QString("%1 I:").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
        ui->msgBrowser->append(QString("%1").arg(ui->msgEdit->toPlainText()));
        sendMessage(m_name, QString(ui->msgEdit->toPlainText()), recv_name);
        qDebug()<<QString("%1").arg(ui->msgEdit->toPlainText());
        ui->msgEdit->setText("");
    }
    else{
        QMessageBox::warning(NULL, "", "不能发送空消息！");
    }
}
void MainWindow::online_click(QModelIndex index)
{
    recv_name = index.data().toString();
    recv_state = 1;
    QString s = "--To "+recv_name+"--";
    if(recv_name != ui->recver_label->text()){
        ui->msgBrowser->append(s);
    }
    ui->recver_label->setText(recv_name);
    ui->c_file_send->setEnabled(true);
    ui->c_message_send->setEnabled(true);
    ui->c_off_list->clearSelection();
}
void MainWindow::offline_click(QModelIndex index)
{
    recv_name = index.data().toString();
    recv_state = 0;
    QString s = "--To "+recv_name+"--";
    if(recv_name != ui->recver_label->text()){
        ui->msgBrowser->append(s);
    }
    ui->recver_label->setText(recv_name);
    ui->c_file_send->setEnabled(false);
    ui->c_message_send->setEnabled(true);
    ui->c_on_list->clearSelection();
}

void MainWindow::on_msgBrowser_textChanged()
{
    ui->msgBrowser->moveCursor(QTextCursor::End);
}



void MainWindow::on_boldButton_clicked(bool checked)
{
    if(checked)
        ui->msgEdit->setFontWeight (QFont::Bold);
    else
        ui->msgEdit->setFontWeight (QFont::Normal);
    ui->msgEdit->setFocus ();
}

void MainWindow::on_italicButton_clicked(bool checked)
{

        ui->msgEdit->setFontItalic (checked);

    ui->msgEdit->setFocus ();
}

void MainWindow::on_underlineButton_clicked(bool checked)
{

        ui->msgEdit->setFontUnderline (checked);

    ui->msgEdit->setFocus ();
}

void MainWindow::on_comboBox_currentIndexChanged(const QString &arg1)
{
    ui->msgEdit->setFontPointSize (arg1.toDouble ());
    ui->msgEdit->setFocus ();
}

void MainWindow::on_fontComboBox_currentFontChanged(const QFont &f)
{
    ui->msgEdit->setCurrentFont (f);
    ui->msgEdit->setFocus ();
}
