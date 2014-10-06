
function createDetails(parent, attributeName, attributeValue, userInterest, completionFactor) {

    var component = Qt.createComponent("AttributeDelegate.qml");
    console.log("attribute: " + attributeName + " userInterest: " + userInterest + " completionFactor: " + completionFactor)
    var sprite = component.createObject(parent, { key : attributeName, value : attributeValue,
                                                  userInterest : userInterest, completionFactor: completionFactor});

    if (sprite == null) {
        // Error Handling
        console.log("Error creating object");
    }
    return sprite
}
