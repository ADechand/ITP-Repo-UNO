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
#include "gamemanager.h"
#include "clientconnection.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
using namespace std;

static QJsonObject makeErrorObj(const QString& message)
{
    QJsonObject o;
    o["type"] = "error";
    o["message"] = message;
    return o;
}

static QJsonObject makeOkObj(const QString& type, const QString& code)
{
    QJsonObject o;
    o["type"] = type;
    o["code"] = code;
    return o;
}

static void sendJson(ClientConnection* client, const QJsonObject& obj)
{
    const QByteArray line = QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n";
    client->sendMessage(QString::fromUtf8(line));
}


Server::Server(int p) : serverSocket(-1), port(p) {}

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

    thread(&Server::acceptConnections, this).detach();

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

void Server::handleClient(ClientConnection* client)
{
    while (true) {
        QString msg = client->receiveMessage();

        if (msg.isEmpty()) {
            qDebug() << "Client getrennt.\n";
            close(client->socket);
            break;
        }

        qDebug() << "Nachricht erhalten:" << msg;

        // Optional: echo/broadcast - kann bleiben, aber bei JSON nervig.
        // broadcastMessage("Echo: " + msg);

        // Wenn dein Client "\n" sendet, kann receiveMessage ggf. mehrere Zeilen liefern.
        const QStringList lines = msg.split('\n', Qt::SkipEmptyParts);

        for (const QString& line : lines) {

            QJsonParseError err;
            const QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &err);
            int serverSocket;

            if (err.error != QJsonParseError::NoError || !doc.isObject()) {
                sendJson(client, makeErrorObj("Invalid JSON"));
                continue; // nicht return; (sonst killst du die ganze Client-Verbindung)
            }

            const QJsonObject obj = doc.object();
            const QString type = obj.value("type").toString().trimmed();

            if (type.isEmpty()) {
                sendJson(client, makeErrorObj("Missing field: type"));
                continue;
            }

            // --- HOST CODE: Spiel erstellen oder joinen ---
            if (type == "host_code") {
                const QString code = obj.value("code").toString().trimmed().toUpper();

                if (code.isEmpty()) {
                    sendJson(client, makeErrorObj("Missing field: code"));
                    continue;
                }

                // 1) Existiert Spiel?
                Game* g = m_gameManager.getGame(code);   // <-- Punkt, kein ->
                if (!g) {
                    // 2) Spiel erstellen (Host)
                    m_gameManager.createGame(code, client);
                    sendJson(client, makeOkObj("host_ok", code));
                } else {
                    // 3) Spiel existiert: Client beitreten lassen
                    const bool ok = m_gameManager.addPlayer(code, client);
                    if (!ok) {
                        sendJson(client, makeErrorObj("Join failed"));
                    } else {
                        sendJson(client, makeOkObj("join_ok", code));
                    }
                }

                continue;
            }

            // Optional sauberer Join-Typ
            if (type == "join_code") {
                const QString code = obj.value("code").toString().trimmed().toUpper();
                if (code.isEmpty()) {
                    sendJson(client, makeErrorObj("Missing field: code"));
                    continue;
                }

                if (!m_gameManager.getGame(code)) {
                    sendJson(client, makeErrorObj("Game not found"));
                    continue;
                }

                if (!m_gameManager.addPlayer(code, client)) {
                    sendJson(client, makeErrorObj("Join failed"));
                    continue;
                }

                // nachher
                Game* createGame(const QString& code, ClientConnection* host);
                bool addPlayer(const QString& code, ClientConnection* player);


                sendJson(client, makeOkObj("join_ok", code));
                continue;
            }

            sendJson(client, makeErrorObj("Unknown type: " + type));
        }
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

