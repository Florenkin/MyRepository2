#pragma once

#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

#include <memory>
#include <mutex>

#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif

#include "imaging_global.h"
#include "ImagingConfigs.h"

class ImagingConfigs;
typedef std::shared_ptr<ImagingConfigs> ImagingConfigsPtr;

class CamerasData;
struct CamerasDataPrivates
{
    uint32_t ExposureTimeUs;

    CamerasData *dPtr;
    CamerasDataPrivates(CamerasData *ptr) :dPtr(ptr)
    {
        ExposureTimeUs = 0;
    }
};

typedef std::shared_ptr<CamerasDataPrivates> CamerasDataPrivatesPtr;
typedef std::shared_ptr<CamerasData> CamerasDataPtr;
extern "C" class CamerasData
{
public:
    IMAGING_EXPORT ~CamerasData();

    static IMAGING_EXPORT CamerasDataPtr GetInstance()
    {
        static CamerasDataPtr Instance(new CamerasData());
        return Instance;
    }

public:
    IMAGING_EXPORT bool Init();
    IMAGING_EXPORT void SaveConfig();

    IMAGING_EXPORT uint32_t GetExposureTimeUs();
    IMAGING_EXPORT void SetExposureTimeUs(uint32_t time_us);

private:
    CamerasData();

private:
    CamerasDataPrivatesPtr  mPrivateDataPtr;
    ImagingConfigsPtr       mImagingConfigsPtr;
    std::mutex mMutex;
};


