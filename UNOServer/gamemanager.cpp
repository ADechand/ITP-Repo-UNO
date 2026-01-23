#include "gamemanager.h"
#include "clientconnection.h"
#include <QMutexLocker>

Game* GameManager::getGame(const QString& code)
{
    QMutexLocker lock(&mutex);
    return games.value(code, nullptr);
}

void GameManager::createGame(const QString& code, ClientConnection* host)
{
    QMutexLocker lock(&mutex);

    if (games.contains(code))
        return;

    Game* g = new Game();
    g->code = code;
    g->host = host;
    g->players.insert(host);

    games.insert(code, g);
}

bool GameManager::addPlayer(const QString& code, ClientConnection* player)
{
    QMutexLocker lock(&mutex);

    Game* g = games.value(code, nullptr);
    if (!g) return false;

    g->players.insert(player);
    return true;
}
