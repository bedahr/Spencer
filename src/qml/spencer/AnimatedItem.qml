import QtQuick 2.2

Item {
    property bool inverted : false
    property int animationDuration : 200

    id: animatedItem
    anchors.margins: 30
    state: "hidden"
    width: 300
    states: [
        State {
            name: "hidden"
            PropertyChanges {
                target: animatedItem
                opacity: 0
                anchors.topMargin: inverted ? 10 : -20
            }
        },
        State {
            name: "shown"
            PropertyChanges {
                target: animatedItem
                opacity: 1
                anchors.topMargin: inverted ? -20 : 10
            }
        }

    ]
    onOpacityChanged: {
        if (opacity == 0)
            visible = false
        else
            visible = true
    }

    transitions: [
        Transition {
            from: "*"
            to: "*"
            NumberAnimation { target: animatedItem; properties: "opacity,anchors.topMargin"; duration: animationDuration; easing.type: Easing.InOutQuad }
        }
    ]
}
