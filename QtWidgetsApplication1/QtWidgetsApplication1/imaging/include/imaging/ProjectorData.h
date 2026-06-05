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

enum Polarity
{
    ACTIVE_LOW,
    ACTIVE_HIGH
};

struct ProjectorIlluminationTime
{
    uint32_t PreDarkTimeUs;
    uint32_t ExposureTimeUs;
    uint32_t PostDarkTimeUs;

    ProjectorIlluminationTime()
    {
        PreDarkTimeUs = 0;
        ExposureTimeUs = 0;
        PostDarkTimeUs = 0;
    }
};

struct ProjectorTriggerIn
{
    bool Enable;
    Polarity Polar;

    ProjectorTriggerIn()
    {
        Enable = 0;
        Polar = ACTIVE_LOW;
    }
};

struct ProjectorPatternReady
{
    bool Enable;
    Polarity Polar;

    ProjectorPatternReady()
    {
        Enable = 0;
        Polar = ACTIVE_LOW;
    }
};

struct ProjectorTriggerOut
{
    bool Enable;
    bool Invert;
    uint32_t DelayUs;

    ProjectorTriggerOut()
    {
        Enable = false;
        Invert = false;
        DelayUs = 0;
    }
};

struct ProjectorRgbLedCurrent
{
    uint32_t RedCurrMa;
    uint32_t GreenCurrMa;
    uint32_t BlueCurrMa;

    ProjectorRgbLedCurrent()
    {
        RedCurrMa = 0;
        GreenCurrMa = 0;
        BlueCurrMa = 0;
    }
};

class ProjectorData;
struct ProjectorDataPrivate
{
    bool                            HdrEnable;
    uint32_t                        FrameRate;
    ProjectorIlluminationTime       IllumiTime;
    ProjectorTriggerIn              TriggerIn;
    ProjectorPatternReady           PatternReady;
    ProjectorTriggerOut             TriggerOut;
    ProjectorRgbLedCurrent          RgbLedCurrent;

    ProjectorData* dPtr;
    ProjectorDataPrivate(ProjectorData* ptr) :dPtr(ptr)
    {
        HdrEnable = false;
        FrameRate = 0;
    }
};

typedef std::shared_ptr<ProjectorDataPrivate> ProjectorDataPrivatePtr;
typedef std::shared_ptr<ProjectorData> ProjectorDataPtr;
extern "C" class ProjectorData
{
public:
    IMAGING_EXPORT ~ProjectorData();

    static IMAGING_EXPORT ProjectorDataPtr GetInstance()
    {
        static ProjectorDataPtr Instance(new ProjectorData());
        return Instance;
    }

    IMAGING_EXPORT bool Init();
    IMAGING_EXPORT void SaveConfig();

    IMAGING_EXPORT bool GetHdrEnable();
    IMAGING_EXPORT void SetHdrEnable(bool enable);

    IMAGING_EXPORT uint32_t GetFrameRateFs();
    IMAGING_EXPORT void SetFrameRateFs(uint32_t frame_rate);

    IMAGING_EXPORT ProjectorIlluminationTime GetIlluminationTime();
    IMAGING_EXPORT void SetIlluminationTime(const ProjectorIlluminationTime& illumi_time);

    IMAGING_EXPORT ProjectorTriggerIn GetTriggerIn();
    IMAGING_EXPORT void SetTriggerIn(const ProjectorTriggerIn& trigger_in);

    IMAGING_EXPORT ProjectorPatternReady GetPatternReady();
    IMAGING_EXPORT void SetPatternReady(const ProjectorPatternReady& pattern_ready);

    IMAGING_EXPORT ProjectorTriggerOut GetTriggerOut();
    IMAGING_EXPORT void SetTriggerOut(const ProjectorTriggerOut& trigger_out);

    IMAGING_EXPORT ProjectorRgbLedCurrent GetRgbLedCurrent();
    IMAGING_EXPORT void SetRgbLedCurrent(const ProjectorRgbLedCurrent& rgb_led_current);
    
private:
    ProjectorData();

private:
    ProjectorDataPrivatePtr mPrivateDataPtr;
    ImagingConfigsPtr mImagingConfigsPtr;
    std::mutex mMutex;
};
