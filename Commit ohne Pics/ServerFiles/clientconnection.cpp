#include "clientconnection.h"
#include <string>
#include <qvector.h>
#include <thread>
#include <qmutex.h>
#include <netinet/in.h>
#include <unistd.h>
#include <QString>
#include <QByteArray>
#include <iostream>
using namespace std;


ClientConnection::ClientConnection(int sock) : socket(sock) {}

void ClientConnection::sendMessage(const QString& msg) {
    const char* cmsg = msg.toUtf8().constData();

    send(socket, cmsg, msg.size(), 0);

}

QString ClientConnection::receiveMessage() {

    char buffer[1024];

    int bytes = recv(socket, buffer, sizeof(buffer)-1, 0);

    if (bytes > 0) {

        buffer[bytes] = '\0';

        return QString(buffer);

    }

    return "";

}
