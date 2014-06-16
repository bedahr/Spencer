import QtQuick 2.0
import QtMultimedia 5.0

Item {
    id: avatar
    objectName: "avatar"
    anchors.right: parent.right
    anchors.bottom: parent.bottom

    states: [
        State {
            name: "fullscreen"
            PropertyChanges {
                target: avatar
                width: parent.width
                height: parent.height
            }
        },
        State {
            name: "minimized"
            PropertyChanges {
                target: avatar
                width: 347
                height: 500
            }
        }

    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"
            NumberAnimation { target: avatar; properties: "width,height"; duration: 3000; easing.type: Easing.OutExpo }
        }
    ]

    function reset() {
    }
    function minimize() {
        avatar.state = "minimized"
    }
    function maximize() {
        avatar.state = "fullscreen"
    }
    VideoOutput {
        id: voOutput
        anchors.fill: parent
        source: avatarPlayer
        smooth: false
    }
}
