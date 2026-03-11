import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    id: win
    width: 1280
    height: 720
    visible: true
    title: "UNO"

    property string serverHost: "127.0.0.1"
    property int serverPort: 12345

    //Start Fenster mit der "Spiel hosten" oder "Mit Code beitreten" auswahl
    StackView {
        id: stack
        anchors.fill: parent

        initialItem: StartMenu {
            anchors.fill: parent
            onHostGame: stack.push(startPageComponent)
            onJoinWithCode: stack.push(joinPageComponent)
        }
    }

    //startPageComponent ist das Fenster des Spielhostings
    Component {
        id: startPageComponent
        StartPage {
            anchors.fill: parent
            serverHost: win.serverHost
            serverPort: win.serverPort
            onGoBack: stack.pop()
        }
    }

    //joinPageComponent ist das Fenster des Spielbeitritts
    Component {
        id: joinPageComponent
        JoinPage {
            anchors.fill: parent
            serverHost: win.serverHost
            serverPort: win.serverPort
            onGoBack: stack.pop()
        }
    }

    //gamePageComponent ist das Fenster, in dem das UNO Spiel stattfindet.
    Component {
        id: gamePageComponent
        GamePage {
            anchors.fill: parent
            onGoBack: stack.pop()
        }
    }

    function ensureGamePageOpen() {
        // Wenn nicht schon GamePage, öffnen
        if (stack.currentItem && stack.currentItem.objectName === "GamePage")
            return
        stack.push(gamePageComponent)
    }

    //GameClient regelt den Austausch zwischen Server und Client
    Connections {
        target: gameClient

        //function onInfo(msg) { toast.show(msg) }
        function onError(msg) { toast.show("Server: " + msg) }

        //function onGameCreated(code) { toast.show("Spiel erstellt: " + code) }
        function onJoinOk(code) { toast.show("Beitritt OK: " + code) }

        function onGameStateChanged() {
            if (gameClient.hasGameInit) {
                ensureGamePageOpen()
            }
        }
    }

    //LinkService setzt den Link ins Clipboard
    Connections {
        target: linkService
        function onInfo(message) { toast.show(message) }
        function onError(message) { toast.show(message) }
    }

    //Zeigt die ganzen Toasts an, z.B "Beitritt Ok..."
    Popup {
        id: toast
        modal: false
        focus: false
        x: (parent.width - width) / 2
        y: 20
        width: 600
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
}
