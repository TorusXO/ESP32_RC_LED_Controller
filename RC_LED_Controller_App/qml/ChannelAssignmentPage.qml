import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "components"
import "components/Theme.js" as Theme

FocusScope {
    id: root

    focus: true
    activeFocusOnTab: true

    signal goBack()

    property string saveFeedback: ""

    readonly property var roles: [
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

    Timer {
        id: saveFeedbackTimer
        interval: 3500
        repeat: false
        onTriggered: root.saveFeedback = ""
    }

    Connections {
        target: DeviceController

        function onSettingsSaveCompleted(uploaded, storedLocally) {
            root.saveFeedback = !storedLocally
                ? "Unable to store settings locally"
                : uploaded
                    ? "Settings saved locally and uploaded to ESP32"
                    : "Saved locally; connect controller to upload"
            saveFeedbackTimer.restart()
        }
    }

    function focusFirstControl() {
        var firstRow = channelList.itemAtIndex(0)
        if (firstRow) {
            firstRow.forceActiveFocus()
        }
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_X) {
            DeviceController.ResetDefaults()
            event.accepted = true
        } else if (event.key === Qt.Key_Y) {
            DeviceController.SaveSettings()
            event.accepted = true
        } else if (event.key === Qt.Key_A ||
            event.key === Qt.Key_Back ||
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

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 52

            ActionButton {
                id: backButton
                text: "<"
                implicitWidth: 42
                KeyNavigation.right: saveAssignmentsButton
                KeyNavigation.down: channelList.itemAtIndex(0)
                onClicked: root.goBack()
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3

                Text {
                    text: "Channel setup"
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                }

                Text {
                    text: "Assign a role to each PCA9685 channel"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                }
            }

            ActionButton {
                id: saveAssignmentsButton
                text: "Save assignments"
                controllerKey: Qt.Key_Y
                primary: true
                KeyNavigation.left: backButton
                KeyNavigation.down: channelList.itemAtIndex(0)
                enabled: DeviceController.connected && DeviceController.settingsDirty
                onClicked: DeviceController.SaveSettings()
            }
        }

        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 24
                    spacing: 12

                    Text {
                        Layout.preferredWidth: 82
                        text: "CHANNEL"
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                    }

                    Text {
                        Layout.fillWidth: true
                        text: "ROLE"
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
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
                        id: channelList
                        anchors.fill: parent
                        anchors.margins: 6
                        clip: true
                        spacing: 6
                        model: 16
                        boundsBehavior: Flickable.StopAtBounds

                        ScrollBar.vertical: InsetVerticalScrollBar {}

                        delegate: Rectangle {
                            id: channelRow
                            required property int index

                            width: Math.max(0, channelList.width - 18)
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

                            function openRolePopup() {
                                roleBox.forceActiveFocus()
                                roleBox.popup.open()
                            }

                            onActiveFocusChanged: {
                                if (activeFocus) {
                                    channelList.positionViewAtIndex(
                                        index,
                                        ListView.Contain
                                    )
                                }
                            }

                            KeyNavigation.up: index > 0
                                ? channelList.itemAtIndex(index - 1)
                                : backButton
                            KeyNavigation.down: index < 15
                                ? channelList.itemAtIndex(index + 1)
                                : saveAssignmentsButton

                            Keys.onPressed: function(event) {
                                if (event.key === Qt.Key_Return ||
                                    event.key === Qt.Key_Enter ||
                                    event.key === Qt.Key_Space ||
                                    event.key === Qt.Key_Right ||
                                    event.key === Qt.Key_B) {
                                    channelRow.openRolePopup()
                                    event.accepted = true
                                }
                            }

                            TapHandler {
                                onTapped: channelRow.forceActiveFocus()
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 8
                                spacing: 12

                                Text {
                                    Layout.preferredWidth: 70
                                    text: "CH " + channelRow.index
                                    color: Theme.textPrimary
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 13
                                    font.weight: Font.DemiBold
                                }

                                ComboBox {
                                    id: roleBox
                                    Layout.fillWidth: true
                                    model: root.roles
                                    currentIndex: DeviceController.GetChannelRole(
                                        channelRow.index
                                    )
                                    activeFocusOnTab: false
                                    focus: false
                                    enabled: true

                                    onActivated: {
                                        DeviceController.SetChannelRole(
                                            channelRow.index,
                                            currentIndex
                                        )
                                        popup.close()
                                        channelRow.forceActiveFocus()
                                    }

                                    function selectRole(aRole) {
                                        var role = Math.max(
                                            0,
                                            Math.min(root.roles.length - 1, aRole)
                                        )
                                        currentIndex = role
                                        DeviceController.SetChannelRole(
                                            channelRow.index,
                                            role
                                        )
                                    }

                                    Keys.onLeftPressed: function(event) {
                                        selectRole(currentIndex - 1)
                                        event.accepted = true
                                    }

                                    Keys.onRightPressed: function(event) {
                                        selectRole(currentIndex + 1)
                                        event.accepted = true
                                    }

                                    Keys.onPressed: function(event) {
                                        if (event.key === Qt.Key_A ||
                                            event.key === Qt.Key_Back ||
                                            event.key === Qt.Key_Escape ||
                                            event.key === Qt.Key_Backspace) {
                                            popup.close()
                                            channelRow.forceActiveFocus()
                                            event.accepted = true
                                        } else if (event.key === Qt.Key_Return ||
                                                   event.key === Qt.Key_Enter ||
                                                   event.key === Qt.Key_Space ||
                                                   event.key === Qt.Key_B) {
                                            popup.open()
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
                                        text: "â–¾"
                                        color: Theme.textSecondary
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 12
                                    }

                                    delegate: ItemDelegate {
                                        id: roleDelegate
                                        width: roleBox.width
                                        height: 34
                                        property bool activeRole:
                                            rolePopup.pendingIndex === index
                                        highlighted: activeRole

                                        contentItem: Text {
                                            leftPadding: 10
                                            text: modelData
                                            color: roleDelegate.activeRole
                                                ? Theme.textPrimary
                                                : Theme.textSecondary
                                            font.family: Theme.fontFamily
                                            font.pixelSize: 11
                                            verticalAlignment: Text.AlignVCenter
                                        }

                                        background: Rectangle {
                                            radius: 7
                                            color: "transparent"
                                            border.width: 0
                                            border.color: Theme.accent

                                            Rectangle {
                                                anchors.left: parent.left
                                                anchors.top: parent.top
                                                anchors.bottom: parent.bottom
                                                width: 3
                                                radius: 2
                                                color: roleDelegate.activeRole
                                                    ? Theme.accent
                                                    : "transparent"
                                            }
                                        }
                                    }

                                    popup: Popup {
                                        id: rolePopup
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
                                              Qt.callLater(function() {
                                                  roleList.forceActiveFocus()
                                              })
                                         }
                                         onClosed: channelRow.forceActiveFocus()

                                         Keys.priority: Keys.BeforeItem
                                         Keys.onPressed: function(event) {
                                              var nativeKey = event.nativeVirtualKey !== undefined
                                                  ? event.nativeVirtualKey
                                                  : event.nativeScanCode
                                              if (event.key === Qt.Key_Up ||
                                                  event.key === Qt.Key_Left ||
                                                  nativeKey === 19 ||
                                                  nativeKey === 21) {
                                                 roleList.moveRole(-1)
                                                 event.accepted = true
                                              } else if (event.key === Qt.Key_Down ||
                                                         event.key === Qt.Key_Right ||
                                                         nativeKey === 20 ||
                                                         nativeKey === 22) {
                                                 roleList.moveRole(1)
                                                 event.accepted = true
                                             }
                                         }

                                        contentItem: ListView {
                                            id: roleList
                                            clip: true
                                            focus: true
                                            activeFocusOnTab: true
                                            implicitHeight: Math.min(
                                                contentHeight,
                                                9 * 34
                                            )
                                            model: roleBox.popup.visible
                                                ? roleBox.delegateModel
                                                : null
                                             currentIndex: 0
                                             highlightFollowsCurrentItem: true
                                             highlightMoveDuration: 0

                                             highlight: Rectangle {
                                                 width: Math.max(0, roleList.width - 18)
                                                 height: 34
                                                 radius: 7
                                                 color: Theme.accentMuted
                                                 border.width: 2
                                                 border.color: Theme.accent
                                                 z: -1

                                                 Rectangle {
                                                     anchors.left: parent.left
                                                     anchors.top: parent.top
                                                     anchors.bottom: parent.bottom
                                                     width: 3
                                                     radius: 2
                                                     color: Theme.accent
                                                 }
                                             }

                                             function moveRole(delta) {
                                                 var next = Math.max(
                                                     0,
                                                     Math.min(
                                                         root.roles.length - 1,
                                                         currentIndex + delta
                                                     )
                                                 )
                                                 currentIndex = next
                                                 rolePopup.pendingIndex = next
                                                 positionViewAtIndex(
                                                     next,
                                                     ListView.Contain
                                                 )
                                                  roleList.forceActiveFocus()
                                              }

                                              Keys.onPressed: function(event) {
                                                  var nativeKey = event.nativeVirtualKey !== undefined
                                                      ? event.nativeVirtualKey
                                                      : event.nativeScanCode
                                                  if (event.key === Qt.Key_Up ||
                                                      event.key === Qt.Key_Left ||
                                                      nativeKey === 19 ||
                                                      nativeKey === 21) {
                                                     roleList.moveRole(-1)
                                                     event.accepted = true
                                                  } else if (event.key === Qt.Key_Down ||
                                                             event.key === Qt.Key_Right ||
                                                             nativeKey === 20 ||
                                                             nativeKey === 22) {
                                                     roleList.moveRole(1)
                                                     event.accepted = true
                                                 } else if (event.key === Qt.Key_Return ||
                                                     event.key === Qt.Key_Enter ||
                                                     event.key === Qt.Key_Space ||
                                                     event.key === Qt.Key_B) {
                                                    roleBox.currentIndex =
                                                        roleList.currentIndex
                                                    DeviceController.SetChannelRole(
                                                        channelRow.index,
                                                        roleList.currentIndex
                                                    )
                                                    rolePopup.close()
                                                    channelRow.forceActiveFocus()
                                                    event.accepted = true
                                                } else if (event.key === Qt.Key_A ||
                                                           event.key === Qt.Key_Back ||
                                                           event.key === Qt.Key_Escape ||
                                                           event.key === Qt.Key_Backspace) {
                                                    rolePopup.close()
                                                    channelRow.forceActiveFocus()
                                                    event.accepted = true
                                                }
                                            }

                                            ScrollBar.vertical: InsetVerticalScrollBar {}
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

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 28

            Text {
                text: root.saveFeedback !== ""
                    ? root.saveFeedback
                    : DeviceController.settingsDirty
                        ? "Unsaved changes — press Y to save"
                        : DeviceController.settingsUploadPending
                            ? "Saved locally; waiting to upload"
                            : "Assignments are stored on the ESP32"
                color: root.saveFeedback !== ""
                    ? (root.saveFeedback === "Settings saved locally and uploaded to ESP32"
                        ? Theme.green
                        : Theme.warning)
                    : DeviceController.settingsDirty
                        ? Theme.warning
                        : Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: 10
            }

            Item { Layout.fillWidth: true }

            Text {
                text: "Use D-pad / arrow keys to move between channels"
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: 10
            }
        }
    }
}
