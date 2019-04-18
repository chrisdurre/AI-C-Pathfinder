import QtQuick 2.4
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Item {
    TextField {
        id: fileNameField
    }
    Button {
        id: searchButton
        anchors.top: fileNameField.top
        anchors.left: fileNameField.right
        text: "Search"
        onClicked: {
            maze.readMazeFile(fileNameField.text)
            maze.search()
        }
    }
    Grid {
        id: mazeGrid
        anchors.top: searchButton.bottom
        anchors.left: fileNameField.left
        columns: maze.nCol
        rows: maze.nRow
        transformOrigin: Item.TopLeft
        Repeater {
            id: gridRepeater
            model: maze.nCol * maze.nRow
            Cell {
                width: 50
                height: 50
            }
            onItemAdded: {
                //set the x and y position for the cells
                itemAt(index).y_position = Math.floor(index/maze.nCol)
                itemAt(index).x_position = index % maze.nCol
                itemAt(index).cellColor = maze.getColor(index % maze.nCol, Math.floor(index/maze.nCol))
                itemAt(index).cellText = index
            }
            function update() {
                for(var i = 0; i < gridRepeater.count; i++) {
                    itemAt(i).cellColor = maze.getColor(i % maze.nCol, Math.floor(i/maze.nCol))
                }
            }
        }
    }
    Connections {
        target: maze
        onNodesUpdated: gridRepeater.update()
    }
}
