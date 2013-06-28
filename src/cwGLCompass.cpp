//Our includes
#include "cwGLCompass.h"
#include "cwGLShader.h"
#include "cwGlobalDirectory.h"
#include "cwShaderDebugger.h"
#include "cwCamera.h"

//Qt Includes
#include <QColor>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>



cwGLCompass::cwGLCompass(QObject *parent) :
    cwGLObject(parent)
//    Camera(NULL)
{
}

//void cwGLCompass::setCamera(cwCamera *camera)
//{

//}

///**
// * @brief cwGLCompass::camera
// * @return
// */
//cwCamera *cwGLCompass::camera() const
//{

//}

/**
 * @brief cwGLCompass::initialize
 */
void cwGLCompass::initialize()
{
    initializeGeometry();
    initializeShaders();
    initializeFramebuffer();

}

/**
 * @brief cwGLCompass::draws
 */
void cwGLCompass::draw()
{

    QMatrix4x4 rotationMatrix;
    rotationMatrix.rotate(RotationQuaternion);

    bool lookingDown = rotationMatrix.map(QVector3D(0.0, 0.0, 1.0)).z() > 0.0;

    QMatrix4x4 modelView;
    modelView.translate(0.5, 0.5, 0.0);
    modelView.rotate(RotationQuaternion);
    modelView.translate(-0.5, -0.5, 0.0);
    modelView.translate(0.0, 0.0, -0.25);

    //Draw to the framebuffers
    if(lookingDown) { drawShadow(); }
    drawCompass(CompassFramebuffer, true, rotationMatrix);

    //Draw framebuffers to rendering buffer
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);

    if(lookingDown) { drawFramebuffer(ShadowBufferFramebuffer, modelView); }
    drawFramebuffer(CompassFramebuffer);

    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);

//    //Draw the shadow
//    QMatrix4x4 orthoMatrix;
//    orthoMatrix.ortho(-1.0, 4.0, -1.0, 4.0, -1.0, 1.0);

//    QMatrix4x4 rotationMatrix;
//    rotationMatrix.translate(0.5, 0.5, 0.0);
//    rotationMatrix.rotate(RotationQuaternion);
//    rotationMatrix.translate(-0.5, -0.5, 0.0);

//    glViewport(0, 0, camera()->viewport().width(), camera()->viewport().height());

//    ShadowOutputProgram->bind();
//    ShadowOutputProgram->enableAttributeArray(vVertexShadowOutput);
//    ShadowOutputProgram->setUniformValue(ModelViewProjectionMatrixShadowOutputUniform, orthoMatrix * rotationMatrix);
//    ShadowOutputProgram->setUniformValue(TextureUnitShadowOutputUniform, 0);
//    ShadowOutputProgram->setAttributeBuffer(vVertexShadow, GL_FLOAT, 0, 2, 0);
//    glBindTexture(GL_TEXTURE_2D, ShadowBufferFramebuffer->texture());
//    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//    glBindTexture(GL_TEXTURE_2D, 0);
//    TextureGeometryBuffer.release();
//    ShadowOutputProgram->disableAttributeArray(vVertexShadow);
//    ShadowOutputProgram->release();




}

/**
 * @brief cwGLCompass::setRotation
 * @param quaternion
 *
 * Set's the rotation quaterion
 */
void cwGLCompass::setRotation(QQuaternion quaternion)
{
    RotationQuaternion = quaternion;
}

/**
 * @brief cwGLCompass::rotation
 * @return Get's the rotation quaterion
 */
QQuaternion cwGLCompass::modelView() const
{
    return RotationQuaternion;
}

/**
 * @brief cwGLCompass::initializeGeometry
 */
void cwGLCompass::initializeGeometry()
{
    QVector<CompassGeometry> allPoints;

    generateStarGeometry(allPoints, Top);
    generateStarGeometry(allPoints, Bottom);

    CompassVertexBuffer.create();
    CompassVertexBuffer.bind();
    CompassVertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    CompassVertexBuffer.allocate(allPoints.data(), allPoints.size() * sizeof(CompassGeometry));
    CompassVertexBuffer.release();

    QVector<QVector2D> textureGeometry;
    textureGeometry.append(QVector2D(0.0, 0.0));;
    textureGeometry.append(QVector2D(0.0, 1.0));
    textureGeometry.append(QVector2D(1.0, 0.0));
    textureGeometry.append(QVector2D(1.0, 1.0));

    TextureGeometryBuffer.create();
    TextureGeometryBuffer.bind();
    TextureGeometryBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    TextureGeometryBuffer.allocate(textureGeometry.data(), textureGeometry.size() * sizeof(QVector2D));
    TextureGeometryBuffer.release();

    NumberOfPoints = allPoints.size();
}

/**
 * @brief cwGLCompass::initializeShaders
 */
void cwGLCompass::initializeShaders()
{
    initializeCompassShader();
    initializeShadowShader();
    initializeShadowOutputShader();
}

/**
 * @brief cwGLCompass::initializeShadowShader
 */
void cwGLCompass::initializeShadowShader()
{
    //Setup the shadow X direction shader
    cwGLShader* vertexXShader = new cwGLShader(QOpenGLShader::Vertex);
    vertexXShader->setSourceFile(cwGlobalDirectory::baseDirectory() + "shaders/compass/CompassShadowX.vsh");

    cwGLShader* fragmentShader = new cwGLShader(QOpenGLShader::Fragment);
    fragmentShader->addDefine("HORIZONTAL_BLUR_9");
    fragmentShader->setSourceFile(cwGlobalDirectory::baseDirectory() + "shaders/compass/CompassShadow.fsh");

    XShadowProgram = new QOpenGLShaderProgram();
    XShadowProgram->addShader(vertexXShader);
    XShadowProgram->addShader(fragmentShader);

    bool success = XShadowProgram->link();
    if(!success) {
        qDebug() << "Linking errors:" << XShadowProgram->log();
    }

    shaderDebugger()->addShaderProgram(XShadowProgram);

    vVertexShadow = XShadowProgram->attributeLocation("qt_Vertex");
    TextureUnitShadow = XShadowProgram->attributeLocation("qt_Texture0");
    ModelViewProjectionMatrixShadowUniform = XShadowProgram->uniformLocation("qt_ModelViewProjectionMatrix");

    //Setup the shadow Y direction shader
    cwGLShader* vertexYShader = new cwGLShader(QOpenGLShader::Vertex);
    vertexYShader->setSourceFile(cwGlobalDirectory::baseDirectory() + "shaders/compass/CompassShadowX.vsh");

    cwGLShader* fragmentYShader = new cwGLShader(QOpenGLShader::Fragment);
    fragmentYShader->addDefine("VERTICAL_BLUR_9");
    fragmentYShader->setSourceFile(cwGlobalDirectory::baseDirectory() + "shaders/compass/CompassShadow.fsh");

    YShadowProgram = new QOpenGLShaderProgram();
    YShadowProgram->addShader(vertexYShader);
    YShadowProgram->addShader(fragmentYShader);

    success = YShadowProgram->link();
    if(!success) {
        qDebug() << "Linking errors:" << YShadowProgram->log();
    }

    shaderDebugger()->addShaderProgram(YShadowProgram);

    //If these fail you need to store vVertexShadow, TextureUnitShadow, and ModelViewProjectionMatrixShadowUniform into a different
    //texture
    Q_ASSERT(vVertexShadow == YShadowProgram->attributeLocation("qt_Vertex"));
    Q_ASSERT(TextureUnitShadow == YShadowProgram->attributeLocation("qt_Texture0"));
    Q_ASSERT(ModelViewProjectionMatrixShadowUniform == YShadowProgram->uniformLocation("qt_ModelViewProjectionMatrix"));

}

/**
 * @brief cwGLCompass::initializeShodowOutputShader
 */
void cwGLCompass::initializeShadowOutputShader()
{
    //Setup the normal shader
    cwGLShader* vertexShader = new cwGLShader(QOpenGLShader::Vertex);
    vertexShader->setSourceFile(cwGlobalDirectory::baseDirectory() + "shaders/compass/compassShadowOutput.vsh");

    cwGLShader* fragmentShader = new cwGLShader(QOpenGLShader::Fragment);
    fragmentShader->setSourceFile(cwGlobalDirectory::baseDirectory() + "shaders/compass/compassShadowOutput.fsh");

    ShadowOutputProgram = new QOpenGLShaderProgram();
    ShadowOutputProgram->addShader(vertexShader);
    ShadowOutputProgram->addShader(fragmentShader);

    bool success = ShadowOutputProgram->link();
    if(!success) {
        qDebug() << "Linking errors:" << ShadowOutputProgram->log();
    }

    shaderDebugger()->addShaderProgram(ShadowOutputProgram);

    //If these fail you need to store vVertexShadow, TextureUnitShadow, and ModelViewProjectionMatrixShadowUniform into a different
    //texture
    vVertexShadowOutput = ShadowOutputProgram->attributeLocation("qt_Vertex");
    TextureUnitShadowOutputUniform = ShadowOutputProgram->attributeLocation("qt_Texture0");
    ModelViewProjectionMatrixShadowOutputUniform = ShadowOutputProgram->uniformLocation("qt_ModelViewProjectionMatrix");
}

void cwGLCompass::initializeCompassShader()
{
    //Setup the normal shader
    cwGLShader* vertexShader = new cwGLShader(QOpenGLShader::Vertex);
    vertexShader->setSourceFile(cwGlobalDirectory::baseDirectory() + "shaders/compass/compass.vsh");

    cwGLShader* fragmentShader = new cwGLShader(QOpenGLShader::Fragment);
    fragmentShader->setSourceFile(cwGlobalDirectory::baseDirectory() + "shaders/compass/compass.fsh");

    Program = new QOpenGLShaderProgram();
    Program->addShader(vertexShader);
    Program->addShader(fragmentShader);

    bool success = Program->link();
    if(!success) {
        qDebug() << "Linking errors:" << Program->log();
    }

    shaderDebugger()->addShaderProgram(Program);

    vVertex = Program->attributeLocation("qt_Vertex");
    vColor = Program->attributeLocation("qt_Color");
    ModelViewProjectionMatrixUniform = Program->uniformLocation("qt_ModelViewProjectionMatrix");
    IgnoreColorUniform = Program->uniformLocation("ignoreColor");
}

/**
 * @brief cwGLCompass::initializeFramebuffer
 */
void cwGLCompass::initializeFramebuffer()
{
    //If it support multi-sampling
    QOpenGLFramebufferObjectFormat formatCompass;
    formatCompass.setAttachment(QOpenGLFramebufferObject::Depth);
    formatCompass.setMipmap(true);
    CompassFramebuffer = new QOpenGLFramebufferObject(QSize(512, 512), formatCompass);
    ShadowBufferFramebuffer = new QOpenGLFramebufferObject(QSize(512, 512), formatCompass);
    HorizonalShadowBufferFramebuffer = new QOpenGLFramebufferObject(QSize(512, 512), GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, CompassFramebuffer->texture());
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, ShadowBufferFramebuffer->texture());
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * @brief cwGLCompass::generateStarGeometry
 * @param triangles
 * @param colors
 */
void cwGLCompass::generateStarGeometry(QVector<CompassGeometry> &trianglesPoints,
                                       cwGLCompass::Direction direction)
{
    QVector<QVector3D> defaultPoints; //Draw with line strip
    defaultPoints.append(QVector3D(0.0, 0.0, 0.15));
    defaultPoints.append(QVector3D(-.2, 0.2, 0.0));
    defaultPoints.append(QVector3D(0.0, 0.2, 0.15));
    defaultPoints.append(QVector3D(-.2, 0.2, 0.0));
    defaultPoints.append(QVector3D(0.0, 0.2, 0.15));
    defaultPoints.append(QVector3D(0.0, 1.0, 0.0));

    QMatrix4x4 rotationMatrix;
    rotationMatrix.setToIdentity();

    QVector3D black(0.0, 0.0, 0.0); //Black
    QVector3D white(0.9, 0.9, 0.9); //White
    QVector3D currentColor;

    //Which side should be black first
    int firstBlack;

    switch(direction) {
    case Top:
        firstBlack = 0;
        break;
    case Bottom:
        firstBlack = 1;
        break;
    }

    //The top half of the compass rose
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 2; j++) {
            currentColor = j == firstBlack ? black : white;

            for(int p = 0; p < defaultPoints.size(); p++) {
                QVector3D point;

                if(i == 0 && p == 5) {
                    //Make the north more north
                    QVector3D extendendNorth = QVector3D(0.0, 1.5, 0.0);
                    point = rotationMatrix * extendendNorth;
                } else {
                    point = rotationMatrix * defaultPoints[p];
                }

                if(direction == Bottom) {
                    //The bottom of the compass rose is flat
                    point.setZ(0.0);
                }

                trianglesPoints.append(CompassGeometry(point, currentColor));
            }

            rotationMatrix.scale(-1.0, 1.0, 1.0);
        }
        rotationMatrix.rotate(90.0, 0.0, 0.0, 1.0);
    }
}

/**
 * @brief cwGLCompass::drawShadow
 *
 * This creates the shadow for the compass
 */
void cwGLCompass::drawShadow()
{

    //Save the current framebuffer so we can rebind it
    GLint previousFramebuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);

    drawCompass(ShadowBufferFramebuffer, false);

    QMatrix4x4 orthoMatrix;
    orthoMatrix.setToIdentity();
    orthoMatrix.ortho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);

    //----------- Draw the x shadow blur -----------------
    XShadowProgram->bind();
    XShadowProgram->enableAttributeArray(vVertexShadow);
    XShadowProgram->setUniformValue(ModelViewProjectionMatrixShadowUniform, orthoMatrix);
    XShadowProgram->setUniformValue(TextureUnitShadow, 0);
    TextureGeometryBuffer.bind();
    XShadowProgram->setAttributeBuffer(vVertexShadow, GL_FLOAT, 0, 2, 0);
    qDebug() << "XShadow texture:" << ShadowBufferFramebuffer->texture();
    glBindTexture(GL_TEXTURE_2D, ShadowBufferFramebuffer->texture());

    HorizonalShadowBufferFramebuffer->bind(); //Bind the horizontal framebuffer for blurring in the xDirection
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
    XShadowProgram->disableAttributeArray(vVertexShadow);
    XShadowProgram->release();

    //----------- Draw the x shadow blur -----------------
    YShadowProgram->bind();
    YShadowProgram->enableAttributeArray(vVertexShadow);
    YShadowProgram->setUniformValue(ModelViewProjectionMatrixShadowUniform, orthoMatrix);
    YShadowProgram->setUniformValue(TextureUnitShadow, 0);
    YShadowProgram->setAttributeBuffer(vVertexShadow, GL_FLOAT, 0, 2, 0);
    qDebug() << "YShadow texture:" << HorizonalShadowBufferFramebuffer->texture();

    ShadowBufferFramebuffer->bind(); //Reuse the shadowbufferframebuffer
    glBindTexture(GL_TEXTURE_2D, HorizonalShadowBufferFramebuffer->texture());
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindFramebuffer(GL_FRAMEBUFFER, previousFramebuffer); //Bind the original

    glBindTexture(GL_TEXTURE_2D, 0);
    YShadowProgram->disableAttributeArray(vVertexShadow);
    YShadowProgram->release();
}

/**
 * @brief cwGLCompass::drawCompass
 *
 * Draw the compass
 */
void cwGLCompass::drawCompass(QOpenGLFramebufferObject* framebuffer, bool withColors, QMatrix4x4 rotation)
{
    //Save the current framebuffer so we can rebind it
    GLint previousFramebuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);

    framebuffer->bind();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, framebuffer->width(), framebuffer->height());

    QMatrix4x4 orthoMatrix;
    orthoMatrix.ortho(-1.6, 1.6, -1.6, 1.6, -1.5, 1.5);

    QMatrix4x4 shadowMatrix;
    shadowMatrix.translate(0.0, 0.0, -1.0);
    shadowMatrix.scale(1.25, 1.25, 1.0);

    //----------- Draw the before framebuffer ---------------
    Program->bind();
    Program->enableAttributeArray(vVertex);
    Program->enableAttributeArray(vColor);

    Program->setUniformValue(ModelViewProjectionMatrixUniform, orthoMatrix * rotation);
    Program->setUniformValue(IgnoreColorUniform, !withColors);

    CompassVertexBuffer.bind();

    Program->setAttributeBuffer(vVertex, GL_FLOAT, offsetof(CompassGeometry, Position), 3, sizeof(CompassGeometry));
    Program->setAttributeBuffer(vColor, GL_FLOAT, offsetof(CompassGeometry, Color), 3, sizeof(CompassGeometry));

    //Draw compass rose
    glDrawArrays(GL_TRIANGLES, 0, NumberOfPoints);

    CompassVertexBuffer.release();

    Program->disableAttributeArray(vVertex);
    Program->disableAttributeArray(vColor);
    Program->release();

    glBindFramebuffer(GL_FRAMEBUFFER, previousFramebuffer);
}

void cwGLCompass::drawFramebuffer(QOpenGLFramebufferObject *framebuffer, QMatrix4x4 modelView)
{
    //Draw the shadow
    QMatrix4x4 orthoMatrix;
    orthoMatrix.ortho(-1.0, 4.0, -1.0, 4.0, -1.0, 1.0);

    glViewport(0, 0, camera()->viewport().width(), camera()->viewport().height());

    ShadowOutputProgram->bind();
    ShadowOutputProgram->enableAttributeArray(vVertexShadowOutput);
    ShadowOutputProgram->setUniformValue(ModelViewProjectionMatrixShadowOutputUniform, orthoMatrix * modelView);
    ShadowOutputProgram->setUniformValue(TextureUnitShadowOutputUniform, 0);
    TextureGeometryBuffer.bind();
    ShadowOutputProgram->setAttributeBuffer(vVertexShadow, GL_FLOAT, 0, 2, 0);
    glBindTexture(GL_TEXTURE_2D, framebuffer->texture());

    if(framebuffer->format().mipmap()) {
        qDebug() << "Generating mipmap for " << framebuffer->texture();
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
    TextureGeometryBuffer.release();
    ShadowOutputProgram->disableAttributeArray(vVertexShadow);
    ShadowOutputProgram->release();

}

