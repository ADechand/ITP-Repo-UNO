#include "linkservice.h"

#include <QGuiApplication>
#include <QClipboard>

LinkService::LinkService(QObject* parent) : QObject(parent) {}

//Setzt den Link ins Clipboard
void LinkService::copyToClipboard(const QString& text)
{
    //Prüft ob es einen Code gibt
    if (text.trimmed().isEmpty()) {
        emit error("Nichts zum Kopieren.");
        return;
    }

    //Setzt den Code ins Clipboard
    QClipboard* cb = QGuiApplication::clipboard();
    cb->setText(text);

    //Rückmeldung dass es nun im Clipboard ist
    emit info("In Zwischenablage kopiert: " + text);
}
