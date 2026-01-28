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

    // Hand als Model (statt nur handCount)
    ListModel { id: handModel }
    property int handCount: handModel.count

    // Ablage (zeigt zuletzt gespielte Karte)
    property string lastDiscard: ""

    Connections {
        target: gameClient

        function onGameInit(code, hand, discardTop, drawCount, players, yourIndex) {
            // Hand neu setzen
            handModel.clear()
            for (var i = 0; i < hand.length; i++) {
                handModel.append({ src: "qrc:/assets/images/cards/" + hand[i] })
            }

            // Discard setzen
            if (discardTop && discardTop.length > 0)
                root.lastDiscard = "qrc:/assets/images/cards/" + discardTop

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
            enabled: !mustSayUno

            background: Rectangle { color: "#d9d9d9"; border.color: "black"; border.width: 2; radius: 8 }

            onClicked: {
                // 1 Karte zufällig ziehen
                var c = cardService.draw()
                if (c === "") {
                    infoBanner.show("Keine Karten gefunden (Resources?)")
                    return
                }
                handModel.append({ src: c })

                // wenn man gezogen hat und vorher UNO offen war, reset (optional)
                if (mustSayUno) {
                    mustSayUno = false
                    unoConfirmed = false
                    unoTimer.stop()
                }
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
                            // Karte spielen -> Ablage
                            root.lastDiscard = model.src
                            handModel.remove(index)

                            // UNO-Logik nach dem Spielen
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

                for (var i = 0; i < root.unoPenaltyCards; i++) {
                    var c = cardService.draw()
                    if (c !== "") handModel.append({ src: c })
                }

                infoBanner.show("UNO vergessen! +" + root.unoPenaltyCards + " Karten")
            }
        }
    }

    // Start-Hand (zum Test)
    Component.onCompleted: {
        cardService.resetDeck()
        handModel.clear()
        for (var i = 0; i < 7; i++) {
            var c = cardService.draw()
            if (c !== "") handModel.append({ src: c })
        }
    }
}
