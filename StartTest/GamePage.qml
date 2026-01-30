import QtQuick 2.15
import QtQuick.Controls 2.15
import Qt.labs.platform 1.1

Item {
    id: root
    anchors.fill: parent
    objectName: "GamePage"

    signal goBack()

    property int opponentsCount: 0
    property string lastDiscard: ""
    property string lastDiscardId: ""
    property int lastHandCount: 0
    property int lastDrawCount: 0
    property string cardBase: "qrc:/assets/images/cards/"
    property string pendingWildCard: ""
    property bool unoDeclared: false
    property bool endPopupShown: false
    property var graphData: []

    // Normalisiert alte Namen: "Blau 2" -> "Blau_2.jpg"
    // lässt neue Namen wie "Blau_2.jpg" unverändert
    function normalizeCardName(name) {
        if (!name) return ""
        var s = String(name).trim()

        // falls der Server noch ohne Endung schickt:
        if (!s.endsWith(".jpg")) s = s + ".jpg"

        // Leerzeichen -> Unterstriche
        s = s.replace(/ /g, "_")

        // falls irgendwo doppelte Unterstriche entstehen
        s = s.replace(/__+/g, "_")

        return s
    }

    function fileToQrc(fileName) {
        var n = normalizeCardName(fileName)
        if (n.length === 0) return ""
        return cardBase + n
    }


    function applyStateFromClient() {
        if (!gameClient.hasGameInit) return

        lastDiscard = fileToQrc(gameClient.discardTop)
        lastDiscardId = normalizeCardName(gameClient.discardTop)
        opponentsCount = Math.max(0, gameClient.players - 1)

        // Debug/Status
        lastHandCount = gameClient.hand ? gameClient.hand.length : 0
        lastDrawCount = gameClient.drawCount
        if (lastHandCount !== 1) {
            unoDeclared = false
        }

        infoBanner.show("Hand: " + lastHandCount + " | Ablage: " + gameClient.discardTop + " | Deck: " + lastDrawCount)

        if (gameClient.finished) {
            if (gameClient.winnerIndex === gameClient.yourIndex) {
                infoBanner.show("Spiel beendet! Du hast gewonnen.")
            } else if (gameClient.winnerIndex >= 0) {
                infoBanner.show("Spiel beendet! Gewinner: Spieler " + (gameClient.winnerIndex + 1))
            } else {
                infoBanner.show("Spiel beendet!")
            }
            if (!endPopupShown) {
                endPopupShown = true
                graphData = buildGraphData(gameClient.gameLog, gameClient.players)
                gameGraph.requestPaint()
                endGamePopup.open()
            }
        }
    }

    Component.onCompleted: applyStateFromClient()

    Connections {
        target: gameClient
        function onGameStateChanged() { applyStateFromClient() }
        function onError(msg) { infoBanner.show("Server: " + msg) }
    }

    Rectangle { anchors.fill: parent; color: "#ff9fa0" }

    Button {
        text: "Go Back"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 16
        anchors.topMargin: 12
        background: Rectangle { color: "white"; border.color: "black"; border.width: 2; radius: 4 }
        onClicked: root.goBack()
    }

    Rectangle {
        id: infoBanner
        width: 720; height: 44; radius: 8
        color: "white"; border.color: "black"; border.width: 2
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 10
        opacity: 0
        visible: opacity > 0

        Text { id: infoText; anchors.centerIn: parent; font.pixelSize: 16; color: "black"; text: "" }

        function show(msg) {
            infoText.text = msg
            infoBanner.opacity = 1
            hideTimer.restart()
        }

        Timer { id: hideTimer; interval: 1800; repeat: false; onTriggered: infoBanner.opacity = 0 }
    }

    Popup {
        id: colorPicker
        modal: true
        focus: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 320
        height: 220
        closePolicy: Popup.CloseOnEscape

        background: Rectangle {
            color: "white"
            border.color: "black"
            border.width: 2
        }

        Column {
            anchors.centerIn: parent
            spacing: 10
            Text { text: "Farbe wählen"; font.pixelSize: 18; color: "black" }

            Row {
                spacing: 12
                Button { text: "Rot"; onClicked: chooseColor("Rot") }
                Button { text: "Grün"; onClicked: chooseColor("Gruen") }
                Button { text: "Blau"; onClicked: chooseColor("Blau") }
                Button { text: "Gelb"; onClicked: chooseColor("Gelb") }
            }

            Button {
                text: "Abbrechen"
                onClicked: {
                    pendingWildCard = ""
                    colorPicker.close()
                }
            }
        }
    }

    // Gegnerkarten (oben)
    Row {
        id: enemyRow
        spacing: 24
        anchors.top: parent.top
        anchors.topMargin: 60
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            model: opponentsCount
            delegate: Item {
                height: 130
                property int globalIndex: index < gameClient.yourIndex ? index : index + 1
                property int cardCount: gameClient.handCounts.length > globalIndex ? gameClient.handCounts[globalIndex] : 0
                width: Math.max(90, cardCount * 70)

                Row {
                    spacing: 6
                    Repeater {
                        model: cardCount
                        delegate: Image {
                            width: 60
                            height: 90
                            source: "qrc:/assets/images/cards/Gegnerkarte.jpg"
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }
                    }
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.bottom
                    anchors.topMargin: 6
                    text: showCount ? (cardCount + " Karten") : "?"
                    font.pixelSize: 16
                    color: "black"
                }
            }
        }
    }

    // Mitte (Ablage + Ziehen)
    Item {
        anchors.centerIn: parent
        width: 520
        height: 280

        Rectangle {
            width: 120; height: 180; radius: 8
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            color: "white"; border.color: "black"; border.width: 2

            // Discard (mit Fallback .png <-> .jpg falls mal inkonsistent)
            Image {
                id: discardImg
                anchors.fill: parent
                anchors.margins: 6
                source: lastDiscard
                visible: lastDiscard !== ""
                fillMode: Image.PreserveAspectFit
                smooth: true

                onStatusChanged: {
                    if (status === Image.Error && source.indexOf(".png") !== -1) {
                        source = source.replace(".png", ".jpg")
                    } else if (status === Image.Error && source.indexOf(".jpg") !== -1) {
                        source = source.replace(".jpg", ".png")
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text: lastDiscard === "" ? "Ablage" : ""
                color: "black"
            }
        }

        Button {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 120
            height: 180
            text: "Karte\nziehen"
            font.pixelSize: 16
            enabled: gameClient.hasGameInit && isYourTurn()
            background: Rectangle { color: "#d9d9d9"; border.color: "black"; border.width: 2; radius: 8 }

            onClicked: {
                if (!gameClient.hasGameInit) {
                    infoBanner.show("Spiel nicht initialisiert.")
                    return
                }
                if (!isYourTurn()) {
                    infoBanner.show("Nicht dein Zug.")
                    return
                }
                gameClient.drawCards(1)
            }
        }
    }

    // Hand (unten) – direkt aus gameClient.hand (kein extra ListModel nötig)
    Row {
        id: playerHand
        spacing: 18
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            model: gameClient.hand

            delegate: Item {
                width: 90
                height: 130

                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: "white"
                    border.color: "black"
                    border.width: 2

                    Image {
                        id: cardImg
                        anchors.fill: parent
                        anchors.margins: 6
                        fillMode: Image.PreserveAspectFit
                        smooth: true

                        property string originalName: modelData
                        source: fileToQrc(originalName)

                        // Fallback, falls irgendwo doch noch .png/.jpg nicht passt
                        onStatusChanged: {
                            if (status !== Image.Error) return
                            if (source.indexOf(".png") !== -1) source = source.replace(".png", ".jpg")
                            else if (source.indexOf(".jpg") !== -1) source = source.replace(".jpg", ".png")
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (!isYourTurn()) {
                                infoBanner.show("Nicht dein Zug.")
                                return
                            }
                            if (!isLegalCard(modelData)) {
                                infoBanner.show("Diese Karte ist nicht erlaubt.")
                                return
                            }
                            if (requiresColor(modelData)) {
                                pendingWildCard = modelData
                                colorPicker.open()
                                return
                            }
                            gameClient.playCard(modelData)
                        }
                    }
                }
            }
        }
    }

    Button {
        id: unoButton
        text: "UNO!"
        width: 120
        height: 50
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 30
        anchors.bottomMargin: 120
        visible: gameClient.hasGameInit && !gameClient.finished && gameClient.hand.length === 1
        enabled: !unoDeclared

        background: Rectangle {
            color: unoDeclared ? "#cccccc" : "#ff4444"
            radius: 10
            border.color: "black"
            border.width: 2
        }

        onClicked: {
            unoDeclared = true
            infoBanner.show("UNO!")
            gameClient.declareUno()
        }
    }

    FileDialog {
        id: logFileDialog
        title: "CSV speichern"
        nameFilters: ["CSV Dateien (*.csv)"]
        fileMode: FileDialog.SaveFile
        onAccepted: {
            if (!gameClient.saveGameLog(logFileDialog.file)) {
                infoBanner.show("CSV konnte nicht gespeichert werden.")
            } else {
                infoBanner.show("CSV gespeichert.")
            }
        }
    }

    Popup {
        id: endGamePopup
        modal: true
        focus: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 640
        height: 420
        closePolicy: Popup.NoAutoClose

        background: Rectangle {
            color: "white"
            border.color: "black"
            border.width: 2
        }

        Column {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Text {
                text: gameClient.winnerIndex === gameClient.yourIndex
                      ? "Du hast gewonnen!"
                      : "Du hast verloren!"
                font.pixelSize: 22
                color: "black"
            }

            Button {
                text: "CSV herunterladen"
                enabled: gameClient.hasGameLog
                onClicked: logFileDialog.open()
            }

            Button {
                text: "Graph anzeigen"
                onClicked: gameGraph.visible = !gameGraph.visible
            }

            Canvas {
                id: gameGraph
                width: parent.width - 32
                height: 220
                visible: graphData.length > 0
                onVisibleChanged: if (visible) requestPaint()
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    if (graphData.length === 0) return

                    var seriesCount = graphData[0].length
                    var pointsCount = graphData.length
                    var maxVal = 1
                    for (var i = 0; i < graphData.length; i++) {
                        for (var j = 0; j < graphData[i].length; j++) {
                            maxVal = Math.max(maxVal, graphData[i][j])
                        }
                    }

                    var colors = ["#e74c3c", "#3498db", "#2ecc71", "#f1c40f", "#9b59b6", "#e67e22"]
                    var padding = 20
                    var plotW = width - padding * 2
                    var plotH = height - padding * 2

                    ctx.strokeStyle = "#000000"
                    ctx.beginPath()
                    ctx.moveTo(padding, padding)
                    ctx.lineTo(padding, padding + plotH)
                    ctx.lineTo(padding + plotW, padding + plotH)
                    ctx.stroke()

                    for (var p = 0; p < seriesCount; p++) {
                        ctx.strokeStyle = colors[p % colors.length]
                        ctx.beginPath()
                        for (var x = 0; x < pointsCount; x++) {
                            var value = graphData[x][p]
                            var px = padding + (pointsCount === 1 ? 0 : (x / (pointsCount - 1)) * plotW)
                            var py = padding + plotH - (value / maxVal) * plotH
                            if (x === 0) ctx.moveTo(px, py)
                            else ctx.lineTo(px, py)
                        }
                        ctx.stroke()
                    }
                }
            }
        }
    }

    function isYourTurn() {
        return gameClient.hasGameInit &&
               !gameClient.finished &&
               gameClient.currentPlayerIndex === gameClient.yourIndex
    }

    function activeOpponentIndex() {
        if (!gameClient.hasGameInit || gameClient.players === 0) {
            return -1
        }
        if (gameClient.currentPlayerIndex === gameClient.yourIndex) {
            return (gameClient.yourIndex + 1) % gameClient.players
        }
        return gameClient.currentPlayerIndex
    }

    function parseCard(cardId) {
        var base = normalizeCardName(cardId)
        var dot = base.lastIndexOf(".")
        if (dot >= 0) {
            base = base.slice(0, dot)
        }
        var parts = base.split("_")
        if (parts.length === 0) {
            return { color: "", value: "", wild: false }
        }
        if (parts[0] === "Extra") {
            return { color: "Extra", value: parts.slice(1).join("_"), wild: true }
        }
        return { color: parts[0], value: parts.slice(1).join("_"), wild: false }
    }

    function isLegalCard(cardId) {
        if (!lastDiscardId || lastDiscardId.length === 0) {
            return true
        }
        var playInfo = parseCard(cardId)
        if (playInfo.wild) {
            return true
        }
        var topInfo = parseCard(lastDiscardId)
        if (topInfo.wild) {
            return gameClient.currentColor && playInfo.color === gameClient.currentColor
        }
        return playInfo.color === topInfo.color || playInfo.value === topInfo.value
    }

    function requiresColor(cardId) {
        var info = parseCard(cardId)
        return info.wild
    }

    function chooseColor(colorName) {
        if (pendingWildCard.length === 0) {
            colorPicker.close()
            return
        }
        gameClient.playCard(pendingWildCard, colorName)
        pendingWildCard = ""
        colorPicker.close()
    }

    function buildGraphData(logCsv, playerCount) {
        if (!logCsv || logCsv.length === 0 || playerCount === 0) {
            return []
        }
        var lines = logCsv.split("\n")
        var headerSkipped = false
        var counts = []
        var series = []

        function parseLine(line) {
            var first = line.indexOf(",")
            if (first < 0) return null
            var second = line.indexOf(",", first + 1)
            if (second < 0) return null
            var third = line.indexOf(",", second + 1)
            if (third < 0) return null
            return {
                event: line.slice(first + 1, second),
                playerIndex: parseInt(line.slice(second + 1, third)),
                detail: line.slice(third + 1)
            }
        }

        for (var i = 0; i < lines.length; i++) {
            var line = lines[i].trim()
            if (line.length === 0) continue
            if (!headerSkipped) {
                headerSkipped = true
                continue
            }
            var parsed = parseLine(line)
            if (!parsed) continue
            if (parsed.event === "start") {
                var handsIndex = parsed.detail.indexOf("hands=")
                if (handsIndex >= 0) {
                    var handsPart = parsed.detail.slice(handsIndex + 6)
                    var comma = handsPart.indexOf(",")
                    if (comma >= 0) handsPart = handsPart.slice(0, comma)
                    var parts = handsPart.split("|")
                    counts = []
                    for (var p = 0; p < playerCount; p++) {
                        counts.push(parseInt(parts[p] || "0"))
                    }
                    series.push(counts.slice())
                }
                continue
            }

            if (counts.length === 0) {
                counts = Array(playerCount).fill(0)
            }

            if (parsed.event === "play") {
                if (parsed.playerIndex >= 0 && parsed.playerIndex < playerCount) {
                    counts[parsed.playerIndex] = Math.max(0, counts[parsed.playerIndex] - 1)
                }
            } else if (parsed.event === "draw" || parsed.event === "draw_four" || parsed.event === "uno_penalty") {
                if (parsed.playerIndex >= 0 && parsed.playerIndex < playerCount) {
                    var drawAmount = 0
                    var match = parsed.detail.match(/drawn=(\d+)/)
                    if (match) drawAmount = parseInt(match[1])
                    if (!drawAmount && parsed.event === "draw") drawAmount = parseInt(parsed.detail)
                    counts[parsed.playerIndex] += drawAmount
                }
            }

            series.push(counts.slice())
        }

        return series
    }
}
