import QtQuick 2.2

AnimatedItem {
    id: outerAnim
    property string mode

    Avatar {
        state : "minimized"
    }

    Offer {
        objectName: "currentRecommendation"
        anchors.fill: parent
    }

    ProgressBar {
        id: pbVUMeter
        objectName: "pbVUMeter"
        width: 620
        height: 50
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }


}
