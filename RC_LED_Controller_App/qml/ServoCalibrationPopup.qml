import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "components"
import "components/Theme.js" as Theme

Popup {
    id: root

    width: 620
    height: 388
    modal: true
    focus: true
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    onOpened: closeButton.forceActiveFocus()

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Back ||
            event.key === Qt.Key_Escape ||
            event.key === Qt.Key_Backspace) {
            root.close()
            event.accepted = true
        }
    }

    Overlay.modal: Rectangle {
        color: "#99000000"
    }

    background: Rectangle {
        color: Theme.panel
        radius: 16
        border.width: 1
        border.color: Theme.border
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 42

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3

                Text {
                    text: "Servo calibration"
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 19
                    font.weight: Font.DemiBold
                }

                Text {
                    text: "Set the closed and open positions for the pop-up headlights"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                }
            }

            ActionButton {
                id: closeButton
                text: "Close"
                onClicked: root.close()
            }
        }

        CalibrationRow {
            Layout.fillWidth: true
            title: "Closed position"
            subtitle: "Position used when headlights are closed"
            value: DeviceController.servoClosedPulseUs
            onValueMoved: DeviceController.SetPendingServoClosedPulseUs(value)
        }

        CalibrationRow {
            Layout.fillWidth: true
            title: "Open position"
            subtitle: "Position used when headlights are open"
            value: DeviceController.servoOpenPulseUs
            onValueMoved: DeviceController.SetPendingServoOpenPulseUs(value)
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 42

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 3

                Text {
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
            }

            ActionButton {
                text: "Zero-out servo"
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
            text: "Pulse values are PCA9685 counts at 50 Hz (0–4095). Save settings on the Settings screen to store them on the ESP32."
            color: Theme.textSecondary
            font.family: Theme.fontFamily
            font.pixelSize: 10
            wrapMode: Text.WordWrap
        }
    }

    component CalibrationRow: Rectangle {
        id: row

        property string title: ""
        property string subtitle: ""
        property int value: 0
        signal valueMoved(int value)

        Layout.preferredHeight: 87
        color: Theme.surface
        radius: 12

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

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Text {
                        text: row.title
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 14
                        font.weight: Font.Medium
                    }

                    Text {
                        text: row.subtitle
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                    }
                }

                Rectangle {
                    implicitWidth: 76
                    Layout.preferredWidth: 76
                    Layout.preferredHeight: 27
                    radius: 10
                    color: Theme.background

                    Text {
                        anchors.centerIn: parent
                        text: row.value
                        color: Theme.accent
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }
                }
            }

            ValueSlider {
                Layout.fillWidth: true
                from: 0
                to: 4095
                stepSize: 1
                value: row.value
                onMoved: row.valueMoved(value)
            }
        }
    }
}
