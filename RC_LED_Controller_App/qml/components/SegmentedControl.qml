import QtQuick
import "Theme.js" as Theme

Rectangle {
    id: root

    property bool rightSelected: false
    property string leftText: "Closed"
    property string rightText: "Open"

    signal selectionChanged(bool rightSelected)

    activeFocusOnTab: true
    focus: false
    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Space ||
            event.key === Qt.Key_Return ||
            event.key === Qt.Key_Enter ||
            event.key === Qt.Key_B) {
            root.selectionChanged(!root.rightSelected)
            event.accepted = true
        }
    }

    color: Theme.background
    radius: 12
    implicitHeight: 50
    border.width: activeFocus ? 2 : 1
    border.color: activeFocus ? Theme.accent : Theme.border

    Rectangle {
        x: root.rightSelected ? root.width / 2 : 4
        y: 4
        width: root.width / 2 - 6
        height: root.height - 8
        radius: 9
        color: Theme.accent

        Behavior on x {
            NumberAnimation {
                duration: 130
                easing.type: Easing.OutCubic
            }
        }
    }

    Text {
        x: 4
        width: parent.width / 2 - 6
        height: parent.height
        text: root.leftText
        color: root.rightSelected
            ? Theme.textSecondary
            : Theme.textPrimary

        font.family: Theme.fontFamily
        font.pixelSize: 12
        font.weight: root.rightSelected
            ? Font.Medium
            : Font.DemiBold

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Text {
        x: parent.width / 2 + 2
        width: parent.width / 2 - 6
        height: parent.height
        text: root.rightText
        color: root.rightSelected
            ? Theme.textPrimary
            : Theme.textSecondary

        font.family: Theme.fontFamily
        font.pixelSize: 12
        font.weight: root.rightSelected
            ? Font.DemiBold
            : Font.Medium

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    MouseArea {
        anchors.left: parent.left
        width: parent.width / 2
        height: parent.height
        onClicked: root.selectionChanged(false)
    }

    MouseArea {
        anchors.right: parent.right
        width: parent.width / 2
        height: parent.height
        onClicked: root.selectionChanged(true)
    }
}
