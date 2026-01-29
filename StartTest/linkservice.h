#pragma once
#include <QObject>

class LinkService : public QObject
{
    Q_OBJECT
public:
    explicit LinkService(QObject* parent = nullptr);

    Q_INVOKABLE void copyToClipboard(const QString& text);

signals:
    void info(QString msg);
    void error(QString msg);
};
