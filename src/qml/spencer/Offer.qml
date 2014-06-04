import QtQuick 2.2
import QtMultimedia 5.0

Item {
    Audio {
        id: bong
        source: "/usr/share/sounds/warning.wav"
    }
    Audio {
        id: bing
        source: "/usr/share/sounds/info.wav"
    }
    Audio {
        id: bang
        source: "/usr/share/sounds/email.wav"
    }
    function displayRecognizing() {
        bong.play()
    }

    function recommend(title, image, attributes)
    {
        if (title === teName.text)
            bang.play()
        else
            bing.play()
        teName.changeText(title)
        aiDetails.state = "hidden"

        imImageFader.source = imImage.source
        imImageFader.opacity = 1
        imImage.opacity = 0
        imImage.source = image
        naFadeImages.start()

        //TODO: nice animation
        attributeDisplay.model = attributeModel
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
        width: 256
        height: 256
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
        font.pointSize: 18
        text: ""
        anchors.left: parent.left
        anchors.right: imImage.left
        anchors.rightMargin: 20
        anchors.top: parent.top
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
        ListView {
            id: attributeDisplay
            anchors.fill: parent

            delegate: AttributeDelegate {}
        }
    }

}
