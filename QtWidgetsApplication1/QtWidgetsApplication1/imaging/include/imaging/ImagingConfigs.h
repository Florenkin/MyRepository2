#pragma once

#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

#include <QSettings>

#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif

#include "imaging.h"
#include "imaging_global.h"
#include "ProjectorData.h"

class ProjectorData;
struct ProjectorIlluminationTime;
struct ProjectorTriggerIn;
struct ProjectorPatternReady;
struct ProjectorTriggerOut;
struct ProjectorRgbLedCurrent;
class ImagingConfigs;
typedef std::shared_ptr<ImagingConfigs> ImagingConfigsPtr;

extern "C" class ImagingConfigs
{
private:
	IMAGING_EXPORT ImagingConfigs();

public:
	IMAGING_EXPORT ~ImagingConfigs();

	static IMAGING_EXPORT ImagingConfigsPtr GetInstance()
	{
		static ImagingConfigsPtr Instance(new ImagingConfigs());
		return Instance;
	}

	IMAGING_EXPORT void Init();
	IMAGING_EXPORT void UpDateDevicesIds();

	/* PAUTODO DAIRONGTODO Start **/
	IMAGING_EXPORT IMGING::Imaging::ParamForCamAndProj* GetParamForCamAndProj();
	IMAGING_EXPORT IMGING::Imaging::MotionPlatformScanParam* GetMotionControlScanParam();
	IMAGING_EXPORT IMGING::Imaging::MotionPlatformScanParam* GetMotionControlCalibParam();
	IMAGING_EXPORT IMGING::Imaging::MotionPlatformMotionParam* GetMotionParam();

	/* cameras **/
	IMAGING_EXPORT uint32_t GetExposureTimeUs();
	IMAGING_EXPORT void SetExposureTimeUs(uint32_t time_us);

	/* projector **/
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

	/* Sensor device **/
	IMAGING_EXPORT void SetSensorDevCfgPath(QString &path);
	IMAGING_EXPORT QString GetSensorDevCfgPath();

	IMAGING_EXPORT void UpdateSensorDevCfgToDevParam(QString &dirPath);
	IMAGING_EXPORT bool GetProjectorEnable();
	IMAGING_EXPORT void SetProjectorEnable(bool enable);

	IMAGING_EXPORT bool GetCameraUpEnable();
	IMAGING_EXPORT void SetCameraUpEnable(QString id);
	IMAGING_EXPORT bool GetCameraDownEnable();
	IMAGING_EXPORT void SetCameraDownEnable(QString id);
	IMAGING_EXPORT bool GetCameraCentreEnable();
	IMAGING_EXPORT void SetCameraCentreEnable(QString id);
	IMAGING_EXPORT bool GetCameraLeftEnable();
	IMAGING_EXPORT void SetCameraLeftEnable(QString id);
	IMAGING_EXPORT bool GetCameraRightEnable();
	IMAGING_EXPORT void SetCameraRightEnable(QString id);

	IMAGING_EXPORT QList<QByteArray> GetRealProjectorsID();
	IMAGING_EXPORT QList<QByteArray> GetRealCamerasID();

	IMAGING_EXPORT QByteArray GetProjectorID();
	IMAGING_EXPORT void SetProjectorID(const QByteArray& id);
	IMAGING_EXPORT QByteArray GetCentreCameraID();
	IMAGING_EXPORT void SetCentreCameraID(const QByteArray &id);
	IMAGING_EXPORT QMap<QByteArray, QByteArray> GetSideCameraID();
	IMAGING_EXPORT void SetSideCameraID(const QMap<QByteArray, QByteArray> &ids);

	IMAGING_EXPORT uint32_t GetSideCameraCount();
	IMAGING_EXPORT void SetSideCameraCount(uint32_t count);

	IMAGING_EXPORT IMGING::Imaging::ImagingMode GetImagingMode();
	IMAGING_EXPORT void SetImagingMode(IMGING::Imaging::ImagingMode mode);

	IMAGING_EXPORT void SaveConfig();

	//set motion control parameter
	IMAGING_EXPORT void updateMotionPlatformScanParam(IMGING::Imaging::MotionPlatformScanParam newParameter);
	IMAGING_EXPORT void setMotionDelay(uint32_t time);

private:
	/* PAUTODO DAIRONGTODO Start **/
	void LoadingConfig();

	IMGING::Imaging::ParamForCamAndProj           m_globalParamForAOI;

	IMGING::Imaging::MotionPlatformScanParam      m_globalMotionControlScanParam;  //just init the value on load
	IMGING::Imaging::MotionPlatformScanParam      m_globalMotionControlCalibParam;

	IMGING::Imaging::MotionPlatformMotionParam    m_globalMotionParam;

	QSettings   *m_configureFile;
	QString      m_configureDirPath;

	/* PAUTODO DAIRONGTODO End **/
private:
	QList<QByteArray>   mProjectorsId;
	QList<QByteArray>   mCamerasId;
};

