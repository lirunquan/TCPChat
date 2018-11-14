#ifndef MYSQL_H
#define MYSQL_H
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QDebug>

class MySql
{
public:
    MySql();
    void initSql();
    void createTable();
    void createDatabase();
    int login(QString username, QString password);
    int signup(QString username, QString password);
private:
    QSqlQuery *query;
};

#endif // MYSQL_H
