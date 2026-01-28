#pragma once

#include <QObject>
#include <QString>

class LinkService : public QObject
{
    Q_OBJECT
public:
    explicit LinkService(QObject *parent = nullptr);

    Q_INVOKABLE void createCopyAndSendCode();

signals:
    void info(QString message);
    void error(QString message);
    void codeCreated(QString code);   // optional, falls du ihn anzeigen willst

private:
    QString generateCode(int length = 11) const; // z.B. 11-12 Zeichen wie Among Us
    void sendToServer(const QString &code);

    const QString m_serverIp = "10.0.2.15";
    const quint16 m_serverPort = 12345;
};
