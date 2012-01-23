// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import Cavewhere 1.0
import "Utils.js" as Utils

CheckableGroupBox {

    property Calibration calibration

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.margins: 3
    contentHeight: frontSightContent.height
    text: checked ? "<b>Front Sights</b>" : "Front Sights"

    Column {
        id: frontSightContent
        anchors.left: parent.left
        anchors.right: parent.right

        Row {
            spacing: 15

            Row {
                spacing: 3

                LabelWithHelp {
                    id: compassCalibrationLabel
                    text: "Compass calibration"
                    helpArea: clinoCalibarationHelpArea
                }

                ClickTextInput {
                    id: clinoCalInput
                    text: Utils.fixed(calibration.frontCompassCalibration, 2)

                    onFinishedEditting: {
                        calibration.frontCompassCalibration = newText
                    }
                }
            }

            Row {
                spacing: 3

                LabelWithHelp {
                    id: clinoCalibrationLabel
                    text: "Clino calibration"
                    helpArea: compassCalibarationHelpArea
                }

                ClickTextInput {
                    id: compassCalInput
                    text: Utils.fixed(calibration.frontClinoCalibration, 2)

                    onFinishedEditting: {
                        calibration.frontClinoCalibration = newText
                    }
                }
            }
        }

        HelpArea {
            id: compassCalibarationHelpArea
            anchors.left: parent.left
            anchors.right: parent.right
            text: "Help text for the compass calibration"
        }

        HelpArea {
            id: clinoCalibarationHelpArea
            anchors.left: parent.left
            anchors.right: parent.right
            text: "Help text for the clino calibration"
        }
    }
}
