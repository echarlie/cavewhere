/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

//Our includes
#include "cwImage.h"
#include "cwGunZipReader.h"
#include "cwProject.h"

//Qt includes
#include <QDir>
#include <QDebug>
#include <QVector2D>
#include <QVector>

cwImage::cwImage() :
    Data(new PrivateData())
{

}

bool cwImage::isMipmapsValid() const
{
    if(Data->Mipmaps.isEmpty()) {
        return false;
    }
    auto iter = std::find_if(Data->Mipmaps.begin(), Data->Mipmaps.end(),
                             [](int id) { return !cwImage::isIdValid(id); });
    return iter == Data->Mipmaps.end();
}



cwImage::PrivateData::PrivateData() {
    IconId = -1;
    OriginalId = -1;
    OriginalDotsPerMeter = 0;
}


bool cwImage::operator ==(const cwImage &other) const {

    if(Data == other.Data) {
        return true;
    }

    return Data->OriginalId == other.Data->OriginalId
            && Data->Mipmaps == other.Data->Mipmaps
            && Data->IconId == other.Data->IconId
            && Data->OriginalSize == other.Data->OriginalSize
            && Data->OriginalDotsPerMeter == other.Data->OriginalDotsPerMeter;
}
