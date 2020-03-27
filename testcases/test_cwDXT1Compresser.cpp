//Catch includes
#include "catch.hpp"

//Our includes
#include "cwDXT1Compresser.h"
#include "cwAsyncFuture.h"
#include "DXT1BlockCompare.h"
#include "cwImageProvider.h"

//Qt includes
#include <QImage>

//Async future
#include "asyncfuture.h"

TEST_CASE("cwDXT1Compresser should be able to compress correctly", "[cwDXT1Compresser]") {
    QImage image16x16("://datasets/dx1Cropping/16x16.png");

    cwDXT1Compresser compresser;

    QVector<QColor> colors = {
        {QColor("#000000"), QColor("#ff0000"), QColor("#ff00f7"), QColor("#00ff00"),
         QColor("#aa0000"), QColor("#bb0000"), QColor("#cc0000"), QColor("#dd0000"),
         QColor("#00aa00"), QColor("#00bb00"), QColor("#00cc00"), QColor("#00dd00"),
         QColor("#0000aa"), QColor("#0000bb"), QColor("#0000cc"), QColor("#0000dd")}
    };

    DXT1BlockCompare::TestImage testImage = {{16, 16}, 128, colors};

    auto testCompression = [image16x16, testImage](const QList<cwDXT1Compresser::CompressedImage>& compressedImages) {
        REQUIRE(compressedImages.size() == 1);

        auto image = compressedImages.at(0);
        CHECK(image.size == image16x16.size());
        CHECK(image.data.size() == 128);

        //Check pixels
        DXT1BlockCompare::compare(testImage, cwImageData(image16x16.size(), 0, "test", image.data));
    };

    SECTION("OpenGL no threading") {
        auto future = compresser.openglCompression({image16x16}, false);
        auto completeFuture = AsyncFuture::observe(future).subscribe([future, testCompression](){
            testCompression(future.results());
        },
        [](){ CHECK(false);} //Cancelled
        ).future();
        cwAsyncFuture::waitForFinished(completeFuture, 1000);
    }

    SECTION("OpenGL threading") {
        auto future = compresser.openglCompression({image16x16}, true);
        auto completeFuture = AsyncFuture::observe(future).subscribe([future, testCompression](){
            testCompression(future.results());
        },
        [](){ CHECK(false);} //Cancelled
        ).future();
        cwAsyncFuture::waitForFinished(completeFuture, 1000);
    }

    SECTION("Squish no threading") {
        auto future = compresser.squishCompression({image16x16}, false);
        auto completeFuture = AsyncFuture::observe(future).subscribe([future, testCompression](){
            testCompression(future.results());
        },
        [](){ CHECK(false);} //Cancelled
        ).future();
        cwAsyncFuture::waitForFinished(completeFuture, 1000);
    }

    SECTION("Squish threading") {
        auto future = compresser.squishCompression({image16x16}, true);
        auto completeFuture = AsyncFuture::observe(future).subscribe([future, testCompression](){
            testCompression(future.results());
        },
        [](){ CHECK(false);} //Cancelled
        ).future();
        cwAsyncFuture::waitForFinished(completeFuture, 1000);
    }

    SECTION("Generic") {
        auto future = compresser.compress({image16x16});
        auto completeFuture = AsyncFuture::observe(future).subscribe([future, testCompression](){
            testCompression(future.results());
        },
        [](){ CHECK(false);} //Cancelled
        ).future();
        cwAsyncFuture::waitForFinished(completeFuture, 1000);
    }
};

TEST_CASE("cwDXT1Compresser should be able to be canceled correctly", "[cwDXT1Compresser]") {

    QImage image("://datasets/dx1Cropping/scanCrop.png");
    REQUIRE(!image.isNull());

    QFuture<cwDXT1Compresser::CompressedImage> future;

    {
        cwDXT1Compresser compressor;
        future = compressor.squishCompression({image}, true);
        //Deallocate the compressor
    }

//    int count = 0;
//    AsyncFuture::observe(future).onProgress([&future, &count]() {
////        if(count > 10) {
//        qDebug() << "Cancel!";
//            future.cancel();
////        }
//        count++;
//    });
    future.cancel();

    CHECK(cwAsyncFuture::waitForFinished(future, 2000));
//    CHECK(count == 1);
    CHECK(future.isCanceled());
}
