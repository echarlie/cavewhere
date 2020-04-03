//Catch includes
#include "catch.hpp"

//Our includes
#include "cwProject.h"
#include "cwCavingRegion.h"
#include "cwCave.h"
#include "cwTrip.h"
#include "TestHelper.h"
#include "cwImageProvider.h"
#include "cwRootData.h"
#include "cwSurveyNoteModel.h"
#include "cwTaskManagerModel.h"
#include "cwFutureManagerModel.h"
#include "cwImageDatabase.h"

//Qt includes
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlResult>
#include <QSqlRecord>

TEST_CASE("cwProject isModified should work correctly", "[cwProject]") {
    cwProject project;
    CHECK(project.isModified() == false);

    cwCave* cave1 = new cwCave();
    cave1->setName("cave1");
    project.cavingRegion()->addCave(cave1);

    CHECK(project.isModified() == true);

    QString testFile = prependTempFolder("test_cwProject.cw");
    QFile::remove(testFile);

    project.saveAs(testFile);
    project.waitSaveToFinish();

    CHECK(project.isModified() == false);

    project.newProject();
    CHECK(project.isModified() == false);

    project.loadFile(testFile);
    project.waitLoadToFinish();
    CHECK(project.isModified() == false);

    project.cavingRegion()->cave(0)->addTrip();
    CHECK(project.isModified() == true);

    project.save();
    project.waitSaveToFinish();
    CHECK(project.isModified() == false);

    SECTION("Load file") {
        //If this fails, this is probably because of a version change, or other save changes
        fileToProject(&project, "://datasets/network.cw");
        project.waitLoadToFinish();
        CHECK(project.isModified() == false);

        REQUIRE(project.cavingRegion()->caveCount() == 1);
        REQUIRE(project.cavingRegion()->cave(0)->tripCount() == 1);

        cwTrip* firstTrip = project.cavingRegion()->cave(0)->trip(0);
        firstTrip->setName("Sauce!");
        CHECK(project.isModified() == true);
    }
}

TEST_CASE("Image data should save and load correctly", "[cwProject]") {

    qRegisterMetaType<QList <cwImage> >("QList<cwImage>");

    auto rootData = std::make_unique<cwRootData>();
    auto project = rootData->project();
    project->saveAs(prependTempFolder("imageTest-" + QUuid::createUuid().toString().left(5)));

    auto region = project->cavingRegion();
    region->addCave();
    auto cave = region->cave(0);
    cave->addTrip();
    auto trip = cave->trip(0);

    QString filename = copyToTempFolder("://datasets/test_cwTextureUploadTask/PhakeCave.PNG");
    QImage originalImage(filename);

    trip->notes()->addFromFiles({QUrl("file:/" + filename)});

    rootData->futureManagerModel()->waitForFinished();

    SECTION("Is Modifed shouldn't effect the save and load") {
        //Make sure is modified doesn't modify the underlying file
        CHECK(project->isModified() == true);
    }

    REQUIRE(trip->notes()->notes().size() == 1);

    cwImage image = trip->notes()->notes().first()->image();
    cwImageProvider provider;
    provider.setProjectPath(project->filename());
    QImage sqlImage = provider.image(image.original());

    CHECK(!sqlImage.isNull());
    CHECK(originalImage == sqlImage);
}

TEST_CASE("Images should load correctly", "[cwProject]") {

    auto image = [](const QColor& color) {
        QImage image(1024, 1024, QImage::Format_ARGB32);
        image.fill(color);
        QString imageFilename = QDir::tempPath() + "/" + QString("cavewhere-cwProject-image%1%2%3.png").arg(color.red()).arg(color.green()).arg(color.blue());
        REQUIRE(image.save(imageFilename, "png"));
        return QUrl::fromLocalFile(imageFilename);
    };

    QVector<QColor> imageColors = {"red", "green", "blue"};
    QList<QUrl> filenames;
    std::transform(imageColors.begin(), imageColors.end(), std::back_inserter(filenames), image);

    filenames += {
        QUrl::fromLocalFile(copyToTempFolder("://datasets/test_cwProject/crashMap.png"))
    };

    auto rootData = std::make_unique<cwRootData>();
    auto project = rootData->project();
    int checked = 0;
    project->addImages(filenames, rootData.get(), [&checked](QList<cwImage> images){
        REQUIRE(images.size() == 1);
        CHECK(images.first().isOriginalValid());
        CHECK(images.first().isIconValid());
        checked++;
    });

    rootData->futureManagerModel()->waitForFinished();
    CHECK(checked == filenames.size());
    CHECK(filenames.size() > 0);
}

TEST_CASE("Images should be removed correctly", "[cwProject]") {

    auto rootData = std::make_unique<cwRootData>();
    auto project = rootData->project();

    QList<QUrl> filenames {
        QUrl::fromLocalFile(copyToTempFolder("://datasets/test_cwTextureUploadTask/PhakeCave.PNG")),
                QUrl::fromLocalFile(copyToTempFolder("://datasets/test_cwProject/crashMap.png"))
    };

    QList<cwImage> loadedImages;
    project->addImages(filenames, rootData.get(), [&loadedImages](QList<cwImage> images){
        loadedImages += images;
    });

    rootData->futureManagerModel()->waitForFinished();

    REQUIRE(loadedImages.size() == 2);
    auto firstImage = loadedImages.at(0);
    auto secondImage = loadedImages.at(1);

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", QString("ImageRemoveTest"));
    database.setDatabaseName(project->filename());
    REQUIRE(database.open());

    auto idExists = [database](int id) {
        INFO("idExists:" << id);
        QString SQL = QString("select count(*) from Images where id = %1").arg(id);

        INFO("sql:" << SQL.toStdString());
        INFO("database:" << database.databaseName());

        REQUIRE(database.isOpen());
        REQUIRE(database.isValid());

        QSqlQuery query(database);
        REQUIRE(query.prepare(SQL));

        REQUIRE(query.exec());
        REQUIRE(query.first());

        auto record = query.record();
        return record.value(0).toInt() == 1;
    };

    auto idNotExists = [idExists](int id) {
        return !idExists(id);
    };

    auto imageIds = [](const cwImage& image) {
        QList<int> ids {
            image.original(),
                    image.icon(),
        };

        ids.append(image.mipmaps());
        return ids;
    };

    auto imageTestFunc = [imageIds](const cwImage& image, auto func) {
        auto ids = imageIds(image);

        return std::accumulate(ids.begin(), ids.end(), true,
                               [func](bool current, int id)
        {
            return current && func(id);
        });
    };

    auto imageExists = [idExists, imageTestFunc](const cwImage& image) {
        return imageTestFunc(image, idExists);
    };

    auto imageNotExists = [idNotExists, imageTestFunc](const cwImage& image) {
        return imageTestFunc(image, idNotExists);
    };

    CHECK(imageExists(firstImage));
    CHECK(imageExists(secondImage));

    cwImageDatabase imageDatabase(project->filename());
    imageDatabase.removeImage(secondImage);

    CHECK(imageNotExists(secondImage));
    CHECK(imageExists(firstImage));

    imageDatabase.removeImages({firstImage.original()});

    QList<int> ids = {
        firstImage.icon()
    };
    ids += firstImage.mipmaps();

    for(auto id : ids){
        INFO("id:" << id);
        CHECK(idExists(id));
    }

    CHECK(idNotExists(firstImage.original()));

    database.close();
}
