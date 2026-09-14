import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root

    width: 960
    height: 540
    minimumWidth: 960
    minimumHeight: 540

    visible: true
    visibility: Qt.platform.os === "android"
        ? Window.FullScreen
        : Window.Windowed

    title: "RC LED Controller"
    // Keep the Figma canvas dark even if a platform's QML singleton cache is
    // stale during an incremental Android deployment.
    color: "#0d0f12"

    background: Rectangle {
        color: "#0d0f12"
    }

    property int currentPage: 0

    // Some Android gamepad drivers deliver D-pad events to Qt without
    // running the attached KeyNavigation handler. Let the focused item use
    // its declared navigation target as a fallback after the item's own key
    // handlers have had a chance to process the event.
    function routeDirectionalKey(event) {
        var target = root.activeFocusItem
        if (!target) {
            return false
        }

        var next = null
        if (event.key === Qt.Key_Up) {
            next = target.KeyNavigation.up
        } else if (event.key === Qt.Key_Down) {
            next = target.KeyNavigation.down
        } else if (event.key === Qt.Key_Left) {
            next = target.KeyNavigation.left
        } else if (event.key === Qt.Key_Right) {
            next = target.KeyNavigation.right
        }

        if (next && next.visible && next.enabled !== false) {
            next.forceActiveFocus()
            return true
        }

        return false
    }

    Keys.priority: Keys.AfterItem
    Keys.onPressed: function(event) {
        if (root.routeDirectionalKey(event)) {
            event.accepted = true
        } else if (event.key === Qt.Key_A && root.currentPage > 0) {
            root.currentPage = root.currentPage === 2 ? 1 : 0
            event.accepted = true
        }
    }

    function focusCurrentPage() {
        Qt.callLater(function() {
            if (root.currentPage === 0) {
                mainPage.focusFirstControl()
            } else if (root.currentPage === 1) {
                settingsPage.focusFirstControl()
            } else {
                channelPage.focusFirstControl()
            }
        })
    }

    onCurrentPageChanged: focusCurrentPage()

    StackLayout {
        anchors.fill: parent
        currentIndex: root.currentPage

        MainControllerPage {
            id: mainPage
            onOpenSettings: root.currentPage = 1
            onOpenTelemetry: telemetryPopup.open()
        }

        SettingsPage {
            id: settingsPage
            onGoBack: root.currentPage = 0
            onOpenTelemetry: telemetryPopup.open()
            onOpenAssignments: root.currentPage = 2
            onOpenServoCalibration: servoCalibrationPopup.open()
        }

        ChannelAssignmentPage {
            id: channelPage
            onGoBack: root.currentPage = 1
        }
    }

    TelemetryPopup {
        id: telemetryPopup
        anchors.centerIn: Overlay.overlay
    }

    ServoCalibrationPopup {
        id: servoCalibrationPopup
        anchors.centerIn: Overlay.overlay
    }

    Component.onCompleted: {
        DeviceController.StartScan()
        focusCurrentPage()
    }
}
