#include "linkservice.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QTcpSocket>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonObject>




LinkService::LinkService(QObject *parent) : QObject(parent) {}

QString LinkService::generateCode(int length) const
{
    static const char alphabet[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    const int n = int(sizeof(alphabet) - 1);

    QString code;
    code.reserve(length);
    for (int i = 0; i < length; ++i) {
        int idx = int(QRandomGenerator::global()->bounded(n));
        code.append(QChar(alphabet[idx]));
    }
    return code;
}

void LinkService::sendToServer(const QString &code)
{
    QJsonObject obj;
    obj["type"] = "host_code";
    obj["code"] = code;

    const QByteArray payload =
        QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n";

    auto *sock = new QTcpSocket(this);

    connect(sock, &QTcpSocket::connected, this, [sock, payload]() {
        sock->write(payload);
        sock->flush();
        sock->disconnectFromHost();
    });

    connect(sock, &QTcpSocket::errorOccurred, this, [this, sock](QAbstractSocket::SocketError) {
        //emit error(QString("Server-Verbindung fehlgeschlagen: %1").arg(sock->errorString()));
        sock->deleteLater();
    });

    connect(sock, &QTcpSocket::disconnected, this, [this, sock]() {
        //emit info("Code an Server gesendet.");
        sock->deleteLater();
    });

    sock->connectToHost(m_serverIp, m_serverPort);
}

void LinkService::createCopyAndSendCode()
{
    const QString code = generateCode(11);

    if (auto *cb = QGuiApplication::clipboard()) {
        cb->setText(code);
    } else {
        emit error("Zwischenablage nicht verfügbar.");
        return;
    }

    emit codeCreated(code);
    emit info("Spielcode kopiert!");

    sendToServer(code);
}
