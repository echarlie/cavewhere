/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

import QtQuick 2.0
import Cavewhere 1.0
import QtQuick.Controls 1.0 as Controls
import QtQuick.Layouts 1.1
import "Utils.js" as Utils

Rectangle {
    id: area

    property alias currentTrip: surveyEditor.currentTrip
    property string viewMode: ""

    function registerSubPages() {
        var oldCarpetPage = PageView.page.childPage("Carpet")
        if(oldCarpetPage !== rootData.pageSelectionModel.currentPage) {
            if(oldCarpetPage !== null) {
                rootData.pageSelectionModel.unregisterPage(oldCarpetPage)
            }

            if(PageView.page.name !== "Carpet") {
                var page = rootData.pageSelectionModel.registerSubPage(area.PageView.page,
                                                                       "Carpet",
                                                                       {"viewMode":"CARPET"});
            }
        }

    }

    /**
      Initilizing properties, if they aren't explicitly specified in the page
      */
    PageView.defaultProperties: {
        "currentTrip":null,
                "viewMode":""
    }

    onViewModeChanged: {
        if(viewMode == "CARPET") {
            notesGallery.setMode("CARPET")
            state = "COLLAPSE" //Hide the survey data on the left side
        } else {
            notesGallery.setMode("DEFAULT")
            state = "" //Show the survey data on the left side
            surveyEditor.visible = true
            notesGallery.visible = true
        }
    }

    SurveyEditor {
        id: surveyEditor
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        visible: true
    }

    Rectangle {
        id: collapseRectangleId
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        border.width: 1

        width: expandButton.width + 6
        visible: false

        ColumnLayout {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 3

            Button {
                id: expandButton
                iconSource: "qrc:/icons/moreArrow.png"
                onClicked: {
                    area.state = ""
                }
            }

            Item {

                implicitWidth: tripNameVerticalText.implicitHeight
                implicitHeight: tripNameVerticalText.implicitWidth

                Text {
                    id: tripNameVerticalText
                    rotation: 270
                    text: currentTrip.name
                    x: -5
                    y: 10
                }
            }
        }

    }

    NotesGallery {
        id: notesGallery
        notesModel: currentTrip.notes
        anchors.left: surveyEditor.right
        anchors.right: parent.right
        anchors.top: area.top
        anchors.bottom: area.bottom
        clip: true

        onImagesAdded: {
            currentTrip.notes.addFromFiles(images, rootData.project)
        }

        onBackClicked: {
            rootData.pageSelectionModel.back()
        }

        onModeChanged: {
            if(mode === "CARPET" && area.viewMode === "") {
                rootData.pageSelectionModel.gotoPageByName(area.PageView.page, "Carpet")
            }
        }
    }

    PageView.onPageChanged: registerSubPages()

//    Component.onCompleted: {
//        registerSubPages()
//    }

    states: [
        State {
            name: "COLLAPSE"

            PropertyChanges {
                target: collapseRectangleId
                visible: true
            }

            AnchorChanges {
                target: notesGallery
                anchors.left: collapseRectangleId.right
            }

            PropertyChanges {
                target: surveyEditor
                visible: false
            }
        }
    ]

    transitions: [
        Transition {
            from: ""
            to: "COLLAPSE"

            PropertyAction { target: surveyEditor; property: "visible"; value: true }
            PropertyAction { target: collapseRectangleId; property: "visible"; value: false }


            PropertyAction {
                target: surveyEditor; property: "clip"; value: true
            }

            ParallelAnimation {
                AnchorAnimation {
                    targets: [ notesGallery ]
                }

                NumberAnimation {
                    target: surveyEditor
                    property: "width"
                    to: 0
                }
            }

            PropertyAction {
                target: surveyEditor; property: "clip"; value: false
            }

            PropertyAction { target: collapseRectangleId; property: "visible"; value: true }
        },

        Transition {
            from: "COLLAPSE"
            to: ""


            PropertyAction { target: notesGallery; property: "anchors.left"; value: surveyEditor.right}

            ParallelAnimation {
                AnchorAnimation {
                    targets: [ notesGallery ]
                }

                NumberAnimation {
                    target: surveyEditor
                    property: "width"
                    to: surveyEditor.contentWidth
                }
            }
        }
    ]
}