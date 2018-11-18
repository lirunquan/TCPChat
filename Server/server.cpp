#include "server.h"
#include "messagequeue.h"
#include "user.h"

#include <QByteArray>
#include <QTime>
int cur;

struct user_data
{
    User* u[M];
    int size;
}*User_data;

MessageQueue* Message;
bool isFirst = false;
int mode[M];
enum{
    AcceptLogin = 0,
    Chat
};
int indexOf;

Server::Server(QObject *parent) : QObject(parent)
{
    User_data = new user_data();
    for(int i=0;i<M;i++){
        User_data->u[i] = NULL;
        mode[i] = Chat;
    }
    indexOf = 0;
    User_data->size = 0;
    cur = 0;
    Message = NULL;
    isFirst = true;
    userLoaded();
    userStateUpdate();

}
void Server::userLoaded()
{

}
