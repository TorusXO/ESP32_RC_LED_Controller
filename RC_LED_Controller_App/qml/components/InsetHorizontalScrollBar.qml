import QtQuick
import QtQuick.Controls

import "Theme.js" as Theme

ScrollBar {
    id: root

    policy: ScrollBar.AsNeeded
    height: 8
    anchors.leftMargin: 8
    anchors.rightMargin: 8
    anchors.bottomMargin: 3

    background: Rectangle {
        color: "transparent"
    }

    contentItem: Rectangle {
        implicitHeight: 3
        height: 3
        radius: 2
        color: Theme.track
    }
}
