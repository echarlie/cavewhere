/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

#ifndef CWGLOBALS_H
#define CWGLOBALS_H

//Qt includes
#include <QString>
#include <QFileDialog>
#include <QDir>

/**
  These are required defines for exporting symbols in the cavewhere lib for
  windows. These do nothing on other platforms like mac and linux
  */
#if defined(CAVEWHERE_LIB)
#  define CAVEWHERE_LIB_EXPORT Q_DECL_EXPORT
#else
#  define CAVEWHERE_LIB_EXPORT Q_DECL_IMPORT
#endif


class CAVEWHERE_LIB_EXPORT cwGlobals
{
public:
    cwGlobals();

    static const double PI;
    static const double RadiansToDegrees;
    static const double DegreesToRadians;

    static QString addExtension(QString filename, QString extensionHint);
    static QString convertFromURL(QString filenameUrl);
    static QString findExecutable(QStringList executables);
    static QString findExecutable(const QStringList& executables, const QList<QDir>& dirs);
    static QList<QDir> systemPaths();
    static QList<QDir> survexPath();
};

#endif // CWGLOBALS_H
