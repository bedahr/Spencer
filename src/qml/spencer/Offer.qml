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
        interval: 2000; running: true; repeat: true
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
    /*

    QMetaObject::invokeMethod(viewer->rootObject()->findChild<QObject*>("currentRecommendation"),
                              "recommend",
                              Q_ARG(QVariant, QVariant::fromValue(offer->getName())),
                              Q_ARG(QVariant, QVariant::fromValue(offer->getPrice())),
                              Q_ARG(QVariant,
                                    QVariant::fromValue(QLatin1String("image://SpencerImages/"))),
                              Q_ARG(QVariant, oldIds)),
                              Q_ARG(QVariant, QVariant::fromValue(offer->getRecords()))*/
    function recommend(title, price, images, labels, attributes)
    {
        imageCycleTimer.stop()
        teName.changeText(title)
        tePrice.changeText("€ " + price)

        aiDetails.state = "hidden"

        for (var i = 0; i < labels.length; ++i) {
            AttrDisplay.createDetails(coDetails, labels[i], attributes[i])

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
            topMargin: 20
            left: parent.left
            right: teName.right
            bottom: parent.bottom
        }
        Flickable {
            anchors.fill : parent
            contentHeight: coDetails.height
            Column {
                id: coDetails
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
