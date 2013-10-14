/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/


//Our includes
#include "cwLoopCloserTask.h"
#include "cwCavingRegion.h"
#include "cwSurveyChunk.h"
#include "cwTrip.h"
#include "cwCave.h"
#include "cwDebug.h"

//Qt includes
#include <QElapsedTimer>

cwLoopCloserTask::cwLoopCloserTask(QObject *parent) :
    cwTask(parent)
{
}

/**
 * @brief cwLoopCloserTask::setRegion
 * @param region
 *
 * Sets the caving region of all the caves and surveyes to process
 */
void cwLoopCloserTask::setRegion(cwCavingRegion *region)
{
    Region = region;
}

/**
 * @brief cwLoopCloserTask::runTask
 */
void cwLoopCloserTask::runTask()
{
    QElapsedTimer timer;
    timer.start();

    //Do the loop closure for all caves
    foreach(cwCave* cave, Region->caves()) {
        processCave(cave);
    }

    qDebug() << "Loop closure task done in:" << timer.elapsed() << "ms";

    done();
}

/**
 * @brief cwLoopCloserTask::processCave
 * @param cave
 *
 * This process a unconnected cave
 */
void cwLoopCloserTask::processCave(cwCave *cave)
{
    //Find all main edges, (main edge only store intersection at the first and last station
    cwMainEdgeProcessor mainEdgeProcessor;
    QList<cwEdgeSurveyChunk*> edges = mainEdgeProcessor.mainEdges(cave);

    //Print out edge results
    printEdges(edges);

    //For Debugging, this makes sure that station don't exist in the middle of the edge
    checkBasicEdges(edges);

    //

    //Find all loops

}

/**
 * @brief cwLoopCloserTask::printEdges
 * @param edges
 *
 * This is for debugging, this print's out all the edges and stations
 */
void cwLoopCloserTask::printEdges(QList<cwLoopCloserTask::cwEdgeSurveyChunk *> edges) const
{
    //Debugging for the resulting edges
    foreach(cwEdgeSurveyChunk* edge, edges) {
        qDebug() << "--Edge:" << edge;
        foreach(cwStation station, edge->stations()) {
            qDebug() << "\t" << station.name();
        }
    }
    qDebug() << "*********** end of edges ***********";
}

/**
 * @brief cwLoopCloserTask::checkBasicEdges
 * @param edges
 *
 * For Debugging, this makes sure that station don't exist in the middle of the edge
 * End points on the survey chunk are the intersections.  The middle station in the edge
 * exist only once in the cave. This makes sure the middle stations only exist once in
 * the whole cave.
 */
void cwLoopCloserTask::checkBasicEdges(QList<cwEdgeSurveyChunk*> edges) const {
    foreach(cwEdgeSurveyChunk* edge, edges) {
        foreach(cwEdgeSurveyChunk* edge2, edges) {
            if(edge != edge2) {
                for(int i = 1; i < edge->stations().size() - 1; i++) {
                    for(int ii = 1; ii < edge2->stations().size() - 1; ii++) {
                        QString stationName1 = edge->stations().at(i).name();
                        QString stationName2 = edge2->stations().at(ii).name();
                        if(stationName1.compare(stationName2, Qt::CaseInsensitive) == 0) {
                            qDebug() << "This is a bug!, stations shouldn't be repeated in the middle of the survey chunk" << stationName1 << "==" << stationName2 << LOCATION;
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief cwLoopCloserTask::mainEdges
 * @param cave
 * @return All the main survey edges in the cave.
 *
 * This will break the survey chunks into small chunks. The smaller chunk has no intersections
 * in the middle of the chunk.  All intersection happend at stations at the beginning and the
 * end of the chunks.  (A survey chunk is made up of consectutive stations and shots)
 *
 * Finding the main edges is useful for finding basic (smallest) loops in the cave. This drastically
 * reduces computationial effort of the loop dectection algroithm
 */
QList<cwLoopCloserTask::cwEdgeSurveyChunk*> cwLoopCloserTask::cwMainEdgeProcessor::mainEdges(cwCave *cave)
{

    foreach(cwTrip* trip, cave->trips()) {
        foreach(cwSurveyChunk* chunk, trip->chunks()) {

            if(!chunk->isValid()) {
                continue;
            }

            if(chunk->stations().size() == 2 &&
                    chunk->stations().first().name().isEmpty() &&
                    chunk->stations().last().name().isEmpty() &&
                    !chunk->shots().first().isValid())
            {
                continue;
            }

            QList<cwStation> stations = chunk->stations();
            QList<cwShot> shots = chunk->shots();

            if(stations.last().name().isEmpty() &&
                    !shots.last().isValid())
            {
                stations.removeLast();
                shots.removeLast();
            }

            //Add survey chunk, split if nessarcy
            cwEdgeSurveyChunk* edgeChunk = new cwEdgeSurveyChunk();
            edgeChunk->setShots(shots);
            edgeChunk->setStations(stations);

            addEdgeChunk(edgeChunk);
        }
    }

    return ResultingEdges.toList();
}

/**
 * @brief cwLoopCloserTask::addEdgeChunk
 * @param chunk
 */
void cwLoopCloserTask::cwMainEdgeProcessor::addEdgeChunk(cwLoopCloserTask::cwEdgeSurveyChunk* newChunk)
{
    QList<cwStation> stations = newChunk->stations();

    for(int i = 0; i < stations.size(); i++) {
        cwStation station = stations.at(i);

        qDebug() << "Lookup contains station:" << station.name() << Lookup.contains(station.name());

        if(Lookup.contains(station.name())) {
            //Station is an intersection

            if(i == 0 || i == stations.size() - 1) {
                //This is the first or last station in the edgeChunk
                splitOnStation(station);
//                qDebug() << "Appending newChunk:" << newChunk << LOCATION;
//                Q_ASSERT(!ResultingEdges.contains(newChunk));
                ResultingEdges.insert(newChunk);

            } else {
                //This is a middle station in the newChunk, split it and potentially another chunk
                //Split new chunk
                cwEdgeSurveyChunk* otherNewHalf = newChunk->split(station.name());

                //Split the other station alread in the lookup on station.name(), if it can
                splitOnStation(station.name());
//                qDebug() << "Appending newChunk:" << newChunk << LOCATION;
//                Q_ASSERT(!ResultingEdges.contains(newChunk));
                ResultingEdges.insert(newChunk);
                addStationInEdgeChunk(newChunk);

                //Update the station, and restart the search with the other half
                stations = otherNewHalf->stations();
                newChunk = otherNewHalf;
                i = -1;
            }
        }
    }

//    if(Lookup.isEmpty()) {
//        qDebug() << "Appending newChunk:" << newChunk << LOCATION;
//        Q_ASSERT(!ResultingEdges.contains(newChunk));
        ResultingEdges.insert(newChunk);
        addStationInEdgeChunk(newChunk);
//    }
}

/**
 * @brief cwLoopCloserTask::splitOnStation
 * @param station
 * @param chunkLoopup
 */
void cwLoopCloserTask::cwMainEdgeProcessor::splitOnStation(cwStation station)
{
    QList<cwEdgeSurveyChunk*> edgeChunks = Lookup.values(station.name());

    qDebug() << "Split on station:" << station.name();

    foreach(cwEdgeSurveyChunk* chunk, edgeChunks) {
        if(chunk->stations().first().name() != station.name() &&
                chunk->stations().last().name() != station.name())
        {
            //Split otherChunk into two seperate bits, if station falls in the middle
            cwEdgeSurveyChunk* otherHalf = chunk->split(station.name());

            //Remove all occurancies of station found in otherHalf from chunk
            QListIterator<cwStation> iter(otherHalf->stations());
            iter.next(); //Skip first station, because it's still in chunk as the last station
            while(iter.hasNext()) {
                cwStation station = iter.next();
                Lookup.remove(station.name(), chunk);
            }

            addStationInEdgeChunk(otherHalf);
//            qDebug() << "Appending newChunk:" << otherHalf << LOCATION;
//            Q_ASSERT(!ResultingEdges.contains(otherHalf));
            ResultingEdges.insert(otherHalf);
        }
    }
}

/**
 * @brief cwLoopCloserTask::cwChunkLookup::addStationInEdgeChunk
 * @param chunk
 *
 * Adds all the station from chunk into this lookup
 */
void cwLoopCloserTask::cwMainEdgeProcessor::addStationInEdgeChunk(cwLoopCloserTask::cwEdgeSurveyChunk *chunk)
{
    foreach(cwStation station, chunk->stations()) {
        qDebug() << "Inserting station name into lookup:" << station.name() << "chunk:" << chunk;
        Lookup.insert(station.name(), chunk);
    }
}

/**
 * @brief cwLoopCloserTask::cwEdgeSurveyChunk::split
 * @param stationName
 * @return Returns the second half of the survey chunk.  If the chunk couldn't be split this return's NULL
 *
 * The chunk will be split by stationName.  The current chunk's last station will have stationName, and
 * first station in chunk that's return will have the station name.  This function does nothing, and returns
 * NULL, if stationName is first or last station in the chunk. This returns NULL if there's no station
 * in the current chunk.
 */
cwLoopCloserTask::cwEdgeSurveyChunk *cwLoopCloserTask::cwEdgeSurveyChunk::split(QString stationName)
{
    //Make sure the station isn't the beginning or the last station
    Q_ASSERT(!Stations.isEmpty());
    Q_ASSERT(Stations.first().name().compare(stationName, Qt::CaseInsensitive) != 0);
    Q_ASSERT(Stations.last().name().compare(stationName, Qt::CaseInsensitive) != 0);

    for(int i = 1; i < Stations.size() - 1; i++) {
        if(Stations.at(i).name().compare(stationName, Qt::CaseInsensitive) == 0) {
            //Split at the current station
            QList<cwStation> newChunkStations = Stations.mid(i);
            Stations.erase(Stations.begin() + i + 1, Stations.end());

            QList<cwShot> newChunkShots = Shots.mid(i);
            Shots.erase(Shots.begin() + i, Shots.end());

            cwEdgeSurveyChunk* newChunk = new cwEdgeSurveyChunk();
            newChunk->setStations(newChunkStations);
            newChunk->setShots(newChunkShots);
            return newChunk;
        }
    }

    Q_ASSERT(false); //Couldn't find station in chunk

    return NULL;
}


/**
 * @brief cwLoopCloserTask::cwLoopDetector::loopEdges
 * @return Returns all the edges that belong to at least one loop
 */
void cwLoopCloserTask::cwLoopDetector::process(QList<cwLoopCloserTask::cwEdgeSurveyChunk *> edges)
{



}

QList<cwLoopCloserTask::cwEdgeSurveyChunk *> cwLoopCloserTask::cwLoopDetector::loopEdges() const
{
    return LoopEdges;
}

/**
 * @brief cwLoopCloserTask::cwLoopDetector::legEdges
 * @return Returns all the edges that make up legs, edges that aren't part of a loop
 */
QList<cwLoopCloserTask::cwEdgeSurveyChunk *> cwLoopCloserTask::cwLoopDetector::legEdges() const
{
    return LegEdges;
}
