/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

#ifndef CWDISTANCEVALIDATOR_H
#define CWDISTANCEVALIDATOR_H

#include "cwValidator.h"
#include "cwGlobals.h"

class CAVEWHERE_LIB_EXPORT cwDistanceValidator : public cwValidator
{
    Q_OBJECT
public:
    explicit cwDistanceValidator(QObject *parent = 0);
    virtual State validate( QString & input, int & pos ) const;
    Q_INVOKABLE virtual int validate( QString input ) const;

    static bool check(double);

signals:

public slots:

};

/**
    Checks to make sure value is correct, greater than 0.0
  */
inline bool cwDistanceValidator::check(double value) {
    return value >= 0.0;
}


#endif // CWDISTANCEVALIDATOR_H
