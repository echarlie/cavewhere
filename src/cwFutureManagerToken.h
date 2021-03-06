#ifndef CWFUTUREMANAGERTOKEN_H
#define CWFUTUREMANAGERTOKEN_H

//Qt includes
#include <QPointer>

//Our includes
class cwFutureManagerModel;
#include "cwFuture.h"
#include "cwGlobals.h"

//This class allows for adding jobs to cwFutureManagerModel
//in a thread safe way.
class CAVEWHERE_LIB_EXPORT cwFutureManagerToken
{
public:
    cwFutureManagerToken(cwFutureManagerModel* model = nullptr);

    void addJob(const cwFuture& job);

    bool operator==(const cwFutureManagerToken& token) const;
    bool operator!=(const cwFutureManagerToken& token) const;

private:
    QPointer<cwFutureManagerModel> Model;
};

inline bool cwFutureManagerToken::operator!=(const cwFutureManagerToken &token) const
{
    return !operator==(token);
}

Q_DECLARE_METATYPE(cwFutureManagerToken);

#endif // CWFUTUREMANAGERTOKEN_H
