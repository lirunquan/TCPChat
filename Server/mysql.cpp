#include "mysql.h"

MySql::MySql()
{

}
void MySql::initSql()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setUserName("root");
    db.setPassword("12345678");
    db.setDatabaseName("Chat");
    if(db.open()){
        qDebug()<<"Database opened successfully.";
        createTable();
        return;
    }
    else{
        qDebug()<<"Database opened failed.";
        createDatabase();
        return;
    }
}
void MySql::createDatabase()
{
    query = new QSqlQuery();
    bool success = query->exec("create database if not exists Chat");
    if(!success){
        qDebug() << "create databse failed.";
        return ;
    }
}
void MySql::createTable()
{
    query = new QSqlQuery();
    bool success = query->exec("create table if not exists users(username VARCHAR(30) primary key unique not null, password VARCHAR(30) not null");
    if(!success){
        qDebug() << "create table failed.";
        return ;
    }
    success = query->exec("insert ignore into users value('root', '123456')");
    if(!success){
        qDebug() << "create root user failed.";
        return ;
    }
}
int MySql::login(QString username, QString password)
{
    query = new QSqlQuery();
    QString string = QString("select * from users where username='%1' and password='%2'").arg(username).arg(password);
//    query->exec(string);
    query->exec(string);
    query->last();
    int record = query->at()+1;
    if(record==0){
        return -1;
    }
    return 1;
}
int MySql::signup(QString username, QString password)
{
    query = new QSqlQuery();
    QString string = QString("select * from users where username='%1'").arg(username);
    query->exec(string);
    query->last();
    int record = query->at()+1;
    if(record!=0){
        return -1;
    }
    string = QString("insert into users value('%1', '%2')").arg(username).arg(password);
    if(!query->exec(string)){
        return 0;
    }
    return 1;
}
