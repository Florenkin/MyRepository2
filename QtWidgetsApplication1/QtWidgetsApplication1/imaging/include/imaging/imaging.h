/** @defgroup   group_imaging_library imaging_library
 *  @brief      Dynamic library with only top interface.
 *
 * The cameras library is a dynamic library which can detect and load plugins automatically.
 *
 * \note        It is a dynamic library which using QT plugin system.
 */

/** @file       imaging.h
 *  @brief      Global Header file for imaging library
 *  @ingroup    group_imaging_library
 *  @copyright  Multi-view_IpS
 */
#ifndef IMAGING_H
#define IMAGING_H

#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

#include <string>                                       // Included for std::string
#include <thread>                                       // Included for std::thread
#include <functional>                                   // Included for std::ref

#include <QObject>
#include <QRect>
#include <QString>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QMap>

#ifdef USE_PCL
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#endif

#include <opencv2/opencv.hpp>

#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif

#include <imaging_global.h>
#include <GlobalData.h>
#include <common/point_cloud/point_cloud.hpp>

#ifdef USE_MULTIPROJECTORS
	#define TILTEDPROJECTORIDPRENAME 	"TiltedProjID_"
	#define TILTEDPROJECTORRIGHT 		"TiltedProjID_1"
	#define TILTEDPROJECTORDOWN			"TiltedProjID_2"	
	#define TILTEDPROJECTORLEFT			"TiltedProjID_3"
	#define TILTEDPROJECTORUP			"TiltedProjID_4"

	inline QString getTiltedProjectorKeyLabel(uint32_t index)	// start from 0
    {
        return TILTEDPROJECTORIDPRENAME + QString::number(index + 1);
    }
#else
	#define TILTEDCAMERAIDPRENAME 	"TiltedCamID_"
	#define TILTEDCAMERARIGHT 		"TiltedCamID_1"
	#define TILTEDCAMERADOWN		"TiltedCamID_2"	
	#define TILTEDCAMERALEFT		"TiltedCamID_3"
	#define TILTEDCAMERAUP			"TiltedCamID_4"
	inline QString getTiltedCameraKeyLabel(uint32_t index)	// start from 0
    {
        return TILTEDCAMERAIDPRENAME + QString::number(index + 1);
    }
#endif

#ifdef USE_PCL
	#ifndef PCL3D_ITER
		#define PCL3D_ITER std::vector<pcl::PointXYZRGB, Eigen::aligned_allocator<pcl::PointXYZRGB>>::iterator
	#endif
#endif

#ifndef DLP3D_ITER
	#define DLP3D_ITER std::vector<dlp::Point>::iterator
#endif

#define NOCAMERAORPROJECTORDEVICE "no device"
#define NOMOTIONCONTROLLERDEVICE "no device"

class ImagingInterface;
class ImagingPrivate;
/** @brief  Contains all imaging classes, functions, etc. */
namespace IMGING {

/** @class      Imaging
 *  @ingroup    group_imaging_library
 *  @brief      The top interface of dynamic imaging library. You can use this to control all
 *              type of cameras and capture 3D point cloud conveniently.
 *
 */
class IMAGING_EXPORT Imaging : public QObject
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(Imaging)
public:

	/** @enum       ImagingMode
	 *  @ingroup    group_imaging_library
	 *  @brief      Imaging mode for capturing 3D Point Cloud.
	 *
	 */
	enum ImagingMode {
		InvalidMode                     = -1,                   /**< error in setting mode*/
		DirectScanMode                  = 0,
		DirectMultiScanMode             = 1,
		DirectScanContinueMode          = 2,
		DirectMultiScanContinueMode     = 3,
		DirectScanModePCL               = 4,
		DirectMultiScanModePCL          = 5,
		DirectScanContinueModePCL       = 6,
		DirectMultiScanContinueModePCL  = 7,
		ControlScanMode                 = 8,
		ControlScanModePCL              = 9,
		ControlScanPathPlannedMode		= 10,
	};
	Q_ENUM(ImagingMode)

	/** @enum      WorkState
	*   @ingroup    group_imaging_library
	*   @brief      working state of current imaging system.
	*
	*/
	enum WorkState {
		ClosedDevices   = -1,
		OpenedDevices   = 0,
		InFocusing      = 1,
		InCalibrating   = 2,
		LoadedCalibData = 3,
		InScanning      = 4,
	};

	enum {
		FILE_CALIB_CENTRE_CAMERA_READY  = 0x00001,
		FILE_CALIB_SIDE_CAMERAS_READY   = 0x00002,
		FILE_CALIB_PROJECTOR_READY      = 0x00004,
		FILE_SCAN_GEOMETRY_READY        = 0x00008,
		FILE_CALIB_DATA_READY           = 0x00010,
		FILE_ALL_READY                  = FILE_CALIB_CENTRE_CAMERA_READY|FILE_CALIB_SIDE_CAMERAS_READY|FILE_CALIB_PROJECTOR_READY|FILE_SCAN_GEOMETRY_READY|FILE_CALIB_DATA_READY,
	};

	/**
	* @struct FringeConfig
	* @brief Configuration parameters for structured light fringe pattern generation
	* @ingroup group_imaging_library
	*
	* Defines all parameters required for generating and processing structured light 
	* patterns, including both Gray code (binary) patterns and phase-shifted sinusoids.
	* Used for 3D reconstruction systems with phase-shifting profilometry.
	*/
	struct FringeConfig {
		// =============== Pattern Generation Parameters ===============
		bool binaryCodeInvertIncluded;   ///< Includes inverse patterns when true (improves robustness)
		uint8_t numBinaryCodeOrderV;     ///< Bit depth for vertical Gray code (2^n patterns)
		uint8_t numBinaryCodeOrderH;     ///< Bit depth for horizontal Gray code (2^n patterns)
		uint8_t numPhaseRepeatV;     ///< Number of phase repetitions in vertical direction
		uint8_t numPhaseRepeatH;     ///< Number of phase repetitions in horizontal direction

		// =============== Pattern Count Statistics ===============
		uint32_t numTotalPic;       ///< Total number of generated patterns
		uint8_t numBinaryPicV;      ///< Number of vertical binary patterns
		uint8_t numBinaryPicH;      ///< Number of horizontal binary patterns
		uint8_t numPhasePicV;       ///< Number of vertical phase-shift patterns
		uint8_t numPhasePicH;       ///< Number of horizontal phase-shift patterns		
		uint8_t numBinarySuppPic;        ///< Number of additional auxiliary patterns
	};

	/** @struct     ProjParameter
	*   @brief      Projector configuration parameters.
	*   @ingroup    group_imaging_library
	*
	* Contains all adjustable parameters for projector operation including timing,
 	* illumination, triggering, and pattern generation settings.
	*/
	struct ProjParameter {
		// Timing and illumination parameters
		uint32_t        frameRate;		///< Projector frame rate in Hz
		uint32_t        illuminationTime;		///< Primary illumination duration (μs)
		uint32_t        preIlluminationDarkTime;	///< Dark period before primary illumination (μs)
		uint32_t        postIlluminationDarkTime;	///< Dark period after primary illumination (μs)
		
		uint32_t		totalProjectionTime;

#ifndef USE_MULTIPROJECTORS
		uint32_t        illuminationTime_2;		///< Secondary illumination duration (μs)
		uint32_t        illuminationTime_3;		///< Tertiary illumination duration (μs)
		uint32_t        preIlluminationDarkTime_2;	///< Dark period before secondary illumination (μs)
		uint32_t        preIlluminationDarkTime_3;	///< Dark period before tertiary illumination (μs)		
		uint32_t        postIlluminationDarkTime_2;	///< Dark period after secondary illumination (μs)
		uint32_t        postIlluminationDarkTime_3;	///< Dark period after tertiary illumination (μs)

		uint32_t		totalProjectionTime_2;
		uint32_t		totalProjectionTime_3;
#endif		

		// Electrical and triggering parameters
		uint32_t        lightBlueCurrentValue;	///< LED current setting for blue light (mA)
		int             triggerOut2Delay;		///< Delay for secondary trigger output (μs)
		bool            triggerOut2Invert;		///< Invert secondary trigger polarity
		bool            triggerInPolarity;		///< Input trigger polarity setting
		bool            patternFromFile;		///< Load patterns from file if true

		FringeConfig	fringeConfig;	
	};
	/** @struct     CamParameter
	*   @brief      The parameter for setting cameras parameters.
	*   @ingroup    group_imaging_library
	*/
	struct CamParameter {
		double 	exposureTime;
		QString virtualImagePath;
	};

	/** @struct     PathPlanningROI
	*   @brief      The parameter of structuring ROI for scanning.
	*   @ingroup    group_imaging_library
	*/
	//struct PathPlanningROI {
	//	cv::          exposureTime;
	//};

	/** @struct     ParamForCamAndProj
	*   @brief      The global parameter for all devices and all configure files' path.
	*   @ingroup    group_imaging_library
	*/
	struct ParamForCamAndProj {
#ifdef USE_MULTIPROJECTORS
		uint32_t    TiltedProjNum;

		QByteArray                      SharedCamera_ID;
		QMap<QByteArray, QByteArray>    TiltedProjectors_each_ID;

		QString CurrentFilePath;
		QString ConfigFileProjector;
		QString ConfigFileCalibrationTiltedProjector;
		QString ConfigFileCalibrationSharedCamera;
		QString ConfigFileGeometry;

		QString CalibrationFilePath;
		QString CalibrationDataFileName;
		QString CalibrationCornerPointsFileName;

		CamParameter        			SharedCamSetParam;
		QMap<QByteArray, ProjParameter>	TiltedProjSetParamVec;
#else
		uint32_t    TiltedCamNum;

		QByteArray                      SharedProjector_ID;
		QByteArray                      CentreCamera_ID;
		QMap<QByteArray, QByteArray>    TiltedCameras_each_ID;

		QString CurrentFilePath;
		QString ConfigFileProjector;
		QString ConfigFileCalibrationProjector;
		QString ConfigFileCalibrationCameraSide;
		QString ConfigFileCalibrationCameraCentre;
		QString ConfigFileGeometry;

		QString CalibrationFilePath;
		QString CalibrationDataFileName;
		QString CalibrationCornerPointsFileName;

		ProjParameter       			SharedProjSetParam;
		CamParameter        			CentreCamSetParam;
		QMap<QByteArray, CamParameter>	TiltedCamSetParamVec;
#endif

		uint8_t		numColorPic;
		uint32_t	numTotalImages;

		ImagingMode ImgMode;
		QSize		CalibBoardSize;
	};

	/** @struct     HeadForPointCloud
	*   @brief      The information for point cloud.
	*   @ingroup    group_imaging_library
	*/
	struct HeadForPointCloud {
		uint32_t    resolution_width;
		uint32_t    resolution_height;

		uint32_t    FOV_resolution_uncut_width;
		uint32_t    FOV_resolution_uncut_height;

		cv::Rect    FOV_resolution_valid_rect;

		uint32_t    FOV_width;
		uint32_t    FOV_height;

		cv::Mat     virtualCam_intrinsic;	//the cropped intrinsic matrix
		std::vector<std::vector<cv::Mat>>    FOV_offset_vec;

		float       pointCloud_offset_z;
		bool        isTelecentric;
	};

	/** @struct     ZProjectionPointCloud
	*   @brief      The information for point cloud from Z projection.
	*   @ingroup    group_imaging_library
	*/
	struct ZProjectionPointCloud {
		cv::Mat     depthImage;
		cv::Mat		colorImage;

		float       x_min;
		float       y_min;
		float       x_max;
		float       y_max;

		float       x_interval;
		float       y_interval;

		float		z_mean;
	};

	/**
	* @brief The path planing item
	*/
	struct PathPlanningItem {
		QMap<QString, QVariant>		componentVec;
		QRect				minBoundingRect;
		cv::Point3f			motionStartPoint;
		cv::Point2i			memoryOffset;
	};

	/**
	* @brief The path planing struct
	*/
	struct PathPlanningStructure {
		std::vector<PathPlanningItem>		itemVec;
		cv::Point3f				motionStartPoint;
		QSize					itemExternalMemorySize;
		QPointF					itemExternalMemoryCentreScalar;
		QPointF					itemExternalZProjCentreScalar;
		QPoint					edge;

		float imageZProjMinThreshX;
		float imageZProjMinThreshY;
		float imageZProjMaxThreshX;
		float imageZProjMaxThreshY;
		float imageZProjIntervalX;
		float imageZProjIntervalY;
		float imageZProjZMean;
	};

	/** @struct     MotionPlatformScanParameter
	 *  @brief      The parameter for setting motion platform in scanning whole PCB board.
	 */
	struct MotionPlatformScanParam{
		QByteArray      motionControllerID;

		cv::Point3f     scanStartPoint;
		cv::Point3f     scanIntervalX;
		cv::Point3f     scanIntervalY;
		cv::Point3f     scanSuggestIntervalX;
		cv::Point3f     scanSuggestIntervalY;
		uint32_t        scanStepNumberX;
		uint32_t        scanStepNumberY;
		uint32_t        motionDelay;

		PathPlanningStructure	pathOptimizedStruct;

		cv::Mat         transMatrixMotion2PointCloud;
		cv::Mat         transMatrixImage2Motion;

		QString toString() {
			QString startPoint = QString("X: %0, Y: %1, Z: %2")
				.arg(scanStartPoint.x)
				.arg(scanStartPoint.y)
				.arg(scanStartPoint.z);

			QString intervalX = QString("X: %0, Y: %1, Z: %2")
				.arg(scanIntervalX.x)
				.arg(scanIntervalX.y)
				.arg(scanIntervalX.z);

			QString intervalY = QString("X: %0, Y: %1, Z: %2")
				.arg(scanIntervalY.x)
				.arg(scanIntervalY.y)
				.arg(scanIntervalY.z);

			return QString("StartPoint:%0 scanIntervalX:%1  scanIntervalY:%2  StepNumberX:%3 StepNumberY:%4 MotionDelay:%5 ")
				.arg(startPoint)
				.arg(intervalX)
				.arg(intervalY)
				.arg(scanStepNumberX)
				.arg(scanStepNumberY)
				.arg(motionDelay);
		}
	};

	/**
	 * @brief The MotionPlatformMotionParameter struct
	 */
	struct MotionPlatformMotionParam{
		uint32_t      vel;
		double        acc;
		double        dec;
		uint32_t      smoothTime;

		MotionPlatformMotionParam(){
			vel = 50;
			acc = 1;
			dec = 1;
			smoothTime = 10;
		}
	};

	explicit Imaging(QObject *parent = nullptr);
	~Imaging();

	bool Init(IMGING::Imaging::ParamForCamAndProj &paramForCP);
	bool SetParamForMotionControl(IMGING::Imaging::MotionPlatformScanParam &paramForMotion);
	//return the moving data of the motion platform and the computed rectangular in memory of components
	bool PathPlan(QList<GlobalData::PackageData> &packages, ZProjectionPointCloud &srcDepthImageStruct);
	QRect ZProjMemoryCoor2MemoryCoor(const QString &uuid, QRect &rec);
	IMGING::Imaging::MotionPlatformScanParam* GetParamForMotionControl();
	IMGING::Imaging::ParamForCamAndProj* GetParamForCamAndProj();

	WorkState GetWorkState();
	uint32_t CheckConfigureFilesReady();
	void SetDebug(bool status);
	bool OpenCamAndProj();
	bool CloseCamAndProj();
	bool CalibratePrepare();
	bool CalibratePrepareClear();
	bool CalibrateFileLoad();
	bool CalibrateCapture();
	bool CalibrateCaptureNext(uint32_t indexPicSet);
	bool CalibrateState(std::vector<bool> *statePicSet);
	bool CalibrateSystem();
	void GenerateVirtualCalibData(std::vector<cv::Mat> &shiftRVec, std::vector<cv::Mat> &shiftTVec, std::string fileName);
	bool Load();
	bool Unload();

	void UpdateCamAndProjParameters(ParamForCamAndProj *param);
	bool CalibrateMotionPlatform();
	bool CalibrateGlobalConsistency();

	bool Scan();
	bool IsVirtualMode();

	bool GetCornersForCalibMotion(dlp::Cloud &srcCloudP, std::vector<cv::Point2f> &imagePoints_vec, std::vector<cv::Point3f> &objectPoints_vec);

	void SetPointCloudCapturedCallback(void(*fun)(dlp::Cloud *, cv::Mat *, cv::Mat *));
	QWidget* MotionControlUI();

	bool SetInterestROI(std::vector<cv::Rect> &roi);
	static void ConvertDLP2Mat_32FC8(dlp::Cloud *srcCloudP, cv::Mat& dstDepthImage);
	static void ConvertDLP2ColorMap_8UC3(dlp::Cloud *srcCloudP, cv::Mat &dstColorImage);
	static void ComputePointCloudRowCol(HeadForPointCloud &pd_head, cv::Point3f srcP, std::vector<cv::Point2i> &loc);
	static void ComputePointCloudRowCol(HeadForPointCloud &pd_head, cv::Point3f srcP, std::vector<cv::Point2i> &loc, std::vector<cv::Point2i> &FOVLoc);
	bool ProjectingAllPatternsThread();
	bool CloseProjectingAllPatternsThread();

	static QList<QByteArray> UpdateAvailableDevices();
	static QList<QByteArray> UpdateAvailableMotionDevices();

	bool SetParameter(const QString key, const QVariant &value);
	QVariant GetParameter(const QString key);
	bool SetParameters(QVariantMap &paramSet);
	QVariantMap GetParameters();

	static Imaging* GetSignalInstance();

Q_SIGNALS:
	void PrintSignal(std::string );
	void Capture3DFiles(dlp::Cloud *);
	void Capture3DFiles(dlp::Cloud *, IMGING::Imaging::HeadForPointCloud*);
	void Capture3DFiles(cv::Mat *);
	void Capture3DFiles(cv::Mat *, IMGING::Imaging::HeadForPointCloud* );
	// 1:srcPD 2:head file 3:ZProjDataStruct 4,Depth 5,Color
	void Capture3DFiles(cv::Mat *, IMGING::Imaging::HeadForPointCloud*, cv::Mat *, cv::Mat *);
	void Capture2DImages(std::vector<std::vector<cv::Mat *>> *);

public:
	static void GetInvMatrix_3_4(const cv::Mat &srcMat, cv::Mat &dstMat);
	static void Mat2ZProjection_32FC8(cv::Mat &srcDepthImage, uint32_t height,  uint32_t width, ZProjectionPointCloud &dstDepthImageStruct);
	static void Mat2ZProjection_32FC8(cv::Mat &srcDepthImage, PathPlanningStructure &PathPlanningStrut, ZProjectionPointCloud &dstDepthImageStruct);
	static void Mat2ZProjection_32FC8(cv::Mat &srcDepthImage, ZProjectionPointCloud &dstDepthImageStruct);
	static void EmptyMatByPathPlan_32FC8(IMGING::Imaging::PathPlanningStructure &pathStructure, cv::Mat &dstDepthImage);
	static void ObjectCoorToProjMemoryCoor(cv::Point3f &objectCoor, ZProjectionPointCloud &dstDepthImageStruct, cv::Point2i &ProjCoor);
	static void MemoryCoorToObjectCoor(cv::Mat &srcDepthImage, cv::Point2i &srcMemoryCoor, cv::Point3f &objectCoor);
	static void MemoryCoorToProjMemoryCoor(cv::Mat &srcDepthImage, cv::Point2i &srcMemoryCoor, ZProjectionPointCloud &dstDepthImageStruct, cv::Point2i &ProjCoor);
	static void ProjMemoryCoorToMemoryCoor(cv::Point2i &ProjCoor, ZProjectionPointCloud &dstDepthImageStruct, HeadForPointCloud &pd_head, std::vector<cv::Point2i> &loc, std::vector<cv::Point2i> &FOVLoc);
	static QRect ZProjMemoryCoor2MemoryCoor(const IMGING::Imaging::MotionPlatformScanParam &paramForMotion, const QString uuid, const QRect &rec, ZProjectionPointCloud &dstDepthImageStruct, HeadForPointCloud &pd_head);
	static void SplitUpPointCloud(const cv::Mat *in_pointcloud, cv::Mat * &out_depth, cv::Mat * &out_color);

	static cv::Point2i ComputeRectCentre(cv::Rect &rec);
	static cv::Point2i ComputeRectCentre(QRect &rec);
	static cv::Rect2i GetRectByCentreSize(const cv::Point2i &centre, cv::Size2i &definedSize);
	static QRect GetRectByCentreSize(const cv::Point2i &centre, QSize &definedSize);
	static cv::Rect2i BoundingRect(std::vector<cv::Rect> &rec);
	static QRect BoundingRect(QMap<QString, QVariant> &rec);
	static cv::Rect2i BoundingRect(std::vector<cv::Rect> &rec, cv::Size2i &definedSize, cv::Point2i &definedEdge);
	static QRect BoundingRect(QMap<QString, QVariant> &rec, QSize &definedSize, QPoint &definedEdge);
	static void PathPlanningOnce(IMGING::Imaging::PathPlanningStructure &pathStructure, QSize &externalSize);
	static void PathPlanning(QMap<QString, QVariant>	&item_vec, IMGING::Imaging::PathPlanningStructure &pathStructure, QSize &externalSize);
	static void DrawPlanningStructure(cv::Mat &srcImg, IMGING::Imaging::PathPlanningStructure &pathStructure);
	static std::vector<std::vector<uint32_t>> ArrayArrangement(std::vector<uint32_t> &arr);
	static void PathPlanningStructureRearrange(IMGING::Imaging::PathPlanningStructure &pathStructure);

	static void ScalarCentreByRect(const QRect & rec, const QPointF scalarXY_memoryCoor, QPointF & centre);
	static bool PathPlan(QList<GlobalData::PackageData>& packages, ZProjectionPointCloud & srcDepthImageStruct, IMGING::Imaging::MotionPlatformScanParam & paramForMotion);
	//	Four coordinate systems:
	//	1st	Original image coordinate system.					metric coordinate
	//	2nd Memory(Image) coordinate system.					pixel coordinate
	//	3rd Z-axis projection space coordinate system.			metric coordinate
	//	4th Z-axis projection Memory(Image) coordinate system.	pixel coordinate
	//	3rd=>1st	paramForMotion.transMatrixMotion2PointCloud
	//  2nd=>3rd	paramForMotion.transMatrixImage2Motion
	//	3rd=>4th	imageZProjIntervalX and imageZProjIntervalY
	static QPointF MemoryCoor2ZProjSpaceCoor(const cv::Mat &memory2ZProj, const QPointF &point);
	static QPointF MemoryCoor2ZProjMemoryCoor(const cv::Mat &memory2ZProj, const float &x_inteval, const float &y_inteval, const QPointF &point);
	static QPointF ZProjSpaceCoor2MemoryCoor(const cv::Mat &memory2ZProj, const QPointF &point);
	static QPointF ZProjMemoryCoor2MemoryCoor(const cv::Mat &memory2ZProj, const float &x_inteval, const float &y_inteval, const QPointF &point);
	static QRect ZProjMemoryCoor2MemoryCoor(const IMGING::Imaging::MotionPlatformScanParam &paramForMotion,const QString uuid, const QRect &rec);
	static QPoint ZProjMemoryCoor2MemoryCoor(ZProjectionPointCloud &srcDepthImageStruct, HeadForPointCloud &pd_head, const QPoint &point);
	static QRect ZProjMemoryCoor2MemoryCoor(ZProjectionPointCloud &srcDepthImageStruct, HeadForPointCloud &pd_head, const QRect &rec);
	static void ConvertMat2ColorDepth_xyzrgb(const cv::Mat *srcCloudP, cv::Mat &out_depth, cv::Mat &out_color, cv::Rect selectedArea = cv::Rect());

#ifdef USE_PCL
	bool SmoothPCL(pcl::PointCloud<pcl::PointXYZRGB> *srcPointCloud);

	void SetPointCloudCapturedCallback_PCL(void(*fun)(pcl::PointCloud<pcl::PointXYZRGB> *, cv::Mat *, cv::Mat *));

	static void ConvertPCL2Mat_32FC8(const pcl::PointCloud<pcl::PointXYZRGB> *srcCloudP, cv::Mat &dstDepthImage);
	static void ConvertPCL2ColorMap_8UC3(pcl::PointCloud<pcl::PointXYZRGB> *srcCloudP, cv::Mat &dstColorImage);
	static void Convert_DLP2PCL_xyz(const dlp::Cloud& srcCloudP, pcl::PointCloud<pcl::PointXYZ> &dstCloudP);
	static void Convert_DLP2PCL_xyzrgb(const dlp::Cloud& srcCloudP, pcl::PointCloud<pcl::PointXYZRGB> &dstCloudP);
	static void Convert_PCL2ColorDepth_xyzrgb(const pcl::PointCloud<pcl::PointXYZRGB> *srcCloudP, cv::Mat &out_depth, cv::Mat &out_color, cv::Rect selectedArea = cv::Rect());
Q_SIGNALS:
	void Capture3DFilesPCL(pcl::PointCloud<pcl::PointXYZRGB> *);
	void Capture3DFilesPCL(pcl::PointCloud<pcl::PointXYZRGB> *, cv::Mat *, cv::Mat*);
#endif

public:
	ImagingPrivate *d_ptr;
};

}   // namespace IMGING

#endif //  IMAGING_H
