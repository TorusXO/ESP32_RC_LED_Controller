import QtQuick
import QtQuick.Controls

import "Theme.js" as Theme

ScrollBar {
    id: root

    policy: ScrollBar.AsNeeded
    width: 8
    anchors.topMargin: 8
    anchors.bottomMargin: 8
    anchors.rightMargin: 8

    background: Rectangle {
        color: "transparent"
    }

    contentItem: Rectangle {
        implicitWidth: 3
        width: 3
        radius: 2
        anchors.horizontalCenter: parent.horizontalCenter
        color: Theme.track
    }
}
