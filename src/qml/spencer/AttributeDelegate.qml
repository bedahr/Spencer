import QtQuick 2.2

/*
Rectangle {
    property string key
    property string value
    color: "red"; width: 20; height: 20
    Text {
        id: label
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 60
        text: key
    }
}
*/
Item {
    width: 400
    height: Math.max(label.height, val.height)

    property alias key : label.text
    property alias value : val.text
    property double userInterest : 0
    property double completionFactor : 0
/*
    onCompletionValueChanged: {
        if (userInterest < 0.05) {
            val.color = "black"
        } else {
            // -1 = 0 is red, 1 = 120 is green
            val.color = Qt.hsla(60 + completionValue * 60, 100, 85)
            //if (completionValue < 0) {
            //}
        }
    }
*/
    Text {
        id: label
        anchors.top: parent.top

        anchors.left: parent.left
        width: 160
        font.pixelSize: 18
        font.weight: Math.round(userInterest * 50 + 25)
        horizontalAlignment: Text.AlignRight
        wrapMode : Text.WordWrap
    }
    Text {
        id: val
        anchors.right: parent.right
        width: parent.width - 180
        wrapMode : Text.WordWrap
        font.pixelSize: 18
        font.weight: label.font.weight
        color: Qt.hsla(0.166666 + completionFactor * 0.166666, userInterest, 0.25) //(userInterest < 0.05) ? "black" : val.color = Qt.hsla(0.166666 + completionValue * 0.166666, 1, 0.85)
    }
}

/*
Item {
    function escapeBrackets(input) {
        return input.replace(" \(.*\)", "")
    }

    height: spencerView.voiceControlled ? 40 : 60
    Text {
        anchors.right: data.left
        anchors.rightMargin: 10
        anchors.top: parent.top
        id: teKey
        text: "<b>"+key+":</b>"
        font.pointSize: spencerView.voiceControlled ? 16 : 14
    }
    Item {
        id: data
        anchors.left: parent.left
        anchors.top: teKey.top
        anchors.leftMargin: Math.max(spencerView.voiceControlled ? 320 : 270, 10 + teKey.width)

        Button {
            id: btChange
            width: 80
            height: 25
            text: qsTr("Ändern") //qsTr("Change")
            visible: !spencerView.voiceControlled && !isCountable
            anchors.left: parent.left
            anchors.top: parent.top
            onClicked: spencer.critique(escapeBrackets(key) + " ÄNDERN")
            //topClickMargin: 20
        }
        Button {
            id: btLower
            width: 80
            height: 25
            visible: (!spencerView.voiceControlled && isCountable) ? true : false
            anchors.left: parent.left
            anchors.top: parent.top
            //topClickMargin: 20
            buttonColor: "white"
            image: "/usr/share/icons/oxygen/22x22/actions/list-remove.png"
            onClicked: spencer.critique("WENIGER " + escapeBrackets(key))
        }
        Text {
            id: teValue
            anchors.top: parent.top
            anchors.left: parent.left

            anchors.leftMargin: (!spencerView.voiceControlled ? (isCountable ?
                                                                       ((btRaise.anchors.leftMargin - btLower.width) / 2) + btLower.width - (width / 2)
                                                                     : btChange.width  + 5)
                                                                : 0)
            text: value
            font.pointSize: spencerView.voiceControlled ? 16 : 14
        }
        Rectangle {
            id: bar
            visible: false //isCountable ? true : false
            width: spencerView.voiceControlled ? 150 : 299
            anchors.top: teValue.bottom
            anchors.topMargin: spencerView.voiceControlled ? 0 : 4
            anchors.left: spencerView.voiceControlled ? teValue.left : parent.left
            height: 5
            border.color: "gray"
            border.width: 1

            Rectangle {
                id: indicator
                anchors.left: bar.left
                anchors.top: bar.top
                height: bar.height
                width: 5
                color: "gray"
                anchors.leftMargin: bar.width * ((realValue - minValue) / (maxValue - minValue)) - 2.5

            }
        }

        Button {
            id: btRaise
            width: 80
            height: 25
            visible: (!spencerView.voiceControlled && isCountable) ? true : false
            anchors.left: parent.left
            anchors.leftMargin: 220
            anchors.top: parent.top
            buttonColor: "white"
            image: "/usr/share/icons/oxygen/22x22/actions/list-add.png"
            onClicked: spencer.critique("MEHR " + escapeBrackets(key))
            //topClickMargin: 20
        }
    }
}
*/
