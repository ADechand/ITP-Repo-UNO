import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    width: 1280
    height: 720
    visible: true
    title: "UNO"

    // Server-Adresse (für Test)
    property string serverHost: "127.0.0.1"
    property int serverPort: 12345

    StackView {
        id: stack
        anchors.fill: parent

        initialItem: StartMenu {
            anchors.fill: parent
            onHostGame: stack.push(startPageComponent)
            onJoinWithCode: stack.push(joinPageComponent)
        }
    }

    Component {
        id: startPageComponent
        StartPage {
            anchors.fill: parent
            onGoBack: stack.pop()

            // Link kopieren bleibt, aber Server-Code-Erstellung läuft über gameClient
            onCopyLinkRequested: linkService.createCopyAndSendLink()

            onOpenSkins: console.log("Skins (todo)")

            // Button "Spiel starten" -> NUR start_game senden (GamePage erst bei game_init)
            onStartGame: {
                if (!gameClient.connected) {
                    toast.show("Nicht verbunden. Erst mit Server verbinden.")
                    return
                }
                if (root.hostCode === "") {
                    toast.show("Noch kein Spielcode. Erst Spiel erstellen / Link kopieren.")
                    return
                }
                gameClient.startGame(root.hostCode)
                toast.show("Starte Spiel...")
            }

            // StartPage braucht Zugriff auf Host/Port
            serverHost: Window.serverHost
            serverPort: Window.serverPort
        }
    }

    Component {
        id: joinPageComponent
        JoinPage {
            anchors.fill: parent
            onGoBack: stack.pop()

            serverHost: Window.serverHost
            serverPort: Window.serverPort

            // Button "Spiel beitreten" -> join_game senden (GamePage erst bei game_init)
            onJoinGame: {
                if (!gameClient.connected) {
                    gameClient.connectToServer(serverHost, serverPort)
                }
                gameClient.joinGame(root.gameCode)
                toast.show("Beitritt gesendet...")
            }
        }
    }

    Component {
        id: gamePageComponent
        GamePage {
            anchors.fill: parent
            onGoBack: stack.pop()
        }
    }

    Popup {
        id: toast
        modal: false
        focus: false
        x: (parent.width - width) / 2
        y: 20
        width: 520
        height: 60
        closePolicy: Popup.NoAutoClose

        background: Rectangle {
            color: "white"
            border.color: "black"
            border.width: 2
        }

        property string msg: ""
        contentItem: Text {
            text: toast.msg
            anchors.centerIn: parent
            font.pixelSize: 16
            color: "black"
        }

        Timer {
            id: toastTimer
            interval: 1600
            running: false
            repeat: false
            onTriggered: toast.close()
        }

        function show(message) {
            toast.msg = message
            toast.open()
            toastTimer.restart()
        }
    }

    // Deine bisherigen LinkService-Toast-Events
    Connections {
        target: linkService
        function onInfo(message) { toast.show(message) }
        function onError(message) { toast.show(message) }
    }

    // NEU: GameClient-Events + Navigation NUR bei game_init
    Connections {
        target: gameClient

        function onInfo(msg) { toast.show(msg) }
        function onError(msg) { toast.show("Server: " + msg) }

        function onGameInit(code, hand, discardTop, drawCount, players, yourIndex, currentPlayerIndex, handCounts) {
            toast.show("Game init: " + code + " (" + players + " Spieler)")
            stack.push(gamePageComponent)
        }
    }
}
