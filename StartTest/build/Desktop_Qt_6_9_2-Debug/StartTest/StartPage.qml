import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    anchors.fill: parent

    property string serverHost: "127.0.0.1"
    property int serverPort: 12345
    property string hostCode: ""

    signal goBack()

    // Wenn noch nicht verbunden: Action merken
    property bool _pendingCreate: false
    property bool _pendingStart: false

    // Reagiere zentral auf Verbindungen
    Connections {
        target: gameClient
        function onConnectedChanged() {
            if (!gameClient.connected) return

            //Erstellt ein Spiel wenn es noch nicht erstellt wurde
            if (root._pendingCreate) {
                root._pendingCreate = false
                gameClient.createGame()
            }
            //Startet das Spiel wenn noch kein Hostcode vorhanden ist
            if (root._pendingStart) {
                root._pendingStart = false
                if (root.hostCode.trim().length > 0)
                    gameClient.startGame(root.hostCode.trim())
            }
        }

        //Kopiert Code in die Zwischenablage
        function onGameCreated(code) {
            root.hostCode = code
            // direkt kopieren + Info kommt als Toast (Main.qml hört auf linkService.info)
            linkService.copyToClipboard(code)
        }
    }

    //Setzt das Hintergrundbild
    Image {
        anchors.fill: parent
        source: "qrc:/assets/images/UNOHintergrund.jpg"
        fillMode: Image.PreserveAspectCrop
        smooth: true
    }

    //Zurück Button zur Host/Join auswahl
    Button {
        text: "Zurueck"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 18
        width: 130
        height: 44
        font.pixelSize: 16
        background: Rectangle { color: "white"; border.color: "black"; border.width: 2 }
        onClicked: root.goBack()
    }

    // Link kopieren: wenn noch kein Spiel => erstellen, sonst Code kopieren
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

        background: Rectangle { color: "#ff8f98"; border.color: "black"; border.width: 2 }

        onClicked: {
            if (root.hostCode.trim().length > 0) {
                linkService.copyToClipboard(root.hostCode.trim())
                return
            }

            // Spiel erstellen
            if (!gameClient.connected) {
                root._pendingCreate = true
                gameClient.connectToServer(root.serverHost, root.serverPort)
                return
            }
            gameClient.createGame()
        }
    }

    //Button um zur Skinauswahl zu kommen
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
        background: Rectangle { color: "#e6ff6a"; border.color: "black"; border.width: 2 }
    }


    Item {
        id: form
        width: 560
        height: 290
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 40

        //Text der hinweist, dass das Feld darunter für den Spielernamen ist
        Rectangle {
            width: 200; height: 34
            x: (form.width - width) / 2
            y: 0
            color: "white"
            border.width: 2
            border.color: "black"
            Text { anchors.centerIn: parent; text: "Spielername"; font.pixelSize: 18; color: "black" }
        }

        //Textfeld für den Spielernamen
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

        //Text, der hinweist, dass das Feld rechts daneben für die Spieleranzahl ist
        Rectangle {
            width: 240; height: 44
            x: 30
            y: 110
            color: "#d9d9d9"
            border.width: 2
            border.color: "black"
            Text { anchors.centerIn: parent; text: "Spieleranzahl:"; font.pixelSize: 18; color: "black" }
        }

        //Feld, bei dem man die Spielerzahl eingibt, Spinbox ermöglicht zeigt Pfeile an, um die Zahl anzupassen
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

        //Button ganz Unten, der das Spiel startet
        Button {
            id: startBtn
            text: "Spiel starten"
            width: 520
            height: 62
            x: (form.width - width) / 2
            y: 170
            font.pixelSize: 22
            background: Rectangle { color: "#75f0ff"; border.width: 2; border.color: "black" }

            onClicked: {
                //Prüft ob das Spiel schon erstellt wurde
                const code = root.hostCode.trim()
                if (code.length === 0) {
                    // Noch kein Spiel erstellt -> Hinweis
                    // (Code wird nach "Link kopieren" / create_game gesetzt)
                    return
                }

                //Prüft ob Client verbunden ist
                if (!gameClient.connected) {
                    root._pendingStart = true
                    gameClient.connectToServer(root.serverHost, root.serverPort)
                    return
                }

                //Beginnt das Spiel
                gameClient.startGame(code)
            }
        }
    }

    //Stellt sicher dass der Client zum Server verbunden ist
    Component.onCompleted: {
        if (!gameClient.connected) {
            gameClient.connectToServer(root.serverHost, root.serverPort)
        }
    }
}
