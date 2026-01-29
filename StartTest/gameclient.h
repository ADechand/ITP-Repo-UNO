#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QStringList>
#include <QJsonObject>

class GameClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

    // Game-State wird im Client gespeichert, damit GamePage ihn auch NACH game_init bekommt.
    Q_PROPERTY(bool hasGameInit READ hasGameInit NOTIFY gameStateChanged)
    Q_PROPERTY(QString gameCode READ gameCode NOTIFY gameStateChanged)
    Q_PROPERTY(QStringList hand READ hand NOTIFY gameStateChanged)
    Q_PROPERTY(QString discardTop READ discardTop NOTIFY gameStateChanged)
    Q_PROPERTY(int drawCount READ drawCount NOTIFY gameStateChanged)
    Q_PROPERTY(int players READ players NOTIFY gameStateChanged)
    Q_PROPERTY(int yourIndex READ yourIndex NOTIFY gameStateChanged)

public:
    explicit GameClient(QObject* parent = nullptr);

    bool connected() const { return m_connected; }

    bool hasGameInit() const { return m_hasGameInit; }
    QString gameCode() const { return m_gameCode; }
    QStringList hand() const { return m_hand; }
    QString discardTop() const { return m_discardTop; }
    int drawCount() const { return m_drawCount; }
    int players() const { return m_players; }
    int yourIndex() const { return m_yourIndex; }

    Q_INVOKABLE void connectToServer(const QString& host, int port);
    Q_INVOKABLE void disconnectFromServer();

    Q_INVOKABLE void createGame();
    Q_INVOKABLE void joinGame(const QString& code);
    Q_INVOKABLE void startGame(const QString& code);

    Q_INVOKABLE void drawCards(int count = 1);

signals:
    void info(QString msg);
    void error(QString msg);

    void connectedChanged();

    // Events (optional fürs UI)
    void gameCreated(QString code);
    void joinOk(QString code);

    // Wird ausgelöst, wenn Game-State aktualisiert wurde (game_init, draw_cards)
    void gameStateChanged();

private:
    void sendJson(const QJsonObject& o);

    QTcpSocket m_sock;
    QByteArray m_buffer;
    bool m_connected = false;

    // Stored state:
    bool m_hasGameInit = false;
    QString m_gameCode;
    QStringList m_hand;
    QString m_discardTop;
    int m_drawCount = 0;
    int m_players = 0;
    int m_yourIndex = -1;
};
