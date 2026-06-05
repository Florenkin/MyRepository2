#ifndef GLOBALDATABASE_H
#define GLOBALDATABASE_H

#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

#include <QList>
#include "GlobalData.h"

#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif

namespace GlobalDatabase
{
    //QList<GlobalData::PackageData> PackageTypeDataList;      //Type
    //QList<GlobalData::PackageData> PackageInstanceDataList;  //roi list
}

#endif // GLOBALDATABASE_H
