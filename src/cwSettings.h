#ifndef CWSETTINGS_H
#define CWSETTINGS_H

//Qt includes
#include <QObject>

//Our includes
class cwOpenGLSettings;
class cwJobSettings;
#include "cwGlobals.h"

class CAVEWHERE_LIB_EXPORT cwSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(cwOpenGLSettings* renderingSettings READ renderingSettings CONSTANT)
    Q_PROPERTY(cwJobSettings* jobSettings READ jobSettings CONSTANT)

public:
    cwOpenGLSettings* renderingSettings() const;
    cwJobSettings* jobSettings() const;

    static void initialize();
    static cwSettings* instance();

signals:

private:
    explicit cwSettings(QObject *parent = nullptr);

    static cwSettings* Singleton;

};

#endif // CWSETTINGS_H
