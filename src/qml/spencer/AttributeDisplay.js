
function createDetails(parent, attributeName, attributeValue) {

    var component = Qt.createComponent("AttributeDelegate.qml");
    var sprite = component.createObject(parent, { key : attributeName, value : attributeValue});

    if (sprite == null) {
        // Error Handling
        console.log("Error creating object");
    }
    return sprite
}
