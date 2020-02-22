//Catch includes
#include <catch.hpp>

//Our includes
#include "cwCropImageTask.h"
#include "cwAddImageTask.h"
#include "cwProject.h"
#include "cwImageProvider.h"
#include "TestHelper.h"
#include "DXT1BlockCompare.h"

//Qt includes
#include <QColor>

TEST_CASE("cwCropImageTask should crop DXT1 images correctly", "[cwCropImageTask]") {

    cwProject project;
    QString filename = project.filename();

    QImage image16x16("://datasets/dx1Cropping/16x16.png");

    auto addImage = [filename](const QImage& image) {
        cwAddImageTask addImageTask;
        addImageTask.setNewImages({image});
        addImageTask.setDatabaseFilename(filename);
        addImageTask.setUsingThreadPool(false);
        addImageTask.start();

        auto images = addImageTask.images();
        REQUIRE(images.size() == 1);

        return images.first();
    };

    auto image = addImage(image16x16);

//    struct Size {
//        QSize dim;
//        int bytes;
//        QVector<QColor> blockColors;
//    };

    QVector<QColor> colors = {
        {QColor("#000000"), QColor("#ff0000"), QColor("#ff00f7"), QColor("#00ff00"),
         QColor("#aa0000"), QColor("#bb0000"), QColor("#cc0000"), QColor("#dd0000"),
         QColor("#00aa00"), QColor("#00bb00"), QColor("#00cc00"), QColor("#00dd00"),
         QColor("#0000aa"), QColor("#0000bb"), QColor("#0000cc"), QColor("#0000dd")}
    };

    QVector<DXT1BlockCompare::TestImage> sizes = {
        {{16, 16}, 128, colors},
        {{8, 8}, 32, {}},
        {{4, 4}, 8, {}},
        {{2, 2}, 8, {}},
        {{1, 1}, 8, {}}
    };


    constexpr int dxt1PixelBlockSize = 4;

    auto checkMipmaps = [filename](const QList<int>& mipmaps, const QVector<DXT1BlockCompare::TestImage> sizes) {
        cwImageProvider provider;
        provider.setProjectPath(filename);

        REQUIRE(mipmaps.size() == sizes.size());

        QList<cwImageData> mipmapImageData;
        mipmapImageData.reserve(mipmaps.size());
        std::transform(mipmaps.begin(), mipmaps.end(), std::back_inserter(mipmapImageData),
                       [&provider](int id)
        {
            return provider.data(id);
        });

        for(int i = 0; i < mipmapImageData.size(); i++) {
            INFO("mipmap:" << i)
            auto mipmap = mipmapImageData.at(i);
            auto size = sizes.at(i);
            DXT1BlockCompare::compare(size, mipmap);
        }
    };

    //Make sure the dxt1 compression is working for addImageTask
    checkMipmaps(image.mipmaps(), sizes);

    SECTION("Crop individual pixels") {
        auto calcBlockSize = [dxt1PixelBlockSize](const cwImage& image) {
            return QSize(image.originalSize().width() / dxt1PixelBlockSize,
                         image.originalSize().height() / dxt1PixelBlockSize);
        };

        auto cropSinglePixel = [filename, dxt1PixelBlockSize, checkMipmaps, calcBlockSize]
                (int xBlock, int yBlock, const cwImage& image, const QVector<QColor>& blockColors) {
            REQUIRE(image.originalSize().width() % dxt1PixelBlockSize == 0);
            REQUIRE(image.originalSize().height() % dxt1PixelBlockSize == 0);

            QSize blockSize = calcBlockSize(image);

            REQUIRE(blockSize.width() * blockSize.height() == blockColors.size());

            QSizeF pixelBlockSize(dxt1PixelBlockSize / static_cast<double>(image.originalSize().width()),
                                  dxt1PixelBlockSize / static_cast<double>(image.originalSize().height()));

            QPointF origin(xBlock * pixelBlockSize.width(),
                           yBlock * pixelBlockSize.height());

            cwCropImageTask cropImageTask;
            cropImageTask.setDatabaseFilename(filename);
            cropImageTask.setUsingThreadPool(false);
            cropImageTask.setRectF(QRectF(origin, pixelBlockSize)); //First pixel
            cropImageTask.setOriginal(image);
            cropImageTask.start();

            cwImage croppedImageId = cropImageTask.croppedImage();

            QVector<QColor> croppedColors = {
                {blockColors.at(yBlock * blockSize.width() + xBlock)},
            };

            QVector<DXT1BlockCompare::TestImage> croppedSizes = {
                {{4, 4}, 8, croppedColors},
            };

            //Check the output
            checkMipmaps(croppedImageId.mipmaps(), croppedSizes);
        };

        auto checkCropSinglePixel = [cropSinglePixel, calcBlockSize](const cwImage& image, const QVector<QColor>& colors) {
            auto blockSize = calcBlockSize(image);
            for(int y = 0; y < blockSize.height(); y++) {
                for(int x = 0; x < blockSize.width(); x++) {
                    INFO("Single pixel:" << x << " " << y);
                    cropSinglePixel(x, y, image, colors);
                }
            }
        };

        auto cropSpecificImage = [filename](const cwImage& image, const QRectF& cropArea) {
            cwCropImageTask cropImageTask;
            cropImageTask.setDatabaseFilename(filename);
            cropImageTask.setUsingThreadPool(false);
            cropImageTask.setRectF(cropArea); //First pixel
            cropImageTask.setOriginal(image);
            cropImageTask.start();

            cwImage croppedImageId = cropImageTask.croppedImage();
            return croppedImageId;
        };

        auto cropImage = [image, cropSpecificImage](const QRectF& cropArea) {
            return cropSpecificImage(image, cropArea);
        };

        //16x16 pixel image
//        checkCropSinglePixel(image, colors);

//        SECTION("Vertical aspect image") {
//            QImage vImage = image16x16.copy(0, 0, 1, image16x16.height());
//            auto addedVImage = addImage(vImage);

//            QVector<QColor> colors = {
//                QColor("#000000"),
//                QColor("#aa0000"),
//                QColor("#00aa00"),
//                QColor("#0000aa"),
//            };

//            checkCropSinglePixel(addedVImage, colors);
//        }

//        SECTION("Horizontal aspect image") {
//            QImage hImage = image16x16.copy(0, 0, image16x16.width(), 1);
//            auto addedHImage = addImage(hImage);

//            QVector<QColor> colors = {
//                QColor("#000000"),
//                QColor("#aa0000"),
//                QColor("#00aa00"),
//                QColor("#0000aa"),
//            };

//            checkCropSinglePixel(addedHImage, colors);
//        }


//        SECTION("Crop larger area top left") {
//            QVector<QColor> croppedColors = {
//                {QColor("#000000"), QColor("#ff0000"),
//                 QColor("#aa0000"), QColor("#bb0000"),
//                 QColor("#00aa00"), QColor("#00bb00"),
//                }
//            };

//            QVector<Size> croppedSizes = {
//                {{8, 12}, 48, croppedColors},
//                {{4, 4}, 8, {}},
//            };

//            //Check the output
//            cwImage croppedImageId = cropImage(QRectF(0.0, 0.0, 0.5, 0.75));
//            checkMipmaps(croppedImageId.mipmaps(), croppedSizes);
//        }

//        SECTION("Crop larger area low right") {
//            QVector<QColor> croppedColors = {
//                {QColor("#cc0000"), QColor("#dd0000"),
//                 QColor("#00cc00"), QColor("#00dd00"),
//                 QColor("#0000cc"), QColor("#0000dd")
//                }
//            };

//            QVector<Size> croppedSizes = {
//                {{8, 12}, 48, croppedColors},
//                {{4, 4}, 8, {}},
//            };

//            //Check the output
//            cwImage croppedImageId = cropImage(QRectF(0.5, 0.25, 0.5, 0.75));
//            checkMipmaps(croppedImageId.mipmaps(), croppedSizes);
//        }

//        SECTION("Crop larger area middle") {
//            QVector<QColor> croppedColors = {
//                {
//                    QColor("#bb0000"), QColor("#cc0000"), QColor("#dd0000"),
//                    QColor("#00bb00"), QColor("#00cc00"), QColor("#00dd00"),
//                }
//            };

//            QVector<Size> croppedSizes = {
//                {{12, 8}, 48, croppedColors},
//                {{4, 4}, 8, {}},
//            };

//            //Check the output
//            cwImage croppedImageId = cropImage(QRectF(0.25, 0.25, 0.75, 0.5));
//            checkMipmaps(croppedImageId.mipmaps(), croppedSizes);
//        }

//        SECTION("Crop bad box") {

//            SECTION("Top left") {
//                QVector<QColor> croppedColors = {
//                    {
//                        QColor("#000000"), QColor("#ff0000")
//                    }
//                };

//                QVector<Size> croppedSizes = {
//                    {{8, 4}, 16, croppedColors},
//                };

//                //Check the output
//                cwImage croppedImageId = cropImage(QRectF(-0.25, -0.25, 0.75, 0.5));
//                checkMipmaps(croppedImageId.mipmaps(), croppedSizes);
//            }

//            SECTION("Top Right") {
//                QVector<QColor> croppedColors = {
//                    {
//                        QColor("#ff00f7"), QColor("#00ff00")
//                    }
//                };

//                QVector<Size> croppedSizes = {
//                    {{8, 4}, 16, croppedColors},
//                };

//                //Check the output
//                cwImage croppedImageId = cropImage(QRectF(0.5, -0.25, 0.75, 0.5));
//                checkMipmaps(croppedImageId.mipmaps(), croppedSizes);
//            }

//            SECTION("Bottom Left") {
//                QVector<QColor> croppedColors = {
//                    {
//                        QColor("#0000aa"), QColor("#0000bb")
//                    }
//                };

//                QVector<Size> croppedSizes = {
//                    {{8, 4}, 16, croppedColors},
//                };

//                //Check the output
//                cwImage croppedImageId = cropImage(QRectF(-0.25, 0.75, 0.75, 0.5));
//                checkMipmaps(croppedImageId.mipmaps(), croppedSizes);
//            }

//            SECTION("Bottom Right") {
//                QVector<QColor> croppedColors = {
//                    {
//                        QColor("#0000cc"), QColor("#0000dd")
//                    }
//                };

//                QVector<Size> croppedSizes = {
//                    {{8, 4}, 16, croppedColors},
//                };

//                //Check the output
//                cwImage croppedImageId = cropImage(QRectF(0.5, 0.75, 0.75, 0.5));
//                checkMipmaps(croppedImageId.mipmaps(), croppedSizes);
//            }
//        }

//        SECTION("No crop") {
//            QVector<QColor> croppedColors = {
//                {
//                    QColor("#000000"), QColor("#ff0000"), QColor("#ff00f7"), QColor("#00ff00"),
//                    QColor("#aa0000"), QColor("#bb0000"), QColor("#cc0000"), QColor("#dd0000"),
//                    QColor("#00aa00"), QColor("#00bb00"), QColor("#00cc00"), QColor("#00dd00"),
//                    QColor("#0000aa"), QColor("#0000bb"), QColor("#0000cc"), QColor("#0000dd")
//                }
//            };

//            QVector<Size> croppedSizes = {
//                {{16, 16}, 128, colors},
//                {{8, 8}, 32, {}},
//                {{4, 4}, 8, {}}
//            };

//            //Check the output
//            cwImage croppedImageId = cropImage(QRectF(QPointF(-0.1, -0.53), QPointF(1.1, 1.4)));
//            checkMipmaps(croppedImageId.mipmaps(), croppedSizes);
//        }


//        SECTION("Weird area") {
//            QVector<QColor> croppedColors = {
//                {
//                    QColor("#00aa00"),
//                    QColor("#0000aa"),
//                }
//            };

//            QVector<Size> croppedSizes = {
//                {{4, 8}, 16, croppedColors},
//            };

//            //Check the output
//            cwImage croppedImageId = cropImage(QRectF(0.1, 0.3, 0.45, 0.7));
//            checkMipmaps(croppedImageId.mipmaps(), croppedSizes);
//        }

        SECTION("Crop real image") {
            //Don't changes thes values, they have been verified with squish compression
            //And work with mipmap rendering with linear interpolation enabled
            QVector<DXT1BlockCompare::TestImage> croppedSizes = {
                {{464, 436}, 101152, {}},
                {{232, 218}, 25520, {}},
                {{116, 109}, 6496, {}},
                {{58, 54}, 1680, {}},
                {{29, 27}, 448, {}},
                {{14, 13}, 128, {}},
                {{7, 6}, 32, {}},
                {{3, 3}, 8, {}},
                {{1, 1}, 8, {}},
            };

            auto realImage = addImage(QImage("://datasets/dx1Cropping/scanCrop.png"));

            //Check the output
            cwImage croppedImageId = cropSpecificImage(realImage, QRectF(0.25, 0.25, 0.5, 0.5));
            checkMipmaps(croppedImageId.mipmaps(), croppedSizes);
        }
    }
}