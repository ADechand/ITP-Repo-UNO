#include "linkservice.h"

#include <QGuiApplication>
#include <QClipboard>

LinkService::LinkService(QObject* parent) : QObject(parent) {}

void LinkService::copyToClipboard(const QString& text)
{
    if (text.trimmed().isEmpty()) {
        emit error("Nichts zum Kopieren.");
        return;
    }

    QClipboard* cb = QGuiApplication::clipboard();
    cb->setText(text);

    emit info("In Zwischenablage kopiert: " + text);
}
