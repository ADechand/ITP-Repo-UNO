import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    anchors.fill: parent

    signal goBack()
    signal copyLinkRequested()
    signal createCopyAndSendCode()
    signal openSkins()
    signal startGame()


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

    // Link kopieren (links, mittig)
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

        onClicked: linkService.createCopyAndSendCode()

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

    // Zentrum: Formular
    Item {
        id: form
        width: 560
        height: 290
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 40

        // Tab "Spielername"
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

        // Name Input
        TextField {
            id: nameField
            width: form.width
            height: 60
            x: 0
            y: 26
            font.pixelSize: 20
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            background: Rectangle {
                color: "white"
                border.width: 2
                border.color: "black"
            }
        }
        signal startGame()

        // Spieleranzahl Label
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


        // Spieleranzahl SpinBox
        SpinBox {
            id: countSpin
            width: 220
            height: 44
            x: 270
            y: 110
            from: 2
            to: 10
            value: 4

            background: Rectangle {
                color: "white"
                border.width: 2
                border.color: "black"
            }
        }

        // Spiel starten Button
        Button {
            id: startBtn
            text: "Spiel starten"
            width: 520
            height: 62
            x: (form.width - width) / 2
            y: 170
            font.pixelSize: 22
            onClicked: root.startGame()

            background: Rectangle {
                color: "#75f0ff"
                border.width: 2
                border.color: "black"
            }
        }
    }
}
