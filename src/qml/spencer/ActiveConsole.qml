import QtQuick 2.2

AnimatedItem {
    id: outerAnim
    objectName: "console"
    property string mode

    AnimatedText {
        id: teTitle
        font.pointSize: 20
        text: ""
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 30
        anchors.rightMargin: 30
    }

    Avatar {
        id: avatar
        state : "fullscreen"
        //state : "minimized"
    }

    AnimatedItem {
        id: innerAnim
        anchors.fill: parent
        animationDuration: 600
        state: (avatar.state == "fullscreen") ? "hidden" : "shown"
        Offer {
            id: offer
            objectName: "currentRecommendation"
            anchors.fill: parent
            titleSkip: teTitle.width
        }
    }

    ProgressBar {
        id: pbVUMeter
        objectName: "pbVUMeter"
        width: 620
        height: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }

    function showTask(description)
    {
        teTitle.changeText(description)
    }

    function noRecommendation()
    {
        offer.noRecommendation()
    }

    function recommend(title, price, rating, images, data, sentiment)
    {
        offer.recommend(title, price, rating, images, data, sentiment)

    }
}
