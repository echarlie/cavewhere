#ifndef CWPROJECTIMAGEPROVIDER_H
#define CWPROJECTIMAGEPROVIDER_H

//Qt includes
#include <QObject>
#include <QDeclarativeImageProvider>
#include <QMutex>

class cwProjectImageProvider : public QObject, public QDeclarativeImageProvider
{
    Q_OBJECT

public:
    static const QString Name;
    static const QByteArray Dxt1_GZ_Extension;

    cwProjectImageProvider();
    virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
    QByteArray requestImageData(int id, QSize* size, QByteArray* type = NULL);

public slots:
    void setProjectPath(QString projectPath);


private:
    static const QString RequestImageSQL;
    QString ProjectPath;
    QMutex ProjectPathMutex;

    QString projectPath() const;
};

#endif // CWPROJECTIMAGEPROVIDER_H

