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

}
