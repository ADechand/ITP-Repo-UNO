#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QJsonObject>
#include <QStringList>

struct GameState {
    QString code;
    QTcpSocket* host = nullptr;
    QList<QTcpSocket*> players;                 // Reihenfolge = yourIndex
    bool started = false;
    int currentPlayerIndex = 0;
    int direction = 1;
    QString currentColor;

    QStringList deck;                           // draw pile (oben = last)
    QStringList discard;                        // discard pile (oben = last)
    QHash<QTcpSocket*, QStringList> hands;      // Handkarten je Spieler
};

class Server : public QObject {
    Q_OBJECT
public:
    explicit Server(QObject* parent = nullptr);

private:
    void onNewConnection();
    void onReadyRead(QTcpSocket* sock);
    void onDisconnected(QTcpSocket* sock);

    void handleMessage(QTcpSocket* sock, const QJsonObject& msg);
    void sendJson(QTcpSocket* sock, const QJsonObject& obj);

    QString createCode() const;
    GameState* getGame(const QString& code);

    void createGame(QTcpSocket* hostSock);
    void joinGame(QTcpSocket* sock, const QString& code);
    void startGame(QTcpSocket* sock, const QString& code);
    void drawCards(QTcpSocket* sock, int count);
    void playCard(QTcpSocket* sock, const QString& card, const QString& chosenColor);

    QStringList buildDeckFromStaticList() const;
    void shuffle(QStringList& list) const;
    void sendStateUpdate(GameState* g, const QString& lastPlayedCard = QString(), int playedBy = -1);
    int indexOfPlayer(GameState* g, QTcpSocket* sock) const;
    bool isCardLegal(const QString& card, const QString& topDiscard, const QString& currentColor) const;
    int advanceIndex(int startIndex, int steps, int direction, int playerCount) const;
    void drawCardsToPlayer(GameState* g, QTcpSocket* sock, int count) const;

private:
    QTcpServer m_server;
    QHash<QTcpSocket*, QByteArray> m_buffers;

    QHash<QString, GameState> m_games;
    QHash<QTcpSocket*, QString> m_socketToGame;
};
