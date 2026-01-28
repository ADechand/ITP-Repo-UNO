#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H
#include <qstring.h>

class ClientConnection
{
public:
    ClientConnection(int sock);
    void sendMessage(const QString& msg);
    QString receiveMessage();
    int socket;
    QString playerName;
};

#endif // CLIENTCONNECTION_H
