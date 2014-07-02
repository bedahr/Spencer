/*
 *   Copyright (C) 2011 Peter Grasch <grasch@simon-listens.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.2

Item {
    id: mainView
    objectName: "mainView"
    width: 1440
    height: 900
    Component.onCompleted: state = "disconnected"

    states: [
        State {
            name: "disconnected"
            PropertyChanges {
                target: teConnectionStatus
                text: qsTr("Disconnected from Simon Server")
            }
        },
        State {
            name: "connected"
            PropertyChanges {
                target: teConnectionStatus
                text: qsTr("Connected to Simon Server")
            }
        },
        State {
            name: "activated"
            PropertyChanges {
                target: teConnectionStatus
                text: qsTr("Recognition Active")
            }
        }
    ]

    OzController {
    }

    Text {
        id: teConnectionStatus
        x: 50
        y: 28
        font.pointSize: 14
        visible: false //spencerView.voiceControlled
    }

    AnimatedItem {
        id: disconnectConsole
        state: (!spencerView.voiceControlled || mainView.state == "disconnected") ? "hidden" : "shown"

        //don't fade out initially
        visible: false
        Timer {
            interval: 500
            running: false//true
            repeat: false
            onTriggered: disconnectConsole.visible = true
        }

        anchors.verticalCenter: teConnectionStatus.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: (mainView.width > 750) ? mainView.width - width - 15 : 50
        anchors.verticalCenterOffset: (mainView.width > 750) ? -15 : 25

        Behavior on anchors.leftMargin {
            NumberAnimation { duration: 200 }
        }
        Behavior on anchors.verticalCenter {
            NumberAnimation { duration: 200 }
        }
        Button {
            id: btDisconnect
            objectName: "btDisconnect"
            width: 250
            text: qsTr("Disconnect")
            visible:false
        }
    }

    ConnectionSettings {
        id: connectionSettings
        objectName: "connectionSettings"
        state: (spencerView.voiceControlled && (mainView.state == "disconnected")) ? "shown" : "hidden"
        x: 150
        anchors.margins: 30
        width: 300
        anchors.top: teConnectionStatus.bottom
    }


    ActiveConsole{
        anchors.fill: parent
        state: (!spencerView.voiceControlled || mainView.state == "activated") ? "shown" : "hidden"
        mode: connectionSettings.mode
    }


    // status indicators & fluff
    Text {
        id: lbStatus
        objectName: "lbStatus"
        visible:  false // debug mode?
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        text: qsTr("Status")
    }

    Item {
        width: 600
        height: 80
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 0

        Dialog {
            id: errorDialog
            anchors.fill: parent
            anchors.bottomMargin: 0
            objectName: "errorDialog"
            title: qsTr("Error")
            clip: false
        }

        Dialog {
            id: noMatchDialog
            anchors.fill: parent
            anchors.bottomMargin: 0
            objectName: "noMatchDialog"
            title: qsTr("Keine Produkte gefunden für:")
            clip: false
            autoClose:  true
        }
    }
}
