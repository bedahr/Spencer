
function createSentimentDisplay(parent, aspect, value, mode) {
    var component = Qt.createComponent("SentimentDelegate.qml");
    var sprite = component.createObject(parent, { key : aspect, value : value, mode : mode});

    if (sprite == null) {
        console.log("Error creating sentiment delegate");
    }
    return sprite
}
