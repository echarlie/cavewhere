/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

#ifndef CWLINEPLOTTASK_H
#define CWLINEPLOTTASK_H

//Our includes
#include "cwTask.h"
class cwCavingRegion;
class cwLinePlotGeometryTask;
#include "cwStationPositionLookup.h"
class cwSurvexExporterRegionTask;
class cwCavernTask;
class cwPlotSauceTask;
class cwPlotSauceXMLTask;
class cwScrap;
class cwTrip;
class cwCave;
class cwLoopCloserTask;

//Qt includes
#include <QTemporaryFile>
#include <QVector3D>
#include <QTime>
#include <QVector>
#include <QSet>

class cwLinePlotTask : public cwTask
{
    Q_OBJECT
public:

    class LinePlotCaveData {
    public:
        LinePlotCaveData();

        void setDepth(double depth);
        void setLength(double length);
        void setStationPositions(cwStationPositionLookup positionLookup);

        double depth() const;
        double length() const;
        cwStationPositionLookup stationPositions() const;

        bool hasDepthLengthChanged() const;
        bool hasStationPositionsChanged() const;

    private:
        bool DepthLengthChanged;
        double Depth;
        double Length;

        bool StationPostionsChanged;
        cwStationPositionLookup Lookup;
    };

    /**
     * @brief The CaveStationData class
     *
     * Holds a pointer to the cave's data that has been changed. This is the pointer to the
     * orginial cave, trips, and scraps. in the main thread
     *
     * Also holds the new station lookup that the cave should be update with.  We don't update
     * the original cave's data directly because of thread safety.
     */
    class LinePlotResultData {
    public:
        LinePlotResultData() { }

        void clear();

        void setCaveData(QMap<cwCave*, LinePlotCaveData> caveData);
        void setTrip(QSet<cwTrip*> trips);
        void setScraps(QSet<cwScrap*> scraps);
        void setPositions(QVector<QVector3D> positions);
        void setPlotIndexData(QVector<unsigned int> indexData);      

        QMap<cwCave*, LinePlotCaveData> caveData() const;
        QSet<cwTrip*> trips() const;
        QSet<cwScrap*> scraps() const;
        QVector<QVector3D> stationPositions() const;
        QVector<unsigned int> linePlotIndexData() const;


        //For testing
        void setPositionsNewMethod(QVector<QVector3D> positions);
        void setPlotIndexDataNewMethode(QVector<unsigned int> indexData);
        QVector<QVector3D> stationPositionsNewMethod() const;
        QVector<unsigned int> linePlotIndexDataNewMethod() const;

    private:
        QMap<cwCave*, LinePlotCaveData> Caves;
        QSet<cwTrip*> Trips;
        QSet<cwScrap*> Scraps;
        QVector<QVector3D> StationPositions;
        QVector<unsigned int> LinePlotIndexData;

        //For testing
        QVector<QVector3D> StationPositionsNewMethod;
        QVector<unsigned int> LinePlotIndexDataNewMethod;

        friend class cwLinePlotTask;
    };

    explicit cwLinePlotTask(QObject *parent = 0);

    LinePlotResultData linePlotData() const;

signals:

protected:
    virtual void runTask();

public slots:
    void setData(const cwCavingRegion &region);

private slots:
    void exportData();
    void runCavern();
    void convertToXML();
    void readXML();
    void generateCenterlineGeometry();
    void linePlotTaskComplete();

    //For setting up all the station positions
    void updateStationPositionForCaves(const cwStationPositionLookup& stationPostions);

    //Update the depth and length data
    void updateDepthLength();

private:
    /**
     * TripDataPtrs, CaveDataPtrs, and RegionDataPtrs, store pointers to original
     * data in this task.  This will allow the task to return the pointer of each object
     * that the station's position has changed.  This is extremely useful for alerting
     * when a specific scrap needs to be updated.
     *
     * It is unsafe to use the pointers in the this data structure.  But are used for
     * book keeping only.
     *
     * This could possibly be replaced by id's in the future
     */
    class TripDataPtrs {
    public:
        TripDataPtrs() {}
        TripDataPtrs(cwTrip* trip);

        cwTrip* Trip;
        QList<cwScrap*> Scraps;
    };

    class CaveDataPtrs {
    public:
        CaveDataPtrs() {}
        CaveDataPtrs(cwCave* cave);

        cwCave* Cave;
        QList<TripDataPtrs> Trips;
    };

    class RegionDataPtrs {
    public:
        RegionDataPtrs() {}
        RegionDataPtrs(const cwCavingRegion& region);

        QList<CaveDataPtrs> Caves;
    };

    /**
     * @brief The StationCaveLookup class
     *
     * Stores a lookup for all the stations and scraps in a cave.  This will a station to multiple
     * trips / scraps
     */
    class StationTripScrapLookup {
    public:
        StationTripScrapLookup(cwCave* cave);
        StationTripScrapLookup() { }

        QList<int> trips(QString stationName) const;
        QList<QPair<int, int> > scraps(QString stationName) const;

    private:
        QHash<QString, int> MapStationToTrip; //Multi map of a station to multiple trips indexes
        QHash<QString, QPair<int, int> > MapStationToScrap; //Multi map of a station to multiple scraps indexes (first index is cave, then scrap index)
    };

    //The region data
    cwCavingRegion* Region; //Local copy of the region, we can modify this

    RegionDataPtrs RegionOriginalPointers; //Allows use to notify the which of the original data has changed
    QVector<cwStationPositionLookup> CaveStationLookups; //Copies of all the cave station lookups that are going to be modified
    QVector<StationTripScrapLookup> TripLookups; //Generated in indexStations()

    //The temparary survex file
    QTemporaryFile* SurvexFile;
    cwSurvexExporterRegionTask* SurvexExporter;

    //Sub tasks
    cwCavernTask* CavernTask;
    cwPlotSauceTask* PlotSauceTask;
    cwPlotSauceXMLTask* PlotSauceParseTask;
    cwLinePlotGeometryTask* CenterlineGeometryTask;

    //The loop closure task
    cwLoopCloserTask* LoopCloserTask;

    //What's returned
    LinePlotResultData Result;

    //For performance testing
    QTime TimeSurvex; //Survex loop closure
    int TimeCavewhere; //Cavewhere loop closure
    cwCavingRegion* RegionNewMethod; //Local copy for testing cavewhere's new loopclosure method
    cwLinePlotGeometryTask* CenterlineGeometryTaskNewMethod; //For test cavewhere's new loopclosure method

    void encodeCaveNames();
    void initializeCaveStationLookups();
    void setStationAsChanged(int caveIndex, QString stationName);
    void indexStations();

    QVector<cwStationPositionLookup> splitLookupByCave(const cwStationPositionLookup& stationPostions);
    void updateInteralCaveStationLookups(QVector<cwStationPositionLookup> caveStations);
    void updateExteralCaveStationLookups();

};

/**
 * @brief cwLinePlotTask::LinePlotResultData::caveData
 * @return The external cave pointer that's should be updated with cwStationPositionLookup
 *
 * This functions aren't thread safe!! You should only call these if the task isn't running
 */
inline QMap<cwCave *, cwLinePlotTask::LinePlotCaveData> cwLinePlotTask::LinePlotResultData::caveData() const
{
    return Caves;
}

/**
 * @brief cwLinePlotTask::LinePlotResultData::trips
 * @return A list of all external trips that there station positions have changed
 *
 * This functions aren't thread safe!! You should only call these if the task isn't running
 */
inline QSet<cwTrip *> cwLinePlotTask::LinePlotResultData::trips() const
{
    return Trips;
}

/**
 * @brief cwLinePlotTask::LinePlotResultData::scraps
 * @return A list of all the scraps that the position has changed
 *
 * This functions aren't thread safe!! You should only call these if the task isn't running
 */
inline QSet<cwScrap *> cwLinePlotTask::LinePlotResultData::scraps() const
{
    return Scraps;
}

/**
 * @brief cwLinePlotTask::LinePlotResultData::stationPositions
 *
 * This returns all the positions of the points.  This is used strictly for rendering.
 * This functions aren't thread safe!! You should only call these if the task isn't running
 */
inline QVector<QVector3D> cwLinePlotTask::LinePlotResultData::stationPositions() const
{
    return StationPositions;
}

/**
 * @brief cwLinePlotTask::LinePlotResultData::linePlotIndexData
 * @return Returns all the plot line data indexes.  This is to construct a line array
 *
 *  This functions aren't thread safe!! You should only call these if the task isn't running
 */
inline QVector<unsigned int> cwLinePlotTask::LinePlotResultData::linePlotIndexData() const
{
    return LinePlotIndexData;
}

/**
 * @brief cwLinePlotTask::LinePlotResultData::setPositionsNewMethod
 * @param positions
 *
 *  For testing between survex and cavewhere
 */
inline void cwLinePlotTask::LinePlotResultData::setPositionsNewMethod(QVector<QVector3D> positions)
{
    StationPositionsNewMethod = positions;
}

/**
 * @brief cwLinePlotTask::LinePlotResultData::setPlotIndexDataNewMethode
 * @param indexData
 *
 *  For testing between survex and cavewhere
 */
inline void cwLinePlotTask::LinePlotResultData::setPlotIndexDataNewMethode(QVector<unsigned int> indexData)
{
    LinePlotIndexDataNewMethod = indexData;
}

/**
 * @brief cwLinePlotTask::LinePlotResultData::stationPositionsNewMethod
 * @return
 *
 *  For testing between survex and cavewhere
 */
inline QVector<QVector3D> cwLinePlotTask::LinePlotResultData::stationPositionsNewMethod() const
{
    return StationPositionsNewMethod;
}

/**
 * @brief cwLinePlotTask::LinePlotResultData::linePlotIndexDataNewMethod
 * @return
 *  For testing between survex and cavewhere
 */
inline QVector<unsigned int> cwLinePlotTask::LinePlotResultData::linePlotIndexDataNewMethod() const
{
    return LinePlotIndexDataNewMethod;
}


/**
 * @brief cwLinePlotTask::LinePlotResultData::setCaveData
 * @param caveData
 */
inline void cwLinePlotTask::LinePlotResultData::setCaveData(QMap<cwCave *, LinePlotCaveData> caveData) {
    Caves = caveData;
}

/**
 * @brief cwLinePlotTask::LinePlotResultData::setTrip
 * @param trips
 */
inline void cwLinePlotTask::LinePlotResultData::setTrip(QSet<cwTrip*> trips) {
    Trips = trips;
}

/**
 * @brief cwLinePlotTask::LinePlotResultData::setScraps
 * @param scraps
 */
inline void cwLinePlotTask::LinePlotResultData::setScraps(QSet<cwScrap*> scraps) {
    Scraps = scraps;
}

/**
 * @brief cwLinePlotTask::LinePlotResultData::setPositions
 * @param positions
 */
inline void cwLinePlotTask::LinePlotResultData::setPositions(QVector<QVector3D> positions) {
    StationPositions = positions;
}

/**
 * @brief cwLinePlotTask::LinePlotResultData::setPlotIndexData
 * @param indexData
 */
inline void cwLinePlotTask::LinePlotResultData::setPlotIndexData(QVector<unsigned int> indexData) {
    LinePlotIndexData = indexData;
}

/**
 * @brief cwLinePlotTask::linePlotData
 * @return The resulting line plot data from the task.
 *
 * This will include, the 3D geometry of the lineplot and the objects that need to be updated
 * based on the station data that has changed
 */
inline cwLinePlotTask::LinePlotResultData cwLinePlotTask::linePlotData() const
{
    return Result;
}

/**
 * @brief cwLinePlotTask::StationTripScrapLookup::trips
 * @param stationName
 * @return All the trips that contain stationName
 */
inline QList<int> cwLinePlotTask::StationTripScrapLookup::trips(QString stationName) const
{
    return MapStationToTrip.values(stationName);
}

/**
 * @brief cwLinePlotTask::StationTripScrapLookup::scraps
 * @param stationName
 * @return All the scraps that contain stationName
 */
inline QList<QPair<int, int> > cwLinePlotTask::StationTripScrapLookup::scraps(QString stationName) const
{
    return MapStationToScrap.values(stationName);
}


inline void cwLinePlotTask::LinePlotCaveData::setDepth(double depth)
{
    Depth = depth;
    DepthLengthChanged = true;
}

/**
 * @brief cwLinePlotTask::LinePlotCaveData::setLength
 * @param length
 */
inline void cwLinePlotTask::LinePlotCaveData::setLength(double length)
{
    Length = length;
    DepthLengthChanged = true;
}

/**
 * @brief cwLinePlotTask::LinePlotCaveData::setStationPositions
 * @param positionLookup
 */
inline void cwLinePlotTask::LinePlotCaveData::setStationPositions(cwStationPositionLookup positionLookup)
{
    Lookup = positionLookup;
    StationPostionsChanged = true;
}

/**
 * @brief cwLinePlotTask::LinePlotCaveData::depth
 * @return The depth of the cave
 */
inline double cwLinePlotTask::LinePlotCaveData::depth() const
{
    return Depth;
}

/**
 * @brief cwLinePlotTask::LinePlotCaveData::length
 * @return The length of the cave
 */
inline double cwLinePlotTask::LinePlotCaveData::length() const
{
    return Length;
}

/**
 * @brief cwLinePlotTask::LinePlotCaveData::stationPositions
 * @return All the station positions of the cave
 */
inline cwStationPositionLookup cwLinePlotTask::LinePlotCaveData::stationPositions() const
{
    return Lookup;
}

/**
 * @brief cwLinePlotTask::LinePlotCaveData::hasDepthLengthChanged
 *
 * Return's true if the depth and the legth has     changed
 */
inline bool cwLinePlotTask::LinePlotCaveData::hasDepthLengthChanged() const
{
    return DepthLengthChanged;
}

/**
 * @brief cwLinePlotTask::LinePlotCaveData::hasStationPositionsChanged
 *
 * Return's true if the station positions have changed
 */
inline bool cwLinePlotTask::LinePlotCaveData::hasStationPositionsChanged() const
{
    return StationPostionsChanged;
}





#endif // CWLINEPLOTTASK_H
