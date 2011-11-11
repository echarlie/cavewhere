import QtQuick 1.0
import Cavewhere 1.0

Item {

    property NoteTransform noteTransform

    signal northUpInteractionActivated()

    height: childrenRect.height

    Row {
        id: row
        spacing: 5

        anchors.left: parent.left
        anchors.right: parent.right

        Button {
            id: setNorthButton

            width: 24

            onClicked: {
                interactionManager.active(northInteraction)
            }
        }

        LabelWithHelp {
            id: labelId
            helpArea: helpAreaId
            text: "North"
            anchors.verticalCenter: parent.verticalCenter
        }

        ClickTextInput {
            id: clickInput
            color: "blue"
            text: noteTransform.northUp.toFixed(2)
            onFinishedEditting: noteTransform.northUp = newText
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            id: unit
            textFormat: Text.RichText
            text: "&deg"
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    HelpArea {
        id: helpAreaId
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: row.bottom
        anchors.topMargin: 4
    }
}
