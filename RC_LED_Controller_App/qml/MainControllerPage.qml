import QtQuick
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "components"
import "components/Theme.js" as Theme

FocusScope {
    id: root

    focus: true
    activeFocusOnTab: true

    property bool pendingHeadlightsOpen: DeviceController.headlightsOpen

    function focusFirstControl() {
        passiveRow.forceActiveFocus()
    }

    function scrollRightColumn(direction) {
        var maximum = Math.max(
            0,
            rightColumn.contentHeight - rightColumn.height
        )
        rightColumn.contentY = Math.max(
            0,
            Math.min(
                maximum,
                rightColumn.contentY + direction * 120
            )
        )
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_F1) {
            root.openSettings()
            event.accepted = true
        } else if (event.key === Qt.Key_F2) {
            root.openSettings()
            event.accepted = true
        } else if (coolingRow.activeFocus &&
                   (event.key === Qt.Key_Down ||
                    event.key === Qt.Key_PageDown)) {
            root.scrollRightColumn(1)
            event.accepted = true
        } else if (event.key === Qt.Key_Back ||
                   event.key === Qt.Key_Escape ||
                   event.key === Qt.Key_Backspace) {
            // B/Escape has no destructive action on the home screen.
            event.accepted = true
        }
    }

    signal openSettings()
    signal openTelemetry()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 52

            Column {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 3

                Text {
                    text: "RC Controller"
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                }

                Text {
                    text: "180SX lighting and cooling"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                }
            }

            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 10

                Rectangle {
                    id: connectionButton
                    width: connectionText.implicitWidth + 40
                    height: 29
                    activeFocusOnTab: true
                    focus: false
                    radius: 16
                    color: Theme.panel
                    border.width: activeFocus ? 2 : 1
                    border.color: activeFocus ? Theme.accent : Theme.border
                    KeyNavigation.right: settingsButton

                    Keys.onPressed: function(event) {
                        if (event.key === Qt.Key_Return ||
                            event.key === Qt.Key_Enter ||
                            event.key === Qt.Key_Space) {
                            if (DeviceController.connected) {
                                root.openTelemetry()
                            } else {
                                DeviceController.StartScan()
                            }
                            event.accepted = true
                        }
                    }

                    Row {
                        anchors.centerIn: parent
                        spacing: 8

                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 8
                            height: 8
                            radius: 4
                            color: DeviceController.connected
                                ? Theme.green
                                : Theme.warning
                        }

                        Text {
                            id: connectionText
                            text: DeviceController.connectionStatus
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            font.weight: Font.Medium
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (DeviceController.connected) {
                                root.openTelemetry()
                            } else {
                                DeviceController.StartScan()
                            }
                        }
                    }

                }

                Rectangle {
                    id: settingsButton
                    width: 36
                    height: 36
                    activeFocusOnTab: true
                    focus: false
                    radius: 12
                    color: Theme.panel
                    border.width: activeFocus ? 2 : 1
                    border.color: activeFocus ? Theme.accent : Theme.border
                    KeyNavigation.left: connectionButton
                    KeyNavigation.down: passiveRow

                    Keys.onPressed: function(event) {
                        if (event.key === Qt.Key_Return ||
                            event.key === Qt.Key_Enter ||
                            event.key === Qt.Key_Space) {
                            root.openSettings()
                            event.accepted = true
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        anchors.verticalCenterOffset: -2
                        text: "..."
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.openSettings()
                    }

                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            Card {
                Layout.preferredWidth: 560
                Layout.fillHeight: true
                Layout.minimumHeight: 340

                ColumnLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 18
                    anchors.rightMargin: 18
                    anchors.topMargin: 16
                    anchors.bottomMargin: 22
                    spacing: 10

                    Column {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 37
                        spacing: 3

                        Text {
                            text: "Lighting"
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 17
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: "Compact manual controls for show mode"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                        }
                    }

                    ControlRow {
                        id: passiveRow
                        Layout.fillWidth: true
                        focus: true
                        KeyNavigation.down: activeRow
                        KeyNavigation.right: headlightsControl
                        KeyNavigation.up: settingsButton
                        switchEnabled: true
                        checked: DeviceController.passiveLightsEnabled
                        title: "Passive lights"
                        subtitle: "Parking and ambient LEDs"

                        onToggled: function(checked) {
                            DeviceController.SetPassiveLightsEnabled(
                                checked
                            )
                        }
                    }

                    ControlRow {
                        id: activeRow
                        Layout.fillWidth: true
                        KeyNavigation.up: passiveRow
                        KeyNavigation.down: exhaustRow
                        KeyNavigation.right: headlightsControl
                        switchEnabled: true
                        checked: DeviceController.activeLightsEnabled
                        title: "Active lights"
                        subtitle: "Driving and show lighting"

                        onToggled: function(checked) {
                            DeviceController.SetActiveLightsEnabled(
                                checked
                            )
                        }
                    }

                    ControlRow {
                        id: exhaustRow
                        Layout.fillWidth: true
                        KeyNavigation.up: activeRow
                        KeyNavigation.down: coolingRow
                        KeyNavigation.right: coolingRow
                        switchEnabled: true
                        checked: DeviceController.exhaustEnabled
                        title: "Exhaust LEDs"
                        subtitle: "Rear exhaust glow effects"

                        onToggled: function(checked) {
                            DeviceController.SetExhaustEnabled(
                                checked
                            )
                        }
                    }
                }
            }

            Flickable {
                id: rightColumn
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: width - 18
                contentHeight: rightColumnContent.implicitHeight
                clip: true
                activeFocusOnTab: true
                boundsBehavior: Flickable.StopAtBounds

                Behavior on contentY {
                    NumberAnimation {
                        duration: 180
                        easing.type: Easing.OutCubic
                    }
                }

                Keys.onPressed: function(event) {
                    if (event.key === Qt.Key_Up ||
                        event.key === Qt.Key_Down ||
                        event.key === Qt.Key_PageUp ||
                        event.key === Qt.Key_PageDown) {
                        var direction = event.key === Qt.Key_Up ||
                            event.key === Qt.Key_PageUp ? -1 : 1
                        var amount = event.key === Qt.Key_PageUp ||
                            event.key === Qt.Key_PageDown ? 220 : 120
                        var maximum = Math.max(
                            0,
                            rightColumn.contentHeight - rightColumn.height
                        )
                        rightColumn.contentY = Math.max(
                            0,
                            Math.min(
                                maximum,
                                rightColumn.contentY + direction * amount
                            )
                        )
                        event.accepted = true
                    }
                }

                ScrollBar.vertical: InsetVerticalScrollBar {}

                ColumnLayout {
                    id: rightColumnContent
                    width: rightColumn.contentWidth
                    spacing: 16

                    Card {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 220

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 18
                            anchors.rightMargin: 18
                            anchors.topMargin: 16
                            anchors.bottomMargin: 16
                            spacing: 12

                            Column {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 38
                                spacing: 4

                                Text {
                                    text: "Pop-up headlights"
                                    color: Theme.textPrimary
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 17
                                    font.weight: Font.DemiBold
                                }

                                Text {
                                    text: "Servo position"
                                    color: Theme.textSecondary
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 11
                                }
                            }

                            SegmentedControl {
                                id: headlightsControl
                                Layout.fillWidth: true
                                enabled: true
                                rightSelected: root.pendingHeadlightsOpen
                                KeyNavigation.left: passiveRow
                                KeyNavigation.right: headlightsApplyButton
                                KeyNavigation.down: headlightsApplyButton

                                onSelectionChanged: function(rightSelected) {
                                    root.pendingHeadlightsOpen = rightSelected
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 34

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2

                                    Text {
                                        text: "Current position"
                                        color: Theme.textSecondary
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 10
                                    }

                                    Text {
                                        text: DeviceController.headlightsOpen
                                            ? "Open"
                                            : "Closed"
                                        color: Theme.textPrimary
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 10
                                        font.weight: Font.Medium
                                    }
                                }

                                ActionButton {
                                    id: headlightsApplyButton
                                    text: "Apply"
                                    enabled: root.pendingHeadlightsOpen !==
                                        DeviceController.headlightsOpen
                                    KeyNavigation.up: headlightsControl
                                    KeyNavigation.down: coolingRow
                                    KeyNavigation.left: headlightsControl
                                    onClicked: {
                                        DeviceController.SetHeadlightsOpen(
                                            root.pendingHeadlightsOpen
                                        )
                                    }
                                }
                            }
                        }
                    }

                    Card {
                        Layout.fillWidth: true
                        Layout.topMargin: 14
                        Layout.preferredHeight: 178

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 20
                            anchors.rightMargin: 20
                            anchors.topMargin: 18
                            anchors.bottomMargin: 18
                            spacing: 12

                            ControlRow {
                                id: coolingRow
                                Layout.fillWidth: true
                                Layout.leftMargin: 2
                                Layout.rightMargin: 2
                                switchRightMargin: 8
                                checked: DeviceController.fansEnabled
                                title: "Cooling fans"
                                subtitle: "ESC and motor cooling"
                                KeyNavigation.up: headlightsApplyButton
                                KeyNavigation.left: exhaustRow
                                KeyNavigation.down: null

                                onToggled: function(checked) {
                                    DeviceController.SetFansEnabled(checked)
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.leftMargin: 2
                                Layout.rightMargin: 2
                                Layout.preferredHeight: 33
                                color: Theme.surface
                                radius: 10

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 12
                                    anchors.rightMargin: 12

                                    Text {
                                        text: "Manual mode"
                                        color: Theme.textSecondary
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 11
                                    }

                                    Item { Layout.fillWidth: true }

                                    Text {
                                        text: DeviceController.fansEnabled
                                            ? DeviceController.fanSpeedPercent + "%"
                                            : "Off"
                                        color: Theme.textPrimary
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 11
                                        font.weight: Font.Medium
                                    }
                                }
                            }
                        }
                    }
                }
        }

        }

        Card {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            radius: 12

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14
                anchors.rightMargin: 14

                Rectangle {
                    Layout.preferredWidth: 7
                    Layout.preferredHeight: 7
                    radius: 3.5
                    color: DeviceController.connected
                        ? Theme.green
                        : Theme.warning
                }

                Text {
                    text: DeviceController.connected
                        ? "BT link stable"
                        : DeviceController.connectionStatus

                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    font.weight: Font.Medium
                }

                Text {
                    text: DeviceController.deviceName
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: "Firmware " +
                        DeviceController.firmwareVersion

                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                }

                Text {
                    text: "Landscape"
                    color: Theme.accent
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    font.weight: Font.Medium
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.openTelemetry()
            }
        }
    }
}
