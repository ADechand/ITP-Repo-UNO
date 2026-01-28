#include "server.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QRandomGenerator>

Server::Server(QObject* parent) : QObject(parent)
{
    connect(&m_server, &QTcpServer::newConnection, this, &Server::onNewConnection);

    const quint16 port = 12345;
    if (!m_server.listen(QHostAddress::Any, port)) {
        qFatal("Server listen failed");
    }
}

void Server::onNewConnection()
{

    while (m_server.hasPendingConnections()) {
        QTcpSocket* sock = m_server.nextPendingConnection();
        qInfo() << "[NET] Client connected from"
                << sock->peerAddress().toString() << ":" << sock->peerPort();

        connect(sock, &QTcpSocket::readyRead, this, [this, sock]() { onReadyRead(sock); });
        connect(sock, &QTcpSocket::disconnected, this, [this, sock]() { onDisconnected(sock); });

        m_buffers.insert(sock, QByteArray());
    }
}

void Server::onDisconnected(QTcpSocket* sock)
{
    m_buffers.remove(sock);

    const QString code = m_socketToGame.take(sock);
    if (!code.isEmpty() && m_games.contains(code)) {
        GameState& g = m_games[code];
        g.players.removeAll(sock);
        g.hands.remove(sock);
        if (g.host == sock) g.host = nullptr;

        if (g.players.isEmpty())
            m_games.remove(code);
    }
    qInfo() << "[NET] Client disconnected";

    sock->deleteLater();
}

void Server::onReadyRead(QTcpSocket* sock)
{
    QByteArray& buf = m_buffers[sock];
    buf += sock->readAll();

    while (true) {
        const int nl = buf.indexOf('\n');
        if (nl < 0) break;

        const QByteArray line = buf.left(nl).trimmed();
        buf.remove(0, nl + 1);

        if (line.isEmpty()) continue;

        QJsonParseError err;
        const QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            sendJson(sock, QJsonObject{{"type","error"},{"message","Invalid JSON"}});
            continue;
        }

        handleMessage(sock, doc.object());
    }
}

void Server::handleMessage(QTcpSocket* sock, const QJsonObject& msg)
{
    const QString type = msg.value("type").toString();

    qInfo() << "[RX]" << msg;

    if (type == "create_game") {
        createGame(sock);
        return;
    }

    if (type == "join_game") {
        const QString code = msg.value("code").toString().trimmed().toUpper();
        if (code.isEmpty()) {
            sendJson(sock, QJsonObject{{"type","error"},{"message","Missing code"}});
            return;
        }
        joinGame(sock, code);
        return;
    }

    if (type == "start_game") {
        const QString code = msg.value("code").toString().trimmed().toUpper();
        if (code.isEmpty()) {
            sendJson(sock, QJsonObject{{"type","error"},{"message","Missing code"}});
            return;
        }
        startGame(sock, code);
        return;
    }

    sendJson(sock, QJsonObject{{"type","error"},{"message","Unknown message type"}});
}

void Server::sendJson(QTcpSocket* sock, const QJsonObject& obj)
{
    const QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n";
    sock->write(payload);
    sock->flush();
}

QString Server::createCode() const
{
    static const char alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    QString code;
    code.reserve(4);
    for (int i = 0; i < 4; ++i) {
        const int idx = QRandomGenerator::global()->bounded(int(sizeof(alphabet) - 1));
        code.append(alphabet[idx]);
    }
    return code;
}

GameState* Server::getGame(const QString& code)
{
    if (!m_games.contains(code)) return nullptr;
    return &m_games[code];
}

void Server::createGame(QTcpSocket* hostSock)
{

    if (m_socketToGame.contains(hostSock)) {
        sendJson(hostSock, QJsonObject{{"type","error"},{"message","Already in a game"}});
        return;
    }

    QString code;
    for (int tries = 0; tries < 20; ++tries) {
        code = createCode();
        if (!m_games.contains(code)) break;
    }
    if (m_games.contains(code)) {
        sendJson(hostSock, QJsonObject{{"type","error"},{"message","Could not create code"}});
        return;
    }
    qInfo() << "[GAME] created code" << code << "host=" << hostSock;

    GameState g;
    g.code = code;
    g.host = hostSock;
    g.players = { hostSock };

    m_games.insert(code, g);
    m_socketToGame.insert(hostSock, code);

    sendJson(hostSock, QJsonObject{{"type","game_created"},{"code",code}});
}

void Server::joinGame(QTcpSocket* sock, const QString& code)
{
    if (m_socketToGame.contains(sock)) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Already in a game"}});
        return;
    }

    GameState* g = getGame(code);
    if (!g) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Game not found"}});
        return;
    }
    if (g->started) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Game already started"}});
        return;
    }

    if (!g->players.contains(sock))
        g->players.append(sock);

    m_socketToGame.insert(sock, code);

    sendJson(sock, QJsonObject{{"type","join_ok"},{"code",code}});

    qInfo() << "[GAME]" << code << "player joined, total=" << g->players.size();
}

void Server::shuffle(QStringList& list) const
{
    for (int i = list.size() - 1; i > 0; --i) {
        const int j = QRandomGenerator::global()->bounded(i + 1);
        list.swapItemsAt(i, j);
    }
}

QStringList Server::buildDeckFromStaticList() const
{
    // Für den Start: Liste muss zu deinen Dateinamen passen.
    // Besser: später in cards.json auslagern oder echte UNO-IDs verwenden.
    return QStringList{
        "Rot 1.png","Rot 2.png","Rot 3.png","Rot 4.png","Rot 5.png","Rot 6.png","Rot 7.png","Rot 8.png","Rot 9.png",
        "Gruen 1.png","Gruen 2.png","Gruen 3.png","Gruen 4.png","Gruen 5.png","Gruen 6.png","Gruen 7.png","Gruen 8.png","Gruen 9.png",
        "Blau 1.png","Blau 2.png","Blau 3.png","Blau 4.png","Blau 5.png","Blau 6.png","Blau 7.png","Blau 8.png","Blau 9.png",
        "Gelb 1.png","Gelb 2.png","Gelb 3.png","Gelb 4.png","Gelb 5.png","Gelb 6.png","Gelb 7.png","Gelb 8.png","Gelb 9.png",
        "Rot Sperre.png","Gruen Sperre.png","Blau Sperre.png","Gelb Sperre.png",
        "Rot Richtungswechsel.png","Gruen Richtungswechsel.png","Blau Richtungswechsel.png","Gelb Richtungswechsel.png",
        "Extra Farbwechsel.png","Extra +4.png"
    };
}

void Server::startGame(QTcpSocket* sock, const QString& code)
{
    GameState* g = getGame(code);
    if (!g) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Game not found"}});
        return;
    }
    if (g->host != sock) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Only host can start"}});
        return;
    }
    if (g->started) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Game already started"}});
        return;
    }

    g->deck = buildDeckFromStaticList();
    if (g->deck.size() < (g->players.size() * 6 + 1)) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Not enough cards in deck list"}});
        return;
    }
    shuffle(g->deck);

    g->hands.clear();
    g->discard.clear();

    // 6 Karten pro Spieler
    for (QTcpSocket* p : g->players) {
        QStringList hand;
        for (int i = 0; i < 6; ++i) {
            hand.append(g->deck.takeLast());
        }
        g->hands.insert(p, hand);
    }

    // erste Ablagekarte
    g->discard.append(g->deck.takeLast());
    g->started = true;

    // an alle senden
    const int players = g->players.size();
    const QString discardTop = g->discard.last();
    const int drawCount = g->deck.size();

    for (int i = 0; i < g->players.size(); ++i) {
        QTcpSocket* p = g->players[i];

        QJsonArray handArr;
        for (const QString& c : g->hands.value(p))
            handArr.append(c);

        QJsonObject init{
            {"type","game_init"},
            {"code",code},
            {"players",players},
            {"yourIndex",i},
            {"discardTop",discardTop},
            {"drawCount",drawCount},
            {"hand",handArr}
        };

        sendJson(p, init);
    }
}
