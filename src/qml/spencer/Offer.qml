import QtQuick 2.2
import QtMultimedia 5.0
import "AttributeDisplay.js" as AttrDisplay
import "SentimentDisplay.js" as SentimentDisplay

Item {
    id: offer

    property int currentImageIndex : 0
    property variant currentImages : []
    property variant currentDetails : []
    property variant currentSentiment : []
    anchors.margins: 30

    Timer {
        id: imageCycleTimer
        interval: 3000; running: true; repeat: true
        onTriggered: updatePicture()
    }

    function displayRecognizing() {
    }

    function updatePicture() {
        if (currentImageIndex > currentImages.length) {
            return;
        }
        imImage.updateImage(currentImages[currentImageIndex])
        currentImageIndex = (currentImageIndex+1) % currentImages.length
    }

    function sortHelper(a, b) {
        var out = b["value"] - a["value"]
        return out
    }

    function displaySentiment(parent, data, mode) {
        for (var i = 0; i < data.length; ++i) {
            currentSentiment.push(SentimentDisplay.createSentimentDisplay(parent, data[i]["aspect"], data[i]["value"], mode))
        }
    }

    function recommend(title, price, rating, images, data, sentiment)
    {
        for (var item in currentDetails)
            item.destroy()
        for (var si in currentSentiment)
            si.destroy()

        imageCycleTimer.stop()
        teName.changeText(title)
        tePrice.changeText("€ " + price)

        aiDetails.state = "hidden"
        for (var i = 0; i < data.length; ++i) {
            currentDetails.push( AttrDisplay.createDetails(coDetails, data[i].name, data[i].value))
        }
        var pos_sentiment = []
        var neg_sentiment = []
        for (var aspect in sentiment) {

            console.log("Aspect sentiment..." + aspect + " = " + sentiment[aspect])
            var value = sentiment[aspect]
            if (value > 0.01) {
                data = Math.min(value, 1)
                pos_sentiment.push({"aspect" : aspect, "value" : data})
            } else if (value < -0.01) {
                data = Math.min(-1 * value, 1)
                neg_sentiment.push({"aspect" : aspect, "value" : data})
            }
        }
        pos_sentiment = pos_sentiment.sort(sortHelper)
        neg_sentiment = neg_sentiment.sort(sortHelper)
        displaySentiment(coSentimentPos, pos_sentiment, "positive")
        displaySentiment(coSentimentNeg, neg_sentiment, "negative")
        rwRating.rating = rating

        currentImages = images
        updatePicture()
        imageCycleTimer.start()
        imBackgroundImage.updateImage(currentImages[0])
    }
    FadingImage {
        id: imImage
        anchors.top: parent.top
        anchors.right: parent.right
        width: 347
        height: 347
        fillMode: Image.PreserveAspectFit
        z: 5
    }
    FadingImage {
        id: imBackgroundImage
        anchors {
            bottom: parent.bottom
            left: parent.left
            bottomMargin: -300
            leftMargin: -200
        }

        width: 1200
        height: 1200
        fillMode: Image.PreserveAspectFit
        maxOpacity: 0.1
        z: -5
    }

    Button {
        id: btBack
        width: 80
        height: 20
        text: qsTr("Zurück") // qsTr("Back")
        visible: !spencerView.voiceControlled
        anchors.right: imImage.right
        anchors.bottom: parent.bottom
        onClicked: spencer.critique("ZURÜCK")
        topClickMargin: 20
        bottomClickMargin: 20
    }
    Item {
        id:header
        anchors.left: parent.left
        anchors.right: imImage.left
        anchors.rightMargin: 50

        height: teName.height + rwRating.height

        AnimatedText {
            id: teName
            font.pointSize: 20
            text: ""
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
        }

        AnimatedText {
            id: tePrice
            font.pointSize: 20
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
        }

        RatingWidget {
            id: rwRating
            width: 120
            height: 32
            rating:  0
            anchors {
                top: teName.bottom
                left: parent.left
            }
        }
    }
    Item {
        id:body
        anchors {
            top: header.bottom
            topMargin: 40
            left: header.left
            right: header.right
            leftMargin: 60
            rightMargin: 60
            bottom: parent.bottom
        }

        Item {
            id: aiDetails
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: teDetailsHeader.height + coDetails.height
            Text {
                id: teDetailsHeader
                anchors.left: parent.left
                anchors.top: parent.top
                font.pixelSize: 18
                font.bold: true
                text: "Details"
            }
            Flickable {
                anchors {
                    left: parent.left
                    top: teDetailsHeader.bottom
                    topMargin: 20
                    right: parent.right
                    bottom: parent.bottom
                }
                contentHeight: coDetails.height
                Column {
                    id: coDetails
                    spacing: 5
                }
            }
        }


        Item {
            id: aiUserSentiment
            anchors {
                top: aiDetails.bottom
                topMargin: 50
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            Text {
                id: teSentimentHeader
                anchors.left: parent.left
                anchors.top: parent.top
                font.pixelSize: 18
                font.bold: true
                text: "Kundenmeinungen"
            }
            Row {
                spacing: 40
                anchors {
                    left: parent.left
                    top: teSentimentHeader.bottom
                    topMargin: 20
                    right: parent.right
                    bottom: parent.bottom
                }
                Column {
                    width: 280
                    id: coSentimentNeg
                    spacing: 5
                }
                Column {
                    width: 280
                    id: coSentimentPos
                    spacing: 5
                }
            }

            /*
            ListView {
                id: attributeDisplay
                anchors.fill: parent

                delegate: AttributeDelegate {}
            } */
        }
    }


}
