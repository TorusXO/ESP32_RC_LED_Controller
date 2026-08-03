import QtQuick
import QtQuick.Controls
import "Theme.js" as Theme

Slider {
    id: root

    property color fillColor: Theme.accent

    implicitHeight: 24
    activeFocusOnTab: true

    Keys.onLeftPressed: function(event) {
        decrement()
        event.accepted = true
    }

    Keys.onRightPressed: function(event) {
        increment()
        event.accepted = true
    }

    background: Item {
        x: root.leftPadding
        y: root.topPadding + root.availableHeight / 2 - 2
        width: root.availableWidth
        height: 4

        Rectangle {
            anchors.fill: parent
            radius: 2
            color: Theme.track
        }

        Rectangle {
            width: root.visualPosition * parent.width
            height: parent.height
            radius: 2
            color: root.fillColor
        }
    }

    handle: Rectangle {
        x: root.leftPadding +
            root.visualPosition *
            (root.availableWidth - width)

        y: root.topPadding +
            root.availableHeight / 2 -
            height / 2

        width: 16
        height: 16
        radius: 8
        color: "#ffffff"
        border.width: root.activeFocus ? 2 : 0
        border.color: Theme.accent
    }
}
