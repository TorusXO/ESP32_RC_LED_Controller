import QtQuick
import QtQuick.Layouts
import "Theme.js" as Theme

Rectangle {
    id: root

    property string title: ""
    property string subtitle: ""
    property bool checked: false
    property bool switchEnabled: true

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

    color: Theme.surface
    radius: 12
    border.width: activeFocus ? 2 : 0
    border.color: Theme.accent
    implicitHeight: 76

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5

            Text {
                text: root.title
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 15
                font.weight: Font.Medium
            }

            Text {
                text: root.subtitle
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: 11
            }
        }

        AppSwitch {
            Layout.preferredWidth: 42
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            activeFocusOnTab: false
            focus: false
            checked: root.checked
            enabled: root.switchEnabled
            onToggled: function(checked) {
                root.toggled(checked)
            }
        }
    }
}
