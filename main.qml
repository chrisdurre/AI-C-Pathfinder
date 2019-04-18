import QtQuick 2.9
import QtQuick.Controls 2.2

ApplicationWindow {
    id: window
    visible: true
    width: 640
    height: 480
    title: qsTr("Stack")

    StackView {
        id: stackView
        initialItem: "Maze.qml"
        anchors.fill: parent
    }
    //Sets connections for use in C++, maze.cpp
    Connections {
        target: maze
    }
}
