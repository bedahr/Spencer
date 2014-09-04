import QtQuick 2.0

Item {
    focus: true
    Keys.onPressed: {
        if (event.key === Qt.Key_R) {
            console.log("R pressed")
            spencer.reset()
        }
        if (event.key === Qt.Key_1) {
            console.log("Initiator 1 requested")
            spencer.overwriteDialogStrategy(1)
        }
        if (event.key === Qt.Key_2) {
            console.log("Initiator 2 requested")
            spencer.overwriteDialogStrategy(2)
        }
        if (event.key === Qt.Key_3) {
            console.log("Initiator 3 requested")
            spencer.overwriteDialogStrategy(3)
        }
        if (event.key === Qt.Key_4) {
            console.log("Initiator 4 requested")
            spencer.overwriteDialogStrategy(4)
        }
        if (event.key === Qt.Key_5) {
            console.log("Initiator 5 requested")
            spencer.overwriteDialogStrategy(5)
        }
        if (event.key === Qt.Key_6) {
            console.log("Recommendation requested")
            spencer.overwriteDialogStrategy(6)
        }
    }
}
