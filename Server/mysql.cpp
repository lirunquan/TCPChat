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
    db.setDatabaseName("ChatUser");
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
    QSqlQuery query;
    query.exec("create database if not exists chat");
}
void MySql::createTable()
{
    QSqlQuery query;
    query.exec("create table if not exists users(username VARCHAR(30) primary key unique not null, password VARCHAR(30) not null");
    query.exec("insert ignore into users value('root', '123456')");
}
bool MySql::login(QString username, QString password)
{

}
bool MySql::signup(QString username, QString password)
{

}
