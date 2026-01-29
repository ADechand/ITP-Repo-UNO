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
    qInfo() << "[NET] Server listening on port" << port;
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

    if (type == "draw_cards") {
        const int count = msg.value("count").toInt(1);
        drawCards(sock, count);
        return;
    }

    sendJson(sock, QJsonObject{{"type","error"},{"message","Unknown message type"}});
}

void Server::drawCards(QTcpSocket* sock, int count)
{
    if (count < 1) count = 1;
    if (count > 10) count = 10;

    const QString code = m_socketToGame.value(sock);
    if (code.isEmpty()) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Not in a game"}});
        return;
    }
    GameState* g = getGame(code);
    if (!g || !g->started) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Game not started"}});
        return;
    }
    if (!g->players.contains(sock)) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Not a player"}});
        return;
    }

    QStringList& hand = g->hands[sock];
    QJsonArray cardsArr;

    for (int i = 0; i < count; ++i) {
        if (g->deck.isEmpty()) break;
        const QString card = g->deck.takeLast();
        hand.append(card);
        cardsArr.append(card);
    }

    if (cardsArr.isEmpty()) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Deck is empty"}});
        return;
    }

    sendJson(sock, QJsonObject{
                       {"type","cards_drawn"},
                       {"cards",cardsArr},
                       {"drawCount",g->deck.size()}
                   });

    qInfo() << "[GAME]" << code << "draw_cards count=" << cardsArr.size()
            << "remaining=" << g->deck.size();
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
    // Einheitliches Naming:
    // - Unterstriche statt Leerzeichen
    // - Dateiendungen exakt wie im assets/images/cards Ordner

    return QStringList{
        // Rot
        "Rot_1.jpg","Rot_2.jpg","Rot_3.jpg","Rot_4.jpg","Rot_5.jpg","Rot_6.jpg","Rot_7.jpg","Rot_8.jpg","Rot_9.jpg",

        // Gruen
        "Gruen_1.jpg","Gruen_2.jpg","Gruen_3.jpg","Gruen_4.jpg","Gruen_5.jpg","Gruen_6.jpg","Gruen_7.jpg","Gruen_8.jpg","Gruen_9.jpg",

        // Blau
        "Blau_1.jpg","Blau_2.jpg","Blau_3.jpg","Blau_4.jpg","Blau_5.jpg","Blau_6.jpg","Blau_7.jpg","Blau_8.jpg","Blau_9.jpg",

        // Gelb
        "Gelb_1.jpg","Gelb_2.jpg","Gelb_3.jpg","Gelb_4.jpg","Gelb_5.jpg","Gelb_6.jpg","Gelb_7.jpg","Gelb_8.jpg","Gelb_9.jpg",

        // Sperre
        "Rot_Sperre.jpg","Gruen_Sperre.jpg","Blau_Sperre.jpg","Gelb_Sperre.jpg",

        // Richtungswechsel
        "Rot_Richtungswechsel.jpg","Gruen_Richtungswechsel.jpg",
        "Blau_Richtungswechsel.jpg","Gelb_Richtungswechsel.jpg",

        // Spezialkarten
        "Extra_Farbwechsel.jpg",
        "Extra_4plus.jpg"
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

    for (QTcpSocket* p : g->players) {
        QStringList hand;
        for (int i = 0; i < 6; ++i)
            hand.append(g->deck.takeLast());
        g->hands.insert(p, hand);
    }

    g->discard.append(g->deck.takeLast());
    g->started = true;

    const int players = g->players.size();
    const QString discardTop = g->discard.last();
    const int drawCount = g->deck.size();

    qInfo() << "[GAME]" << code << "STARTED players=" << players
            << "discardTop=" << discardTop
            << "drawCount=" << drawCount;

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
