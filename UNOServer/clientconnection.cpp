#include "clientconnection.h"
#include <string.h>
#include <qvector.h>
#include <thread>
#include <qmutex.h>
#include <netinet/in.h>
#include <unistd.h>
using namespace std;


ClientConnection::ClientConnection(int sock) : socket(sock) {}

void ClientConnection::sendMessage(const QString& msg) {

    send(socket, msg.c_str(), msg.size(), 0);

}

string ClientConnection::receiveMessage() {

    char buffer[1024];

    int bytes = recv(socket, buffer, sizeof(buffer)-1, 0);

    if (bytes > 0) {

        buffer[bytes] = '\0';

        return string(buffer);

    }

    return "";

}
