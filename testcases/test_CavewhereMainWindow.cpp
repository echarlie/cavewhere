//Catch includes
#include <catch.hpp>

//Our includes
#include "cwGlobalDirectory.h"
#include "cwQMLRegister.h"
#include "cwRootData.h"
#include "cwQMLReload.h"
#include "TestHelper.h"
#include "cwImageProvider.h"
#include "cwSurveyNoteModel.h"
#include "cwTaskManagerModel.h"
#include "cwFutureManagerModel.h"
#include "cwCavingRegion.h"
#include "cwCave.h"
#include "cwTrip.h"
#include "cwScrap.h"

//Qt includes
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <QDebug>
#include <QSettings>


class MainHelper {
public:
    static QQmlApplicationEngine* createApplicationEnigne() {
        QQmlApplicationEngine* applicationEnigine = new QQmlApplicationEngine();
        cwRootData* rootData = new cwRootData(applicationEnigine);

        rootData->qmlReloader()->setApplicationEngine(applicationEnigine);

        QQmlContext* context =  applicationEnigine->rootContext();

        context->setContextObject(rootData);
        context->setContextProperty("rootData", rootData);

        applicationEnigine->load(cwGlobalDirectory::mainWindowSourcePath());
        return applicationEnigine;
    }

    static QObject* findMainWidow(QQmlApplicationEngine* engine) {
        REQUIRE(!engine->rootObjects().isEmpty());
        auto mainWindow = engine->rootObjects().first();
        REQUIRE(mainWindow->objectName().toStdString() == "applicationWindow");
        return mainWindow;
    }
};


TEST_CASE("Test that the cavewhere main window remember size and position", "[CavewhereMainWindow]") {

    cwGlobalDirectory::setupBaseDirectory();

    //Register all of the cavewhere types
    cwQMLRegister::registerQML();

    {
        QSettings settings;
        settings.clear();
    }

    auto firstAppEngine = MainHelper::createApplicationEnigne();

    QEventLoop loop;
    QTimer::singleShot(2000, [firstAppEngine, &loop]() {
        REQUIRE(!firstAppEngine->rootObjects().isEmpty());
        auto mainWindow = MainHelper::findMainWidow(firstAppEngine);
        REQUIRE(mainWindow->objectName().toStdString() == "applicationWindow");

        mainWindow->setProperty("x", 75);
        mainWindow->setProperty("y", 50);

        CHECK(mainWindow->property("width").toInt() == 1024);
        CHECK(mainWindow->property("height").toInt() == 576);
        CHECK(mainWindow->property("x").toInt() == 75);
        CHECK(mainWindow->property("y").toInt() == 50);

        mainWindow->setProperty("width", 250);
        mainWindow->setProperty("height", 200);
        mainWindow->setProperty("x", 323);
        mainWindow->setProperty("y", 386);

        QTimer::singleShot(1000, [&loop, firstAppEngine]() {

            delete firstAppEngine;

            auto secondEngine = MainHelper::createApplicationEnigne();

            QTimer::singleShot(2000, [&loop, secondEngine]() {
                auto mainWindow2 = MainHelper::findMainWidow(secondEngine);

                CHECK(mainWindow2->property("width").toInt() == 250);
                CHECK(mainWindow2->property("height").toInt() == 200);
                CHECK(mainWindow2->property("x").toInt() == 323);
                CHECK(mainWindow2->property("y").toInt() == 386);

                delete secondEngine;

                loop.quit();
            });
        });
    });

    loop.exec();

    {
        QSettings settings;
        settings.clear();
    }
}

//This testcase is mostly here for checking memory leaks and close crashing
TEST_CASE("Main window should load file and close the window", "[CavewhereMainWindow]") {
    cwGlobalDirectory::setupBaseDirectory();

    //Register all of the cavewhere types
    cwQMLRegister::registerQML();

    auto firstAppEngine = MainHelper::createApplicationEnigne();
    cwRootData* rootData = qobject_cast<cwRootData*>(firstAppEngine->rootContext()->contextProperty("rootData").value<QObject*>());
    REQUIRE(rootData);

    auto filename = copyToTempFolder("://datasets/scrapGuessNeighbor/scrapGuessNeigborPlanContinuous.cw");


    QEventLoop loop;
    QTimer::singleShot(2000, [rootData, filename, firstAppEngine, &loop]() {
        rootData->project()->loadFile(filename);

        QTimer::singleShot(2000, [firstAppEngine, &loop](){
            delete firstAppEngine;
            loop.quit();
        });
    });

    loop.exec();
    CHECK(true);
}

TEST_CASE("Load project with no images for scraps", "[CavewhereMainWindow]") {
    cwGlobalDirectory::setupBaseDirectory();

    //Register all of the cavewhere types
    cwQMLRegister::registerQML();

    auto firstAppEngine = MainHelper::createApplicationEnigne();
    cwRootData* rootData = qobject_cast<cwRootData*>(firstAppEngine->rootContext()->contextProperty("rootData").value<QObject*>());
    REQUIRE(rootData);

    auto filename = copyToTempFolder("://datasets/test_cwProject/Phake Cave 3000.cw");


    QEventLoop loop;
    QTimer::singleShot(2000, [rootData, filename, &loop]() {

        auto project = rootData->project();
        fileToProject(project, "://datasets/test_cwProject/Phake Cave 3000.cw");

        rootData->taskManagerModel()->waitForTasks();
        rootData->futureManagerModel()->waitForFinished();

        REQUIRE(project->cavingRegion()->caveCount() == 1);
        auto cave = project->cavingRegion()->cave(0);

        REQUIRE(cave->tripCount() == 1);
        auto trip = cave->trip(0);

        REQUIRE(trip->notes()->notes().size() == 1);
        auto note = trip->notes()->notes().at(0);

        cwImageProvider imageProvider;
        imageProvider.setProjectPath(project->filename());

        CHECK(note->scraps().size() == 2);

        for(auto scrap : note->scraps()) {
            auto triangleData = scrap->triangulationData();
            CHECK(triangleData.croppedImage().isValid());

            QList<int> ids = {
                triangleData.croppedImage().original(),
                triangleData.croppedImage().icon()
            };
            ids += triangleData.croppedImage().mipmaps();

            for(auto id : ids) {
                auto data = imageProvider.data(id, true);
                CHECK(data.size().isValid());
            }
        }

        loop.quit();
    });
    loop.exec();
    CHECK(true);
}

