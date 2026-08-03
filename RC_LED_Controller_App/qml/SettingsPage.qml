import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "components"
import "components/Theme.js" as Theme

FocusScope {
    id: root

    focus: true
    activeFocusOnTab: true

    property int activeCategory: 0
    readonly property var settingsRoles: [
        "Unused",
        "Exhaust light 1",
        "Exhaust light 2",
        "Passive lights",
        "Tail lights",
        "Left turning lights",
        "Right turning lights",
        "Headlight servo",
        "Cooling fans"
    ]

    signal goBack()
    signal openTelemetry()
    signal openAssignments()
    signal openServoCalibration()

    function focusFirstControl() {
        focusActiveCategory()
    }

    function focusActiveCategory() {
        if (root.activeCategory === 0) {
            accelerometerRow.forceActiveFocus()
        } else if (root.activeCategory === 1) {
            brightnessRow.slider.forceActiveFocus()
        } else if (root.activeCategory === 2) {
            var firstChannel = settingsChannelList.itemAtIndex(0)
            if (firstChannel) {
                firstChannel.forceActiveFocus()
            }
        } else if (root.activeCategory === 3) {
            servoClosedSlider.forceActiveFocus()
        } else if (root.activeCategory === 4) {
            livePanel.forceActiveFocus()
        } else {
            livePanel.forceActiveFocus()
        }
    }

    function activateCategory(category) {
        root.activeCategory = category
        Qt.callLater(root.focusActiveCategory)
    }

    function cycleCategory(direction) {
        var category = root.activeCategory + direction
        if (category < 0) {
            category = 4
        } else if (category > 4) {
            category = 0
        }
        root.activateCategory(category)
    }

    function focusChannel(index) {
        if (index < 0) {
            backButton.forceActiveFocus()
            return
        }
        if (index > 15) {
            resetDefaultsButton.forceActiveFocus()
            return
        }

        var targetIndex = Math.max(0, Math.min(15, index))
        settingsChannelList.currentIndex = targetIndex
        settingsChannelList.positionViewAtIndex(
            targetIndex,
            ListView.Contain
        )
        Qt.callLater(function() {
            var target = settingsChannelList.itemAtIndex(targetIndex)
            if (target) {
                target.forceActiveFocus()
            }
        })
    }

    function scrollActiveContent(direction) {
        var amount = 150 * direction
        if (root.activeCategory === 0) {
            accelerometerFlickable.contentY = Math.max(
                0,
                Math.min(
                    Math.max(0, accelerometerFlickable.contentHeight - accelerometerFlickable.height),
                    accelerometerFlickable.contentY + amount
                )
            )
        } else if (root.activeCategory === 1) {
            outputLevelsFlickable.contentY = Math.max(
                0,
                Math.min(
                    Math.max(0, outputLevelsFlickable.contentHeight - outputLevelsFlickable.height),
                    outputLevelsFlickable.contentY + amount
                )
            )
        } else if (root.activeCategory === 2) {
            settingsChannelList.contentY = Math.max(
                0,
                Math.min(
                    Math.max(0, settingsChannelList.contentHeight - settingsChannelList.height),
                    settingsChannelList.contentY + amount
                )
            )
        } else if (root.activeCategory === 3) {
            servoFlickable.contentY = Math.max(
                0,
                Math.min(
                    Math.max(0, servoFlickable.contentHeight - servoFlickable.height),
                    servoFlickable.contentY + amount
                )
            )
        }
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_MediaPrevious) {
            root.cycleCategory(-1)
            event.accepted = true
        } else if (event.key === Qt.Key_MediaNext) {
            root.cycleCategory(1)
            event.accepted = true
        } else if (event.key === Qt.Key_PageUp) {
            root.scrollActiveContent(-1)
            event.accepted = true
        } else if (event.key === Qt.Key_PageDown) {
            root.scrollActiveContent(1)
            event.accepted = true
        } else if (event.key === Qt.Key_Back ||
                   event.key === Qt.Key_Escape ||
                   event.key === Qt.Key_Backspace) {
            root.goBack()
            event.accepted = true
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 52

            RowLayout {
                anchors.fill: parent
                spacing: 14

                Row {
                    Layout.preferredWidth: 230
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 12

                    ActionButton {
                        id: backButton
                        text: "<"
                        implicitWidth: 42
                        KeyNavigation.right: accelerometerTab
                        KeyNavigation.down: accelerometerRow
                        onClicked: root.goBack()
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 3

                        Text {
                            text: "Settings"
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 20
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: "A Back  •  LT / RT Select menu"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                        }
                    }
                }

                Flickable {
                    id: categoryStrip
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: categoryRow.implicitWidth
                    contentHeight: height
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    Row {
                        id: categoryRow
                        y: Math.max(0, (categoryStrip.height - height) / 2)
                        spacing: 8

                        SettingsTab {
                            id: accelerometerTab
                            category: 0
                            label: "Exhaust accelerometer"
                            KeyNavigation.left: backButton
                            KeyNavigation.right: outputTab
                            KeyNavigation.down: accelerometerRow
                            onActivated: root.activateCategory(category)
                        }

                        SettingsTab {
                            id: outputTab
                            category: 1
                            label: "Output levels"
                            KeyNavigation.left: accelerometerTab
                            KeyNavigation.right: channelTab
                            KeyNavigation.down: brightnessRow.slider
                            onActivated: root.activateCategory(category)
                        }

                        SettingsTab {
                            id: channelTab
                            category: 2
                            label: "Channel setup"
                            KeyNavigation.left: outputTab
                            KeyNavigation.right: servoTab
                            KeyNavigation.down: settingsChannelList.itemAtIndex(0)
                            onActivated: root.activateCategory(category)
                        }

                        SettingsTab {
                            id: servoTab
                            category: 3
                            label: "Servo calibration"
                            KeyNavigation.left: channelTab
                            KeyNavigation.right: liveTab
                            KeyNavigation.down: servoClosedSlider
                            onActivated: root.activateCategory(category)
                        }

                        SettingsTab {
                            id: liveTab
                            category: 4
                            label: "Live values"
                            KeyNavigation.left: servoTab
                            KeyNavigation.right: accelerometerTab
                            KeyNavigation.down: livePanel
                            onActivated: root.activateCategory(category)
                        }

                    }

                    ScrollBar.horizontal: ScrollBar {
                        policy: ScrollBar.AsNeeded
                        contentItem: Rectangle {
                            implicitHeight: 3
                            radius: 2
                            color: Theme.track
                        }
                    }
                }
            }
        }

        StackLayout {
            id: settingsContent
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.activeCategory

            Card {
                highlighted: root.activeCategory === 0
                Flickable {
                    id: accelerometerFlickable
                    anchors.fill: parent
                    anchors.leftMargin: 18
                    anchors.rightMargin: 18
                    anchors.topMargin: 16
                    anchors.bottomMargin: 16
                    contentWidth: width
                    contentHeight: accelerometerContent.implicitHeight
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                        contentItem: Rectangle {
                            implicitWidth: 4
                            radius: 2
                            color: Theme.track
                        }
                    }

                    ColumnLayout {
                        id: accelerometerContent
                        width: accelerometerFlickable.width
                        spacing: 14

                        ControlRow {
                            id: accelerometerRow
                            Layout.fillWidth: true
                            checked: DeviceController.accelerometerEnabled
                            title: "Exhaust accelerometer"
                            subtitle: "Flash exhaust LEDs when acceleration exceeds the threshold"
                            KeyNavigation.up: null
                            KeyNavigation.down: thresholdSlider
                            KeyNavigation.right: thresholdSlider

                            onToggled: function(checked) {
                                DeviceController.SetPendingAccelerometerEnabled(checked)
                            }
                        }

                        SettingsValuePanel {
                            id: triggerThresholdPanel
                            Layout.fillWidth: true
                            highlighted: thresholdSlider.activeFocus
                            title: "Trigger threshold"
                            subtitle: "Acceleration delta required to activate the effect"
                            valueText: DeviceController.triggerThresholdG.toFixed(2) + " g"
                            valueColor: Theme.accent

                            ValueSlider {
                                id: thresholdSlider
                                anchors.fill: parent
                                from: 0.01
                                to: 0.50
                                stepSize: 0.01
                                value: DeviceController.triggerThresholdG
                                KeyNavigation.up: accelerometerRow
                                KeyNavigation.down: testExhaustButton
                                KeyNavigation.right: testExhaustButton

                                onMoved: {
                                    DeviceController.SetPendingTriggerThresholdG(value)
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 88
                            color: Theme.surface
                            radius: 12

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 14
                                anchors.rightMargin: 14
                                anchors.topMargin: 13
                                anchors.bottomMargin: 13
                                spacing: 12

                                RowLayout {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 32

                                    Column {
                                        Layout.fillWidth: true
                                        spacing: 4

                                        Text {
                                            text: "Live sensor value"
                                            color: Theme.textPrimary
                                            font.family: Theme.fontFamily
                                            font.pixelSize: 13
                                            font.weight: Font.Medium
                                        }

                                        Text {
                                            text: "Use this preview while calibrating the threshold"
                                            color: Theme.textSecondary
                                            font.family: Theme.fontFamily
                                            font.pixelSize: 10
                                        }
                                    }

                                    Rectangle {
                                        Layout.preferredWidth: 58
                                        Layout.preferredHeight: 27
                                        radius: 10
                                        color: Theme.background

                                        Text {
                                            anchors.centerIn: parent
                                            text: DeviceController.filteredForwardAccelerationG.toFixed(2) + " g"
                                            color: Theme.green
                                            font.family: Theme.fontFamily
                                            font.pixelSize: 11
                                            font.weight: Font.DemiBold
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 18
                                    spacing: 8

                                    Item {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 18

                                        Rectangle {
                                            anchors.left: parent.left
                                            anchors.right: parent.right
                                            anchors.verticalCenter: parent.verticalCenter
                                            height: 4
                                            radius: 2
                                            color: Theme.track
                                        }

                                        Rectangle {
                                            anchors.left: parent.left
                                            anchors.verticalCenter: parent.verticalCenter
                                            width: Math.min(
                                                parent.width,
                                                parent.width * Math.abs(
                                                    DeviceController.filteredForwardAccelerationG
                                                ) / 0.50
                                            )
                                            height: 4
                                            radius: 2
                                            color: Math.abs(
                                                DeviceController.filteredForwardAccelerationG
                                            ) >= DeviceController.triggerThresholdG
                                                ? Theme.warning
                                                : Theme.green
                                        }

                                        Rectangle {
                                            anchors.verticalCenter: parent.verticalCenter
                                            x: Math.max(
                                                0,
                                                Math.min(
                                                    parent.width - width,
                                                    parent.width * Math.abs(
                                                        DeviceController.filteredForwardAccelerationG
                                                    ) / 0.50 - width / 2
                                                )
                                            )
                                            width: 16
                                            height: 16
                                            radius: 8
                                            color: "#ffffff"
                                        }
                                    }

                                    Text {
                                        text: Math.abs(
                                            DeviceController.filteredForwardAccelerationG
                                        ) >= DeviceController.triggerThresholdG
                                            ? "Triggered"
                                            : "Below threshold"
                                        color: Math.abs(
                                            DeviceController.filteredForwardAccelerationG
                                        ) >= DeviceController.triggerThresholdG
                                            ? Theme.warning
                                            : Theme.textSecondary
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 10
                                        font.weight: Font.Medium
                                    }
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 33

                            Text {
                                Layout.fillWidth: true
                                text: "Send a short exhaust LED pulse without moving the car"
                                color: Theme.textSecondary
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                            }

                            ActionButton {
                                id: testExhaustButton
                                text: "Test exhaust"
                                enabled: DeviceController.connected
                                KeyNavigation.up: thresholdSlider
                                KeyNavigation.right: brightnessRow.slider
                                onClicked: DeviceController.TestExhaust()
                            }
                        }
                    }
                }
            }

            Card {
                highlighted: root.activeCategory === 1
                Flickable {
                    id: outputLevelsFlickable
                    anchors.fill: parent
                    anchors.leftMargin: 18
                    anchors.rightMargin: 18
                    anchors.topMargin: 16
                    anchors.bottomMargin: 16
                    contentWidth: width
                    contentHeight: outputLevelsContent.implicitHeight
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                        contentItem: Rectangle {
                            implicitWidth: 4
                            radius: 2
                            color: Theme.track
                        }
                    }

                    ColumnLayout {
                        id: outputLevelsContent
                        width: outputLevelsFlickable.width
                        spacing: 12

                        Column {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 38
                            spacing: 4

                            Text {
                                text: "Output levels"
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 17
                                font.weight: Font.DemiBold
                            }

                            Text {
                                text: "Saved as controller variables"
                                color: Theme.textSecondary
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                            }
                        }

                        SettingsSliderRow {
                            id: brightnessRow
                            Layout.fillWidth: true
                            upControl: null
                            downControl: dimBrightnessRow.slider
                            title: "Brightness"
                            subtitle: "Active light output"
                            valueText: DeviceController.activeBrightnessPercent + "%"
                            value: DeviceController.activeBrightnessPercent

                            onValueMoved: function(value) {
                                DeviceController.SetPendingActiveBrightnessPercent(Math.round(value))
                            }
                        }

                        SettingsSliderRow {
                            id: dimBrightnessRow
                            Layout.fillWidth: true
                            upControl: brightnessRow.slider
                            downControl: fanSpeedRow.slider
                            title: "Dim brightness"
                            subtitle: "Passive light output"
                            valueText: DeviceController.dimBrightnessPercent + "%"
                            value: DeviceController.dimBrightnessPercent

                            onValueMoved: function(value) {
                                DeviceController.SetPendingDimBrightnessPercent(Math.round(value))
                            }
                        }

                        SettingsSliderRow {
                            id: fanSpeedRow
                            Layout.fillWidth: true
                            upControl: dimBrightnessRow.slider
                            downControl: null
                            title: "Fan speed"
                            subtitle: "Manual cooling output"
                            valueText: DeviceController.fanSpeedPercent + "%"
                            value: DeviceController.fanSpeedPercent
                            fillColor: Theme.green

                            onValueMoved: function(value) {
                                DeviceController.SetPendingFanSpeedPercent(Math.round(value))
                            }
                        }
                    }
                }
            }

            Card {
                id: channelPanel
                highlighted: root.activeCategory === 2

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 38

                        Column {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                text: "Channel setup"
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 17
                                font.weight: Font.DemiBold
                            }

                            Text {
                                text: "Assign a role to each PCA9685 channel"
                                color: Theme.textSecondary
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                            }
                        }

                        Text {
                            text: "A opens role selector"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: Theme.background
                        radius: 10
                        border.width: 1
                        border.color: Theme.border
                        clip: true

                        ListView {
                            id: settingsChannelList
                            anchors.fill: parent
                            anchors.margins: 6
                            clip: true
                            spacing: 6
                            model: 16
                            boundsBehavior: Flickable.StopAtBounds
                            currentIndex: 0

                            ScrollBar.vertical: ScrollBar {
                                policy: ScrollBar.AsNeeded
                                contentItem: Rectangle {
                                    implicitWidth: 4
                                    radius: 2
                                    color: Theme.track
                                }
                            }

                            delegate: Rectangle {
                                id: settingsChannelRow
                                required property int index

                                width: settingsChannelList.width
                                height: 48
                                radius: 8
                                activeFocusOnTab: true
                                focus: index === 0
                                color: index % 2 === 0
                                    ? Theme.surface
                                    : Theme.panel
                                border.width: activeFocus ? 2 : 1
                                border.color: activeFocus
                                    ? Theme.accent
                                    : Theme.border

                                onActiveFocusChanged: {
                                    if (activeFocus) {
                                        settingsChannelList.currentIndex = index
                                        settingsChannelList.positionViewAtIndex(
                                            index,
                                            ListView.Contain
                                        )
                                    }
                                }

                                function openRolePopup() {
                                    roleBox.popup.open()
                                }

                                Keys.onPressed: function(event) {
                                    if (event.key === Qt.Key_Up) {
                                        root.focusChannel(index - 1)
                                        event.accepted = true
                                    } else if (event.key === Qt.Key_Down) {
                                        root.focusChannel(index + 1)
                                        event.accepted = true
                                    } else if (event.key === Qt.Key_Return ||
                                               event.key === Qt.Key_Enter ||
                                               event.key === Qt.Key_Space ||
                                               event.key === Qt.Key_B ||
                                               event.key === Qt.Key_Right) {
                                        settingsChannelRow.openRolePopup()
                                        event.accepted = true
                                    }
                                }

                                TapHandler {
                                    onTapped: settingsChannelRow.forceActiveFocus()
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 12
                                    anchors.rightMargin: 8
                                    spacing: 12

                                    Text {
                                        Layout.preferredWidth: 70
                                        text: "CH " + settingsChannelRow.index
                                        color: Theme.textPrimary
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 13
                                        font.weight: Font.DemiBold
                                    }

                                    ComboBox {
                                        id: roleBox
                                        Layout.fillWidth: true
                                        model: root.settingsRoles
                                        currentIndex: DeviceController.GetChannelRole(
                                            settingsChannelRow.index
                                        )
                                        activeFocusOnTab: false
                                        focus: false
                                        enabled: true

                                        onActivated: {
                                            DeviceController.SetChannelRole(
                                                settingsChannelRow.index,
                                                currentIndex
                                            )
                                            popup.close()
                                            settingsChannelRow.forceActiveFocus()
                                        }

                                        Keys.onPressed: function(event) {
                                            if (event.key === Qt.Key_Return ||
                                                event.key === Qt.Key_Enter ||
                                                event.key === Qt.Key_Space ||
                                                event.key === Qt.Key_B) {
                                                roleBox.popup.open()
                                                event.accepted = true
                                            }
                                        }

                                        contentItem: Text {
                                            leftPadding: 12
                                            rightPadding: 30
                                            text: roleBox.displayText
                                            color: Theme.textPrimary
                                            font.family: Theme.fontFamily
                                            font.pixelSize: 11
                                            verticalAlignment: Text.AlignVCenter
                                            elide: Text.ElideRight
                                        }

                                        background: Rectangle {
                                            implicitHeight: 34
                                            radius: 10
                                            color: Theme.panel
                                            border.width: roleBox.activeFocus ? 2 : 1
                                            border.color: roleBox.activeFocus
                                                ? Theme.accent
                                                : Theme.border
                                        }

                                        indicator: Text {
                                            x: roleBox.width - width - 10
                                            y: (roleBox.height - height) / 2
                                            text: "▼"
                                            color: Theme.textSecondary
                                            font.family: Theme.fontFamily
                                            font.pixelSize: 10
                                        }

                                        delegate: ItemDelegate {
                                            id: settingsRoleDelegate
                                            width: roleBox.width
                                            height: 34
                                             highlighted: settingsRolePopup.pendingIndex === index

                                            contentItem: Text {
                                                leftPadding: 10
                                                text: modelData
                                                color: Theme.textPrimary
                                                font.family: Theme.fontFamily
                                                font.pixelSize: 11
                                                verticalAlignment: Text.AlignVCenter
                                            }

                                            background: Rectangle {
                                                radius: 7
                                                color: settingsRoleDelegate.highlighted
                                                    ? Theme.accentMuted
                                                    : Theme.panel
                                                border.width: settingsRoleDelegate.highlighted ? 1 : 0
                                                border.color: Theme.accent
                                            }
                                        }

                                        popup: Popup {
                                            id: settingsRolePopup
                                            y: roleBox.height + 4
                                            width: roleBox.width
                                            padding: 4
                                            modal: true
                                            focus: true
                                            property int pendingIndex: roleBox.currentIndex

                                            onOpened: {
                                                roleList.currentIndex = roleBox.currentIndex
                                                pendingIndex = roleBox.currentIndex
                                                roleList.positionViewAtIndex(
                                                    roleList.currentIndex,
                                                    ListView.Contain
                                                )
                                                roleList.forceActiveFocus()
                                            }

                                            onClosed: settingsChannelRow.forceActiveFocus()

                                            Keys.priority: Keys.BeforeItem
                                            Keys.onPressed: function(event) {
                                                if (event.key === Qt.Key_Up ||
                                                    event.key === Qt.Key_Left) {
                                                    roleList.moveRole(-1)
                                                    event.accepted = true
                                                } else if (event.key === Qt.Key_Down ||
                                                    event.key === Qt.Key_Right) {
                                                    roleList.moveRole(1)
                                                    event.accepted = true
                                                } else if (event.key === Qt.Key_PageUp) {
                                                    roleList.moveRole(-5)
                                                    event.accepted = true
                                                } else if (event.key === Qt.Key_PageDown) {
                                                    roleList.moveRole(5)
                                                    event.accepted = true
                                                }
                                            }

                                            contentItem: ListView {
                                                id: roleList
                                                clip: true
                                                focus: true
                                                activeFocusOnTab: true
                                                interactive: true
                                                implicitHeight: Math.min(
                                                    contentHeight,
                                                    9 * 34
                                                )
                                                model: settingsRolePopup.visible
                                                    ? roleBox.delegateModel
                                                    : null
                                                currentIndex: 0

                                                function moveRole(delta) {
                                                    var next = Math.max(
                                                        0,
                                                        Math.min(
                                                    root.settingsRoles.length - 1,
                                                            currentIndex + delta
                                                        )
                                                    )
                                                    currentIndex = next
                                                    settingsRolePopup.pendingIndex = next
                                                    positionViewAtIndex(
                                                        next,
                                                        ListView.Contain
                                                    )
                                                }

                                                Keys.onPressed: function(event) {
                                                    if (event.key === Qt.Key_Return ||
                                                               event.key === Qt.Key_Enter ||
                                                               event.key === Qt.Key_Space ||
                                                               event.key === Qt.Key_B) {
                                                        roleBox.currentIndex = roleList.currentIndex
                                                        DeviceController.SetChannelRole(
                                                            settingsChannelRow.index,
                                                            roleList.currentIndex
                                                        )
                                                        settingsRolePopup.close()
                                                        event.accepted = true
                                                    } else if (event.key === Qt.Key_Back ||
                                                               event.key === Qt.Key_Escape ||
                                                               event.key === Qt.Key_Backspace) {
                                                        settingsRolePopup.close()
                                                        event.accepted = true
                                                    }
                                                }

                                                ScrollBar.vertical: ScrollBar {
                                                    policy: ScrollBar.AsNeeded
                                                }
                                            }

                                            background: Rectangle {
                                                color: Theme.panel
                                                radius: 10
                                                border.width: 1
                                                border.color: Theme.border
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Card {
                id: servoPanel
                highlighted: root.activeCategory === 3

                Flickable {
                    id: servoFlickable
                    anchors.fill: parent
                    anchors.leftMargin: 18
                    anchors.rightMargin: 18
                    anchors.topMargin: 16
                    anchors.bottomMargin: 16
                    contentWidth: width
                    contentHeight: servoContent.implicitHeight
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                        contentItem: Rectangle {
                            implicitWidth: 4
                            radius: 2
                            color: Theme.track
                        }
                    }

                    ColumnLayout {
                        id: servoContent
                        width: parent.width
                        spacing: 14

                        Column {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 38
                            spacing: 4

                            Text {
                                text: "Servo calibration"
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 17
                                font.weight: Font.DemiBold
                            }

                            Text {
                                text: "Set the closed and open positions for the pop-up headlights"
                                color: Theme.textSecondary
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                            }
                        }

                        ServoSettingRow {
                            id: servoClosedSlider
                            Layout.fillWidth: true
                            title: "Closed position"
                            subtitle: "Position used when headlights are closed"
                            value: DeviceController.servoClosedPulseUs
                            upControl: null
                            downControl: servoOpenSlider
                            onValueMoved: DeviceController.SetPendingServoClosedPulseUs(value)
                        }

                        ServoSettingRow {
                            id: servoOpenSlider
                            Layout.fillWidth: true
                            title: "Open position"
                            subtitle: "Position used when headlights are open"
                            value: DeviceController.servoOpenPulseUs
                            upControl: servoClosedSlider
                            downControl: zeroServoButton
                            onValueMoved: DeviceController.SetPendingServoOpenPulseUs(value)
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 42

                            Text {
                                Layout.fillWidth: true
                                text: DeviceController.servoZeroed
                                    ? "Zero position command sent"
                                    : "No position sensor is fitted; zero-out drives the servo to Closed"
                                color: DeviceController.servoZeroed
                                    ? Theme.green
                                    : Theme.textSecondary
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                                wrapMode: Text.WordWrap
                            }

                            ActionButton {
                                id: zeroServoButton
                                text: "Zero-out servo"
                                controllerKey: Qt.Key_X
                                KeyNavigation.up: servoOpenSlider
                                onClicked: DeviceController.ZeroServo()
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
                            color: Theme.border
                        }

                        Text {
                            Layout.fillWidth: true
                            text: "Pulse values are PCA9685 counts at 50 Hz (0–4095). Save settings below to store them on the ESP32."
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }

            Card {
                id: livePanel
                highlighted: root.activeCategory === 4
                activeFocusOnTab: true
                focus: false
                KeyNavigation.up: liveTab

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 12

                    Column {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        spacing: 4

                        Text {
                            text: "Live values"
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 17
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: DeviceController.connected
                                ? DeviceController.deviceName
                                : "Controller disconnected"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        columns: 3
                        rows: 2
                        columnSpacing: 12
                        rowSpacing: 12

                        TelemetryCell {
                            title: "CH1 steering"
                            value: DeviceController.steeringPercent + "%"
                            detail: DeviceController.steeringPulseUs > 0
                                ? DeviceController.steeringPulseUs + " µs"
                                : "No signal"
                        }

                        TelemetryCell {
                            title: "CH2 throttle"
                            value: DeviceController.throttlePercent + "%"
                            detail: DeviceController.throttlePulseUs > 0
                                ? DeviceController.throttlePulseUs + " µs"
                                : "No signal"
                        }

                        TelemetryCell {
                            title: "Lighting state"
                            value: DeviceController.activeLightsEnabled
                                ? "ACTIVE"
                                : "NONE"
                            detail: DeviceController.exhaustPulseActive
                                ? "Exhaust pulse active"
                                : "Active lighting enabled"
                        }

                        TelemetryCell {
                            title: "Acceleration"
                            value: DeviceController.filteredForwardAccelerationG.toFixed(3) + " g"
                            detail: "FWD " + DeviceController.forwardAccelerationG.toFixed(3) +
                                "  SIDE " + DeviceController.sideAccelerationG.toFixed(3)
                        }

                        TelemetryCell {
                            title: "Gyroscope X / Y"
                            value: DeviceController.gyroscopeXDps.toFixed(1) +
                                " / " + DeviceController.gyroscopeYDps.toFixed(1)
                            detail: "degrees per second"
                        }

                        TelemetryCell {
                            title: "Gyroscope Z"
                            value: DeviceController.gyroscopeZDps.toFixed(1) + " dps"
                            detail: "Firmware " + DeviceController.firmwareVersion
                        }
                    }
                }
            }

        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 44

                        Text {
                            text: DeviceController.settingsDirty
                                ? DeviceController.connected
                                    ? "Unsaved changes"
                                    : "Unsaved changes • Connect controller to save"
                                : "Values are stored on the ESP32 controller"
                color: DeviceController.settingsDirty
                    ? Theme.warning
                    : Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: 10
            }

            Item { Layout.fillWidth: true }

            Text {
                text: "Y Save   X Restore defaults"
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: 10
            }

            ActionButton {
                id: resetDefaultsButton
                text: "Reset defaults"
                controllerKey: Qt.Key_X
                enabled: true
                KeyNavigation.up: root.activeCategory === 0
                    ? testExhaustButton
                    : root.activeCategory === 1
                        ? fanSpeedRow.slider
                        : root.activeCategory === 2
                            ? settingsChannelList.itemAtIndex(15)
                            : root.activeCategory === 3
                                ? zeroServoButton
                                : livePanel
                KeyNavigation.right: saveSettingsButton
                onClicked: DeviceController.ResetDefaults()
            }

            ActionButton {
                id: saveSettingsButton
                text: "Save settings"
                controllerKey: Qt.Key_Y
                primary: true
                KeyNavigation.left: resetDefaultsButton
                KeyNavigation.up: root.activeCategory === 0
                    ? testExhaustButton
                    : root.activeCategory === 1
                        ? fanSpeedRow.slider
                        : root.activeCategory === 2
                            ? settingsChannelList.itemAtIndex(15)
                            : root.activeCategory === 3
                                ? zeroServoButton
                                : livePanel
                enabled: DeviceController.settingsDirty
                onClicked: DeviceController.SaveSettings()
            }
        }
    }

    component SettingsTab: Rectangle {
        id: tab

        property int category: 0
        property string label: ""
        signal activated()

        width: Math.max(94, tabLabel.implicitWidth + 28)
        height: 34
        radius: 11
        activeFocusOnTab: true
        color: root.activeCategory === tab.category
            ? Theme.accentMuted
            : Theme.panel
        border.width: activeFocus || root.activeCategory === tab.category ? 2 : 1
        border.color: activeFocus || root.activeCategory === tab.category
            ? Theme.accent
            : Theme.border

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Back ||
                event.key === Qt.Key_Escape ||
                event.key === Qt.Key_Backspace) {
                root.goBack()
                event.accepted = true
            } else if (event.key === Qt.Key_Return ||
                event.key === Qt.Key_Enter ||
                event.key === Qt.Key_Space ||
                event.key === Qt.Key_B) {
                tab.activated()
                event.accepted = true
            }
        }

        Text {
            id: tabLabel
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            text: tab.label
            color: root.activeCategory === tab.category
                ? Theme.accent
                : Theme.textPrimary
            font.family: Theme.fontFamily
            font.pixelSize: 10
            font.weight: root.activeCategory === tab.category
                ? Font.DemiBold
                : Font.Medium
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                tab.forceActiveFocus()
                tab.activated()
            }
        }
    }

    component SettingsValuePanel: Rectangle {
        id: valuePanel

        property string title: ""
        property string subtitle: ""
        property string valueText: ""
        property color valueColor: Theme.accent
        property bool highlighted: false

        color: Theme.surface
        radius: 12
        border.width: valuePanel.highlighted ? 2 : 0
        border.color: Theme.accent
        implicitHeight: 96

        ColumnLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            anchors.topMargin: 13
            anchors.bottomMargin: 13
            spacing: 16

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 33

                Column {
                    Layout.fillWidth: true
                    spacing: 4

                    Text {
                        text: valuePanel.title
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 14
                        font.weight: Font.Medium
                    }

                    Text {
                        text: valuePanel.subtitle
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                    }
                }

                Rectangle {
                    Layout.preferredWidth: valueChip.implicitWidth + 20
                    Layout.preferredHeight: 27
                    radius: 10
                    color: Theme.background

                    Text {
                        id: valueChip
                        anchors.centerIn: parent
                        text: valuePanel.valueText
                        color: valuePanel.valueColor
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }
                }
            }

            default property alias content: valueContent.data

            Item {
                id: valueContent
                Layout.fillWidth: true
                Layout.preferredHeight: 18
            }
        }
    }

    component SettingsSliderRow: Rectangle {
        id: settingsRow

        property string title: ""
        property string subtitle: ""
        property string valueText: ""
        property real value: 0
        property color fillColor: Theme.accent
        property Item upControl: null
        property Item downControl: null
        property alias slider: settingsSlider

        signal valueMoved(real value)

        color: Theme.surface
        radius: 12
        border.width: settingsSlider.activeFocus ? 2 : 0
        border.color: Theme.accent
        implicitHeight: 87

        ColumnLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            anchors.topMargin: 13
            anchors.bottomMargin: 13
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 33

                Column {
                    Layout.fillWidth: true
                    spacing: 4

                    Text {
                        text: settingsRow.title
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 14
                        font.weight: Font.Medium
                    }

                    Text {
                        text: settingsRow.subtitle
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                    }
                }

                Rectangle {
                    Layout.preferredWidth: Math.max(44, chipText.implicitWidth + 20)
                    Layout.preferredHeight: 27
                    radius: 10
                    color: Theme.background

                    Text {
                        id: chipText
                        anchors.centerIn: parent
                        text: settingsRow.valueText
                        color: settingsRow.fillColor
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }
                }
            }

            ValueSlider {
                id: settingsSlider
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: settingsRow.value
                fillColor: settingsRow.fillColor
                KeyNavigation.up: settingsRow.upControl
                KeyNavigation.down: settingsRow.downControl

                onMoved: settingsRow.valueMoved(value)
            }
        }
    }

    component ServoSettingRow: Rectangle {
        id: servoRow

        property string title: ""
        property string subtitle: ""
        property int value: 0
        property Item upControl: null
        property Item downControl: null
        property alias slider: servoSlider

        signal valueMoved(int value)

        color: Theme.surface
        radius: 12
        border.width: servoSlider.activeFocus ? 2 : 0
        border.color: Theme.accent
        implicitHeight: 87

        ColumnLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            anchors.topMargin: 13
            anchors.bottomMargin: 13
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 33

                Column {
                    Layout.fillWidth: true
                    spacing: 4

                    Text {
                        text: servoRow.title
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 14
                        font.weight: Font.Medium
                    }

                    Text {
                        text: servoRow.subtitle
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                    }
                }

                Rectangle {
                    Layout.preferredWidth: 58
                    Layout.preferredHeight: 27
                    radius: 10
                    color: Theme.background

                    Text {
                        anchors.centerIn: parent
                        text: servoRow.value
                        color: Theme.accent
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }
                }
            }

            ValueSlider {
                id: servoSlider
                Layout.fillWidth: true
                from: 0
                to: 4095
                stepSize: 1
                value: servoRow.value
                KeyNavigation.up: servoRow.upControl
                KeyNavigation.down: servoRow.downControl

                onMoved: servoRow.valueMoved(value)
            }
        }
    }

    component TelemetryCell: Rectangle {
        id: telemetryCell

        property string title: ""
        property string value: ""
        property string detail: ""

        Layout.fillWidth: true
        Layout.fillHeight: true
        color: Theme.surface
        radius: 12

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 7

            Text {
                Layout.fillWidth: true
                text: telemetryCell.title
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: 11
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                text: telemetryCell.value
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 20
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                text: telemetryCell.detail
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: 10
                elide: Text.ElideRight
            }
        }
    }

    component SettingsCategoryCard: Card {
        id: categoryCard

        property int category: 0
        property string title: ""
        property string subtitle: ""
        property string hint: ""

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 10

            Text {
                text: categoryCard.title
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 20
                font.weight: Font.DemiBold
            }

            Text {
                text: categoryCard.subtitle
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: 12
            }

            Item { Layout.fillHeight: true }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 52
                color: Theme.surface
                radius: 12

                Text {
                    anchors.centerIn: parent
                    text: categoryCard.hint
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                }
            }

            Item { Layout.fillHeight: true }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.activateCategory(categoryCard.category)
        }
    }
}
