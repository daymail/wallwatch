import QtQuick
import WallWatchModule // This matches the URI in your CMake

Window {
    visible: true
    width: 400
    height: 300
    color: Theme.background // Testing your Themer integration!

    Text {
        anchors.centerIn: parent
        text: "WallWatch Active"
        color: Theme.onBackground
    }
}
