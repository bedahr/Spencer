import QtQuick 2.0
import QtMultimedia 5.0

Rectangle {
    objectName: "avatar"
    function reset() {
    }
    VideoOutput {
        id: voOutput
        anchors.fill: parent
        onSourceChanged: console.log("source changed")
        source: avatarPlayer

    }

}
