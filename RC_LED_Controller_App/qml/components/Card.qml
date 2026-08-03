import QtQuick
import "Theme.js" as Theme

Rectangle {
    property bool highlighted: false

    color: highlighted ? Theme.surface : Theme.panel
    radius: 16
    border.width: highlighted ? 2 : 1
    border.color: highlighted ? Theme.accent : Theme.border
    clip: true
}
