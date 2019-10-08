import QtQuick 2.0
import QtQuick.Layouts 1.1
import Cavewhere 1.0 as Cavewhere
import QtQuick.Controls 2.0

ColumnLayout {

    property TurnTableInteraction turnTableInteraction

    implicitWidth: gridId.width
//    height: 200

    GridLayout {
        id: gridId
        rows: 3
        columns: 3

        InformationButton {
            showItemOnClick: directionHelpAreaId
        }

        Button {
            id: northButton
            text: "North"
            onClicked: {
                azimuthAnimationId.restartRotation(0.0)
            }
            enabled: turnTableInteraction.azimuth !== 0.0
        }

        LockButton {
            Layout.alignment: Qt.AlignRight
            down: turnTableInteraction.azimuthLocked
            onClicked: {
                turnTableInteraction.azimuthLocked = !turnTableInteraction.azimuthLocked
            }
        }

        Button {
            id: westButton
            text: "West"
            width: northButton.width
            onClicked: {
                azimuthAnimationId.restartRotation(270.0)
            }
            enabled: turnTableInteraction.azimuth !== 270.0
        }

        RowLayout {

            Layout.alignment: Qt.AlignCenter
            width: northButton.width

            ClickTextInput {
                text: Number(turnTableInteraction.azimuth).toFixed(1)
                validator: doubleValidatorId
                onFinishedEditting: {
                    azimuthAnimationId.to = newText
                    azimuthAnimationId.restart()
                }
            }

            Cavewhere.CompassValidator {
                id: doubleValidatorId
            }

            Text {
                text: "°"
            }
        }

        Button {
            id: eastButton
            text: "East"
            width: northButton.width
            onClicked: {
                azimuthAnimationId.restartRotation(90.0)
            }
            enabled: turnTableInteraction.azimuth !== 90.0
        }

        Item { width:1; height:1 }
        Button {
            id: southButton
            text: "South"
            width: northButton.width
            onClicked: {
                azimuthAnimationId.restartRotation(180.0)
            }
            enabled: turnTableInteraction.azimuth !== 180.0
        }
        Item { width:1; height:1 }
    }

    GroupBox {
        title: "Animate"

        RowLayout {

            Button {
                id: animateAzimuthButton
                text: checked ? "Stop" : "Start"
                checkable: true
                onCheckedChanged: {
                    if(checked) {
                        fullRotationAnimation.startCurrentPosition()
                    } else {
                        fullRotationAnimation.stop()
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                text: "Duration"
            }

            TextField {
                id: durationTextField
                implicitWidth: 200
                text: fullRotationAnimation.duration
                onEditingFinished: {
                    var animationRunning = fullRotationAnimation.running

                    fullRotationAnimation.duration = Number(text)
                    fullRotationAnimation.stop();
                    if(animationRunning) {
                        fullRotationAnimation.startCurrentPosition()
                    }
                }

                validator: IntValidator {
                    bottom: 0
                }
            }

            Label {
                text: "ms"
            }
        }
    }

    NumberAnimation {
        id: fullRotationAnimation
        target: turnTableInteraction
        property: "azimuth"
        duration: 10000
        loops: Animation.Infinite

        function startCurrentPosition() {
            fullRotationAnimation.from = turnTableInteraction.azimuth
            fullRotationAnimation.to = turnTableInteraction.azimuth + 359.999
            fullRotationAnimation.start()
        }
    }

    NumberAnimation {
        id: azimuthAnimationId
        target: turnTableInteraction;
        property: "azimuth";
        duration: 200;
        easing.type: Easing.InOutQuad

        function restartRotation(toRotation) {
            var from = turnTableInteraction.azimuth;
            var to = toRotation
            if(to > 180 && from <= 0.0) {
                from = 360
            } else if(to <= 0.0 && from > 180 ) {
                to = 360
            }

            azimuthAnimationId.from = from
            azimuthAnimationId.to = to

            azimuthAnimationId.restart()
        }
    }

    HelpArea {
        id: directionHelpAreaId
        text: "The views azimuth (in degrees between 0.0 and 360.0) is
             the compass direction that the view is facing.
            <ul><li>0.0° for North<li>90.0° for East<li>180.0° for South<li>270.0° for West</ul>";
    }
}
