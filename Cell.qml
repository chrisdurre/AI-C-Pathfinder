import QtQuick 2.0

Rectangle {
    id: cell
    height: cellSize
    width: cellSize
    property string cellText
    property string cellColor
    property int x_position
    property int y_position
    border.width: 1
    color: cellColor
    Text {
        text: cellText
        anchors.centerIn: parent
    }
}
