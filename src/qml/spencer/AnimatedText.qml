import QtQuick 2.2

Item {
    id: outerText
    property alias text: primaryText.text
    property alias font: primaryText.font
    property alias horizontalAlignment : primaryText.horizontalAlignment
    property alias verticalAlignment : primaryText.verticalAlignment

    property string targetText : ""
    height: primaryText.height
    width: primaryText.width

    Timer {
        id: clearTimer
        interval: 20
        running: false
        repeat: true
        onTriggered: {
            if (primaryText.text.length == 0) {
                running = false
                fillTimer.start()
            } else {
                primaryText.text = primaryText.text.substring(0, primaryText.text.length - 1)
            }
        }
    }
    Timer {
        id: fillTimer
        interval: 20
        running: false
        repeat: true
        onTriggered: {
            if (primaryText.text.length == targetText.length) {
                running = false
            } else
                primaryText.text = targetText.substring(0, primaryText.text.length+1)
        }
    }

    function changeText(newText) {
        if (newText === targetText)
            return;
        fillTimer.stop()
        targetText = newText
        clearTimer.start()
    }

    Text {
        id: primaryText
        anchors.left: parent.left
        anchors.right: parent.right
    }
}
