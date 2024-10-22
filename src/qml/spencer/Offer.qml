import QtQuick 2.2
import QtMultimedia 5.0
import "AttributeDisplay.js" as AttrDisplay
import "SentimentDisplay.js" as SentimentDisplay

Item {
    id: offer

    property int currentImageIndex : 0
    property variant currentImages : []
    property variant currentSentiment : []
    property int titleSkip : 0

    state: "nooffer"

    states: [
        State {
            name: "nooffer"
            PropertyChanges {
                target: body
                opacity: 0
            }
            PropertyChanges {
                target: header
                opacity: 0
            }
            PropertyChanges {
                target: imBackgroundImage
                opacity: 0
            }
            PropertyChanges {
                target: imImage
                opacity: 0
            }
            PropertyChanges {
                target: imNoRecommendation
                opacity: 1
            }
        },
        State {
            name: "hasoffer"
            PropertyChanges {
                target: body
                opacity: 1
            }
            PropertyChanges {
                target: header
                opacity: 1
            }
            PropertyChanges {
                target: imBackgroundImage
                opacity: 1
            }
            PropertyChanges {
                target: imImage
                opacity: 1
            }
            PropertyChanges {
                target: imNoRecommendation
                opacity: 0
            }
        }

    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"
            NumberAnimation { targets: [body,header,imBackgroundImage,imImage,imNoRecommendation]; properties: "opacity"; duration: 200 }
        }
    ]

    anchors.margins: 30

    Timer {
        id: imageCycleTimer
        interval: 3000; running: true; repeat: true
        onTriggered: updatePicture()
    }

    function displayRecognizing() {
    }

    function updatePicture() {
        if (currentImages.length == 0)
            return

        if (currentImageIndex >= currentImages.length) {
            currentImageIndex = 0
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
            SentimentDisplay.createSentimentDisplay(parent, data[i]["aspect"], data[i]["value"], mode)
        }
    }

    function noRecommendation()
    {
        offer.state = "nooffer"
    }

    function clearChildren(item)
    {
        for(var i = item.children.length; i > 0 ; i--) {
          item.children[i-1].destroy()
        }
    }

    function recommend(title, price, rating, images, data, sentiment)
    {
        clearChildren(coDetails1)
        clearChildren(coDetails2)
        clearChildren(coSentimentPos)
        clearChildren(coSentimentNeg)

        imageCycleTimer.stop()
        teName.changeText(title)
        tePrice.changeText("€ " + price.toFixed(2))

        aiDetails.state = "hidden"
        for (var i = 0; i < data.length; ++i) {
            AttrDisplay.createDetails(
                                    i < Math.ceil(data.length / 2) ? coDetails1 : coDetails2,
                                                                     data[i].name, data[i].value,
                                                                     data[i].expressedUserInterest,
                                                                     data[i].completionFactor)
        }
        var pos_sentiment = []
        var neg_sentiment = []
        for (var aspect in sentiment) {
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
        offer.state = "hasoffer"
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

    Image {
        id: imNoRecommendation
        source: "img/unknown_laptop.png"
        anchors.centerIn: parent
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
            anchors.leftMargin: titleSkip
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
            leftMargin: 40
            rightMargin: 40
            bottom: parent.bottom
        }

        Item {
            id: aiDetails
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            height: teDetailsHeader.height + 20 + Math.max(coDetails1.height, coDetails2.height)
            Text {
                id: teDetailsHeader
                anchors.left: parent.left
                anchors.top: parent.top
                font.pixelSize: 18
                font.bold: true
                text: "Details"
            }
            Column {
                id: coDetails1
                spacing: 5
                anchors {
                    left: parent.left
                    top: teDetailsHeader.bottom
                    topMargin: 20
                }
                width: parent.width / 2
            }
            Column {
                id: coDetails2
                width: parent.width / 2
                anchors {
                    top: teDetailsHeader.bottom
                    topMargin: 20
                    right: parent.right
                }
                spacing: 5
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
                    width: 380
                    id: coSentimentNeg
                    spacing: 5
                }
                Column {
                    width: 380
                    id: coSentimentPos
                    spacing: 5
                }
            }
        }
    }


}
