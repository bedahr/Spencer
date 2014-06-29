import QtQuick 2.0

Item {
    property alias fillMode: imImage.fillMode
    property double maxOpacity: 1

    function updateImage(src) {
        imImageFader.source = imImage.source
        imImageFader.opacity = maxOpacity
        imImage.opacity = 0
        imImage.source = src
        naFadeImages.start()
    }

    Image {
        id: imImageFader
        anchors.fill: imImage
        opacity: 0
        smooth: true
        width: imImage.width
        height: imImage.height
        fillMode: imImage.fillMode
        z: 5
    }
    Image {
        id: imImage
        anchors.top: parent.top
        anchors.right: parent.right
        width: parent.width
        height: parent.height
        smooth: true
        opacity: maxOpacity
        z: 5

    }
    ParallelAnimation {
        id: naFadeImages
        NumberAnimation {
            target: imImage;
            properties: "opacity";
            duration: 500;
            to: maxOpacity
        }
        NumberAnimation {
            target: imImageFader;
            properties: "opacity";
            duration: 500;
            to: 0
        }

    }
}
