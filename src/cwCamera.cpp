/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

//Our includes
#include "cwCamera.h"
#include "cwDebug.h"

//Qt includes
#include <Qt3DCore/QTransform>

using namespace Qt3DRender;
using namespace Qt3DCore;

cwCamera::cwCamera(Qt3DCore::QNode *parent) :
    Qt3DCore::QEntity(parent),
    ZoomScale(1.0),
    Transform(new Qt3DCore::QTransform(this)),
    CameraLens(new Qt3DRender::QCameraLens(this))
//    Qt3dCamera(nullptr)
{
    ViewProjectionMatrixIsDirty = true;
    connect(this, &cwCamera::projectionChanged, this, &cwCamera::pixelsPerMeterChanged);

    connect(this, &cwCamera::projectionChanged, this, [&](){CameraLens->setProjectionMatrix(projectionMatrix());});
    connect(this, &cwCamera::viewMatrixChanged, this, [&](){Transform->setMatrix(viewMatrix());});

    //Test code
//    setProjection(orthoProjectionDefault());

//    setQt3dCamera(new QCamera());

    addComponent(Transform);
    addComponent(CameraLens);
}

/**
  This unprojects point with viewDepth.  The point should be in the
  windows coordinates and the viewDepth should be queried from the depth buffer.

  If the model matrix is the identity, then this returns in world coordinates, otherwise
  it returns in local coordinates of the model.

  This assumes that point is already in opengl viewport pixel coordinates.  Use mapToGLViewport
  to convert from Qt viewport coordinates
  */
QVector3D cwCamera::unProject(QPoint point, float viewDepth, QMatrix4x4 viewMatrix, QMatrix4x4 modelMatrix) const {

   QMatrix4x4 modelViewProjectionMatrix = Projection.matrix() * viewMatrix * modelMatrix;

   bool canInvert;
   QMatrix4x4 inverseMatrix = modelViewProjectionMatrix.inverted(&canInvert);

   if(!canInvert) {
       qDebug() << "Can't invert matrix!!";
       return QVector3D();
   }

   float x = 2.0f * ((float)point.x() - (float)Viewport.x()) / (float)Viewport.width() - 1.0f;
   float y = 2.0f * ((float)point.y() - (float)Viewport.y()) / (float)Viewport.height() - 1.0f;
   float z = 2.0f * viewDepth - 1.0f;

   QVector3D viewportPoint(x, y, z);
   QVector3D unprojectPoint = inverseMatrix.map(viewportPoint);
   return unprojectPoint;
}

/**
  This maps the point in Qt viewport where the 0, 0 is in the top left to
  opengl viewport where the 0, 0 is in the bottom left
  */
QPoint cwCamera::mapToGLViewport(QPoint qtViewportPoint) const {
    int flippedY = Viewport.y() + (Viewport.height() - qtViewportPoint.y());
    return QPoint(qtViewportPoint.x(), flippedY);
}

/**
  This maps the point in GL Viewport where the 0, 0 is in the bottom left to
  qt viewport where the 0, 0 is in the top left
  */
QPointF cwCamera::mapToQtViewport(QPointF glViewportPoint) const
{
    double flippedY = Viewport.y() + (Viewport.height() - glViewportPoint.y());
    return QPointF(glViewportPoint.x(), flippedY + 1);
}

/**
 * @brief cwCamera::setCustomProjection
 * @param matrix - This set a custom projection matrix.
 */
void cwCamera::setCustomProjection(QMatrix4x4 matrix)
{
   Projection.setMatrix(matrix);
   ViewProjectionMatrixIsDirty = true;
   emit projectionChanged();
}

/**
  Sets the projection matrix for the camera
  */
void cwCamera::setProjection(cwProjection projection) {
    Projection = projection;
    ViewProjectionMatrixIsDirty = true;
    emit projectionChanged();
}

/**
  Sets the view matrix for the camera
  */
void cwCamera::setViewMatrix(QMatrix4x4 matrix) {
    ViewMatrix = matrix;
    ViewProjectionMatrixIsDirty = true;
    emit viewMatrixChanged();
}

/**
  \brief Gets the view projection matrix for the camera
  */
QMatrix4x4 cwCamera::viewProjectionMatrix() {
    if(ViewProjectionMatrixIsDirty) {
        ViewProjectionMatrix = Projection.matrix() * ViewMatrix;
        ViewProjectionMatrix.optimize();
        ViewProjectionMatrixIsDirty = false;
    }
    return ViewProjectionMatrix;
}

/**
  \brief This maps normalized screen coordinates to Qt Viewport

  \param point - The point, in normalize screen coordinates.  This is the point after it's been
  transform with modelViewProjection matrix

  \returns This return QVector3D in QtViewport. The qt viewport is the orgin (0, 0) is upper left
  corner of the window
  */
QVector3D cwCamera::mapNormalizeScreenToGLViewport(const QVector3D& point) const {
    return mapNormalizeScreenToGLViewport(point, Viewport);
}

/**
  \brief See mapNormalizeScreenToQtViewport

  The only differenc, is the viewport is send to this function, instead of using the camera's viewport
  */
 QVector3D cwCamera::mapNormalizeScreenToGLViewport(const QVector3D& point, const QRect& viewport) {
     //Transform the point into screen pixels
     float x = viewport.x() + (viewport.width() * (point.x() + 1.0)) / 2.0;
     float y = viewport.y() + (viewport.height() * (point.y() + 1.0)) / 2.0;
     float z = (point.z() + 1.0) / 2.0;

     return QVector3D(x, y, z);
 }

 /**
  * @brief cwCamera::project
  * @param point - Projects the point into screen coordinates
  * @param viewMatrix
  * @param modelMatrix
  * @return Returns the point in screen coordiante
  */
 QPointF cwCamera::project(QVector3D point, QMatrix4x4 viewMatrix, QMatrix4x4 modelMatrix) const
 {
      QMatrix4x4 modelViewProjectionMatrix = Projection.matrix() * viewMatrix * modelMatrix;
      QVector3D projectedPoint = modelViewProjectionMatrix * point;
      QVector3D viewportQt = mapNormalizeScreenToGLViewport(projectedPoint);
      return mapToQtViewport(viewportQt.toPointF());
 }

 /**
  * @brief cwCamera::pixelsPerMeter
  * @return double - pixels per meter
  *
  * This is a utility function for the ScaleBar.  This shows the number of pixels per
  * meter on the screen.  This function only make sense if the projection in an orthoganal
  * one.
  */
 double cwCamera::pixelsPerMeter() const {
     QVector3D meter(1.0, 0.0, 0.0);
     QVector3D normalizeScreen = projectionMatrix().mapVector(meter);
     QVector3D pixelVector = mapNormalizeScreenToGLViewport(normalizeScreen);
     pixelVector = pixelVector - QVector3D(viewport().width() / 2.0, viewport().height() / 2.0, 0.0);
     return pixelVector.x();
 }

 /**
* @brief cwCamera::setZoomScale
* @param zoomScale
*/
 void cwCamera::setZoomScale(double zoomScale) {
     if(projection().type() != cwProjection::Ortho) {
         qWarning() << "Can't set zoom scale because the current projection isn't Ortho" << LOCATION;
        return;
     }

     if(ZoomScale != zoomScale) {
         ZoomScale = zoomScale;
         setProjection(orthoProjectionDefault());
         emit zoomScaleChanged();
     }
 }

 /**
  * @brief cw3dRegionViewer::orthoProjection
  * @return This get the current ortho projection at the current zoom level
  */
 cwProjection cwCamera::orthoProjectionDefault() const
 {
     cwProjection projection;
     QRectF viewport = this->viewport();
     projection.setOrtho(-viewport.width() / 2.0 * ZoomScale,
                         viewport.width() / 2.0 * ZoomScale,
                         -viewport.height() / 2.0 * ZoomScale,
                         viewport.height() / 2.0 * ZoomScale, -10000, 10000);
     return projection;
 }

 /**
  * @brief cw3dRegionViewer::perspectiveProjection
  * @return The current prespective projection for the viewer
  */
 cwProjection cwCamera::perspectiveProjectionDefault() const
 {
     cwProjection projection;
     QRectF viewport = this->viewport();
     projection.setPerspective(55, viewport.width() / (float)viewport.height(), 1, 10000);
     return projection;
 }

 /**
* @brief cwCamera::setQt3dCamera
* @param qt3dCamera
*/
// void cwCamera::setQt3dCamera(Qt3DRender::QCamera* qt3dCamera) {
//     if(Qt3dCamera != qt3dCamera) {

//         if(Qt3dCamera != nullptr) {
//             disconnect(this, 0, Qt3dCamera, 0);
//         }

//         Qt3dCamera = qt3dCamera;

//         if(Qt3dCamera != nullptr) {
//             auto updateProjection = [&]() {
//                  Qt3dCamera->setProjectionMatrix(projectionMatrix());
//             };

//             auto updateView = [&]() {
//                 Qt3dCamera->transform()->setMatrix(viewMatrix());
//             };

//             connect(this, &cwCamera::projectionChanged, Qt3dCamera, updateProjection);
//             connect(this, &cwCamera::viewMatrixChanged, Qt3dCamera, updateView);

//             updateProjection();
//             updateView();
//         }

//         emit qt3dCameraChanged();
//     }
// }

 /**
   Sets the viewport for the camera
   */
 void cwCamera::setViewport(QRect viewport) {
    if(Viewport != viewport) {
        Viewport = viewport;
        emit viewportChanged();
        setProjection(orthoProjectionDefault());
    }
 }
