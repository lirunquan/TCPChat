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
    bool login(QString username, QString password);
    bool signup(QString username, QString password);
};

#endif // MYSQL_H
