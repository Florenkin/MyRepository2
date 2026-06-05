#pragma once

#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

#include <QLoggingCategory>
#include <qglobal.h>
#include <QObject>

#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif

#define IMAGING_VERSION       ""

#ifdef USE_MULTIPROJECTORS
    #define IMAGING_PLUGIN_SERVICE "com.daniel.imagingmultiproj"
#else
    #define IMAGING_PLUGIN_SERVICE "com.daniel.imagingmulticam"
#endif

#define IMAGING_PLUGIN_DIR	  "imaging"

#ifdef IMAGING_LIBRARY
#define IMAGING_EXPORT Q_DECL_EXPORT
#else
#define IMAGING_EXPORT Q_DECL_IMPORT
#endif
