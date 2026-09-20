import QtQuick

Rectangle {
    id: root

    property var s: ({})
    property string mode: "study"
    property string searchDraft: ""
    property string browseBook: ""
    property int browseChapter: 1

    signal wordSelected(int index)
    signal strongSelected(string strongId)
    signal goBack()
    signal navigate(string direction)
    signal searchVerse(string query)
    signal navigateTo(string reference)
    signal occurrencePageSizeChanged(int size)

    property int reportedOccurrencePageSize: 0

    readonly property bool landscape: width > height
    readonly property color paper: "#FFFFFF"
    readonly property color ink: "#000000"
    readonly property color faint: "#5A5A5A"
    readonly property color accent: "#A4123F"
    readonly property real designLong: 2160
    readonly property real designShort: Math.round(
        designLong * Math.min(width, height) / Math.max(width, height))
    readonly property real canvasW: landscape ? designLong : designShort
    readonly property real canvasH: landscape ? designShort : designLong

    function v(key, fallback) {
        return s && s[key] !== undefined && s[key] !== null ? s[key] : fallback
    }

    function selected(key, fallback) {
        var word = v("selectedWord", {})
        return word && word[key] !== undefined && word[key] !== null
               ? word[key] : fallback
    }

    function bookInfo(name) {
        var books = v("books", [])
        for (var i = 0; i < books.length; ++i) {
            if (books[i].name === name)
                return books[i]
        }
        return { name: name, chapters: [] }
    }

    function finishSearch() {
        root.searchVerse(root.searchDraft)
    }

    function dismissSearchInput() {
        searchInput.focus = false
        Qt.inputMethod.hide()
    }

    function submitSearch() {
        root.finishSearch()
        root.dismissSearchInput()
    }

    function syncReadingPosition() {
        if (root.mode !== "read")
            return
        Qt.callLater(function() {
            var verses = root.v("chapterVerses", [])
            var targetVerse = root.v("verse", 1)
            var targetIndex = -1
            for (var i = 0; i < verses.length; ++i) {
                if (Number(verses[i].verse) === Number(targetVerse)) {
                    targetIndex = i
                    break
                }
            }
            if (targetIndex >= 0 && readingList.count > 0)
                readingList.positionViewAtIndex(targetIndex, ListView.Center)
        })
    }

    function reportOccurrencePageSize() {
        Qt.callLater(function() {
            var available = occurrencesList.height
            if (available <= 0)
                return
            var count = Math.max(1, Math.floor((available + 10) / 68))
            count = Math.min(12, count)
            if (count === root.reportedOccurrencePageSize)
                return
            root.reportedOccurrencePageSize = count
            root.occurrencePageSizeChanged(count)
        })
    }

    onSChanged: {
        if (browseBook.length === 0)
            browseBook = v("book", "Genesis")
        if (browseChapter <= 0)
            browseChapter = v("chapter", 1)
        syncReadingPosition()
        reportOccurrencePageSize()
    }

    onModeChanged: syncReadingPosition()
    onWidthChanged: reportOccurrencePageSize()
    onHeightChanged: reportOccurrencePageSize()

    color: paper

    Item {
        id: canvas
        width: root.canvasW
        height: root.canvasH
        anchors.centerIn: parent
        scale: Math.min(root.width / width, root.height / height)

        Column {
            id: shell
            anchors.fill: parent
            anchors.leftMargin: 68
            anchors.rightMargin: 68
            anchors.topMargin: 52
            anchors.bottomMargin: 42
            spacing: 0

            Item {
                width: parent.width
                height: 124

                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: "THE WORD"
                    color: root.ink
                    font.pixelSize: 44
                    font.weight: Font.Bold
                    font.letterSpacing: 5
                }

                Row {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 12

                    Rectangle {
                        width: 150
                        height: 68
                        color: root.mode === "read" ? root.ink : root.paper
                        border.width: 4
                        border.color: root.ink

                        Text {
                            anchors.centerIn: parent
                            text: "READ"
                            color: root.mode === "read" ? root.paper : root.ink
                            font.pixelSize: 28
                            font.weight: Font.Bold
                            font.letterSpacing: 2
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: root.mode = "read"
                        }
                    }

                    Rectangle {
                        width: 176
                        height: 68
                        color: root.mode === "browse" ? root.ink : root.paper
                        border.width: 4
                        border.color: root.ink

                        Text {
                            anchors.centerIn: parent
                            text: "BROWSE"
                            color: root.mode === "browse" ? root.paper : root.ink
                            font.pixelSize: 28
                            font.weight: Font.Bold
                            font.letterSpacing: 2
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: root.mode = "browse"
                        }
                    }
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 5
                    color: root.ink
                }
            }

            Item {
                width: parent.width
                height: 94

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: searchButton.left
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    height: 70
                    border.width: 4
                    border.color: root.faint
                    color: root.paper

                    TextInput {
                        id: searchInput
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 20
                        anchors.rightMargin: searchDoneButton.visible ? 126 : 20
                        text: root.searchDraft
                        color: root.ink
                        font.pixelSize: 34
                        selectByMouse: true
                        clip: true
                        onTextChanged: root.searchDraft = text
                        onAccepted: root.submitSearch()

                        Text {
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            text: "Search verse or reference"
                            color: root.faint
                            font.pixelSize: 32
                            visible: searchInput.text.length === 0
                        }
                    }

                    Rectangle {
                        id: searchDoneButton
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.rightMargin: 10
                        width: 104
                        height: 54
                        visible: searchInput.activeFocus
                        color: root.paper
                        border.width: 3
                        border.color: root.ink

                        Text {
                            anchors.centerIn: parent
                            text: "DONE"
                            color: root.ink
                            font.pixelSize: 20
                            font.weight: Font.Bold
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: root.dismissSearchInput()
                        }
                    }
                }

                Rectangle {
                    id: searchButton
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    width: 166
                    height: 70
                    color: root.ink

                    Text {
                        anchors.centerIn: parent
                        text: "SEARCH"
                        color: root.paper
                        font.pixelSize: 28
                        font.weight: Font.Bold
                        font.letterSpacing: 2
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.submitSearch()
                    }
                }
            }

            Text {
                width: parent.width
                height: 34
                text: root.v("searchStatus", "")
                color: root.accent
                font.pixelSize: 26
                font.weight: Font.Bold
                visible: text.length > 0
            }

            Item {
                id: referenceBar
                width: parent.width
                height: 114

                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.v("reference", "Genesis 1:1")
                    color: root.ink
                    font.pixelSize: root.landscape ? 70 : 64
                    font.weight: Font.Bold
                }

                Text {
                    anchors.right: prevButton.left
                    anchors.rightMargin: 28
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.mode === "read" ? "READING VIEW" : root.v("corpusStatus", "DEMO CORPUS")
                    color: root.faint
                    font.pixelSize: 27
                    font.weight: Font.Bold
                    font.letterSpacing: 2
                }

                Rectangle {
                    id: prevButton
                    anchors.right: nextButton.left
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    width: 68
                    height: 68
                    border.width: 4
                    border.color: (root.mode === "read"
                                   ? root.v("canPrevChapter", false)
                                   : root.v("canPrev", false))
                                  ? root.ink : root.faint
                    color: root.paper

                    Text {
                        anchors.centerIn: parent
                        text: "‹"
                        color: prevButton.border.color
                        font.pixelSize: 60
                        font.weight: Font.Bold
                    }
                    MouseArea {
                        anchors.fill: parent
                        enabled: root.mode === "read"
                                 ? root.v("canPrevChapter", false)
                                 : root.v("canPrev", false)
                        onClicked: root.navigate(root.mode === "read"
                                                 ? "prevChapter" : "prev")
                    }
                }

                Rectangle {
                    id: nextButton
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    width: 68
                    height: 68
                    border.width: 4
                    border.color: (root.mode === "read"
                                   ? root.v("canNextChapter", false)
                                   : root.v("canNext", false))
                                  ? root.ink : root.faint
                    color: root.paper

                    Text {
                        anchors.centerIn: parent
                        text: "›"
                        color: nextButton.border.color
                        font.pixelSize: 60
                        font.weight: Font.Bold
                    }
                    MouseArea {
                        anchors.fill: parent
                        enabled: root.mode === "read"
                                 ? root.v("canNextChapter", false)
                                 : root.v("canNext", false)
                        onClicked: root.navigate(root.mode === "read"
                                                 ? "nextChapter" : "next")
                    }
                }
            }

            Item {
                id: content
                width: parent.width
                height: parent.height - 124 - 94 - 34 - referenceBar.height - 58

                Item {
                    id: studyView
                    anchors.fill: parent
                    visible: root.mode === "study"

                    Item {
                        id: versePanel
                        x: 0
                        y: 0
                        width: root.landscape ? parent.width * 0.54 : parent.width
                        height: root.landscape ? parent.height : parent.height * 0.44

                        Flow {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.topMargin: 26
                            anchors.bottom: parent.bottom
                            spacing: 16

                            Repeater {
                                model: root.v("words", [])

                                WordToken {
                                    required property var modelData
                                    required property int index
                                    label: modelData.text
                                    tagged: modelData.tagged
                                    selected: index === root.v("selectedIndex", 0)
                                    textSize: root.landscape ? 54 : 60
                                    implicitHeight: textSize * 1.5
                                    ink: root.ink
                                    rule: root.faint
                                    onTapped: root.wordSelected(index)
                                }
                            }
                        }

                    }

                    Rectangle {
                        x: root.landscape ? content.width * 0.54 + 34 : 0
                        y: root.landscape ? 0 : content.height * 0.44 + 28
                        width: root.landscape ? 5 : content.width
                        height: root.landscape ? content.height : 5
                        color: root.faint
                    }

                    Item {
                        id: details
                        x: root.landscape ? content.width * 0.54 + 76 : 0
                        y: root.landscape ? 0 : content.height * 0.44 + 54
                        width: root.landscape ? content.width * 0.46 - 76 : content.width
                        height: root.landscape ? content.height : content.height * 0.56 - 54
                        clip: true

                        Item {
                            id: detailToolbar
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: parent.top
                            height: root.v("canGoBack", false) ? 74 : 0

                            Rectangle {
                                id: backButton
                                visible: root.v("canGoBack", false)
                                anchors.right: parent.right
                                anchors.top: parent.top
                                width: 300
                                height: 56
                                color: root.paper
                                border.width: 4
                                border.color: root.ink

                                Text {
                                    anchors.centerIn: parent
                                    text: "BACK TO WORD  ›"
                                    color: root.ink
                                    font.pixelSize: 24
                                    font.weight: Font.Bold
                                    font.letterSpacing: 1
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.goBack()
                                }
                            }
                        }

                        Row {
                            id: selectedHeading
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: detailToolbar.bottom
                            anchors.topMargin: root.v("canGoBack", false) ? 12 : 0
                            height: root.landscape ? 62 : 72
                            spacing: 18

                            Text {
                                text: root.v("selectedWord", {}).text || "Tap a word"
                                color: root.ink
                                font.pixelSize: root.landscape ? 54 : 60
                                font.weight: Font.Bold
                            }

                            Text {
                                text: root.selected("strongId", "")
                                visible: text.length > 0
                                color: root.ink
                                font.pixelSize: 46
                                font.weight: Font.Bold
                                anchors.baseline: parent.children[0].baseline
                            }
                        }

                        Text {
                            id: lemmaLine
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: selectedHeading.bottom
                            anchors.topMargin: 10
                            text: root.selected("transliteration", "").length > 0
                                  ? root.selected("transliteration", "")
                                  : (root.selected("pronunciation", "").length > 0
                                     ? root.selected("pronunciation", "")
                                     : "Transliteration not available")
                            color: root.ink
                            font.pixelSize: 34
                            font.weight: Font.Bold
                            elide: Text.ElideRight
                        }

                        Text {
                            id: metaLine
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: lemmaLine.bottom
                            anchors.topMargin: 8
                            text: root.selected("language", "")
                                  + (root.selected("partOfSpeech", "").length > 0
                                     ? "  ·  " + root.selected("partOfSpeech", "") : "")
                            visible: root.selected("strongId", "").length > 0
                            color: root.faint
                            font.pixelSize: 28
                            font.weight: Font.Bold
                            elide: Text.ElideRight
                        }

                        Text {
                            id: glossLine
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: metaLine.bottom
                            anchors.topMargin: 16
                            text: root.selected("strongId", "").length > 0
                                  ? "BIBLICAL USES  ·  "
                                    + root.selected("usageOutline", root.selected("gloss", ""))
                                  : "Select a word for its entry"
                            color: root.ink
                            font.pixelSize: 34
                            font.weight: Font.Bold
                            wrapMode: Text.WordWrap
                            maximumLineCount: root.landscape ? 1 : 2
                            elide: Text.ElideRight
                        }

                        Text {
                            id: definitionLine
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: glossLine.bottom
                            anchors.topMargin: 10
                            text: root.selected("definition", "")
                            color: root.ink
                            font.pixelSize: root.landscape ? 28 : 32
                            wrapMode: Text.WordWrap
                            maximumLineCount: root.landscape ? 2 : 3
                            elide: Text.ElideRight
                        }

                        Text {
                            id: derivationLine
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: definitionLine.bottom
                            anchors.topMargin: 12
                            text: "ROOT / DERIVATION  ·  "
                                  + (root.selected("derivation", "").length > 0
                                     ? root.selected("derivation", "")
                                     : "Not supplied")
                            visible: root.selected("strongId", "").length > 0
                            color: root.faint
                            font.pixelSize: root.landscape ? 25 : 28
                            font.weight: Font.Bold
                            wrapMode: Text.WordWrap
                            maximumLineCount: root.landscape ? 1 : 2
                            elide: Text.ElideRight
                        }

                        Text {
                            id: relatedLabel
                            anchors.left: parent.left
                            anchors.top: derivationLine.bottom
                            anchors.topMargin: 14
                            text: "RELATED / LEXICAL FAMILY"
                            visible: root.selected("relatedEntries", []).length > 0
                            color: root.faint
                            font.pixelSize: 26
                            font.weight: Font.Bold
                            font.letterSpacing: 2
                            elide: Text.ElideRight
                        }

                        Flow {
                            id: relatedFlow
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: relatedLabel.bottom
                            anchors.topMargin: 8
                            spacing: 10

                            Repeater {
                                model: root.selected("relatedEntries", [])

                                Rectangle {
                                    required property var modelData
                                    width: relationText.implicitWidth + 24
                                    height: 58
                                    border.width: 4
                                    border.color: root.ink
                                    color: root.paper

                                    Text {
                                        id: relationText
                                        anchors.centerIn: parent
                                        text: modelData.label
                                        color: root.ink
                                        font.pixelSize: 27
                                        font.weight: Font.Bold
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: root.strongSelected(modelData.strongId)
                                    }
                                }
                            }
                        }

                        Text {
                            id: occurrencesLabel
                            anchors.left: parent.left
                            anchors.top: relatedFlow.bottom
                            anchors.topMargin: 12
                            text: root.v("occurrenceTotal", 0) + " OCCURRENCES"
                            visible: root.selected("strongId", "").length > 0
                            color: root.faint
                            font.pixelSize: 26
                            font.weight: Font.Bold
                            font.letterSpacing: 2
                            elide: Text.ElideRight
                        }

                        ListView {
                            id: occurrencesList
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: occurrencesLabel.bottom
                            anchors.topMargin: 6
                            anchors.bottom: occurrencePager.top
                            clip: true
                            spacing: 10
                            interactive: false
                            visible: root.selected("strongId", "").length > 0
                            model: root.v("occurrences", [])

                            delegate: Rectangle {
                                required property var modelData
                                width: occurrencesList.width
                                height: 58
                                border.width: 3
                                border.color: root.ink
                                color: root.paper

                                Text {
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.leftMargin: 14
                                    anchors.rightMargin: 14
                                    text: modelData.reference + "  ·  " + modelData.text
                                    color: root.ink
                                    font.pixelSize: 26
                                    font.weight: Font.Bold
                                    elide: Text.ElideRight
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        root.navigateTo(modelData.reference)
                                        root.mode = "study"
                                    }
                                }
                            }
                        }

                        Item {
                            id: occurrencePager
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 72
                            visible: root.selected("strongId", "").length > 0
                                     && root.v("occurrenceTotal", 0) > 0

                            Row {
                                anchors.fill: parent
                                spacing: 12

                                Rectangle {
                                    id: previousOccurrencePage
                                    width: 170
                                    height: 64
                                    anchors.verticalCenter: parent.verticalCenter
                                    color: root.paper
                                    border.width: 4
                                    border.color: root.v("canPrevOccurrencePage", false)
                                                  ? root.ink : root.faint

                                    Text {
                                        anchors.centerIn: parent
                                        text: "PREV"
                                        color: previousOccurrencePage.border.color
                                        font.pixelSize: 24
                                        font.weight: Font.Bold
                                        font.letterSpacing: 1
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        enabled: root.v("canPrevOccurrencePage", false)
                                        onClicked: root.navigate("prevOccurrencePage")
                                    }
                                }

                                Text {
                                    width: Math.max(0, occurrencePager.width - 364)
                                    height: 64
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: "PAGE " + (root.v("occurrencePage", 0) + 1)
                                          + " / " + root.v("occurrencePageCount", 1)
                                    color: root.faint
                                    font.pixelSize: 24
                                    font.weight: Font.Bold
                                    font.letterSpacing: 1
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Rectangle {
                                    id: nextOccurrencePage
                                    width: 170
                                    height: 64
                                    anchors.verticalCenter: parent.verticalCenter
                                    color: root.paper
                                    border.width: 4
                                    border.color: root.v("canNextOccurrencePage", false)
                                                  ? root.ink : root.faint

                                    Text {
                                        anchors.centerIn: parent
                                        text: "NEXT"
                                        color: nextOccurrencePage.border.color
                                        font.pixelSize: 24
                                        font.weight: Font.Bold
                                        font.letterSpacing: 1
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        enabled: root.v("canNextOccurrencePage", false)
                                        onClicked: root.navigate("nextOccurrencePage")
                                    }
                                }
                            }
                        }

                    }
                }

                Item {
                    id: readingView
                    anchors.fill: parent
                    visible: root.mode === "read"

                    Text {
                        id: readingHeader
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        text: "READING VIEW  ·  " + root.v("book", "Genesis")
                              + " " + root.v("chapter", 1)
                        color: root.faint
                        font.pixelSize: 28
                        font.weight: Font.Bold
                        font.letterSpacing: 2
                    }

                    ListView {
                        id: readingList
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: readingHeader.bottom
                        anchors.topMargin: 28
                        anchors.bottom: parent.bottom
                        clip: true
                        model: root.v("chapterVerses", [])
                        spacing: 18

                        delegate: Item {
                            required property var modelData
                            width: readingList.width
                            height: verseText.paintedHeight + 58

                            Rectangle {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                width: modelData.verse === root.v("verse", 1) ? 8 : 2
                                color: modelData.verse === root.v("verse", 1)
                                       ? root.accent : root.faint
                                opacity: modelData.verse === root.v("verse", 1) ? 1 : 0.45
                            }

                            Text {
                                id: verseNumber
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.leftMargin: 24
                                text: modelData.verse
                                color: modelData.verse === root.v("verse", 1)
                                       ? root.accent : root.faint
                                font.pixelSize: root.landscape ? 30 : 34
                                font.weight: Font.Bold
                            }

                            Text {
                                id: verseText
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.leftMargin: 82
                                anchors.rightMargin: 24
                                text: modelData.text
                                color: root.ink
                                font.pixelSize: root.landscape ? 38 : 44
                                lineHeight: 1.15
                                lineHeightMode: Text.ProportionalHeight
                                wrapMode: Text.WordWrap
                            }

                            Rectangle {
                                anchors.left: verseText.left
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                height: 2
                                color: root.faint
                                opacity: 0.35
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.navigateTo(modelData.reference)
                                    root.mode = "study"
                                }
                            }
                        }
                    }
                }

                Item {
                    id: browseView
                    anchors.fill: parent
                    visible: root.mode === "browse"

                    Text {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        text: "CHOOSE A PASSAGE"
                        color: root.faint
                        font.pixelSize: 28
                        font.weight: Font.Bold
                        font.letterSpacing: 2
                    }

                    Row {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.topMargin: 62
                        anchors.bottom: doneBrowseButton.top
                        anchors.bottomMargin: 24
                        spacing: 18

                        Rectangle {
                            width: (parent.width - 36) / 3
                            height: parent.height
                            color: root.paper
                            border.width: 4
                            border.color: root.ink
                            clip: true

                            Text {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.margins: 18
                                text: "BOOK"
                                color: root.accent
                                font.pixelSize: 26
                                font.weight: Font.Bold
                                font.letterSpacing: 2
                            }

                            ListView {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                anchors.leftMargin: 5
                                anchors.rightMargin: 5
                                anchors.topMargin: 66
                                clip: true
                                spacing: 8
                                model: root.v("books", [])

                                delegate: Rectangle {
                                    required property var modelData
                                    width: ListView.view.width
                                    height: 58
                                    color: root.browseBook === modelData.name ? root.ink : root.paper

                                    Text {
                                        anchors.left: parent.left
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.leftMargin: 14
                                        text: modelData.name
                                        color: root.browseBook === modelData.name ? root.paper : root.ink
                                        font.pixelSize: 30
                                        font.weight: Font.Bold
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            root.browseBook = modelData.name
                                            root.browseChapter = modelData.chapters[0]
                                            root.navigateTo(modelData.name + " 1:1")
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            width: (parent.width - 36) / 3
                            height: parent.height
                            color: root.paper
                            border.width: 4
                            border.color: root.ink
                            clip: true

                            Text {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.margins: 18
                                text: "CHAPTER"
                                color: root.accent
                                font.pixelSize: 26
                                font.weight: Font.Bold
                                font.letterSpacing: 2
                            }

                            ListView {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                anchors.leftMargin: 5
                                anchors.rightMargin: 5
                                anchors.topMargin: 66
                                clip: true
                                spacing: 8
                                model: root.bookInfo(root.browseBook).chapters

                                delegate: Rectangle {
                                    required property var modelData
                                    width: ListView.view.width
                                    height: 58
                                    color: root.browseChapter === modelData ? root.ink : root.paper

                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData
                                        color: root.browseChapter === modelData ? root.paper : root.ink
                                        font.pixelSize: 32
                                        font.weight: Font.Bold
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            root.browseChapter = modelData
                                            root.navigateTo(root.browseBook + " "
                                                            + modelData + ":1")
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            width: (parent.width - 36) / 3
                            height: parent.height
                            color: root.paper
                            border.width: 4
                            border.color: root.ink
                            clip: true

                            Text {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.margins: 18
                                text: "VERSE"
                                color: root.accent
                                font.pixelSize: 26
                                font.weight: Font.Bold
                                font.letterSpacing: 2
                            }

                            ListView {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                anchors.leftMargin: 5
                                anchors.rightMargin: 5
                                anchors.topMargin: 66
                                clip: true
                                spacing: 8
                                model: root.v("chapterVerses", [])

                                delegate: Rectangle {
                                    required property var modelData
                                    width: ListView.view.width
                                    height: 58
                                    color: root.paper
                                    border.width: 3
                                    border.color: root.ink

                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData.verse
                                        color: root.ink
                                        font.pixelSize: 32
                                        font.weight: Font.Bold
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            root.navigateTo(modelData.reference)
                                            root.mode = "study"
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        id: doneBrowseButton
                        anchors.left: parent.left
                        anchors.bottom: parent.bottom
                        width: 230
                        height: 66
                        color: root.ink

                        Text {
                            anchors.centerIn: parent
                            text: "STUDY PASSAGE"
                            color: root.paper
                            font.pixelSize: 25
                            font.weight: Font.Bold
                            font.letterSpacing: 1
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                root.navigateTo(root.browseBook + " "
                                                + root.browseChapter + ":1")
                                root.mode = "study"
                            }
                        }
                    }
                }
            }

            Item {
                width: parent.width
                height: 58

                Text {
                    visible: false
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: ""
                    color: root.faint
                    font.pixelSize: 24
                    font.weight: Font.Bold
                    font.letterSpacing: 2
                }

                Text {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.v("verseIndex", 0) + 1 + " / " + root.v("verseCount", 1)
                    color: root.ink
                    font.pixelSize: 24
                    font.weight: Font.Bold
                }
            }
        }
    }
}
