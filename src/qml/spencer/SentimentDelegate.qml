import QtQuick 2.0


Item {
    height: label.height + 3
    width: 280

    property alias key : label.text
    property double value
    property string mode : "positive"

    Text {
        id: label
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        horizontalAlignment: (parent.mode == "positive") ? Text.AlignLeft : Text.AlignRight

        font.pixelSize: 18
        wrapMode : Text.WordWrap
    }
    Rectangle {
        id: val
        color: (parent.mode == "positive") ? "green" : "red"
        width: parent.width * parent.value
        height: 3
        anchors.top: label.baseline
        anchors.topMargin: 2


        x: (parent.mode != "positive") ? parent.width - width : 0
    }

}
