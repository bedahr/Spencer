import QtQuick 2.2


AnimatedItem {
    property alias title: teTitle.text
    property alias text: teBody.text
    property alias backgroundColor: reBg.color
    property bool autoClose : false
    property alias autoCloseDelay: tiAutoClose.interval

    id: dialog
    inverted: true

    function show(error) {
        if (dialog.state == "shown") {
            // append error message
            teBody.text = teBody.text + "\n" + error;
        } else {
            //replace text and show
            teBody.text = error
            dialog.state = "shown"
        }
    }

    onStateChanged: {
        if (state == "shown" && autoClose)
            tiAutoClose.restart()
    }


    Timer {
        id: tiAutoClose
        running: false
        interval: 2500
        repeat: false
        onTriggered: dialog.state = "hidden"
    }

    Rectangle {
        id: reBg
        anchors.fill: parent
        border.color: "darkgrey"
        border.width: 1
        color: "darkred"
        opacity: 0.8

        Text {
            id: teTitle
            anchors.verticalCenter: btnClose.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.topMargin: 10
            font.pointSize: 15
            color: "white"
        }
        Button {
            id: btnClose
            visible: !dialog.autoClose
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: 20
            anchors.topMargin: 10
            width: 200
            text: qsTr("Close")
            onClicked: {
                dialog.state = "hidden"
            }
        }

        Text {
            id: teBody
            anchors.left: teTitle.left
            anchors.top: btnClose.bottom
            anchors.topMargin: 5
            font.pointSize: 14
            color: "white"

        }
    }
}
