import QtQuick

import "Theme.js" as Theme

Rectangle {
    id: root

    property var options: []
    property int currentIndex: 0

    signal selectionChanged(int index)

    activeFocusOnTab: true
    focus: false
    implicitHeight: 34
    implicitWidth: 180
    color: Theme.background
    radius: 10
    border.width: activeFocus ? 2 : 1
    border.color: activeFocus ? Theme.accent : Theme.border

    function setIndex(index) {
        var nextIndex = Math.max(
            0,
            Math.min(options.length - 1, index)
        )

        if (nextIndex === root.currentIndex) {
            return
        }

        root.currentIndex = nextIndex
        root.selectionChanged(nextIndex)
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Left) {
            root.setIndex(root.currentIndex - 1)
            event.accepted = true
        } else if (event.key === Qt.Key_Right) {
            root.setIndex(root.currentIndex + 1)
            event.accepted = true
        } else if (event.key === Qt.Key_Up || event.key === Qt.Key_Down) {
            event.accepted = false
        }
    }

    Rectangle {
        x: root.options.length > 0
            ? root.currentIndex * root.width / root.options.length + 3
            : 3
        y: 3
        width: root.options.length > 0
            ? root.width / root.options.length - 6
            : root.width - 6
        height: root.height - 6
        radius: 8
        color: Theme.accent

        Behavior on x {
            NumberAnimation {
                duration: 120
                easing.type: Easing.OutCubic
            }
        }
    }

    Repeater {
        model: root.options

        delegate: MouseArea {
            required property int index

            x: index * root.width / root.options.length
            width: root.width / root.options.length
            height: root.height
            onClicked: root.setIndex(index)

            Text {
                anchors.fill: parent
                text: root.options[parent.index]
                color: root.currentIndex === parent.index
                    ? Theme.textPrimary
                    : Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: 11
                font.weight: root.currentIndex === parent.index
                    ? Font.DemiBold
                    : Font.Medium
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
