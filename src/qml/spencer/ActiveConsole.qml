import QtQuick 2.2

AnimatedItem {
    id: outerAnim
    property string mode

    Avatar {
        state : "minimized"
    }

    Offer {
        anchors.fill: parent
    }

}
