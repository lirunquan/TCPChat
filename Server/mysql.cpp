#include "mysql.h"

MySql::MySql()
{

}
void MySql::initSql()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "ChatConnection");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setUserName("root");
    db.setPassword("12345678");
//    db.setDatabaseName("ChatUser");
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
    query = new QSqlQuery;
    query->
}
void MySql::createTable()
{

}
bool MySql::login(QString username, QString password)
{

}
bool MySql::signup(QString username, QString password)
{

}
