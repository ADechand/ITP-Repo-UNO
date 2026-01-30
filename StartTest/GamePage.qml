import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    anchors.fill: parent
    objectName: "GamePage"

    signal goBack()

    property int opponentsCount: 0
    property string lastDiscard: ""
    property string lastDiscardId: ""
    property int lastHandCount: 0
    property int lastDrawCount: 0
    property string cardBase: "qrc:/assets/images/cards/"
    property string pendingWildCard: ""

    // Normalisiert alte Namen: "Blau 2" -> "Blau_2.jpg"
    // lässt neue Namen wie "Blau_2.jpg" unverändert
    function normalizeCardName(name) {
        if (!name) return ""
        var s = String(name).trim()

        // falls der Server noch ohne Endung schickt:
        if (!s.endsWith(".jpg")) s = s + ".jpg"

        // Leerzeichen -> Unterstriche
        s = s.replace(/ /g, "_")

        // falls irgendwo doppelte Unterstriche entstehen
        s = s.replace(/__+/g, "_")

        return s
    }

    function fileToQrc(fileName) {
        var n = normalizeCardName(fileName)
        if (n.length === 0) return ""
        return cardBase + n
    }


    function applyStateFromClient() {
        if (!gameClient.hasGameInit) return

        lastDiscard = fileToQrc(gameClient.discardTop)
        lastDiscardId = normalizeCardName(gameClient.discardTop)
        opponentsCount = Math.max(0, gameClient.players - 1)

        // Debug/Status
        lastHandCount = gameClient.hand ? gameClient.hand.length : 0
        lastDrawCount = gameClient.drawCount

        infoBanner.show("Hand: " + lastHandCount + " | Ablage: " + gameClient.discardTop + " | Deck: " + lastDrawCount)
    }

    Component.onCompleted: applyStateFromClient()

    Connections {
        target: gameClient
        function onGameStateChanged() { applyStateFromClient() }
        function onError(msg) { infoBanner.show("Server: " + msg) }
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

    Rectangle {
        id: infoBanner
        width: 720; height: 44; radius: 8
        color: "white"; border.color: "black"; border.width: 2
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 10
        opacity: 0
        visible: opacity > 0

        Text { id: infoText; anchors.centerIn: parent; font.pixelSize: 16; color: "black"; text: "" }

        function show(msg) {
            infoText.text = msg
            infoBanner.opacity = 1
            hideTimer.restart()
        }

        Timer { id: hideTimer; interval: 1800; repeat: false; onTriggered: infoBanner.opacity = 0 }
    }

    Popup {
        id: colorPicker
        modal: true
        focus: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 320
        height: 220
        closePolicy: Popup.CloseOnEscape

        background: Rectangle {
            color: "white"
            border.color: "black"
            border.width: 2
        }

        Column {
            anchors.centerIn: parent
            spacing: 10
            Text { text: "Farbe wählen"; font.pixelSize: 18; color: "black" }

            Row {
                spacing: 12
                Button { text: "Rot"; onClicked: chooseColor("Rot") }
                Button { text: "Grün"; onClicked: chooseColor("Gruen") }
                Button { text: "Blau"; onClicked: chooseColor("Blau") }
                Button { text: "Gelb"; onClicked: chooseColor("Gelb") }
            }

            Button {
                text: "Abbrechen"
                onClicked: {
                    pendingWildCard = ""
                    colorPicker.close()
                }
            }
        }
    }

    // Gegnerkarten (oben)
    Row {
        id: enemyRow
        spacing: 24
        anchors.top: parent.top
        anchors.topMargin: 60
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            model: opponentsCount
            delegate: Item {
                width: 90; height: 130
                property int globalIndex: index < gameClient.yourIndex ? index : index + 1
                property bool showCount: globalIndex === activeOpponentIndex()
                property int cardCount: gameClient.handCounts.length > globalIndex ? gameClient.handCounts[globalIndex] : 0
                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: "white"
                    border.color: "black"
                    border.width: 2
                    Image {
                        anchors.fill: parent
                        anchors.margins: 6
                        source: "qrc:/assets/images/cards/Gegnerkarte.jpg"
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

    // Mitte (Ablage + Ziehen)
    Item {
        anchors.centerIn: parent
        width: 520
        height: 280

        Rectangle {
            width: 120; height: 180; radius: 8
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            color: "white"; border.color: "black"; border.width: 2

            // Discard (mit Fallback .png <-> .jpg falls mal inkonsistent)
            Image {
                id: discardImg
                anchors.fill: parent
                anchors.margins: 6
                source: lastDiscard
                visible: lastDiscard !== ""
                fillMode: Image.PreserveAspectFit
                smooth: true

                onStatusChanged: {
                    if (status === Image.Error && source.indexOf(".png") !== -1) {
                        source = source.replace(".png", ".jpg")
                    } else if (status === Image.Error && source.indexOf(".jpg") !== -1) {
                        source = source.replace(".jpg", ".png")
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text: lastDiscard === "" ? "Ablage" : ""
                color: "black"
            }
        }

        Button {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 120
            height: 180
            text: "Karte\nziehen"
            font.pixelSize: 16
            enabled: gameClient.hasGameInit && isYourTurn()
            background: Rectangle { color: "#d9d9d9"; border.color: "black"; border.width: 2; radius: 8 }

            onClicked: {
                if (!gameClient.hasGameInit) {
                    infoBanner.show("Spiel nicht initialisiert.")
                    return
                }
                if (!isYourTurn()) {
                    infoBanner.show("Nicht dein Zug.")
                    return
                }
                gameClient.drawCards(1)
            }
        }
    }

    // Hand (unten) – direkt aus gameClient.hand (kein extra ListModel nötig)
    Row {
        id: playerHand
        spacing: 18
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            model: gameClient.hand

            delegate: Item {
                width: 90
                height: 130

                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: "white"
                    border.color: "black"
                    border.width: 2

                    Image {
                        id: cardImg
                        anchors.fill: parent
                        anchors.margins: 6
                        fillMode: Image.PreserveAspectFit
                        smooth: true

                        property string originalName: modelData
                        source: fileToQrc(originalName)

                        // Fallback, falls irgendwo doch noch .png/.jpg nicht passt
                        onStatusChanged: {
                            if (status !== Image.Error) return
                            if (source.indexOf(".png") !== -1) source = source.replace(".png", ".jpg")
                            else if (source.indexOf(".jpg") !== -1) source = source.replace(".jpg", ".png")
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (!isYourTurn()) {
                                infoBanner.show("Nicht dein Zug.")
                                return
                            }
                            if (!isLegalCard(modelData)) {
                                infoBanner.show("Diese Karte ist nicht erlaubt.")
                                return
                            }
                            gameClient.playCard(modelData)
                        }
                    }
                }
            }
        }
    }

    function isYourTurn() {
        return gameClient.hasGameInit && gameClient.currentPlayerIndex === gameClient.yourIndex
    }

    function activeOpponentIndex() {
        if (!gameClient.hasGameInit || gameClient.players === 0) {
            return -1
        }
        if (gameClient.currentPlayerIndex === gameClient.yourIndex) {
            return (gameClient.yourIndex + 1) % gameClient.players
        }
        return gameClient.currentPlayerIndex
    }

    function parseCard(cardId) {
        var base = normalizeCardName(cardId)
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
        if (!lastDiscardId || lastDiscardId.length === 0) {
            return true
        }
        var playInfo = parseCard(cardId)
        if (playInfo.wild) {
            return true
        }
        var topInfo = parseCard(lastDiscardId)
        if (topInfo.wild) {
            return true
        }
        return playInfo.color === topInfo.color || playInfo.value === topInfo.value
    }
}
