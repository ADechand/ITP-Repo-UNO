#pragma once
#include <QString>
#include <QHash>
#include <QSet>
#include <QMutex>
#include <QStringList>

class ClientConnection;

struct Game {
    QString code;
    ClientConnection* host = nullptr;
    QSet<ClientConnection*> players;

    bool started = false;

    // Karten (als Dateinamen oder IDs)
    QStringList deck;        // draw pile (oben = last)
    QStringList discard;     // discard pile (oben = last)
    QHash<ClientConnection*, QStringList> hands; // pro Spieler 6 Karten
};

class GameManager {
public:
    GameManager() = default;

    Game* getGame(const QString& code);
    void createGame(const QString& code, ClientConnection* host);
    bool addPlayer(const QString& code, ClientConnection* player);

    // NEU:
    bool startGame(const QString& code, QString* error);

private:
    QStringList loadDeckFromFolder(QString* error) const;
    void shuffle(QStringList& list) const;

    QHash<QString, Game*> games;
    QMutex mutex;
};
