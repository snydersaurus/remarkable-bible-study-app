import QtQuick

Item {
    id: root

    property string label: ""
    property bool selected: false
    property bool tagged: false
    property real textSize: 54
    property color ink: "#000000"
    property color rule: "#5A5A5A"

    signal tapped()

    implicitWidth: word.implicitWidth + 26
    implicitHeight: textSize * 1.65

    Text {
        id: word
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 5
        text: root.label
        color: root.ink
        font.pixelSize: root.textSize
        font.weight: root.selected ? Font.Bold : Font.Normal
        horizontalAlignment: Text.AlignHCenter
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: root.selected ? 8 : (root.tagged ? 4 : 0)
        color: root.selected ? root.ink : root.rule
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.tapped()
    }
}
