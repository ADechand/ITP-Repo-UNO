import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    anchors.fill: parent

    signal goBack()
    //signal joinGame(string playerName, string gameCode)
    signal joinGame()


    // Hintergrund
    Image {
        anchors.fill: parent
        source: "qrc:/assets/images/UNOHintergrund.jpg"   // <-- anpassen
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

    Item {
        id: form
        width: 560
        height: 290
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 20

        Rectangle {
            width: 200; height: 34
            x: (form.width - width)/2
            y: 0
            color: "white"
            border.width: 2
            border.color: "black"
            Text { anchors.centerIn: parent; text: "Spielername"; font.pixelSize: 18; color: "black" }
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
            width: 240; height: 44
            x: 30
            y: 110
            color: "#d9d9d9"
            border.width: 2
            border.color: "black"
            Text { anchors.centerIn: parent; text: "Spielcode:"; font.pixelSize: 18; color: "black" }
        }

        TextField {
            id: codeField
            width: 220
            height: 44
            x: 270
            y: 110
            font.pixelSize: 18
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            background: Rectangle { color: "white"; border.width: 2; border.color: "black" }

            onTextChanged: {
                const up = text.toUpperCase()
                if (text !== up) text = up
            }
        }

        Button {
            id: joinBtn
            text: "Spiel beitreten"
            width: 520
            height: 62
            x: (form.width - width)/2
            y: 170
            font.pixelSize: 22

            background: Rectangle { color: "#75f0ff"; border.width: 2; border.color: "black" }

            enabled: nameField.text.trim().length > 0 && codeField.text.trim().length > 0
            opacity: enabled ? 1.0 : 0.65
            onClicked: root.joinGame()
            //onClicked: root.joinGame(nameField.text.trim(), codeField.text.trim())
        }
    }
}
