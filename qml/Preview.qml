import QtQuick
import QtQuick.Window

Window {
    id: win
    visible: true
    color: "#FFFFFF"
    title: "Word Study"

    width: shotMode ? (landscape ? panelH : panelW) : (landscape ? 850 : 477)
    height: shotMode ? (landscape ? panelW : panelH) : (landscape ? 477 : 850)

    Board {
        anchors.fill: parent
        s: feed.state
        onWordSelected: feed.selectIndex(index)
        onStrongSelected: feed.selectStrongId(strongId)
        onGoBack: feed.goBack()
        onNavigate: feed.navigate(direction)
        onSearchVerse: feed.searchVerse(query)
        onNavigateTo: feed.navigateTo(reference)
        onNoteSaved: feed.saveNote(key, note)
    }
}
