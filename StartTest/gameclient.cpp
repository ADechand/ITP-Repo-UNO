#include "gameclient.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

GameClient::GameClient(QObject* parent) : QObject(parent)
{
    connect(&m_sock, &QTcpSocket::connected, this, [this]() {
        m_connected = true;
        emit connectedChanged();
        emit info("Verbunden.");
    });

    connect(&m_sock, &QTcpSocket::disconnected, this, [this]() {
        m_connected = false;
        emit connectedChanged();
        emit info("Getrennt.");
    });

    connect(&m_sock, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        emit error(m_sock.errorString());
    });

    connect(&m_sock, &QTcpSocket::readyRead, this, [this]() {
        m_buffer += m_sock.readAll();

        while (true) {
            const int nl = m_buffer.indexOf('\n');
            if (nl < 0) break;

            const QByteArray line = m_buffer.left(nl).trimmed();
            m_buffer.remove(0, nl + 1);
            if (line.isEmpty()) continue;

            QJsonParseError err;
            const QJsonDocument doc = QJsonDocument::fromJson(line, &err);
            if (err.error != QJsonParseError::NoError || !doc.isObject()) {
                emit error("Server: invalid JSON");
                continue;
            }

            const QJsonObject o = doc.object();
            const QString type = o.value("type").toString();

            if (type == "error") {
                emit error(o.value("message").toString());
                continue;
            }

            if (type == "game_created") {
                emit gameCreated(o.value("code").toString());
                continue;
            }

            if (type == "join_ok") {
                emit joinOk(o.value("code").toString());
                continue;
            }

            if (type == "game_init") {
                m_hasGameInit = true;
                m_gameCode = o.value("code").toString();
                m_discardTop = o.value("discardTop").toString();
                m_drawCount = o.value("drawCount").toInt();
                m_players = o.value("players").toInt();
                m_yourIndex = o.value("yourIndex").toInt();

                m_hand.clear();
                const QJsonArray arr = o.value("hand").toArray();
                for (const QJsonValue& v : arr) m_hand << v.toString();

                emit gameStateChanged();
                emit info(QString("game_init: hand=%1 discard=%2").arg(m_hand.size()).arg(m_discardTop));
                continue;
            }

            if (type == "cards_drawn") {
                const QJsonArray arr = o.value("cards").toArray();
                for (const QJsonValue& v : arr) m_hand << v.toString();
                m_drawCount = o.value("drawCount").toInt();

                emit gameStateChanged();
                emit info(QString("cards_drawn: +%1").arg(arr.size()));
                continue;
            }

            emit info("Server msg: " + type);
        }
    });
}

void GameClient::connectToServer(const QString& host, int port)
{
    if (m_sock.state() == QAbstractSocket::ConnectedState ||
        m_sock.state() == QAbstractSocket::ConnectingState) {
        return;
    }
    m_sock.connectToHost(host, quint16(port));
}

void GameClient::disconnectFromServer()
{
    m_sock.disconnectFromHost();
}

void GameClient::sendJson(const QJsonObject& o)
{
    const QByteArray payload = QJsonDocument(o).toJson(QJsonDocument::Compact) + "\n";
    m_sock.write(payload);
    m_sock.flush();
}

void GameClient::createGame()
{
    sendJson(QJsonObject{{"type","create_game"}});
}

void GameClient::joinGame(const QString& code)
{
    sendJson(QJsonObject{{"type","join_game"},{"code",code.trimmed().toUpper()}});
}

void GameClient::startGame(const QString& code)
{
    sendJson(QJsonObject{{"type","start_game"},{"code",code.trimmed().toUpper()}});
}

void GameClient::drawCards(int count)
{
    if (count < 1) count = 1;
    if (count > 10) count = 10;
    sendJson(QJsonObject{{"type","draw_cards"},{"count",count}});
}
