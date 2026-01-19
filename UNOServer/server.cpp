#include "server.h"
#include <string.h>
#include <qvector.h>
#include <thread>
#include <qmutex.h>
#include <netinet/in.h>
#include <unistd.h>
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

        cout << "Server läuft auf Port " << port << "...\n";

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

            cout << "Neuer Client verbunden!\n";

            thread(&Server::handleClient, this, client).detach();

        }

    }

}

void Server::handleClient(ClientConnection* client) {

    while (true) {

        string msg = client->receiveMessage();

        if (msg.empty()) {

            cout << "Client getrennt.\n";

            close(client->socket);

            break;

        }

        cout << "Nachricht erhalten: " << msg << "\n";

        broadcastMessage("Echo: " + msg);

    }

}

void Server::broadcastMessage(const string& msg) {

    lock_guard<QMutex> lock(clientMutex);

    for (auto* client : clients) {

        client->sendMessage(msg);

    }

}

void Server::shutdown() {

    close(serverSocket);

    cout << "Server heruntergefahren.\n";

}

