import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "components"
import "components/Theme.js" as Theme

Popup {
    id: root

    width: 720
    height: 388
    modal: true
    focus: true
    padding: 0
    closePolicy:
        Popup.CloseOnEscape |
        Popup.CloseOnPressOutside

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

    background: Card {
        radius: 16
    }

    contentItem: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 42

            Column {
                Layout.fillWidth: true
                spacing: 3

                Text {
                    text: "Live telemetry"
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 19
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

            ActionButton {
                id: closeButton
                text: "Close"
                onClicked: root.close()
            }
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 3
            columnSpacing: 12
            rowSpacing: 12

            TelemetryCard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "CH1 steering"
                primaryValue:
                    DeviceController.steeringPercent +
                    "%"

                details:
                    DeviceController.steeringPulseUs > 0
                        ? DeviceController.steeringPulseUs +
                            " µs"
                        : "No signal"

                accentColor: Theme.accent
                cardColor: Theme.surface
                secondaryColor: Theme.textSecondary
                fontName: Theme.fontFamily
            }

            TelemetryCard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "CH2 throttle"
                primaryValue:
                    DeviceController.throttlePercent +
                    "%"

                details:
                    DeviceController.throttlePulseUs > 0
                        ? DeviceController.throttlePulseUs +
                            " µs"
                        : "No signal"

                accentColor: Theme.green
                cardColor: Theme.surface
                secondaryColor: Theme.textSecondary
                fontName: Theme.fontFamily
            }

            TelemetryCard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "Lighting state"
                primaryValue:
                    DeviceController.exhaustPulseActive
                        ? "Exhaust pulse"
                        : DeviceController.brakeActive
                            ? "Brake"
                            : DeviceController.turnDirection

                details:
                    DeviceController.activeLightsEnabled
                        ? "Active lighting enabled"
                        : "Active lighting disabled"

                accentColor:
                    DeviceController.exhaustPulseActive
                        ? Theme.warning
                        : Theme.textPrimary
                cardColor: Theme.surface
                secondaryColor: Theme.textSecondary
                fontName: Theme.fontFamily
            }

            TelemetryCard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "Acceleration"
                primaryValue:
                    DeviceController.filteredForwardAccelerationG.toFixed(3) +
                    " g"

                details:
                    "FWD " +
                    DeviceController.forwardAccelerationG.toFixed(3) +
                    "  SIDE " +
                    DeviceController.sideAccelerationG.toFixed(3) +
                    "  VERT " +
                    DeviceController.verticalAccelerationG.toFixed(3)

                accentColor: Theme.green
                cardColor: Theme.surface
                secondaryColor: Theme.textSecondary
                fontName: Theme.fontFamily
            }

            TelemetryCard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "Gyroscope X / Y"
                primaryValue:
                    DeviceController.gyroscopeXDps.toFixed(1) +
                    " / " +
                    DeviceController.gyroscopeYDps.toFixed(1)

                details: "degrees per second"
                accentColor: Theme.accent
                cardColor: Theme.surface
                secondaryColor: Theme.textSecondary
                fontName: Theme.fontFamily
            }

            TelemetryCard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "Gyroscope Z"
                primaryValue:
                    DeviceController.gyroscopeZDps.toFixed(1) +
                    " dps"

                details:
                    "Firmware " +
                    DeviceController.firmwareVersion

                accentColor: Theme.accent
                cardColor: Theme.surface
                secondaryColor: Theme.textSecondary
                fontName: Theme.fontFamily
            }
        }
    }

    component TelemetryCard: Rectangle {
        id: telemetryCard

        property string title: ""
        property string primaryValue: ""
        property string details: ""
        property color accentColor: "#f4f6f8"
        property color cardColor: "#20242b"
        property color secondaryColor: "#8e96a3"
        property string fontName: "Inter"

        color: telemetryCard.cardColor
        radius: 12

        Column {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 7

            Text {
                text: telemetryCard.title
                color: telemetryCard.secondaryColor
                font.family: telemetryCard.fontName
                font.pixelSize: 11
            }

            Text {
                text: telemetryCard.primaryValue
                color: telemetryCard.accentColor
                font.family: telemetryCard.fontName
                font.pixelSize: 20
                font.weight: Font.DemiBold
            }

            Text {
                width: parent.width
                text: telemetryCard.details
                color: telemetryCard.secondaryColor
                font.family: telemetryCard.fontName
                font.pixelSize: 9
                elide: Text.ElideRight
            }
        }
    }
}
