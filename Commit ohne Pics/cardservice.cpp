#include "cardservice.h"

#include <QDir>
#include <QUrl>
#include <QRandomGenerator>
#include <algorithm>

static QString toQrcUrl(const QString &qrcPathWithLeadingColon)
{
    // Beispiel input:  ":/assets/images/cards/Rot 9.png"
    // output:          "qrc:/assets/images/cards/Rot%209.png"
    const QString asQrc = "qrc:" + qrcPathWithLeadingColon.mid(1); // remove leading ':'
    return QUrl::fromUserInput(asQrc).toString();
}

CardService::CardService(QObject *parent)
    : QObject(parent)
{
    loadAllCardsFromResources();
    resetDeck();
}

void CardService::loadAllCardsFromResources()
{
    m_allCards.clear();

    // Ressourcen-Pfad (durch qt_add_resources PREFIX "/" => ":/assets/...")
    QDir dir(":/assets/images/cards");
    if (!dir.exists())
        return;

    const QStringList filters = { "*.png", "*.jpg", "*.jpeg" };
    const QStringList files = dir.entryList(filters, QDir::Files | QDir::Readable, QDir::Name);

    for (const QString &file : files) {
        const QString fullQrcPath = dir.absoluteFilePath(file); // ":/assets/images/cards/..."
        m_allCards.append(toQrcUrl(fullQrcPath));
    }
}

void CardService::shuffleDeck()
{
    // Fisher-Yates Shuffle
    for (int i = m_deck.size() - 1; i > 0; --i) {
        const int j = QRandomGenerator::global()->bounded(i + 1);
        m_deck.swapItemsAt(i, j);
    }
}

void CardService::resetDeck()
{
    m_deck = m_allCards;
    shuffleDeck();
}

QString CardService::draw()
{
    if (m_deck.isEmpty())
        resetDeck();

    if (m_deck.isEmpty())
        return QString(); // falls wirklich keine Karten gefunden wurden

    const QString card = m_deck.back();
    m_deck.removeLast();
    return card;
}
