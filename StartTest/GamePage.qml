import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    anchors.fill: parent

    // Navigation
    signal goBack()

    // ---- Spiel-Events (später an Server koppeln) ----
    signal cardPlayed(string cardId)     // wenn Spieler Karte gelegt hat
    signal drawRequested()               // wenn Spieler "Karte ziehen" nutzt
    signal unoPressed()                  // wenn UNO gedrückt wurde
    signal penaltyDraw(int count)        // wenn Strafe gezogen wurde (z.B. 2)

    // ---- UI/Spielzustand (Prototyp) ----
    property int opponentsCount: 6
    property int handCount: 2            // wie viele Karten hat der Spieler unten
    property bool mustSayUno: false      // nach vorletzter Karte -> UNO nötig
    property bool unoConfirmed: false    // ob UNO rechtzeitig gedrückt wurde
    property int unoPenaltyCards: 2      // Strafe bei vergessenem UNO
    property int unoWindowMs: 2500       // Zeitfenster fürs UNO drücken
    property bool unoDeclared: false

    // Hintergund (wie Mockup: rosa)
    Rectangle {
        anchors.fill: parent
        color: "#ff9fa0"
    }

    // --- Zurück Button (oben links) ---
    Button {
        id: backBtn
        text: "Go Back"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 16
        anchors.topMargin: 12

        background: Rectangle {
            color: "white"
            border.color: "black"
            border.width: 2
            radius: 4
        }
        onClicked: root.goBack()
    }

    // --- Gegnerkarten oben (graue Rückseiten) ---
    Row {
        id: enemyRow
        spacing: 18
        anchors.top: parent.top
        anchors.topMargin: 40
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            model: opponentsCount
            delegate: Rectangle {
                width: 70
                height: 105
                color: "#d9d9d9"
                border.color: "#bdbdbd"
                border.width: 1
                radius: 2
            }
        }
    }

    // --- Ablagestapel (mittig) ---
    Item {
        id: centerArea
        anchors.centerIn: parent
        width: 420
        height: 240

        // Fake-Stack (3 Karten)
        Repeater {
            model: 3
            delegate: Rectangle {
                width: 95
                height: 140
                radius: 6
                color: "white"
                border.color: "black"
                border.width: 1

                x: 140 + index * 6
                y: 40 + index * 6
                rotation: -18 + index * 8

                // "gelbe Karte" innen
                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width * 0.85
                    height: parent.height * 0.85
                    radius: 6
                    color: "#f4c400"
                    border.color: "white"
                    border.width: 2
                }
            }
        }

        // --- Ziehstapel Button rechts (grau) ---
        Button {
            id: drawBtn
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.verticalCenter: parent.verticalCenter
            width: 90
            height: 120
            text: "Karte\nziehen"
            font.pixelSize: 16

            enabled: !mustSayUno // wenn UNO offen ist, erstmal UNO oder Strafe abwarten

            background: Rectangle {
                color: "#d9d9d9"
                border.color: "black"
                border.width: 1
                radius: 2
            }

            onClicked: {
                root.drawRequested()
                // Prototyp: Karte hinzufügen
                root.handCount += 1
            }
        }
    }

    // --- UNO Button (oben rechts) ---
    Button {
        id: unoButton
        text: "UNO!"
        width: 140
        height: 60

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 120
        anchors.right: parent.right
        anchors.rightMargin: 40

        visible: handCount === 2
        enabled: handCount === 2

        background: Rectangle {
            color: unoDeclared ? "#cccccc" : "#ff4444"
            radius: 10
            border.color: "black"
            border.width: 2
        }

        onClicked: {
            unoDeclared = true
            console.log("UNO gedrückt")
            // später: Server informieren
        }
    }


    // --- Info Banner (kleines Feedback) ---
    Rectangle {
        id: infoBanner
        width: 360
        height: 44
        radius: 8
        color: "white"
        border.color: "black"
        border.width: 2
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: playerHand.top
        anchors.bottomMargin: 14
        opacity: 0
        visible: opacity > 0

        Text {
            id: infoText
            anchors.centerIn: parent
            font.pixelSize: 18
            color: "black"
            text: ""
        }

        function show(msg) {
            infoText.text = msg
            infoBanner.opacity = 1
            hideTimer.restart()
        }

        Timer {
            id: hideTimer
            interval: 1400
            repeat: false
            onTriggered: infoBanner.opacity = 0
        }
    }

    // --- Spielerhand unten ---
    Row {
        id: playerHand
        spacing: 18
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            model: root.handCount
            delegate: Item {
                width: 78
                height: 118

                // Karte
                Rectangle {
                    id: cardRect
                    anchors.fill: parent
                    radius: 6
                    color: "white"
                    border.color: "black"
                    border.width: 1

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width * 0.86
                        height: parent.height * 0.86
                        radius: 6
                        color: "#f4c400"
                        border.color: "white"
                        border.width: 2
                    }

                    // Click = "Karte ausspielen" (Prototyp)
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            // Wenn UNO offen ist und du spielst weiter: das ist im echten UNO nicht sauber,
                            // aber wir blocken NICHT hart – nur prototypisch.
                            if (root.handCount <= 0)
                                return

                            // Karte "spielen"
                            root.handCount -= 1
                            root.cardPlayed("card_" + index)

                            // Wenn jetzt noch 1 Karte übrig -> UNO muss gedrückt werden
                            if (root.handCount === 1) {
                                root.mustSayUno = true
                                root.unoConfirmed = false
                                unoTimer.restart()
                                infoBanner.show("UNO drücken!")
                            } else {
                                // wenn vorher UNO offen war, aber jetzt nicht mehr 1 Karte: reset
                                if (root.mustSayUno && root.handCount !== 1) {
                                    root.mustSayUno = false
                                    unoTimer.stop()
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // --- UNO Timer: wenn nicht gedrückt -> Strafe ziehen ---
    Timer {
        id: unoTimer
        interval: root.unoWindowMs
        repeat: false
        onTriggered: {
            if (root.mustSayUno && !root.unoConfirmed) {
                // Strafe: ziehen
                root.mustSayUno = false
                root.handCount += root.unoPenaltyCards
                root.penaltyDraw(root.unoPenaltyCards)
                infoBanner.show("UNO vergessen! +" + root.unoPenaltyCards + " Karten")
            }
        }
    }
}
