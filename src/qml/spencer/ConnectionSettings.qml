import QtQuick 2.2

AnimatedItem {
    property string mode : cbPushToTalk.checked ? "pushToTalk" : "vad"

    id: connectionSettings
    Text {
        id: lbHostName

        anchors.left: parent.left
        anchors.verticalCenter: teHost.verticalCenter

        text: qsTr("Host:")
    }

    LineEdit {
        id: teHost
        objectName: "teHost"
        anchors.right: parent.right
        width: 200
        text: qsTr("192.168.1.1")
    }

    Text {
        id: lbPort

        anchors.left: parent.left
        anchors.verticalCenter: tePort.verticalCenter

        text: qsTr("Port:")
    }

    LineEdit {
        id: tePort
        objectName: "tePort"
        anchors.top: teHost.bottom
        anchors.topMargin: 10
        anchors.right: parent.right
        width: 200
        inputMask: "Dddddddd"
        text: qsTr("4444")
    }

    Text {
        id: lbUserName
        anchors.left: parent.left
        anchors.verticalCenter: teUserName.verticalCenter

        text: qsTr("Username:")
    }

    LineEdit {
        id: teUserName
        objectName: "teUserName"
        anchors.right: parent.right
        anchors.top: tePort.bottom
        anchors.topMargin: 10
        width: 200
        text: "default"
    }
    Text {
        id: lbPassword
        anchors.left: parent.left
        anchors.verticalCenter: tePassword.verticalCenter

        text: qsTr("Password:")
    }

    LineEdit {
        id: tePassword
        objectName: "tePassword"
        anchors.right: parent.right
        anchors.top: teUserName.bottom
        anchors.topMargin: 10
        width: 200
        echoMode: TextInput.Password
        text: "nopassword"
    }

    CheckBox {
        id: cbAutoConnect
        objectName: "cbAutoConnect"
        anchors.top: tePassword.bottom
        anchors.topMargin: 20
        text: qsTr("Connect automatically")
        width: 200
    }

    CheckBox {
        id: cbPushToTalk
        objectName: "cbPushToTalk"
        anchors.top: cbAutoConnect.bottom
        anchors.topMargin: 10
        text: qsTr("Push to talk")
        width: 200
    }

    Button {
        id: btConnect
        objectName: "btConnect"
        anchors.top: cbPushToTalk.bottom
        anchors.topMargin: 15
        anchors.horizontalCenter: parent.horizontalCenter
        width: 250
        text: qsTr("Connect")
    }
}
