#pragma once

#define RECONSTRUCTION3D_VERSION       ""

#ifdef _WIN32
#ifdef RECONSTRUCTION3D_LIBRARY
#define RECONSTRUCTION3D_EXPORT __declspec(dllexport)
#else
#define RECONSTRUCTION3D_EXPORT __declspec(dllimport)
#endif
#else
#define RECONSTRUCTION3D_EXPORT
#endif

#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
    #include <stdint.h>
    #include <direct.h>
    #define MKDIR(dir) _mkdir(dir)
    const char PATH_SEPARATOR = '\\';
#else
    #include <unistd.h>
    #include <sys/time.h>
    #include <netinet/in.h>
    #include "unistd.h"
    #include "sys/sysinfo.h"
    #include <sys/stat.h>
    #define MKDIR(dir) mkdir(dir, 0777)
    const char PATH_SEPARATOR = '/';
#endif

#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif

uint64_t RECONSTRUCTION3D_EXPORT RECON_GetTickCount();
uint32_t RECONSTRUCTION3D_EXPORT RECON_GetCPUCount();

