#pragma once
#include <QString>
#include <QHash>
#include <QSet>
#include <QMutex>

class ClientConnection;

struct Game {
    QString code;
    ClientConnection* host = nullptr;
    QSet<ClientConnection*> players;
};

class GameManager {
public:
    GameManager() = default;

    Game* getGame(const QString& code);
    void createGame(const QString& code, ClientConnection* host);
    bool addPlayer(const QString& code, ClientConnection* player);

private:
    QHash<QString, Game*> games;
    QMutex mutex;
};
