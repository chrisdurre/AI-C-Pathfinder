import QtQuick 2.4
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Item {
    TextField {
        id: fileNameField
    }
    ComboBox {
        id: algorithmCombo
        anchors.left: fileNameField.right
        anchors.top: fileNameField.top
        currentIndex: 0
        model: ListModel {
            id: algorithmModel
            ListElement { text: "Breadth-First"; }
            ListElement { text: "Depth-First"; }
            ListElement { text: "Greedy Best-First"; }
            ListElement { text: "Iterative Deepening"; }
        }
    }

    Button {
        id: searchButton
        anchors.top: fileNameField.top
        anchors.left: algorithmCombo.right
        text: "Search"
        onClicked: {
            maze.setFileName(fileNameField.text)
            maze.readMazeFile()
            maze.search(algorithmCombo.currentText)
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
