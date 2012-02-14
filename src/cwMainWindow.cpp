//Glew includes
#include "GL/glew.h"

//Our includes
#include "cwMainWindow.h"
#include "cwQMLRegister.h"
#include "cwProject.h"
#include "cwRegionTreeModel.h"
#include "cwLinePlotManager.h"
#include "cwImportSurvexDialog.h"
#include "cwProjectImageProvider.h"
#include "cwScrapManager.h"
#include "cwGlobalDirectory.h"
#include "cwRootData.h"
#include "cwSurveyExportManager.h"

//Qt includes
#include <QDeclarativeContext>
#include <QDeclarativeComponent>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include <QFileDialog>
#include <QDebug>
#include <QSettings>
#include <QTreeView>
#include <QMessageBox>
#include <QThread>
#include <QGLWidget>
#include <QTemporaryFile>
#include <QDesktopWidget>

//---------------- For boost testing should be removed -------------
//Std includes
#include <iostream>
#include <fstream>

//Boost includes
#include "cwSerialization.h"
#include "cwQtSerialization.h"

#if defined(QMLJSDEBUGGER)
#include <qt_private/qdeclarativedebughelper_p.h>
#endif

#if defined(QMLJSDEBUGGER) && !defined(NO_JSDEBUGGER)
#include <jsdebuggeragent.h>
#endif
#if defined(QMLJSDEBUGGER) && !defined(NO_QMLOBSERVER)
#include <qdeclarativeviewobserver.h>
#endif

#if defined(QMLJSDEBUGGER)

// Enable debugging before any QDeclarativeEngine is created
struct QmlJsDebuggingEnabler
{
    QmlJsDebuggingEnabler()
    {
        QDeclarativeDebugHelper::enableDebugging();
    }
};

// Execute code in constructor before first QDeclarativeEngine is instantiated
static QmlJsDebuggingEnabler enableDebuggingHelper;

#endif // QMLJSDEBUGGER

cwMainWindow::cwMainWindow(QWidget *parent) :
    QMainWindow(parent),
    SurvexExporter(NULL),
    Data(new cwRootData(this))
{
    setupUi(this);

#if defined(QMLJcwSurveyEditorMainWindowSDEBUGGER) && !defined(NO_JSDEBUGGER)
    new QmlJSDebugger::JSDebuggerAgent(DeclarativeView->engine());
#endif
#if defined(QMLJSDEBUGGER) && !defined(NO_QMLOBSERVER)
    new QmlJSDebugger::QDeclarativeViewObserver(DeclarativeView, this);
#endif

    //Setup the export menus
    setupExportMenus();

    //Setup undo redo
    UndoStack = new QUndoStack(this);

    connect(UndoStack, SIGNAL(canUndoChanged(bool)), ActionUndo, SLOT(setEnabled(bool)));
    connect(UndoStack, SIGNAL(canRedoChanged(bool)), ActionRedo, SLOT(setEnabled(bool)));
    connect(UndoStack, SIGNAL(undoTextChanged(QString)), SLOT(updateUndoText(QString)));
    connect(UndoStack, SIGNAL(redoTextChanged(QString)), SLOT(updateRedoText(QString)));
    connect(ActionUndo, SIGNAL(triggered()), UndoStack, SLOT(undo()));
    connect(ActionRedo, SIGNAL(triggered()), UndoStack, SLOT(redo()));

    connect(actionSave, SIGNAL(triggered()), SLOT(save()));
    connect(actionLoad, SIGNAL(triggered()), SLOT(load()));
    connect(actionSurvexImport, SIGNAL(triggered()), SLOT(importSurvex()));
    connect(actionReloadQML, SIGNAL(triggered()), SLOT(reloadQML()));

    Data->region()->setUndoStack(UndoStack);

    connect(actionCompute_Scraps, SIGNAL(triggered()), Data->scrapManager(), SLOT(updateAllScraps()));

    reloadQML();

//    Data->project()->load("bcc.cw");
  //  Project->load("/Users/philipschuchardt/test.cw");

    //Positions and resize the main window
    initialWindowShape();
}

void cwMainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi(this);
        break;
    default:
        break;
    }
}

/**
  \brief Opens the suvrex import dialog
  */
void cwMainWindow::importSurvex() {
    cwImportSurvexDialog* survexImportDialog = new cwImportSurvexDialog(Data->region(), this);
    survexImportDialog->setUndoStack(UndoStack);
    survexImportDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    survexImportDialog->open();
}

void cwMainWindow::reloadQML() {

    delete DeclarativeView;
   // verticalLayout->removeWidget(DeclarativeView);
    DeclarativeView = new QDeclarativeView();
    verticalLayout->addWidget(DeclarativeView);

    QGLWidget* glWidget = createGLWidget();
    DeclarativeView->setViewport(glWidget);
    DeclarativeView->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    DeclarativeView->setRenderHint(QPainter::SmoothPixmapTransform, true);
    DeclarativeView->setRenderHint(QPainter::Antialiasing, true);

    QDeclarativeContext* context = DeclarativeView->rootContext();
    context->setParent(this);

    //Register all the qml types
    cwQMLRegister::registerQML();

    //Update the data class
    Data->setGLWidget(glWidget);
    context->setContextObject(Data);

    //This allow to extra image data from the project's image database
    cwProjectImageProvider* imageProvider = new cwProjectImageProvider();
    imageProvider->setProjectPath(Data->project()->filename());
    connect(Data->project(), SIGNAL(filenameChanged(QString)), imageProvider, SLOT(setProjectPath(QString)));
    context->engine()->addImageProvider(cwProjectImageProvider::Name, imageProvider);

    DeclarativeView->setSource(QUrl::fromLocalFile(cwGlobalDirectory::baseDirectory() + "qml/CavewhereMainWindow.qml"));

    //Allow for the DoubleClickTextInput to work correctly
    context->setContextProperty("rootObject", DeclarativeView->rootObject());
}

void cwMainWindow::updateUndoText(QString undoText) {
    ActionUndo->setText(QString("Undo %1").arg(undoText));
}

void cwMainWindow::updateRedoText(QString redoText) {
    ActionRedo->setText(QString("Redo %1").arg(redoText));
}

/**
  \brief Saves the project
  */
void cwMainWindow::save() {
    Data->project()->save();
}

/**
  \brief Ask the user to load a cw project file
  */
void cwMainWindow::load() {
    QFileDialog* loadDialog = new QFileDialog(NULL, "Load Cavewhere Project", "", "Cavewhere Project (*.cw)");
    loadDialog->setFileMode(QFileDialog::ExistingFile);
    loadDialog->setAcceptMode(QFileDialog::AcceptOpen);
    loadDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    loadDialog->open(Data->project(), SLOT(load(QString)));
}

/**
  This init's glew so opengl extentions work!
  */
void cwMainWindow::initGLEW() {
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        QString informationText = QString("Error: %1").arg((const char*)glewGetErrorString(err));

        QMessageBox* box = new QMessageBox();
        box->setText("Problem: glewInit failed, something is seriously wrong.");
        box->setInformativeText(informationText);
        box->setIcon(QMessageBox::Critical);
        box->show();
        connect(box, SIGNAL(accepted()), QApplication::instance(), SLOT(quit()));
    }
}

/**
  \brief This creates the gl widget that the main declarative view paints to
  */
QGLWidget* cwMainWindow::createGLWidget() {
    QGLFormat format;
    format.setSampleBuffers(true);
    format.setSwapInterval(1);

    QGLWidget* glWidget = new QGLWidget(format, this);
    glWidget->makeCurrent();
    initGLEW();

    return glWidget;
}

/**
  This positions the main window in the center of the screen. The size
  of the window will be half the size of the screen
  */
void cwMainWindow::initialWindowShape() {
    QRect screenGeometry;
    if(QApplication::desktop()->screenCount() == 2) {
        int primaryScreenId = QApplication::desktop()->primaryScreen();
        int secondaryScreenId = primaryScreenId == 0 ? 1 : 0;

        QRect primaryScreen = QApplication::desktop()->screenGeometry(primaryScreenId);
        QRect secondarySceen = QApplication::desktop()->screenGeometry(secondaryScreenId);

        secondarySceen.moveLeft(primaryScreen.width());

        screenGeometry = secondarySceen;
    } else {
        //Use the screen geometry of the primary screen
        screenGeometry = QApplication::desktop()->availableGeometry();
    }

    //double size = 0.8;
    double size = 0.8;
    double position = (1.0 - size) / 2.0;

    int x = screenGeometry.x() + qRound(screenGeometry.width() * position);
    int y = screenGeometry.y() + qRound(screenGeometry.height() * position);
    int width = qRound(screenGeometry.width() * size);
    int height = qRound(screenGeometry.height() * size);

    setGeometry(x, y, width, height);
}

/**
  Sets up the export menus in the main window
  */
void cwMainWindow::setupExportMenus() {
    foreach(QMenu* menu, Data->surveyExportManager()->menus()) {
        menuExport->addMenu(menu);
    }
}
