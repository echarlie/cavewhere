import QtQuick 1.0

Rectangle {
    id: dataSideBar
   // anchors.fill: parent



    CompactTabWidget {
        id: tabWidget
        anchors.fill: parent;

        children: [
            Image {
                id: splitter
                fillMode: Image.TileVertically
                source: "icons/verticalLine.png"
               // anchors.bottom: parent.bottom
                height: tabWidget.areaY + 4
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.rightMargin: 1
                z: 1
            }
        ]

        CaveDataSidebarPage {
            property string label: "Caves"
            property string icon: "icons/cave-64x64.png"
            anchors.margins: 4
        }

        Text {
            property string label: "Surface"
            property string icon: "icons/surface.png"
            text: "This is the Surface page"
        }


    }





}
