import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    anchors.fill: parent

    signal goBack()

    // UNO-Logik
    property bool mustSayUno: false
    property bool unoConfirmed: false
    property int unoPenaltyCards: 2
    property int unoWindowMs: 2500
    property int opponentsCount: 6   // Anzahl Gegner (später vom Server)
    property int playersCount: 0
    property int yourIndex: 0
    property int currentPlayerIndex: 0
    property var handCounts: []
    property string lastDiscardId: ""

    // Hand als Model (statt nur handCount)
    ListModel { id: handModel }
    property int handCount: handModel.count

    // Ablage (zeigt zuletzt gespielte Karte)
    property string lastDiscard: ""

    Connections {
        target: gameClient

        function onGameInit(code, hand, discardTop, drawCount, players, yourIndex, currentPlayerIndex, handCounts) {
            // Hand neu setzen
            handModel.clear()
            for (var i = 0; i < hand.length; i++) {
                handModel.append({ src: "qrc:/assets/images/cards/" + hand[i], cardId: hand[i] })
            }

            // Discard setzen
            if (discardTop && discardTop.length > 0) {
                root.lastDiscardId = discardTop
                root.lastDiscard = "qrc:/assets/images/cards/" + discardTop
            }

            root.playersCount = players
            root.yourIndex = yourIndex
            root.currentPlayerIndex = currentPlayerIndex
            root.handCounts = handCounts
            root.opponentsCount = Math.max(0, players - 1)
            infoBanner.show("Game gestartet! Spieler: " + players)
        }

        function onConnectedChanged() {
                if (!gameClient.connected) {
                    // zurück aus dem Spiel, wenn Verbindung weg
                    root.goBack()
                }
            }

        function onError(msg) { infoBanner.show("Server: " + msg) }

        function onCardsDrawn(cards, drawCount, currentPlayerIndex) {
            for (var i = 0; i < cards.length; i++) {
                handModel.append({ src: "qrc:/assets/images/cards/" + cards[i], cardId: cards[i] })
            }
            root.currentPlayerIndex = currentPlayerIndex
        }

        function onStateUpdate(discardTop, drawCount, currentPlayerIndex, handCounts) {
            if (discardTop && discardTop.length > 0) {
                root.lastDiscardId = discardTop
                root.lastDiscard = "qrc:/assets/images/cards/" + discardTop
            }
            root.currentPlayerIndex = currentPlayerIndex
            root.handCounts = handCounts
        }

        function onCardPlayed(playerIndex, card) {
            if (playerIndex === root.yourIndex) {
                removeCardFromHand(card)
            } else {
                infoBanner.show("Gegner hat gespielt.")
            }
        }
    }

    Rectangle { anchors.fill: parent; color: "#ff9fa0" }

    Button {
        text: "Go Back"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 16
        anchors.topMargin: 12
        background: Rectangle { color: "white"; border.color: "black"; border.width: 2; radius: 4 }
        onClicked: root.goBack()
    }

    // Info Banner
    Rectangle {
        id: infoBanner
        width: 360; height: 44; radius: 8
        color: "white"; border.color: "black"; border.width: 2
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: playerHand.top
        anchors.bottomMargin: 14
        opacity: 0
        visible: opacity > 0

        Text { id: infoText; anchors.centerIn: parent; font.pixelSize: 18; color: "black"; text: "" }

        function show(msg) {
            infoText.text = msg
            infoBanner.opacity = 1
            hideTimer.restart()
        }

        Timer { id: hideTimer; interval: 1400; repeat: false; onTriggered: infoBanner.opacity = 0 }
    }

    // Mitte: Ablage + Ziehstapel
    Item {
        id: centerArea
        anchors.centerIn: parent
        width: 520
        height: 280

        // Ablage (links)
        Rectangle {
            width: 120; height: 180; radius: 8
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            color: "white"; border.color: "black"; border.width: 2

            Image {
                anchors.fill: parent
                anchors.margins: 6
                source: root.lastDiscard
                visible: root.lastDiscard !== ""
                fillMode: Image.PreserveAspectFit
                smooth: true
            }

            Text {
                anchors.centerIn: parent
                text: root.lastDiscard === "" ? "Ablage" : ""
                color: "black"
            }
        }

        // Ziehstapel Button (rechts)
        Button {
            id: drawBtn
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 120
            height: 180
            text: "Karte\nziehen"
            font.pixelSize: 16
            enabled: !mustSayUno && isYourTurn()

            background: Rectangle { color: "#d9d9d9"; border.color: "black"; border.width: 2; radius: 8 }

            onClicked: {
                if (!isYourTurn()) {
                    infoBanner.show("Nicht dein Zug.")
                    return
                }
                gameClient.drawCards(1)
            }
        }
    }

    // UNO Button
    Button {
        id: unoButton
        text: "UNO!"
        width: 140
        height: 60

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 120
        anchors.right: parent.right
        anchors.rightMargin: 40

        visible: root.handCount === 1
        enabled: root.handCount === 1 && !root.unoConfirmed

        background: Rectangle {
            color: root.unoConfirmed ? "#cccccc" : "#ff4444"
            radius: 10
            border.color: "black"
            border.width: 2
        }

        onVisibleChanged: {
            if (visible) root.unoConfirmed = false
        }

        onClicked: {
            root.unoConfirmed = true
            root.mustSayUno = false
            unoTimer.stop()
            infoBanner.show("UNO!")
        }
    }

    // Spielerhand unten
    Row {
        id: playerHand
        spacing: 18
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            model: handModel

            delegate: Item {
                width: 90
                height: 130

                // Kartenbild
                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: "white"
                    border.color: "black"
                    border.width: 2

                    Image {
                        anchors.fill: parent
                        anchors.margins: 6
                        source: model.src
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            if (!isYourTurn()) {
                                infoBanner.show("Nicht dein Zug.")
                                return
                            }
                            if (!isLegalCard(model.cardId)) {
                                infoBanner.show("Diese Karte ist nicht erlaubt.")
                                return
                            }
                            gameClient.playCard(model.cardId)
                        }
                    }
                }
            }
        }
    }

    // ================================
    // Gegnerkarten (oben)
    // ================================
    Row {
        id: enemyRow
        spacing: 24
        anchors.top: parent.top
        anchors.topMargin: 40
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            model: root.opponentsCount

            delegate: Item {
                width: 90
                height: 130
                property int globalIndex: index < root.yourIndex ? index : index + 1
                property bool showCount: globalIndex === activeOpponentIndex()
                property int cardCount: root.handCounts.length > globalIndex ? root.handCounts[globalIndex] : 0

                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: "white"
                    border.color: "black"
                    border.width: 2

                    Image {
                        anchors.fill: parent
                        anchors.margins: 6
                        source: "qrc:/assets/images/cards/Gegnerkarte.png"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.bottom
                    anchors.topMargin: 6
                    text: showCount ? (cardCount + " Karten") : "?"
                    font.pixelSize: 16
                    color: "black"
                }
            }
        }
    }





    // UNO Timer -> Strafe ziehen
    Timer {
        id: unoTimer
        interval: root.unoWindowMs
        repeat: false
        onTriggered: {
            if (root.mustSayUno && !root.unoConfirmed) {
                root.mustSayUno = false
                infoBanner.show("UNO vergessen!")
            }
        }
    }

    function isYourTurn() {
        return root.playersCount > 0 && root.currentPlayerIndex === root.yourIndex
    }

    function activeOpponentIndex() {
        if (root.playersCount === 0) {
            return -1
        }
        if (root.currentPlayerIndex === root.yourIndex) {
            return (root.yourIndex + 1) % root.playersCount
        }
        return root.currentPlayerIndex
    }

    function removeCardFromHand(cardId) {
        for (var i = 0; i < handModel.count; i++) {
            if (handModel.get(i).cardId === cardId) {
                handModel.remove(i)
                handlePostPlay()
                return
            }
        }
    }

    function handlePostPlay() {
        if (root.handCount === 1) {
            root.mustSayUno = true
            root.unoConfirmed = false
            unoTimer.restart()
            infoBanner.show("UNO drücken!")
        } else {
            if (root.mustSayUno && root.handCount !== 1) {
                root.mustSayUno = false
                root.unoConfirmed = false
                unoTimer.stop()
            }
        }
    }

    function parseCard(cardId) {
        var base = cardId
        var dot = base.lastIndexOf(".")
        if (dot >= 0) {
            base = base.slice(0, dot)
        }
        var parts = base.split("_")
        if (parts.length === 0) {
            return { color: "", value: "", wild: false }
        }
        if (parts[0] === "Extra") {
            return { color: "Extra", value: parts.slice(1).join("_"), wild: true }
        }
        return { color: parts[0], value: parts.slice(1).join("_"), wild: false }
    }

    function isLegalCard(cardId) {
        if (!root.lastDiscardId || root.lastDiscardId.length === 0) {
            return true
        }
        var playInfo = parseCard(cardId)
        if (playInfo.wild) {
            return true
        }
        var topInfo = parseCard(root.lastDiscardId)
        if (topInfo.wild) {
            return true
        }
        return playInfo.color === topInfo.color || playInfo.value === topInfo.value
    }
}
