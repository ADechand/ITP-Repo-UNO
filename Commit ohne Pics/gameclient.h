#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QStringList>
#include <QJsonObject>
#include <QVariantList>

class GameClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

public:
    explicit GameClient(QObject* parent = nullptr);

    bool connected() const { return m_connected; }

    Q_INVOKABLE void connectToServer(const QString& host, int port);
    Q_INVOKABLE void disconnectFromServer();

    Q_INVOKABLE void createGame();
    Q_INVOKABLE void joinGame(const QString& code);
    Q_INVOKABLE void startGame(const QString& code);
    Q_INVOKABLE void drawCards(int count);
    Q_INVOKABLE void playCard(const QString& card);

signals:
    void info(QString msg);
    void error(QString msg);

    void connectedChanged();

    void gameCreated(QString code);
    void joinOk(QString code);
    void gameInit(QString code, QStringList hand, QString discardTop, int drawCount, int players, int yourIndex, int currentPlayerIndex, QVariantList handCounts);
    void cardsDrawn(QStringList cards, int drawCount, int currentPlayerIndex);
    void stateUpdate(QString discardTop, int drawCount, int currentPlayerIndex, QVariantList handCounts);
    void cardPlayed(int playerIndex, QString card);

private:
    void sendJson(const QJsonObject& o);

    QTcpSocket m_sock;
    QByteArray m_buffer;
    bool m_connected = false;
};
