//Our includes
#include "cwShot.h"
#include "cwSurveyChunk.h"
#include "cwStation.h"


cwShot::cwShot(QObject *parent) :
    QObject(parent)
{
    Distance = "";
    Compass = "";
    BackCompass = "";
    Clino = "";
    BackClino = "";

}

cwShot::cwShot(QVariant distance,
               QVariant compass,
               QVariant backCompass,
               QVariant clino,
               QVariant backClino,
               QObject* parent) :
    QObject(parent)
{

    Distance = distance;
    Compass = compass;
    BackCompass = backCompass;
    Clino = clino;
    BackClino = backClino;
}

cwShot::cwShot(const cwShot& shot) {
    Distance = shot.Distance;
    Compass = shot.Compass;
    BackCompass = shot.BackCompass;
    Clino = shot.Clino;
    BackClino = shot.Clino;
}


void cwShot::SetDistance(QVariant distance) {
    if(Distance != distance) {
        Distance = distance;
        emit DistanceChanged();
    }
}

void cwShot::SetCompass(QVariant compass) {
    if(Compass != compass) {
        Compass = compass;
        emit CompassChanged();
    }
}

void cwShot::SetBackCompass(QVariant backCompass) {
    if(BackCompass != backCompass) {
        BackCompass = backCompass;
        emit BackCompassChanged();
    }
}

void cwShot::SetClino(QVariant clino) {
    if(Clino != clino) {
        Clino = clino;
        emit ClinoChanged();
    }
}

void cwShot::SetBackClino(QVariant backClino) {
    if(BackClino != backClino) {
        BackClino = backClino;
        emit BackClinoChanged();
    }
}

/**
  \brief The parent chunk that this shot is connected to
  */
cwSurveyChunk* cwShot::parentChunk() const {
    cwSurveyChunk* parentChunk = qobject_cast<cwSurveyChunk*>(parent());
    return parentChunk;
}

/**
  \brief The to station of this shot
  */
cwStation* cwShot::toStation() const {
    cwSurveyChunk* chunk = parentChunk();
    if(chunk != NULL) {
        return chunk->ToFromStations(this).second;
    }
    return NULL;
}

/**
  \brief The from station of these shot
  */
cwStation* cwShot::fromStation() const {
    cwSurveyChunk* chunk = parentChunk();
    if(chunk != NULL) {
        return chunk->ToFromStations(this).first;
    }
    return NULL;
}
