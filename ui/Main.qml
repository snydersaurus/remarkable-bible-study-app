import QtQuick
import net.asivery.AppLoad 1.0

Item {
    id: root
    anchors.fill: parent

    readonly property int msgGeometry: 4
    readonly property int msgState: 101
    readonly property int msgHello: 1
    readonly property int msgSelectWord: 2
    readonly property int msgNavigate: 3
    readonly property int msgSelectStrong: 5
    readonly property int msgGoBack: 6
    readonly property int msgSearchVerse: 7
    readonly property int msgNavigateTo: 9
    readonly property int msgOccurrencePageSize: 10

    function reportGeometry() {
        appload.sendMessage(root.msgGeometry,
                            Math.round(width) + "x" + Math.round(height))
    }

    onWidthChanged: reportGeometry()
    onHeightChanged: reportGeometry()

    AppLoad {
        id: appload
        applicationID: "word-study"
        onMessageReceived: (type, contents) => {
            if (type !== root.msgState)
                return
            try {
                board.s = JSON.parse(contents)
            } catch (error) {
                console.log("The Word: bad state payload", error)
            }
        }
        Component.onCompleted: {
            appload.sendMessage(root.msgHello, "")
            root.reportGeometry()
        }
    }

    Board {
        id: board
        anchors.fill: parent

        onWordSelected: appload.sendMessage(root.msgSelectWord, String(index))
        onStrongSelected: appload.sendMessage(root.msgSelectStrong, strongId)
        onGoBack: appload.sendMessage(root.msgGoBack, "")
        onNavigate: appload.sendMessage(root.msgNavigate, direction)
        onSearchVerse: appload.sendMessage(root.msgSearchVerse, query)
        onNavigateTo: appload.sendMessage(root.msgNavigateTo, reference)
        onOccurrencePageSizeChanged: appload.sendMessage(root.msgOccurrencePageSize,
                                                         String(size))
    }
}
