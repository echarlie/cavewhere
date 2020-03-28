/**************************************************************************
**
**    Copyright (C) 2015 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

//Qt includes
#include <QApplication>
#include <QThread>
#include <QThreadPool>
#include <QMetaObject>
#include <QThreadPool>

//Our includes
#include "cwSettings.h"
#include "cwTask.h"

int main( int argc, char* argv[] )
{
  QApplication app(argc, argv);

  QApplication::setOrganizationName("Vadose Solutions");
  QApplication::setOrganizationDomain("cavewhere.com");
  QApplication::setApplicationName("cavewhere-test");
  QApplication::setApplicationVersion("1.0");

  cwSettings::initialize();

  app.thread()->setObjectName("Main QThread");

  int result = 0;
  QMetaObject::invokeMethod(&app, [&result, argc, argv]() {
      result = Catch::Session().run( argc, argv );
      QThreadPool::globalInstance()->waitForDone();
      cwTask::threadPool()->waitForDone();
      QApplication::quit();
  }, Qt::QueuedConnection);

  app.exec();

  return result;
}

