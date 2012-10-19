#ifndef CWIMAGETEXTURE_H
#define CWIMAGETEXTURE_H

#ifdef WIN32
//GLew includes
#include <GL/glew.h>
#endif

//Qt include
#include <QObject>
#include <QFutureWatcher>
#include <QPair>
#include <QByteArray>
#include <QSize>
#include <QGLFunctions>
#include <QOpenGLBuffer>

//Our includes
#include "cwImage.h"

class cwImageTexture : public QObject
{
    Q_OBJECT
public:
    explicit cwImageTexture(QObject *parent = 0);
    ~cwImageTexture();

    Q_PROPERTY(QString project READ project WRITE setProject NOTIFY projectChanged)
    Q_PROPERTY(cwImage image READ image WRITE setImage NOTIFY imageChanged)

    void initialize();

    void bind();
    void release();

    cwImage image() const;
    void setImage(cwImage image);

    QString project() const;
    void setProject(QString project);

    bool isDirty() const;

signals:
    void projectChanged();
    void imageChanged();
    void textureUploaded();

public slots:
    void updateData();

private:
    QString ProjectFilename; //!<
    cwImage Image; //!< The image that this texture represent

    bool TextureDirty; //!< true when the image needs to be updated
    GLuint TextureId; //!< Texture object

    QOpenGLBuffer NoteVertexBuffer; //!< The vertex buffer

    //For loading the image from disk
    QFutureWatcher<QPair<QByteArray, QSize> >* LoadNoteWatcher;

    /**
      This class allow use to load the mipmaps in a thread way
      from the database

      The project filename must be set so we can load the data
*/
    class LoadImage {
    public:
        LoadImage(QString projectFilename) :
            Filename(projectFilename) {    }

        typedef QPair<QByteArray, QSize> result_type;

        QPair<QByteArray, QSize> operator()(int imageId);

        QString Filename;
    };

    void startLoadingImage();
    void reinitilizeLoadNoteWatcher();

private slots:
    void markAsDirty();
};

/**
 * @brief cwImageTexture::isDirty
 * @return true if the texture out of date
 */
inline bool cwImageTexture::isDirty() const
{
    return TextureDirty;
}


/**
Gets project
*/
inline QString cwImageTexture::project() const {
    return ProjectFilename;
}

/**
  Gets image
  */
inline cwImage cwImageTexture::image() const {
    return Image;
}

/**
  This binds the texture to current texture unit
  */
inline void cwImageTexture::bind() {
    glBindTexture(GL_TEXTURE_2D, TextureId);
    NoteVertexBuffer.bind();
}

/**
    Releases the texture
  */
inline void cwImageTexture::release() {
    glBindTexture(GL_TEXTURE_2D, 0);
    NoteVertexBuffer.release();
}

#endif // CWIMAGETEXTURE_H
