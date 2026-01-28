import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    anchors.fill: parent

    // Server config von Main.qml
    property string serverHost: "127.0.0.1"
    property int serverPort: 12345

    // Server-Spielcode (wichtig fürs Starten)
    property string hostCode: ""

    signal goBack()
    signal copyLinkRequested()
    signal openSkins()
    signal startGame()

    // Hintergrund
    Image {
        anchors.fill: parent
        source: "qrc:/assets/images/UNOHintergrund.jpg"
        fillMode: Image.PreserveAspectCrop
        smooth: true
    }

    // Go Back Button (oben links)
    Button {
        text: "Zurueck"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 18
        width: 130
        height: 44
        font.pixelSize: 16

        background: Rectangle {
            color: "white"
            border.color: "black"
            border.width: 2
        }

        onClicked: root.goBack()
    }

    // Link kopieren (links, mittig) -> jetzt: Server connect + create_game + optional Clipboard via LinkService
    Button {
        id: copyBtn
        text: "Link kopieren"
        width: 230
        height: 72
        anchors.left: parent.left
        anchors.leftMargin: 120
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 10
        font.pixelSize: 20

        background: Rectangle {
            color: "#ff8f98"
            border.color: "black"
            border.width: 2
        }

        onClicked: {
            if (!gameClient.connected) {
                gameClient.connectToServer(root.serverHost, root.serverPort)
            }
            root.hostCode = ""      // neu anfordern
            gameClient.createGame() // Code kommt via onGameCreated
        }
    }

    // Skins (rechts, mittig)
    Button {
        id: skinsBtn
        text: "Skins"
        width: 230
        height: 72
        anchors.right: parent.right
        anchors.rightMargin: 120
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 10
        font.pixelSize: 20

        background: Rectangle {
            color: "#e6ff6a"
            border.color: "black"
            border.width: 2
        }

        onClicked: root.openSkins()
    }

    // Zentrum: Formular (unverändert)
    Item {
        id: form
        width: 560
        height: 290
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 40

        Rectangle {
            width: 200
            height: 34
            x: (form.width - width) / 2
            y: 0
            color: "white"
            border.width: 2
            border.color: "black"

            Text {
                anchors.centerIn: parent
                text: "Spielername"
                font.pixelSize: 18
                color: "black"
            }
        }

        TextField {
            id: nameField
            width: form.width
            height: 60
            x: 0
            y: 26
            font.pixelSize: 20
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            background: Rectangle { color: "white"; border.width: 2; border.color: "black" }
        }

        Rectangle {
            width: 240
            height: 44
            x: 30
            y: 110
            color: "#d9d9d9"
            border.width: 2
            border.color: "black"

            Text {
                anchors.centerIn: parent
                text: "Spieleranzahl:"
                font.pixelSize: 18
                color: "black"
            }
        }

        SpinBox {
            id: countSpin
            width: 220
            height: 44
            x: 270
            y: 110
            from: 2
            to: 10
            value: 4

            background: Rectangle { color: "white"; border.width: 2; border.color: "black" }
        }

        Button {
            id: startBtn
            text: "Spiel starten"
            width: 520
            height: 62
            x: (form.width - width) / 2
            y: 170
            font.pixelSize: 22

            background: Rectangle { color: "#75f0ff"; border.width: 2; border.color: "black" }

            onClicked: root.startGame()
        }
    }

    // Auto-connect beim Öffnen (damit “Nicht verbunden” nicht dauernd kommt)
    Component.onCompleted: {
        if (!gameClient.connected) {
            gameClient.connectToServer(root.serverHost, root.serverPort)
        }
    }

    Connections {
        target: gameClient

        function onGameCreated(code) {
            root.hostCode = code
            // optional: in Zwischenablage kopieren über deinen bestehenden Service:
            // (dein LinkService generiert sonst eigenen Code, daher hier NICHT createCopyAndSendCode nutzen)
            // Wenn du Clipboard exakt willst: sag kurz, dann gebe ich dir copyToClipboard(code) für LinkService.
            console.log("HOST CODE:", code)
        }
    }
}
