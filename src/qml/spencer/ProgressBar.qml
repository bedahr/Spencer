import QtQuick 2.2

Rectangle {
    property int minimumValue : 0
    property int maximumValue : 100
    property int value : 20
    id: outer

    border.color: "grey"
    border.width: 1


    Rectangle {
        id: inner
        color: "grey"
        anchors.fill: parent
        anchors.rightMargin: outer.width * (maximumValue - value) / maximumValue
    }
}
