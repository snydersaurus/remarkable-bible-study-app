import QtQuick
import QtQuick.Window

Window {
    id: win
    visible: true
    color: "#FFFFFF"
    title: "The Word"

    width: shotMode ? (landscape ? panelH : panelW) : (landscape ? 850 : 620)
    height: shotMode ? (landscape ? panelW : panelH) : (landscape ? 477 : 920)
    minimumWidth: shotMode ? 0 : 560
    minimumHeight: shotMode ? 0 : 760

    Board {
        anchors.fill: parent
        compactMode: !shotMode
        s: feed.state
        onWordSelected: feed.selectIndex(index)
        onStrongSelected: feed.selectStrongId(strongId)
        onGoBack: feed.goBack()
        onNavigate: feed.navigate(direction)
        onSearchVerse: feed.searchVerse(query)
        onNavigateTo: feed.navigateTo(reference)
    }
}
