#ifndef CW3DREGIONVIEWER_H
#define CW3DREGIONVIEWER_H

//Qt includes
#include <QVector3D>
#include <QMatrix4x4>

//Our includes
#include "cwGLRenderer.h"
class cwGLTerrain;
class cwGLLinePlot;
class cwGLScraps;
class cwGLGridPlane;

class cw3dRegionViewer : public cwGLRenderer
{
    Q_OBJECT
    Q_PROPERTY(cwCavingRegion* cavingRegion READ cavingRegion WRITE setCavingRegion NOTIFY cavingRegionChanged)
    Q_PROPERTY(cwGLLinePlot* linePlot READ linePlot)
    Q_PROPERTY(cwGLScraps* scraps READ scraps)

public:
    cw3dRegionViewer(QQuickItem* parent = 0);

    virtual void paint(QPainter* painter);
    Q_INVOKABLE virtual void initializeGL();
public slots:
    cwGLLinePlot* linePlot();
    cwGLScraps* scraps() const;

    void setCavingRegion(cwCavingRegion* region);
    cwCavingRegion* cavingRegion() const;

    void startPanning(QPoint currentMousePos);
    void pan(QPoint currentMousePos);

    void startRotating(QPoint currentMousePos);
    void rotate(QPoint currentMousePos);

    void zoom(QPoint position, int delta);
signals:
    void cavingRegionChanged();

private slots:
    //Interaction events
    void resetView();

    virtual void resizeGL();

protected:

    QVector3D unProject(QPoint point);



    virtual void paintFramebuffer();

private:

    /**
      \brief This is used to render text labels correctly

      This will transform the points in a multi thread manor.
      */
    class TransformPoint {
    public:
        typedef QVector3D result_type;

        TransformPoint(QMatrix4x4 modelViewProjection, QRect viewport) {
            ModelViewProjection = modelViewProjection;
            Viewport = viewport;
        }

        /**
          \brief Transforms the point
          */
        void operator()(QPair<QString, QVector3D>& stationName);

    private:
        QMatrix4x4 ModelViewProjection;
        QRect Viewport;
    };

    bool Initilized; //True if it's initilized and false if it's not

    //For interaction
    QVector3D LastMouseGlobalPosition; //For panning
    QPointF LastMousePosition; //For rotation
    QQuaternion CurrentRotation;
    float Pitch;
    float Azimuth;

    //The terrain that's rendered
    cwGLTerrain* Terrain;
    cwGLLinePlot* LinePlot;
    cwGLScraps* Scraps;
    cwGLGridPlane* Plane;

    //For rendering label
    cwCavingRegion* Region;
    cwCollisionRectKdTree LabelKdTree;

    void renderStationLabels(QPainter* painter);
    void renderStationLabels(QPainter* painter, cwCave* cave);

};

Q_DECLARE_METATYPE(cw3dRegionViewer*)

/**
  \brief Returns the object that renderes the line plot
  */
inline cwGLLinePlot* cw3dRegionViewer::linePlot() { return LinePlot; }

inline cwGLScraps *cw3dRegionViewer::scraps() const
{
    return Scraps;
}

/**
  \brief Returns the caving region that's owned by the renderer
  */
inline cwCavingRegion* cw3dRegionViewer::cavingRegion() const {
    return Region;
}


#endif // CW3DREGIONVIEWER_H
