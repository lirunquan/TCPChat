#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>

class Session : public QObject
{
    Q_OBJECT
public:
    explicit Session(QObject *parent = nullptr);
    void ManageJson();
    void Login(QString username, QString password);
    void SignUp(QString username, QString password);
    void SendMessage(QString message);
    void LoginSuccess();
    void LoginFailed();
    void SignUpSuccess();
    void SignUpFailed();
signals:

public slots:
private:
    QTcpSocket _sokect;
};

#endif // SESSION_H
