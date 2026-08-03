import QtQuick
import "Theme.js" as Theme

Item {
    id: root

    property bool checked: false
    signal toggled(bool checked)

    activeFocusOnTab: true
    focus: false
    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Space ||
            event.key === Qt.Key_Return ||
            event.key === Qt.Key_Enter ||
            event.key === Qt.Key_B) {
            root.toggled(!root.checked)
            event.accepted = true
        }
    }

    width: 42
    height: 24
    implicitWidth: 42
    implicitHeight: 24
    opacity: enabled ? 1.0 : 0.45

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: root.checked ? Theme.green : Theme.track
        border.width: root.activeFocus ? 2 : 0
        border.color: Theme.accent

        Behavior on color {
            ColorAnimation { duration: 110 }
        }
    }

    Rectangle {
        width: 18
        height: 18
        radius: 9
        color: "#ffffff"
        x: root.checked ? 21 : 3
        y: 3

        Behavior on x {
            NumberAnimation {
                duration: 110
                easing.type: Easing.OutCubic
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.enabled
        onClicked: root.toggled(!root.checked)
    }
}
