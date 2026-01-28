import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    // wichtig: NICHT hart auf 1280x720 festnageln
    anchors.fill: parent

    signal hostGame()
    signal joinWithCode()

    // Hintergrund füllt immer das ganze Fenster
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#2fd000" }
            GradientStop { position: 1.0; color: "#148800" }
        }
    }

    Column {
        id: col
        anchors.centerIn: parent
        spacing: 40

        // großer Button
        Button {
            id: hostBtn
            text: "Spiel hosten"
            width: 420
            height: 90
            font.pixelSize: 36
            anchors.horizontalCenter: parent.horizontalCenter

            background: Rectangle {
                color: "white"
                border.color: "black"
                border.width: 2
            }

            onClicked: root.hostGame()
        }

        // kleiner Button (aber trotzdem exakt mittig)
        Button {
            id: joinBtn
            text: "Mit Code beitreten"
            width: 260
            height: 50
            font.pixelSize: 18
            anchors.horizontalCenter: parent.horizontalCenter

            background: Rectangle {
                color: "white"
                border.color: "black"
                border.width: 2
            }

            onClicked: root.joinWithCode()
        }
    }
}
