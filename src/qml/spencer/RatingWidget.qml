import QtQuick 2.0

Item {
    property variant images : [star1, star2, star3, star4, star5]
    property double rating : 0

    onRatingChanged: setRating(rating)

    function setRating(rating) {
        var ratingD = "img/rating_d.png"
        var ratingH = "img/rating_h.png"
        var ratingA = "img/rating_a.png"

        for (var i = 0; i < 5; i++) {
            if (rating < 0.5)
                images[i].source = ratingD
            else if (rating < 1)
                images[i].source = ratingH
            else
                images[i].source = ratingA
            --rating
        }
        console.log("Set rating to " + rating)
    }
    Image {
        id: star1
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
        }
        width: height
        smooth: true
    }
    Image {
        id: star2
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: star1.right
        }
        width: height
        smooth: true
    }
    Image {
        id: star3
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: star2.right
        }
        width: height
        smooth: true
    }
    Image {
        id: star4
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: star3.right
        }
        width: height
        smooth: true
    }
    Image {
        id: star5
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: star4.right
        }
        width: height
        smooth: true
    }
}
