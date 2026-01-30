#include "server.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QDateTime>

namespace {
struct CardInfo {
    QString color;
    QString value;
    bool isWild = false;
};

CardInfo parseCardInfo(const QString& cardName)
{
    QString base = cardName;
    const int dot = base.lastIndexOf('.');
    if (dot >= 0)
        base = base.left(dot);

    const QStringList parts = base.split('_');
    CardInfo info;
    if (parts.isEmpty())
        return info;

    if (parts.first() == "Extra") {
        info.isWild = true;
        info.color = "Extra";
        info.value = parts.mid(1).join("_");
        return info;
    }

    info.color = parts.value(0);
    info.value = parts.mid(1).join("_");
    return info;
}
} // namespace

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

    if (type == "play_card") {
        const QString card = msg.value("card").toString();
        if (card.isEmpty()) {
            sendJson(sock, QJsonObject{{"type","error"},{"message","Missing card"}});
            return;
        }
        const QString chosenColor = msg.value("chosenColor").toString();
        playCard(sock, card, chosenColor);
        return;
    }

    if (type == "declare_uno") {
        declareUno(sock);
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
    if (g->finished) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Game finished"}});
        return;
    }
    if (!g->players.contains(sock)) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Not a player"}});
        return;
    }
    if (g->currentPlayerIndex != g->players.indexOf(sock)) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Not your turn"}});
        return;
    }

    applyUnoPenaltyIfNeeded(g, g->currentPlayerIndex);

    if (g->pendingUnoPlayerIndex == g->currentPlayerIndex) {
        g->pendingUnoPlayerIndex = -1;
        g->pendingUnoDeclared = false;
    }

    QStringList& hand = g->hands[sock];
    QJsonArray cardsArr;

    for (int i = 0; i < count; ++i) {
        refillDeck(g);
        if (g->deck.isEmpty()) break;
        const QString card = g->deck.takeLast();
        hand.append(card);
        cardsArr.append(card);
    }

    if (cardsArr.isEmpty()) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Deck is empty"}});
        return;
    }

    const int drawingPlayerIndex = g->currentPlayerIndex;
    g->currentPlayerIndex = advanceIndex(g->currentPlayerIndex, 1, g->direction, g->players.size());

    sendJson(sock, QJsonObject{
                       {"type","cards_drawn"},
                       {"cards",cardsArr},
                       {"drawCount",g->deck.size()},
                       {"currentPlayerIndex",g->currentPlayerIndex}
                   });

    sendStateUpdate(g);
    appendLog(g, "draw", drawingPlayerIndex, QString::number(cardsArr.size()));

    qInfo() << "[GAME]" << code << "draw_cards count=" << cardsArr.size()
            << "remaining=" << g->deck.size();
}

void Server::sendJson(QTcpSocket* sock, const QJsonObject& obj)
{
    const QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n";
    sock->write(payload);
    sock->flush();
}

int Server::indexOfPlayer(GameState* g, QTcpSocket* sock) const
{
    return g ? g->players.indexOf(sock) : -1;
}

bool Server::isCardLegal(const QString& card, const QString& topDiscard, const QString& currentColor) const
{
    if (topDiscard.isEmpty())
        return true;

    const CardInfo playInfo = parseCardInfo(card);
    if (playInfo.isWild)
        return true;

    const CardInfo topInfo = parseCardInfo(topDiscard);
    if (topInfo.isWild)
        return !currentColor.isEmpty() && playInfo.color == currentColor;

    return playInfo.color == topInfo.color || playInfo.value == topInfo.value;
}

int Server::advanceIndex(int startIndex, int steps, int direction, int playerCount) const
{
    if (playerCount <= 0)
        return 0;
    int idx = startIndex;
    for (int i = 0; i < steps; ++i) {
        idx = (idx + direction) % playerCount;
        if (idx < 0) idx += playerCount;
    }
    return idx;
}

QStringList Server::drawCardsToPlayer(GameState* g, QTcpSocket* sock, int count)
{
    QStringList drawn;
    if (!g || !sock || count <= 0)
        return drawn;

    QStringList& hand = g->hands[sock];
    for (int i = 0; i < count; ++i) {
        refillDeck(g);
        if (g->deck.isEmpty())
            break;
        const QString card = g->deck.takeLast();
        hand.append(card);
        drawn.append(card);
    }
    return drawn;
}

void Server::refillDeck(GameState* g)
{
    if (!g || !g->deck.isEmpty())
        return;

    if (g->discard.size() <= 1)
        return;

    const QString top = g->discard.takeLast();
    g->deck = g->discard;
    g->discard.clear();
    g->discard.append(top);
    shuffle(g->deck);
    appendLog(g, "reshuffle", -1, QString("deck=%1").arg(g->deck.size()));
}

void Server::appendLog(GameState* g, const QString& event, int playerIndex, const QString& detail)
{
    if (!g) return;
    const QString timestamp = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    QString cleanedDetail = detail;
    cleanedDetail.replace('\n', ' ');
    g->logLines.append(QString("%1,%2,%3,%4")
                       .arg(timestamp,
                            event,
                            QString::number(playerIndex),
                            cleanedDetail));
}

void Server::applyUnoPenaltyIfNeeded(GameState* g, int currentPlayerIndex)
{
    if (!g) return;
    if (g->pendingUnoPlayerIndex < 0 || g->pendingUnoDeclared)
        return;
    if (g->pendingUnoPlayerIndex == currentPlayerIndex)
        return;

    const int penalizedIndex = g->pendingUnoPlayerIndex;
    QTcpSocket* penalizedSock = g->players.value(penalizedIndex, nullptr);
    if (!penalizedSock)
        return;

    QStringList drawn = drawCardsToPlayer(g, penalizedSock, 2);
    if (!drawn.isEmpty()) {
        QJsonArray cardsArr;
        for (const QString& c : drawn)
            cardsArr.append(c);
        sendJson(penalizedSock, QJsonObject{
                                   {"type","cards_drawn"},
                                   {"cards", cardsArr},
                                   {"drawCount", g->deck.size()},
                                   {"currentPlayerIndex", g->currentPlayerIndex}
                               });
        appendLog(g, "uno_penalty", penalizedIndex, QString("drawn=%1").arg(drawn.size()));
    }

    g->pendingUnoPlayerIndex = -1;
    g->pendingUnoDeclared = false;
}

void Server::sendStateUpdate(GameState* g, const QString& lastPlayedCard, int playedBy)
{
    if (!g) return;

    QJsonArray counts;
    for (QTcpSocket* p : g->players)
        counts.append(g->hands.value(p).size());

    QJsonObject state{
        {"type","state_update"},
        {"discardTop", g->discard.isEmpty() ? QString() : g->discard.last()},
        {"drawCount", g->deck.size()},
        {"currentPlayerIndex", g->currentPlayerIndex},
        {"handCounts", counts},
        {"currentColor", g->currentColor},
        {"finished", g->finished}
    };

    if (!lastPlayedCard.isEmpty())
        state.insert("lastPlayedCard", lastPlayedCard);
    if (playedBy >= 0)
        state.insert("playedBy", playedBy);

    for (QTcpSocket* p : g->players)
        sendJson(p, state);
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
    g->currentPlayerIndex = 0;
    g->direction = 1;
    g->finished = false;
    g->pendingUnoPlayerIndex = -1;
    g->pendingUnoDeclared = false;
    g->logLines.clear();
    g->logLines.append("timestamp,event,playerIndex,detail");

    const CardInfo topInfo = parseCardInfo(g->discard.last());
    if (topInfo.isWild) {
        g->currentColor = "Rot";
    } else {
        g->currentColor = topInfo.color;
    }
    appendLog(g, "start", -1, QString("discard=%1").arg(g->discard.last()));

    const int players = g->players.size();
    const QString discardTop = g->discard.last();
    const int drawCount = g->deck.size();
    QJsonArray handCounts;
    for (QTcpSocket* p : g->players)
        handCounts.append(g->hands.value(p).size());

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
            {"hand",handArr},
            {"currentPlayerIndex",g->currentPlayerIndex},
            {"handCounts",handCounts},
            {"currentColor", g->currentColor},
            {"finished", g->finished}
        };

        sendJson(p, init);
    }
}

void Server::playCard(QTcpSocket* sock, const QString& card, const QString& chosenColor)
{
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
    if (g->finished) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Game finished"}});
        return;
    }

    const int playerIndex = indexOfPlayer(g, sock);
    if (playerIndex < 0) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Not a player"}});
        return;
    }

    if (playerIndex != g->currentPlayerIndex) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Not your turn"}});
        return;
    }

    applyUnoPenaltyIfNeeded(g, g->currentPlayerIndex);

    if (!isCardLegal(card, g->discard.isEmpty() ? QString() : g->discard.last(), g->currentColor)) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Illegal card"}});
        return;
    }

    const CardInfo playInfo = parseCardInfo(card);
    if (playInfo.isWild && chosenColor.isEmpty()) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Missing chosen color"}});
        return;
    }

    if (playInfo.isWild) {
        const QString upper = chosenColor.trimmed();
        if (upper != "Rot" && upper != "Gruen" && upper != "Blau" && upper != "Gelb") {
            sendJson(sock, QJsonObject{{"type","error"},{"message","Invalid color"}});
            return;
        }
    }

    QStringList& hand = g->hands[sock];
    if (!hand.removeOne(card)) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Card not in hand"}});
        return;
    }

    if (g->pendingUnoPlayerIndex == playerIndex) {
        g->pendingUnoPlayerIndex = -1;
        g->pendingUnoDeclared = false;
    }

    g->discard.append(card);

    if (playInfo.isWild) {
        g->currentColor = chosenColor.trimmed();
    } else {
        g->currentColor = playInfo.color;
    }

    const int playerCount = g->players.size();
    QStringList drawnCards;
    int drawnByIndex = -1;
    if (playInfo.isWild && playInfo.value == "4plus") {
        const int targetIndex = advanceIndex(g->currentPlayerIndex, 1, g->direction, playerCount);
        QTcpSocket* targetSock = g->players[targetIndex];
        refillDeck(g);
        drawnCards = drawCardsToPlayer(g, targetSock, 4);
        drawnByIndex = targetIndex;
        g->currentPlayerIndex = advanceIndex(g->currentPlayerIndex, 2, g->direction, playerCount);
    } else if (playInfo.value == "Sperre") {
        g->currentPlayerIndex = advanceIndex(g->currentPlayerIndex, 2, g->direction, playerCount);
    } else if (playInfo.value == "Richtungswechsel") {
        g->direction = -g->direction;
        if (playerCount == 2) {
            g->currentPlayerIndex = advanceIndex(g->currentPlayerIndex, 2, g->direction, playerCount);
        } else {
            g->currentPlayerIndex = advanceIndex(g->currentPlayerIndex, 1, g->direction, playerCount);
        }
    } else {
        g->currentPlayerIndex = advanceIndex(g->currentPlayerIndex, 1, g->direction, playerCount);
    }

    appendLog(g, "play", playerIndex, QString("%1|color=%2").arg(card, g->currentColor));

    QJsonObject played{
        {"type","card_played"},
        {"playerIndex", playerIndex},
        {"card", card}
    };

    for (QTcpSocket* p : g->players)
        sendJson(p, played);

    if (!drawnCards.isEmpty() && drawnByIndex >= 0) {
        QTcpSocket* targetSock = g->players[drawnByIndex];
        QJsonArray cardsArr;
        for (const QString& c : drawnCards)
            cardsArr.append(c);
        sendJson(targetSock, QJsonObject{
                               {"type","cards_drawn"},
                               {"cards", cardsArr},
                               {"drawCount", g->deck.size()},
                               {"currentPlayerIndex", g->currentPlayerIndex}
                           });
        appendLog(g, "draw_four", drawnByIndex, QString("drawn=%1").arg(drawnCards.size()));
    }

    if (hand.size() == 1) {
        g->pendingUnoPlayerIndex = playerIndex;
        g->pendingUnoDeclared = false;
        appendLog(g, "uno_pending", playerIndex, "needs_declare");
    }

    if (hand.isEmpty()) {
        g->finished = true;
        appendLog(g, "win", playerIndex, "hand_empty");
        QJsonObject finished{
            {"type","game_finished"},
            {"winnerIndex", playerIndex},
            {"logCsv", g->logLines.join("\n")}
        };
        for (QTcpSocket* p : g->players)
            sendJson(p, finished);
    }

    sendStateUpdate(g, card, playerIndex);

    qInfo() << "[GAME]" << code << "play_card player=" << playerIndex << "card=" << card;
}

void Server::declareUno(QTcpSocket* sock)
{
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

    const int playerIndex = indexOfPlayer(g, sock);
    if (playerIndex < 0) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","Not a player"}});
        return;
    }

    if (g->pendingUnoPlayerIndex != playerIndex) {
        sendJson(sock, QJsonObject{{"type","error"},{"message","UNO not required"}});
        return;
    }

    g->pendingUnoDeclared = true;
    appendLog(g, "uno_declared", playerIndex, "ok");
    sendJson(sock, QJsonObject{{"type","uno_ok"}});
}
