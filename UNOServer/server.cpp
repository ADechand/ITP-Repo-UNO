#include "server.h"
#include <string.h>
#include <qvector.h>
#include <thread>
#include <qmutex.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <QDebug>
using namespace std;

Server::Server(int p) : port(p), serverSocket(-1) {}

bool Server::start() {

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);


    if (serverSocket < 0) {

        cerr << "Fehler: Socket konnte nicht erstellt werden\n";

        return false;

    }

    sockaddr_in serverAddr{};

    serverAddr.sin_family = AF_INET;

    serverAddr.sin_addr.s_addr = INADDR_ANY;

    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {

        cerr << "Fehler: Bind fehlgeschlagen\n";

        return false;

    }

    if (listen(serverSocket, 5) < 0) {

        cerr << "Fehler: Listen fehlgeschlagen\n";

        return false;

    }

    qDebug() << "Server läuft auf Port " << port << "...\n";

    std::thread(&Server::acceptConnections, this).detach();

    return true;

}

void Server::acceptConnections() {

    while (true) {

        sockaddr_in clientAddr{};

        socklen_t clientLen = sizeof(clientAddr);

        int clientSock = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);

        if (clientSock >= 0) {

            lock_guard<QMutex> lock(clientMutex);

            auto* client = new ClientConnection(clientSock);

            clients.push_back(client);

            qDebug() << "Neuer Client verbunden!\n";

            thread(&Server::handleClient, this, client).detach();

        }

    }

}

void Server::handleClient(ClientConnection* client) {

    while (true) {

        QString msg = client->receiveMessage();

        if (msg.isEmpty()) {

            qDebug() << "Client getrennt.\n";

            close(client->socket);

            break;

        }
        string stdStr = msg.toStdString();
        qDebug() << "Nachricht erhalten: " << stdStr << "\n";

        broadcastMessage("Echo: " + msg);

    }

}

void Server::broadcastMessage(const QString& msg) {

    lock_guard<QMutex> lock(clientMutex);

    for (auto* client : clients) {

        client->sendMessage(msg);

    }

}

void Server::shutdown() {

    close(serverSocket);

    qDebug() << "Server heruntergefahren.\n";

}

