import QtQuick 2.2

Rectangle {
    property alias text: teText.text
    // text is anything, "numbers" only allows numbers
    property string mode: "text"
    property alias echoMode: teText.echoMode
    property alias inputMask: teText.inputMask

    color: "white"
    border.color: (teText.focus) ? "black" : "darkgrey"
    border.width: (teText.focus) ? 2 : 1
    height: teText.height + 10
    clip: true

    TextInput {
        id: teText
        anchors.left: parent.left
        anchors.right:  parent.right
        anchors.centerIn: parent
        onAccepted: focus = false
    }
    MouseArea {
        anchors.fill: parent
        onClicked: teText.focus = true
    }
}
