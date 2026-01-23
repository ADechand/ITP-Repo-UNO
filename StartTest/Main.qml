import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    width: 1280
    height: 720
    visible: true
    title: "UNO"

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

            // Link kopieren (dein C++ Service)
            onCopyLinkRequested: linkService.createCopyAndSendLink()

            onOpenSkins: console.log("Skins (todo)")

            // >>> NEU: Spiel starten -> GamePage <<<
            onStartGame: stack.push(gamePageComponent)
        }
    }

    Component {
        id: joinPageComponent
        JoinPage {
            anchors.fill: parent
            onGoBack: stack.pop()

            // >>> NEU: Spiel beitreten -> GamePage <<<
            onJoinGame: stack.push(gamePageComponent)
        }
    }

    // >>> NEU: GamePage als letzte Seite <<<
    Component {
        id: gamePageComponent
        GamePage {
            anchors.fill: parent
            onGoBack: stack.pop()
        }
    }

    // --- Toast bleibt gleich ---
    Popup {
        id: toast
        modal: false
        focus: false
        x: (parent.width - width) / 2
        y: 20
        width: 340
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

    // >>> WICHTIG: target muss zu dem Namen passen, den du aus C++ exposest.
    // Du nutzt unten "linkService", also nutze auch oben linkService.createCopyAndSendLink()
    Connections {
        target: linkService
        function onInfo(message) { toast.show(message) }
        function onError(message) { toast.show(message) }
        function onCodeCreated(code) { /* optional */ }
    }
}
