import QtQuick
import QtQuick.Controls
import "Theme.js" as Theme

Button {
    id: root

    property bool primary: false
    property int controllerKey: -1

    implicitHeight: 34
    activeFocusOnTab: true
    leftPadding: 18
    rightPadding: 18

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Return ||
            event.key === Qt.Key_Enter ||
            event.key === Qt.Key_Space ||
            event.key === Qt.Key_B) {
            root.clicked()
            event.accepted = true
        } else if (event.key === root.controllerKey &&
                   root.controllerKey !== -1) {
            root.clicked()
            event.accepted = true
        }
    }

    contentItem: Text {
        text: root.text
        color: root.primary ? "#ffffff" : Theme.textPrimary
        font.family: Theme.fontFamily
        font.pixelSize: 11
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        radius: 11
        color: root.primary
            ? (root.down ? Theme.accent : Theme.accentMuted)
            : Theme.panel

        border.width: root.activeFocus ? 2 : 1
        border.color: root.primary
            ? Theme.accent
            : root.activeFocus ? Theme.accent : Theme.border

        opacity: root.enabled
            ? (root.down ? 0.78 : 1.0)
            : 0.4
    }
}
