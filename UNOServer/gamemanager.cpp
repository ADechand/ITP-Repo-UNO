#include "gamemanager.h"
#include "clientconnection.h"

#include <QMutexLocker>
#include <QDir>
#include <QFileInfoList>
#include <QRandomGenerator>

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

static bool isPngJpg(const QString& name)
{
    const QString n = name.toLower();
    return n.endsWith(".png") || n.endsWith(".jpg") || n.endsWith(".jpeg");
}

QStringList GameManager::loadDeckFromFolder(QString* error) const
{
    // Erwartet einen Ordner "cards" im Arbeitsverzeichnis des Servers:
    // UNOServer/build/.../cards/*.png
    // -> Leg dort deine PNGs rein (oder kopier den cards-Ordner neben die .exe)
    QDir dir(QDir::currentPath() + "/cards");
    if (!dir.exists()) {
        if (error) *error = "Server: cards-Ordner nicht gefunden: " + dir.absolutePath();
        return {};
    }

    const QFileInfoList files = dir.entryInfoList(QDir::Files, QDir::Name);
    QStringList deck;
    for (const QFileInfo& fi : files) {
        if (isPngJpg(fi.fileName()))
            deck.push_back(fi.fileName()); // wir senden nur den Dateinamen an den Client
    }

    if (deck.isEmpty()) {
        if (error) *error = "Server: cards-Ordner ist leer oder keine png/jpg gefunden.";
    }
    return deck;
}

void GameManager::shuffle(QStringList& list) const
{
    // Fisher-Yates
    for (int i = list.size() - 1; i > 0; --i) {
        const int j = QRandomGenerator::global()->bounded(i + 1);
        list.swapItemsAt(i, j);
    }
}

bool GameManager::startGame(const QString& code, QString* error)
{
    QMutexLocker lock(&mutex);

    Game* g = games.value(code, nullptr);
    if (!g) {
        if (error) *error = "Game not found";
        return false;
    }
    if (g->started) {
        if (error) *error = "Game already started";
        return false;
    }
    if (g->players.size() < 1) {
        if (error) *error = "No players";
        return false;
    }

    QString deckErr;
    QStringList deck = loadDeckFromFolder(&deckErr);
    if (deck.isEmpty()) {
        if (error) *error = deckErr;
        return false;
    }

    shuffle(deck);

    g->deck = deck;
    g->discard.clear();
    g->hands.clear();

    // 6 Karten pro Spieler
    for (ClientConnection* p : g->players) {
        QStringList hand;
        for (int i = 0; i < 6; ++i) {
            if (g->deck.isEmpty()) break;
            hand.push_back(g->deck.takeLast());
        }
        g->hands.insert(p, hand);
    }

    // erste Karte auf Ablage (top discard)
    if (g->deck.isEmpty()) {
        if (error) *error = "Deck empty after dealing";
        return false;
    }
    g->discard.push_back(g->deck.takeLast());

    g->started = true;
    return true;
}
