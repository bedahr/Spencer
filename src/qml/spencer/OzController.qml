import QtQuick 2.0

Item {
    focus: true
    Keys.onPressed: {
        if (event.key == Qt.Key_R) {
            console.log("R pressed")
            spencer.reset()
        }
    }
}
