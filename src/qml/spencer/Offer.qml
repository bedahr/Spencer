import QtQuick 2.2
import QtMultimedia 5.0
import "AttributeDisplay.js" as AttrDisplay

Item {
    id: offer

    property int currentImageIndex : 0
    property variant currentImages : []
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

        imImageFader.source = imImage.source
        imImageFader.opacity = 1
        imImage.opacity = 0
        imImage.source = currentImages[currentImageIndex]
        naFadeImages.start()
        currentImageIndex = (currentImageIndex+1) % currentImages.length
    }

    Component.onCompleted: console.log(" ready for creating dynamics]");

    function recommend(title, price, rating, images, data, sentiment)
    {
        imageCycleTimer.stop()
        teName.changeText(title)
        tePrice.changeText("€ " + price)

        aiDetails.state = "hidden"
        console.log("Got " + data.length+" attributes")
        for (var i = 0; i < data.length; ++i) {
            AttrDisplay.createDetails(coDetails, data[i].name, data[i].value)

        }
        for (var key in sentiment) {
            console.log(key + " = " + sentiment[key])
        }

        currentImages = images
        updatePicture()
        imageCycleTimer.start()
    }
    ParallelAnimation {
        id: naFadeImages
        NumberAnimation {
            target: imImage;
            properties: "opacity";
            duration: 500;
            to: 1
        }
        NumberAnimation {
            target: imImageFader;
            properties: "opacity";
            duration: 500;
            to: 0
        }

    }

    Image {
        id: imImageFader
        anchors.fill: imImage
        opacity: 0
        smooth: true
        fillMode: Image.PreserveAspectFit
        z: 5
    }
    Image {
        id: imImage
        anchors.top: parent.top
        anchors.right: parent.right
        width: 347
        height: 347
        smooth: true
        fillMode: Image.PreserveAspectFit
        z: 5
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

    AnimatedText {
        id: teName
        font.pointSize: 20
        text: ""
        anchors.left: parent.left
        anchors.right: imImage.left
        anchors.rightMargin: 20
        anchors.top: parent.top
    }

    AnimatedText {
        id: tePrice
        font.pointSize: 20
        anchors.top: imImage.bottom
        anchors.left: imImage.left
        anchors.right: imImage.right
        anchors.topMargin: 10
        horizontalAlignment: Text.AlignHCenter

    }


    Item {
        id: aiDetails
        anchors {
            top: teName.bottom
            topMargin: 30
            left: parent.left
            right: teName.right
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
            topMargin: 30
            left: parent.left
            right: teName.right
            bottom: parent.bottom
        }

        Text {
            anchors.left: parent.left
            anchors.top: parent.top
            font.pixelSize: 18
            font.bold: true
            text: "Kundenmeinungen"
        }

        /*
        ListView {
            id: attributeDisplay
            anchors.fill: parent

            delegate: AttributeDelegate {}
        } */
    }

}
