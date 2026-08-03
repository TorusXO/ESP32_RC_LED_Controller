import QtQuick
import QtQuick.Controls
import "Theme.js" as Theme

Slider {
    id: root

    property color fillColor: Theme.accent
    signal dpadMoved(real value)

    implicitHeight: 24
    activeFocusOnTab: true

    function moveByStep(direction) {
        var step = root.stepSize > 0
            ? root.stepSize
            : (root.to - root.from) * 0.01
        var next = root.value + direction * step
        next = Math.max(root.from, Math.min(root.to, next))

        if (root.stepSize > 0) {
            next = root.from + Math.round((next - root.from) / step) * step
            next = Math.max(root.from, Math.min(root.to, next))
        }

        if (Math.abs(next - root.value) < 0.000001) {
            return
        }

        root.value = next
        root.dpadMoved(next)
    }

    Keys.onLeftPressed: function(event) {
        root.moveByStep(-1)
        event.accepted = true
    }

    Keys.onRightPressed: function(event) {
        root.moveByStep(1)
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
