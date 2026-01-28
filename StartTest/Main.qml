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
            id: hostPage
            anchors.fill: parent

            serverHost: Window.serverHost
            serverPort: Window.serverPort

            onGoBack: stack.pop()

            // Dein Button "Spiel starten" löst dieses Signal aus:
            onStartGame: {
                if (!gameClient.connected) {
                    toast.show("Nicht verbunden. Erst mit Server verbinden.")
                    return
                }
                if (hostPage.hostCode.trim().length === 0) {
                    toast.show("Noch kein Spielcode. Erst 'Link kopieren' / Spiel erstellen.")
                    return
                }
                gameClient.startGame(hostPage.hostCode)
                toast.show("start_game gesendet: " + hostPage.hostCode)
            }
        }
    }

    Component {
        id: joinPageComponent
        JoinPage {
            id: joinPage
            anchors.fill: parent

            serverHost: Window.serverHost
            serverPort: Window.serverPort

            onGoBack: stack.pop()

            // Dein Button "Spiel beitreten" löst dieses Signal aus:
            onJoinGame: {
                if (!gameClient.connected) {
                    gameClient.connectToServer(joinPage.serverHost, joinPage.serverPort)
                }
                if (joinPage.gameCode.trim().length === 0) {
                    toast.show("Bitte Spielcode eingeben.")
                    return
                }
                gameClient.joinGame(joinPage.gameCode)
                toast.show("join_game gesendet: " + joinPage.gameCode)
            }
        }
    }

    Connections {
        target: gameClient

        function onGameCreated(code) { toast.show("Spiel erstellt: " + code) }
        function onJoinOk(code) { toast.show("Beitritt OK: " + code) }
        function onError(msg) { toast.show("Server: " + msg) }

        function onGameInit(code, hand, discardTop, drawCount, players, yourIndex) {
            toast.show("game_init erhalten (" + players + " Spieler)")
            stack.push(gamePageComponent)
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

        function onGameInit(code, hand, discardTop, drawCount, players, yourIndex) {
            toast.show("Game init: " + code + " (" + players + " Spieler)")
            stack.push(gamePageComponent)
        }
    }
}
