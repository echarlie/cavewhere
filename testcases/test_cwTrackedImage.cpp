//Catch includes
#include "catch.hpp"

//Our includes
#include "cwTrackedImage.h"
#include "cwRootData.h"
#include "cwProject.h"
#include "cwFutureManagerModel.h"
#include "cwImage.h"
#include "TestHelper.h"
#include "cwImageProvider.h"
#include "cwAsyncFuture.h"

//Qt includes
#include <QDebug>

//Async includes
#include "asyncfuture.h"

TEST_CASE("cwTrackedImage should delete images from database", "[cwTrackedImage]") {

    auto rootData = std::make_unique<cwRootData>();
    auto project = rootData->project();

    cwTrackedImage trackedImage;

    int count = 0;
    project->addImages({QUrl::fromLocalFile(copyToTempFolder("://datasets/test_cwTextureUploadTask/PhakeCave.PNG"))},
                       QCoreApplication::instance(),
                       [project, &trackedImage, &count](QList<cwImage> newImages)
    {
        REQUIRE(newImages.size() == 1);

        auto newImage = newImages.first();

        CHECK(newImage.isIconValid());
        CHECK(newImage.isMipmapsValid());
        CHECK(newImage.isOriginalValid());

        trackedImage = cwTrackedImage(newImage, project->filename());
        count++;
    });

    rootData->futureManagerModel()->waitForFinished();

    CHECK(count == 1);
    CHECK(trackedImage.isIconValid());
    CHECK(trackedImage.isMipmapsValid());
    CHECK(trackedImage.isOriginalValid());

    trackedImage.deleteImagesFromDatabase();

    auto checkThatImageWasDeleted = [project](int id) {
        cwImageProvider provider;
        provider.setProjectPath(project->filename());
        auto imageData = provider.data(id);
        CHECK(imageData.data().isEmpty());
        CHECK(!imageData.size().isValid());
    };

    for(int id : trackedImage.ids()) {
        checkThatImageWasDeleted(id);
    }
}

TEST_CASE("cwTrackImage should work with QSharedPointer's custom delete function", "[cwTrackImage]") {

    QSharedPointer<cwTrackedImage> trackedImage;

    auto rootData = std::make_unique<cwRootData>();
    auto project = rootData->project();

    cwTrackedImagePtr trackImagePtr;

    auto run = [project, &trackImagePtr]() {
        QEventLoop loop;

        project->addImages({QUrl::fromLocalFile(copyToTempFolder("://datasets/test_cwTextureUploadTask/PhakeCave.PNG"))},
                           &loop,
                           [project, &loop, &trackImagePtr](QList<cwImage> newImages)
        {
            REQUIRE(newImages.size() == 1);

            auto newImage = newImages.first();

            CHECK(newImage.isIconValid());
            CHECK(newImage.isMipmapsValid());
            CHECK(newImage.isOriginalValid());

            trackImagePtr = cwTrackedImage::createShared(newImage, project->filename());
            loop.quit();
        });

        loop.exec();
    };

    auto future = QtConcurrent::run(run);

    cwAsyncFuture::waitForFinished(future);

    REQUIRE(!trackImagePtr.isNull());

    auto checkThatImageExist = [project](int id) {
        cwImageProvider provider;
        provider.setProjectPath(project->filename());
        auto imageData = provider.data(id);
        CHECK(!imageData.data().isEmpty());
        CHECK(imageData.size().isValid());
    };

    auto checkThatImageWasDeleted = [project](int id) {
        cwImageProvider provider;
        provider.setProjectPath(project->filename());
        auto imageData = provider.data(id);
        CHECK(imageData.data().isEmpty());
        CHECK(!imageData.size().isValid());
    };

    for(auto id : trackImagePtr->ids()) {
        checkThatImageExist(id);
    }

    SECTION("Check that delete works correctly with shared") {
        cwImage image = *trackImagePtr;
        trackImagePtr.clear();
        for(auto id : image.ids()) {
            checkThatImageWasDeleted(id);
        }
    }

    SECTION("Check that take works correctly with shared") {
        cwImage image = trackImagePtr->take();
        trackImagePtr.clear();
        for(auto id : image.ids()) {
            checkThatImageExist(id);
        }
    }
}
