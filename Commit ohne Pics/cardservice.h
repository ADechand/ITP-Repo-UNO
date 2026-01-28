#pragma once

#include <QObject>
#include <QStringList>

class CardService : public QObject
{
    Q_OBJECT

public:
    explicit CardService(QObject *parent = nullptr);

    Q_INVOKABLE QString draw();      // zieht die nächste zufällige Karte (Deck-Logik)
    Q_INVOKABLE void resetDeck();    // mischt neu

private:
    void loadAllCardsFromResources();
    void shuffleDeck();

    QStringList m_allCards;  // alle Karten (qrc:/... URLs)
    QStringList m_deck;      // gemischtes Deck
};
