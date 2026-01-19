#ifndef SERVER_H
#define SERVER_H
#include <qvector.h>
#include <qmutex.h>
#include <string.h>
#include <clientconnection.h>

class Server
{
private:
    int serverSocket;
    int port;
    QVector<ClientConnection*> clients;
    QMutex clientMutex;
public:
    Server(int p);
    bool tart();
    void acceptConnections();
    void handleClient(ClientConnection* client);
    void broadcastMessage(const string& msg);
    void shutdown();
};

#endif // SERVER_H
