import QtQuick 2.2

Item {
    property bool checked: false
    property alias text: teText.text

    height: Math.max(reBox.height, teText.height)

    Rectangle {
        id: reBox
        width: 20
        height: 20
        border.color: checked ? "black" : "darkgrey"
        color: "white"
        anchors.left: parent.left
        anchors.verticalCenter: teText.verticalCenter

        Rectangle {
            anchors.centerIn: parent
            width: parent.width - 8
            height: parent.height - 8

            color: "black"
            visible: checked
        }
    }

    Text {
        id: teText
        anchors.left: reBox.right
        anchors.leftMargin: 10
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            checked = !checked
        }
    }
}
