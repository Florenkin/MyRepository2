/** @file       geometry.hpp
 *  @ingroup    group_Geometry
 *  @brief      Contains definition for the reconstruction 3D library geometry classes
 *  @copyright  Multi-view_IpS
 */

#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

#include <opencv2/opencv.hpp>

#include <vector>
#include <string>
#include <omp.h>

#ifdef USE_PCL
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#endif

#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif

#include <reconstruction3d_global.h>

#include <common/debug.hpp>
#include <common/returncode.hpp>
#include <common/point_cloud/point_cloud.hpp>
#include <common/module.hpp>
#include <calibration/calibration.hpp>

#define GEOMETRY_CALIBRATION_NOT_COMPLETE                   "GEOMETRY_CALIBRATION_NOT_COMPLETE"
#define GEOMETRY_NULL_POINTER                               "GEOMETRY_NULL_POINTER"
#define GEOMETRY_SETTINGS_EMPTY                             "GEOMETRY_SETTINGS_EMPTY"
#define GEOMETRY_WITHOUT_COAXIAL_SHARED_DEVICE				"GEOMETRY_WITHOUT_COAXIAL_SHARED_DEVICE"
#define GEOMETRY_CORNERS_NOT_FOUND                          "GEOMETRY_CORNERS_NOT_FOUND"
#define GEOMETRY_POINT_CLOUD_EMPTY                          "GEOMETRY_POINT_CLOUD_EMPTY"

#define GEOMETRY_PARAMETERS_POINT_DISTANCE_MAX_MISSING                  "GEOMETRY_PARAMETERS_POINT_DISTANCE_MAX_MISSING"
#define GEOMETRY_PARAMETERS_POINT_DISTANCE_MIN_MISSING                  "GEOMETRY_PARAMETERS_POINT_DISTANCE_MIN_MISSING"
#define GEOMETRY_PARAMETERS_SMOOTH_ENABLE_MISSING                       "GEOMETRY_PARAMETERS_SMOOTH_ENABLE_MISSING"
#define GEOMETRY_PARAMETERS_SMOOTH_FILTER_RADIUS_MISSING                "GEOMETRY_PARAMETERS_SMOOTH_FILTER_RADIUS_MISSING"
#define GEOMETRY_PARAMETERS_SMOOTH_FILTER_SPACE_SIGMA_MISSING           "GEOMETRY_PARAMETERS_SMOOTH_FILTER_SPACE_SIGMA_MISSING"
#define GEOMETRY_PARAMETERS_SMOOTH_FILTER_VALUE_SIGMA_MISSING           "GEOMETRY_PARAMETERS_SMOOTH_FILTER_VALUE_SIGMA_MISSING"
#define GEOMETRY_PARAMETERS_SMOOTH_PATCH_RADIUS_MISSING                 "GEOMETRY_PARAMETERS_SMOOTH_PATCH_RADIUS_MISSING"
#define GEOMETRY_PARAMETERS_SMOOTH_PATCH_SPACE_SIGMA_MISSING            "GEOMETRY_PARAMETERS_SMOOTH_PATCH_SPACE_SIGMA_MISSING"
#define GEOMETRY_PARAMETERS_SMOOTH_REMOVE_MAX_DISTANCE_MISSING          "GEOMETRY_PARAMETERS_SMOOTH_REMOVE_MAX_DISTANCE_MISSING"
#define GEOMETRY_PARAMETERS_LINE_LINE_MIN_DISTANCE_MISSING              "GEOMETRY_PARAMETERS_LINE_LINE_MIN_DISTANCE_MISSING"
#define GEOMETRY_PARAMETERS_RECONSTRUCTION_LEAST_SQUARE_ENABLE_MISSING  "GEOMETRY_PARAMETERS_RECONSTRUCTION_LEAST_SQUARE_ENABLE_MISSING"

#define GRAY_CODE_PARAMETERS_SEQUENCE_COUNT_HORIZONTAL_MISSING      "GRAY_CODE_PARAMETERS_SEQUENCE_COUNT_HORIZONTAL_MISSING"
#define GRAY_CODE_PARAMETERS_SEQUENCE_COUNT_VERTICAL_MISSING        "GRAY_CODE_PARAMETERS_SEQUENCE_COUNT_VERTICAL_MISSING"
#define GRAY_CODE_PARAMETERS_PIXEL_THRESHOLD_MISSING                "GRAY_CODE_PARAMETERS_PIXEL_THRESHOLD_MISSING"
#define GRAY_CODE_PARAMETERS_PATTERN_INCLUDE_INVERTED_MISSING       "GRAY_CODE_PARAMETERS_PATTERN_INCLUDE_INVERTED_MISSING"

#define GEOM_FriOrient dlp::Geometry::FringeOrientation
#define GEOM_FriConfig dlp::Geometry::FringeConfiguration
#define GEOM_FriCache dlp::Geometry::FringeSolvingCache
#define GEOM_DevSet dlp::Geometry::SystemDevIDSet
#define GEOM_SteAlign dlp::Geometry::StereoAlignment
#define GEOM_SteOrient dlp::Geometry::StereoOrientation
#define GEOM_FriDeCtx dlp::Geometry::FringeDecodeContext
#define GEOM_SysConfig dlp::Geometry::ProjectorSystemConfiguration
#define GEOM_FriDeConfig dlp::Geometry::FringeDecodeConfig
#define GEOM_PCConfig dlp::Geometry::PCloudGeomConfig
#define GEOM_PCCtx dlp::Geometry::PCloudReconCtx
#define GEOM_FriDeParam dlp::Geometry::FringeDecodeParams
#define GEOM_Pair dlp::Geometry::ProjCamPair

#ifdef USE_PCL
	#define PCL3D_ITER std::vector<pcl::PointXYZRGB, Eigen::aligned_allocator<pcl::PointXYZRGB>>::iterator
#endif
#define DLP3D_ITER std::vector<dlp::Point>::iterator

#define GEOMETRY_TAN_2                  -2.18503986326152
//vertical and then horizontal
#define DIRECTIONNUM                    2
#define IMAGEPIXELMAXVALUETHRED         0xF0
#define BILATERALFILTERVALUEMAX         256
#define OVERLAPPOINTVECNUM              5

#define GEOMETRY_PHASE_STEPS            4

#define GLOBALCONSISTENCY_PLANE_NUM		3
#define POLYNOMIALORDER					4

// The models of GEOMETRICINTERSECTION and LINEARINVERSEPHASEHEIGHT are one out of three.
#ifdef RECON_WITH_CUDA
	#define LINEARINVERSEPHASEHEIGHT
	namespace GEOMCUDA {
		struct FringeDecodeConfig;
	}
#else
	// #define GEOMETRICINTERSECTION
	#define LINEARINVERSEPHASEHEIGHT
#endif

/** @brief  Contains all reconstruction 3D library classes, functions, etc. */
namespace dlp{

/** @class      Geometry
 *  @ingroup    group_Geometry
 *  @brief      Calculates real world location of points using data from
 *              decoded projection coordinate maps and calibration data
 *
 *  The geometry class calculates points in 3D space (real world) using the
 *  decoded projection coordinate map and calibration data. The geometry class 
 *  also allows setting the origin to the projector or the camera.
 *
 */
class Geometry: public dlp::Module{
public:
	/** @class      Parameters
	 *  @ingroup    group_Geometry
	 *  @brief      Parameters for configure the algorithm of getting point cloud
	 */
	class Parameters{
	public:
		DLP_NEW_PARAMETERS_ENTRY(ViewOnProjector,	"GEOMETRY_PARAMETERS_VIEW_ON_PROJECTOR",   bool, true);
		DLP_NEW_PARAMETERS_ENTRY(CentreViewOffsetX,	"GEOMETRY_PARAMETERS_CENTRE_VIEW_OFFSET_X", int, 	0);
		DLP_NEW_PARAMETERS_ENTRY(CentreViewOffsetY,	"GEOMETRY_PARAMETERS_CENTRE_VIEW_OFFSET_Y", int, 	0);
		DLP_NEW_PARAMETERS_ENTRY(CentreViewOffsetW,	"GEOMETRY_PARAMETERS_CENTRE_VIEW_OFFSET_W", int, 	0);
		DLP_NEW_PARAMETERS_ENTRY(CentreViewOffsetH,	"GEOMETRY_PARAMETERS_CENTRE_VIEW_OFFSET_H", int, 	0);

		DLP_NEW_PARAMETERS_ENTRY(PointDistanceOffsetZ,  "GEOMETRY_PARAMETERS_POINT_DISTANCE_OFFSET_Z", float,  0.0);
		DLP_NEW_PARAMETERS_ENTRY(PointDistanceMax,      "GEOMETRY_PARAMETERS_POINT_DISTANCE_MAX",      float,  0.0);
		DLP_NEW_PARAMETERS_ENTRY(PointDistanceMin,      "GEOMETRY_PARAMETERS_POINT_DISTANCE_MIN",      float,  0.0);
		DLP_NEW_PARAMETERS_ENTRY(ScaleFactorX,      	"GEOMETRY_PARAMETERS_SCALE_FACTOR_X",      float,  1.f);
		DLP_NEW_PARAMETERS_ENTRY(ScaleFactorY,      	"GEOMETRY_PARAMETERS_SCALE_FACTOR_Y",      float,  1.f);
		DLP_NEW_PARAMETERS_ENTRY(ScaleFactorZ,      	"GEOMETRY_PARAMETERS_SCALE_FACTOR_Z",      float,  1.f);


		DLP_NEW_PARAMETERS_ENTRY(GlobalConsistCorrection,	"GEOMETRY_PARAMETERS_GLOBAL_CONSISTENCY_ENABLE",	bool,   false);
		DLP_NEW_PARAMETERS_ENTRY(SmoothEnable,              "GEOMETRY_PARAMETERS_SMOOTH_ENABLE",				bool,   false);
		DLP_NEW_PARAMETERS_ENTRY(SmoothFilterRadius,        "GEOMETRY_PARAMETERS_SMOOTH_FILTER_RADIUS",			uint32_t,   3);
		DLP_NEW_PARAMETERS_ENTRY(SmoothFilterSpaceSigma,    "GEOMETRY_PARAMETERS_SMOOTH_FILTER_SPACE_SIGMA",	float,      2);
		DLP_NEW_PARAMETERS_ENTRY(SmoothFilterValueSigma,    "GEOMETRY_PARAMETERS_SMOOTH_FILTER_VALUE_SIGMA",	float,    100);
		DLP_NEW_PARAMETERS_ENTRY(SmoothPatchRadius,         "GEOMETRY_PARAMETERS_SMOOTH_PATCH_RADIUS",			uint32_t,   7);
		DLP_NEW_PARAMETERS_ENTRY(SmoothPatchSpaceSigma,     "GEOMETRY_PARAMETERS_SMOOTH_PATCH_SPACE_SIGMA",		float,      3);
		DLP_NEW_PARAMETERS_ENTRY(SmoothRemoveMaxDistance,   "GEOMETRY_PARAMETERS_SMOOTH_REMOVE_MAX_DISTANCE",	float, 300.f);

		DLP_NEW_PARAMETERS_ENTRY(OCSEpipolarConstraintEnable, 	"GEOMETRY_PARAMETERS_OPTICONSUP_EPIPOLAR_CONSTRAINT_ENABLE", 	bool,  true);
		DLP_NEW_PARAMETERS_ENTRY(OCSMonotonicityConstraintEnable,"GEOMETRY_PARAMETERS_OPTICONSUP_MONOTONICITY_CONSTRAINT_ENABLE",bool, false);
		DLP_NEW_PARAMETERS_ENTRY(OCSCollisionDetection1DEnable, 	"GEOMETRY_PARAMETERS_OPTICONSUP_COLLISION_DETECTION_1D_ENABLE", bool, false);
		DLP_NEW_PARAMETERS_ENTRY(OCSCollisionDetection2DEnable, 	"GEOMETRY_PARAMETERS_OPTICONSUP_COLLISION_DETECTION_2D_ENABLE", bool, false);
		DLP_NEW_PARAMETERS_ENTRY(OCSMinResidualThreshold, 		"GEOMETRY_PARAMETERS_OPTICONSUP_MINIMUM_RESIDUAL_THRESHOLD",   float, 	 10);
		DLP_NEW_PARAMETERS_ENTRY(OCSAmplitudeConstraintEnable, 	"GEOMETRY_PARAMETERS_OPTICONSUP_AMPLITUDE_CONSTRAINT_ENABLE", 	bool,  true);
		DLP_NEW_PARAMETERS_ENTRY(OCSMinAmplitudeThresholdEnable, "GEOMETRY_PARAMETERS_OPTICONSUP_MINIMUM_AMPLITUDE_THRESHOLD",  float,  0.5f);

		DLP_NEW_PARAMETERS_ENTRY(OCSOutputEnable, "GEOMETRY_PARAMETERS_OPTICONSUP_OUTPUT_ENABLE", bool, false);

		DLP_NEW_PARAMETERS_ENTRY(UpsampleColumns,			"GEOMETRY_PARAMETERS_UPSAMPLE_COLUMNS",       		float,	2.f);
		DLP_NEW_PARAMETERS_ENTRY(UpsampleRows,        		"GEOMETRY_PARAMETERS_UPSAMPLE_ROWS",          		float,	2.f);
		DLP_NEW_PARAMETERS_ENTRY(ProjectedSurfaceDiameter,	"GEOMETRY_PARAMETERS_PROJECTED_SURFACE_DIAMETER",	  int,	  2);

		DLP_NEW_PARAMETERS_ENTRY(GRAY_CODE_PARAMETERS_PATTERN_INCLUDE_INVERTED,	"GRAY_CODE_PARAMETERS_PATTERN_INCLUDE_INVERTED",       bool,true);
		DLP_NEW_PARAMETERS_ENTRY(GRAY_CODE_PARAMETERS_SEQUENCE_COUNT_HORIZONTAL,"GRAY_CODE_PARAMETERS_SEQUENCE_COUNT_HORIZONTAL",   uint8_t, 0xa);
		DLP_NEW_PARAMETERS_ENTRY(GRAY_CODE_PARAMETERS_SEQUENCE_COUNT_VERTICAL,	"GRAY_CODE_PARAMETERS_SEQUENCE_COUNT_VERTICAL",     uint8_t, 0x9);
		DLP_NEW_PARAMETERS_ENTRY(PHASE_SHIFTE_PARAMETERS_REPEAT_NUM_VERTICAL,	"PHASE_SHIFTE_PARAMETERS_REPEAT_NUM_VERTICAL",      uint8_t, 0x1);
		DLP_NEW_PARAMETERS_ENTRY(PHASE_SHIFTE_PARAMETERS_REPEAT_NUM_HORIZONTAL,	"PHASE_SHIFTE_PARAMETERS_REPEAT_NUM_HORIZONTAL",    uint8_t, 0x1);		
		DLP_NEW_PARAMETERS_ENTRY(GRAY_CODE_PARAMETERS_PIXEL_THRESHOLD,			"GRAY_CODE_PARAMETERS_PIXEL_THRESHOLD",            uint32_t,  10);		
	};

	struct DirPhaseHeight {
		std::string home;
		std::string calibData;

		bool isModelExist;

		void init(const std::string &root, const std::string &devID);
	};

	struct SystemDevIDSet {
		std::string sharedDevID;
		std::vector<std::string> tiltDevID_vec;
		std::string coaxDevID;

		uint32_t numDev_tilt;
		uint32_t numDev_tilt_share;
		uint32_t numDev_tilt_coax;
		uint32_t numDev;
		bool isMCamConfig;
		bool isViewOnSharedDev;

		bool hasCoaxSharedDev;
		bool hasRingLight2D;

		void init(
			const std::string &shareDevID, const std::vector<std::string> &tiltedDevID_vec, const std::string &coaxalDevID,
			const bool isViewOnShareDev, const dlp::SystemArrangementType &sysConfigType);
	};

	struct ModelPhaseHeight {
		const GEOM_DevSet *devIDSet;

		std::vector<std::vector<std::vector<cv::Point2f>>>	calibPlanes;
		std::vector<cv::Mat> masterData;
		std::vector<cv::Mat*> master;
		std::vector<cv::Mat> slave;
		
		DirPhaseHeight dirPH;

		void init(const GEOM_DevSet *dev_idSet, const std::string &root);

		bool checkPHFilesExist(const std::string& base_filename, int expected_channels);

		std::string masterID(const uint32_t idxTiltDev) const;
		std::string slaveID(const uint32_t idxTiltDev) const;

		std::string masterDirFileName(const uint32_t idxTiltDev) const;

		std::string slaveDirFileName(const uint32_t idxTiltDev) const;

		std::string dataDirFileName(const uint32_t idxTiltDev, const uint32_t pIdx) const;
	};

	/**
	 * @brief Metrics for reference-plane evaluation across multiple tilt views for phase height model.
	 */
	struct CalibPhaseHeight
	{
		std::vector<cv::Mat> MasterDataPre0;
		std::vector<cv::Mat> SlaveDataPre0;
		std::vector<cv::Mat> MasterDataPre1;
		std::vector<cv::Mat> SlaveDataPre1;

		double maeTotalPre0 = DBL_MAX;
		double maeTotalPre1 = DBL_MAX;

		double maeTotal;

		bool isSortedPlanes = false;	// determine whether the index of reference planes are sorted.
		std::vector<cv::Vec2i> sortedIdx;

		/** @brief Point-cloud diff maps for each plane (outer) and each tilt view (inner). */
		std::vector<std::vector<cv::Mat>> pcDiffOnTilt;

		/** @brief Coefficients of reference planes. */
		std::vector<cv::Mat> refPlaneCoefs;

		/** @brief Reprojection errors for each reference plane. */
		std::vector<float> refPlaneRE;

		/** @brief Mean absolute errors (X/Y/Z) per plane per view. */
		std::vector<std::vector<double>> maeX;
		std::vector<std::vector<double>> maeY;
		std::vector<std::vector<double>> maeZ;

		/** @brief Mean errors (X/Y/Z) per plane per view. */
		std::vector<std::vector<double>> meX;
		std::vector<std::vector<double>> meY;
		std::vector<std::vector<double>> meZ;

		/** @brief Total MAE per plane per view. */
		std::vector<std::vector<double>> maeXYZ;

		std::vector<cv::Mat> homoProj2SpaceXY;

		/** @brief Excel output headers for camera-color-plane logging. */
		std::vector<std::vector<std::string>> excelHeaderCamPlane;

		/**
		 * @brief Constructor.
		 * @param planeCount Number of reference planes.
		 * @param viewCount  Number of tilt views.
		 */
		CalibPhaseHeight(const uint32_t planeCount, const uint32_t viewCount)
			: pcDiffOnTilt(planeCount, std::vector<cv::Mat>(viewCount)), refPlaneCoefs(planeCount), refPlaneRE(planeCount),
			maeX(planeCount, std::vector<double>(viewCount)), maeY(planeCount, std::vector<double>(viewCount)),
			maeZ(planeCount, std::vector<double>(viewCount)), meX(planeCount, std::vector<double>(viewCount)),
			meY(planeCount, std::vector<double>(viewCount)), meZ(planeCount, std::vector<double>(viewCount)),
			maeXYZ(planeCount, std::vector<double>(viewCount)), homoProj2SpaceXY(planeCount)
		{}

		/**
		 * @brief Build the Excel header for all devices and planes.
		 * @param devSet System device information.
		 */
		void buildExcelHeader(const GEOM_DevSet &devSet);

		/**
		 * @brief Update point clouds for a specified plane across all tilted views.
		 *
		 * This function reconstructs 3D points for each pixel using either telecentric or
		 * pinhole polar line models and optionally writes results into a shared-view
		 * upsampled XYZ map.
		 *
		 * @param planeIdx     Index of the target calibration plane.
		 * @param control      Pointer to Geometry containing model parameters.
		 * @param xyz_shareView Optional output pointer for shared-view upsampled XYZ maps.
		 */
		void updatePCloud(const uint32_t planeIdx, const Geometry* control, std::vector<cv::Mat> *xyz_shareView = nullptr);

		std::string updateRefPlane(
			const uint32_t planeIdx, std::vector<cv::Point2f>& imgXY, std::vector<cv::Point2f>& spaceXY,
			const std::vector<cv::Mat>& xyz_tiltedView);

		std::string updateHomography(
        	const uint32_t planeIdx, const std::vector<cv::Point2f>& imgXY, const std::vector<cv::Point2f>& spaceXY,
        	const Geometry* control, const int loopCounter);

		void updateMetrics(const uint32_t pIdx, const Geometry *control);

		void updateAverageMAE_XYZ();

		std::string getAllErrString() const;

		void fillMAEIntoExcelData();

		bool writeExcelDataToFile(const std::string& filename) const;

		void backupPHModel(const Geometry *control, const uint32_t iteIdx);

		void updatePHModel(Geometry *control, const uint32_t iteIdx);

		void updatePlaneSequences(const uint32_t iteIdx);

		bool isSuccess(const uint32_t iteIdx);

		int planeIdx(const uint32_t idx);

		bool iteStatus(const uint32_t iteIdx);
	};

	struct FourPhaseIntensities {
		std::vector<uchar*> valueVec0;
		std::vector<uchar*> valueVec1;
		std::vector<uchar*> valueVec2;
		std::vector<uchar*> valueVec3;
		uchar value0;
		uchar value1;	
		uchar value2;
		uchar value3;

		void resize(const size_t num)
		{
			valueVec0.resize(num);
			valueVec1.resize(num);
			valueVec2.resize(num);
			valueVec3.resize(num);
		}
	};

	/** @struct     PlaneEquation
	 *  @ingroup    group_Geometry
	 *  @brief      The coefficients of plane
	 *
	 *  A*X + B*Y + C*Z = D
	 *  A = w.x
	 *  B = w.y
	 *  C = w.z
	 */
	struct PlaneEquation{
		cv::Point3f w;
		float GetA(){return w.x;}
		float GetB(){return w.y;}
		float GetC(){return w.z;}
		float d;
		cv::Point3f inPoint;
	};

	struct DisparityUnilateralPoint_Data{
		cv::Point2f         sidecamera_coordinate_undistorted_x_y;
		float               projector_plane_coordinate_origin;
		uchar               camera_gray;
		bool                valid;
	};
	/** @struct     DisparityUnilateralMap_Camera
	 *  @ingroup    group_Geometry
	 *  @brief      When unilateral scanning, this structure contains the information of each corresponding points
	 *              of cameras and projectors.
	 */
	struct DisparityUnilateralMap_Camera{
		std::vector<std::vector<DisparityUnilateralPoint_Data>> each_camera_data;
	};

	/**
	 * @enum     FringeOrientation
	 * @ingroup  group_Geometry
	 * @brief    Defines the orientation of fringe patterns for structured light projection
	 * 
	 * The orientation determines how the fringe patterns are projected and how phase
	 * information is extracted during 3D reconstruction.
	 */
	enum FringeOrientation {
		Bilateral,              ///< Fringes projected in both directions
		Horizontal,              ///< Horizontal fringe patterns (vertical phase changes)
		Vertical                ///< Vertical fringe patterns (horizontal phase changes)
	};
	/**
	 * @struct   FringeCoreParams
	 * @brief    Configuration parameters for binary code and phase-shifting patterns
	 * 
	 * Contains essential parameters defining both Gray code and phase-shifted fringe patterns
	 * used in structured light 3D reconstruction systems.
	 */
	struct FringeCoreParams {
		bool ringLight2DIncluded;
		bool binCodeInvertIncluded;		///< Flag indicating whether inverted Gray code patterns are included
		uint8_t numBinaryCodeOrderV;		///< Bit depth for vertical Gray code (2^n patterns)
		uint8_t numBinaryCodeOrderH;		///< Bit depth for horizontal Gray code (2^n patterns)
		uint8_t numPhaseRepeatV;			///< Number of phase repetitions in vertical direction
		uint8_t numPhaseRepeatH;			///< Number of phase repetitions in horizontal direction
	};

	/**
	 * @struct   FringeConfiguration
	 * @ingroup  group_Geometry
	 * @brief    Configuration parameters for structured light fringe patterns
	 * 
	 * Contains all necessary parameters to define the properties of both phase-shifted
	 * and Gray code patterns used in structured light projection.
	 */
	struct FringeConfiguration {
		FringeOrientation projectionDirection;	///< Orientation of projected fringe patterns
		FringeCoreParams coreParams;			///< Core parameters for binary and phase pattern generation

		uint8_t numPixelsPerPhasePeriodV;		///< Number of pixels per phase period in vertical direction
		uint8_t numPixelsPerPhasePeriodH;		///< Number of pixels per phase period in horizontal direction

		uint8_t numBinarySuppPic;				///< Number of additional binary patterns for calibration
		uint8_t numBinaryPicV;					///< Total number of vertical binary (Gray code) patterns
		uint8_t numBinaryPicH;					///< Total number of horizontal binary (Gray code) patterns
		uint8_t numPhasePicV;					///< Total number of vertical phase-shifting patterns
		uint8_t numPhasePicH;					///< Total number of horizontal phase-shifting patterns

		 /**
		 * @brief Configure derived parameters based on core parameter values
		 * @param[in] projectorWidth Projector display width in pixels
		 * @param[in] projectorHeight Projector display height in pixels
		 * @param[in] phaseStepCount Number of phase steps (default: GEOMETRY_PHASE_STEPS)
		 * @return bool True if configuration is valid, false otherwise
		 */
		bool configureFromCoreParams(
			const size_t projWidth, const size_t projHeight, uint8_t phaseStepCount = GEOMETRY_PHASE_STEPS);

		 /**
		 * @brief Initialize with core parameters and auto-configure
		 * @param[in] params Core parameters to initialize with
		 * @param[in] projectorWidth Projector display width in pixels
		 * @param[in] projectorHeight Projector display height in pixels
		 * @param[in] phaseStepCount Number of phase steps
		 * @return bool True if initialization successful, false otherwise
		 */
		bool initialize(
			const FringeCoreParams& params, const size_t projWidth, const size_t projHeight,  uint8_t phaseStepCount = GEOMETRY_PHASE_STEPS);
	};

	/**
	 * @struct   FringeSolvingCache
	 * @ingroup  group_Geometry
	 * @brief    Precomputed data for accelerating phase decoding in structured light systems
	 *
	 * Stores intermediate calculation results and validity masks used to optimize
	 * the phase unwrapping and decoding process from projected patterns.
	 */
	struct FringeSolvingCache  {
		cv::Mat libBinCodeVH;			
		cv::Mat libSumBinCodeValid;	
		cv::Mat libBinCodeValid;	
		cv::Mat libFourPhaseV;	
		cv::Mat libFourPhaseH;  
	};

	/**
	 * @struct   ProjectorConfiguration
	 * @ingroup  group_Geometry
	 * @brief    Complete configuration for a structured light projector
	 * 
	 * Contains all parameters and precomputed data required for a specific
	 * structured light projection setup, including pattern configuration
	 * and pre-generated data library.
	 */
	struct ProjectorConfiguration {
		std::string projectorID;			///< Unique identifier for the projector
		
		GEOM_FriConfig fringeConfig;  ///< Configuration of fringe patterns
		GEOM_FriCache fringeSolveCache; ///< Precomputed data library
	};

	/**
	 * @struct   ProjectorSystemConfiguration
	 * @ingroup  group_Geometry
	 * @brief    Configuration for a complete structured light projection system
	 * 
	 * Manages configurations for multiple projectors or a shared projector in a system.
	 */
	struct ProjectorSystemConfiguration {
		SystemArrangementType systemArrangeType;	///< Flag indicating if multiple projector configurations or a single shared projector
		std::vector<ProjectorConfiguration> projectors;  ///< Collection of projector configurations
	};

	enum StereoAlignment {
		Deg0,		// [ -22.5,  22.5]
		Deg45,		// [  22.5,  67.5]
		Deg90,		// [  67.5, 112.5]
		Deg135,		// [ 112.5, 157.5]
		Deg_180,	// [ 157.5, 180] & [-180, -157.5]
		Deg_135,	// [-157.5, -112.5]
		Deg_90,		// [-112.5,  -67.5]
		Deg_45		// [ -67.5,  -22.5]
	};

	enum StereoOrientation {
		Parallel,          // Forward aligned [ -8,  8]
		AntiParallel,      // Reverse aligned [ -180,  -172] & [172, 180]
		AcuteAngle,        // Same direction with angle [ -55,  -8] & [ 8,  55]
		ObtuseAngle,       // Opposite direction with angle [ 125, 172] & [ -172, -125]
		Invalid            // Other angles
	};

	struct FringeDecodeContext {
		std::vector<cv::Mat *> binCodeSetV;
		std::vector<cv::Mat *> phaseSetV;
		std::vector<cv::Mat *> binCodeSetH;
		std::vector<cv::Mat *> phaseSetH;
		const cv::Mat *whitePat;
		const cv::Mat *blackPat;

		cv::Mat decodedProjCoordsMapV;
		cv::Mat decodedProjCoordsMapH;

		cv::Mat residualMap;
		cv::Mat amplitudeFidelityMap;

		cv::Mat pCloudXYZ;
		cv::Mat pCloudGrayValid;
		cv::Mat pCloudError;
		cv::Mat pCloudCollisionCol;
		cv::Mat pCloudCollisionRow;

		GEOM_SteAlign steAlign;
		GEOM_SteOrient steOrientX;
		GEOM_SteOrient steOrientY;

		bool isSteAlignDiagonal = false;
		bool isSteAlignParallel = false;	// indicate the orientation in pattern
		bool isSteVertical = false;			// indicate the pattern set idx
		bool isDecodeCoordsIncreaseV = false;
		bool isDecodeCoordsIncreaseH = false;

		const cv::Mat *projMap;
		cv::Point3f projFixP;

		const cv::Mat *camMapSlave;
		const cv::Mat *camMapMaster;
		cv::Point3f camFixP;

		const ProjectorConfiguration *projConfig;

		void initializePatternSets(const std::vector<cv::Mat *> &imgSet);
	};

	struct PCloudGeomConfig {
		uint32_t ROIWidth = 0;
		uint32_t ROIHeight = 0;
		uint32_t step = 0;

		const uchar	*maskPtr = nullptr;

		bool teleMode;
    	float offsetZ;
		float scaleX;
		float scaleY;
		float scaleZ;

		bool enableSmooth;
		float smoothMaxZ;

    	const cv::Mat* mapXY;
	};

	struct PCloudReconCtx {
		GEOM_PCConfig geomConfig;

		const cv::Mat *texture = nullptr;
		cv::Mat fusedTexture;
		const cv::Rect *textureOffsetRect = nullptr;

		const std::vector<float> *patchWeights;
		const std::vector<std::vector<float>> *filterWeights;

		uint32_t removeRadius;
		std::vector<uint32_t> idxBlockOffRemove;
		uint32_t patchRadius;
		std::vector<uint32_t> idxBlockOffPatch;
		float patchSSigma;
		uint32_t filterRadius;
		std::vector<uint32_t> idxBlockOffFilter;
		float filterSSigma;
		float filterVSigma;
		
		void init(const uint32_t roiW, const uint32_t roiH, const uint32_t s, const uchar* mask);

		void initIdxBlockOffsetRemove(const uint32_t kernelRadius);

		void initIdxBlockOffsetPatch(const uint32_t kernelRadius);

		void initIdxBlockOffsetFilter(const uint32_t kernelRadius);

		void createFusedTexture_MProj(const std::vector<std::vector<cv::Mat *> > &imgSet_vec);

		bool initTexture_MCam(const std::vector<std::vector<cv::Mat *> > &imgSet_vec);
	};

	struct FringeDecodeParams {
		uchar binCodeThresh; 
		float residualThresh;
		float amplThreshMonoCstr;
		float collisionOffset;

		std::vector<int> imageOffsetVec;
		std::vector<float> scoreOffsetVec;
		
		bool enableEpiCstr;
		bool enableAmplitudeCstr;
		bool enableMonoCstr;
		bool enableCD1DCstr;
		bool enableCD2DCstr;		
		bool enableOCS;
		bool enableOCSOutput;
		
		uint32_t imageWidth;
		uint32_t imageHeight;
		bool isImageTele;
		std::vector<cv::Mat *> imageIntrinsic;

		uint32_t projWidth;
		uint32_t projHeight;
		bool isProjTele;
		std::vector<cv::Mat *> projIntrinsic;
		
		float upsampleX;
		float upsampleY;

		cv::Size projEffectSize;

		cv::Mat viewIntrinsic;

		GEOM_PCCtx pcCtx;
	};

	struct FringeDecodeConfig {
		GEOM_DevSet systemDevSet;
		GEOM_FriDeParam decodingParams;
		std::vector<GEOM_FriDeCtx> decodingContexts;

		Cloud pCloud_intermediate_;
		GEOM_PCCtx pCloud_intermediate_data_;

#ifdef RECON_WITH_CUDA
		GEOMCUDA::FringeDecodeConfig *CU_FriDeConfig_dev_ = nullptr;
		const uchar* dstPCloudPtr = nullptr;
		cv::Point2i getPCloudLoc(const uchar* curPtr) {
			cv::Point2i loc(0,0);
			if (dstPCloudPtr && curPtr) {
				const uint32_t step = decodingParams.pcCtx.geomConfig.step;
				if (step > 0) {
                    const ptrdiff_t offset = curPtr - dstPCloudPtr;
                    if (offset >= 0) {
                        // Base pixel = 8 * 4
                        const size_t pixel_idx = static_cast<size_t>(offset) / sizeof(cv::Vec<float, 8>);
                        loc.x = static_cast<int>(pixel_idx % step);
                        loc.y = static_cast<int>(pixel_idx / step);
                    }
                }
			}
			return loc;
		}

		cv::Mat interPCDataHost;
#endif

		void outputDenoiseResults() {
			if (decodingParams.enableOCSOutput) {
				auto &inPCloud = this->pCloud_intermediate_;
				cv::Mat mapPCloud(inPCloud.height, inPCloud.width, CV_32FC(8),
				inPCloud.GetPointsVec()->data());
				cv::Mat mapZ;
				cv::extractChannel(mapPCloud, mapZ, 2); 
				cv::imwrite("outputs/pCloud_remove.tiff", mapZ);
			}
		}
	};

	struct ProjCamPair {
		float projX;
		float projY;

		uint32_t camX;
		uint32_t camY;

		cv::Point3f spaceP;
		float residual;
	};

	/** @brief Default Constructor origin set = false, set name GEOMETRY_DEBUG*/
	RECONSTRUCTION3D_EXPORT Geometry();
	/** @brief Destroys object */
	RECONSTRUCTION3D_EXPORT ~Geometry();

	/** @brief Releases all cv::Mat data objects */
	RECONSTRUCTION3D_EXPORT void Clear();

	/** @brief      Extracting from outside parameters for setup.
	 *  @ingroup    group_Geometry
	 *  @param[in]  settings  \ref dlp::Parameters object
	 */
	RECONSTRUCTION3D_EXPORT ReturnCode Setup(const dlp::Parameters &settings);
	/**
	 * @brief Retrieves object settings
	 * @param[out] settings         \ref dlp::Parameters object is empty
	 * @retval GEOMETRY_NULL_POINTER    Return argument is NULL
	 */
	RECONSTRUCTION3D_EXPORT ReturnCode GetSetup(dlp::Parameters *settings) const;

	/** @brief     Save all defined parameters with default values to a text file for Geometry
	 *  @param[in] filename    Output file name
	 * @retval     RETURN_OK   File saved successfully
	 * @retval     Other error codes File save failed
	 */
	RECONSTRUCTION3D_EXPORT ReturnCode SaveDefaultParameters(const std::string &filename);
	
	/**
	 * @brief Updates all camera and projector views with calibration data
	 * 
	 * This function loads calibration data from files and initializes all system views
	 * including tilted cameras, shared projector, and optional coaxial camera.
	 * It also performs global consistency correction if enabled.
	 * 
	 * @param fileName Calibration data file path
	 * @param sharedDevID Projector identifier
	 * @param tiltedDeviceID_vec Vector of tilted camera device IDs
	 * @param tiltedViewID_vec Output vector of tilted view IDs
	 * @param coaxialSharedDevID Optional coaxial camera ID
	 * @return ReturnCode Success status or error code
	 */
	RECONSTRUCTION3D_EXPORT ReturnCode UpdateAllView(
		const std::string &fileName, 							const std::string &sharedDevID, 
		const std::vector<std::string> &tiltedDeviceID_vec, 	std::vector<uint32_t> &tiltedViewID_vec, 
		const std::vector<FringeCoreParams> &fringeParam_vec,	const std::string &coaxialSharedDevID = "");

	/**
	 * @brief Get fringe configuration for multiple camera system
	 * @param[out] projectorID Retrieved projector identifier
	 * @param[out] projectorConfig Retrieved fringe configuration
	 * @return ReturnCode Success or error information
	 */
	RECONSTRUCTION3D_EXPORT ReturnCode GetFringeConfig_MCam(std::string &projectorID, GEOM_FriConfig &projectorConfig);

	/**
	 * @brief Get fringe configurations for multiple projector system
	 * @param[out] projectorIDs Vector to store retrieved projector identifiers
	 * @param[out] projectorConfigs Vector to store retrieved fringe configurations
	 * @return ReturnCode Success or error information
	 */
	RECONSTRUCTION3D_EXPORT ReturnCode GetFringeConfig_MProj(std::vector<std::string> &projectorID_vec, std::vector<GEOM_FriConfig> &projectorConfig_vec);

	/**
	 * @brief Specifies whether the point cloud is defined in the shared device's view coordinate system.
	 */
	RECONSTRUCTION3D_EXPORT bool IsViewOnSharedDevice();

	/**
	 * @brief Extracts corner points for calibration motion from a point cloud
	 * 
	 * This function processes a point cloud to detect circular grid patterns and 
	 * computes corresponding 3D object points through bilinear interpolation.
	 * 
	 * @param srcPCloud Input point cloud containing color and spatial data
	 * @param boardSize Size of the circular grid pattern (width x height)
	 * @param imagePoints_vec Output vector of detected 2D image points
	 * @param objectPoints_vec Output vector of corresponding 3D object points
	 * @return ReturnCode Success status or error code
	 */
	RECONSTRUCTION3D_EXPORT ReturnCode GetCornersForCalibMotion(
		dlp::Cloud &srcPCloud, const cv::Size &boardSize, std::vector<cv::Point2f> &imagePoints_vec, std::vector<cv::Point3f> &objectPoints_vec);

	/**
	 * @brief Get point cloud resolution and valid region
	 * @param[out] srcPCloudWidth Point cloud width
	 * @param[out] srcPCloudHeight Point cloud height  
	 * @param[out] valid_rect Valid data region
	 * @return Return code
	 */
	RECONSTRUCTION3D_EXPORT ReturnCode GetPCloudResolution(uint32_t &srcPCloudWidth, uint32_t &srcPCloudHeight,cv::Rect &valid_rect);
	/**
	 * @brief Get tilted device resolution
	 * @param[out] width Tilted device width
	 * @param[out] height Tilted device height  
	 * @return Return code
	 */
	RECONSTRUCTION3D_EXPORT ReturnCode GetTiltedDevResolution(uint32_t &width, uint32_t &height);
	/**
	 * @brief Get coaxial shared device resolution
	 * @param[out] width Device width in pixels
	 * @param[out] height Device height in pixels  
	 * @return Return code with error if no coaxial device
	 */
	RECONSTRUCTION3D_EXPORT ReturnCode GetCoaxialSharedDevResolution(uint32_t &width, uint32_t &height);
	/**
	 * @brief Get coaxial shared device FOV resolution
	 * @param[out] width FOV width in pixels
	 * @param[out] height FOV height in pixels  
	 * @return Return code with error if no coaxial device
	 */
	RECONSTRUCTION3D_EXPORT ReturnCode GetCoaxialSharedDevResolutionFOV(uint32_t &width, uint32_t &height);
	/**
	 * @brief Get point cloud Z offset
	 * @param[out] offsetZ Z offset value (negated)
	 * @return Return code
	 */
	RECONSTRUCTION3D_EXPORT ReturnCode GetPCloudOffsetZ(float &offsetZ);
	RECONSTRUCTION3D_EXPORT ReturnCode GetViewIntrinsic(cv::Mat &intrinsic);
	RECONSTRUCTION3D_EXPORT ReturnCode GetViewIsTelecentric(bool &isTelecentric);
	RECONSTRUCTION3D_EXPORT ReturnCode GetCentreViewCentreCoordByRect(cv::Rect &rec, cv::Point2f &centre);
	RECONSTRUCTION3D_EXPORT void SetPCloudReconParams(
		const uint32_t roiW, const uint32_t roiH, const uint32_t step, const uint32_t h,
		const uchar *pCloudPtr, const uchar *maskPtr);

	/**
	 * @brief Main entry point for point cloud generation with optional smoothing path
	 * 
	 * This function routes to either the full point cloud generation pipeline or
	 * a smoothing-only path based on input availability. The smoothing-only path
	 * is used when no pattern data is provided, typically for reprocessing existing data.
	 * 
	 * @param imgSet_vec Vector of pattern image sets for projection decoding, and texture images
	 * @param tiltedViewID_vec Vector of identifiers for tilted camera views
	 * @param dstPCData Destination point cloud data structure
	 * @param dstPCloudItr Output iterator for point cloud points
	 * @param dstDepthPtr Pointer to output depth data buffer
	 * @param dstColorPtr Pointer to output color data buffer
	 * @return true if point cloud generation succeeded, false otherwise
	 */
	RECONSTRUCTION3D_EXPORT bool GeneratePointCloud(
		const std::vector<std::vector<cv::Mat *> > &imgSet_vec,	const std::vector<uint32_t> &tiltedViewID_vec,
		DLP3D_ITER dstPCloudItr, cv::Point3f *dstDepthPtr, cv::Vec3b *dstColorPtr, const cv::Mat *offsetMat);
	
	/** @brief      Point cloud calculation method for multiply scanning of different exposure.
	*  @ingroup    group_Geometry
	*  @param[in]  patternSet_vec         	source 2D image set including all cameras.
	*  @param[in]  textureImg_vec    	source color image set used for rendering point cloud.
	*  @param[in]  tiltedViewID_vec		camera order vector corresponding to 2D image set.
	*  @param[in]  is_first_set        	whether it is the first time of scanning.
	*  @param[in]  is_end_set          	whether it is the final time of scanning.
	*  @param[in]  dstPCloudItr		Pointer to be assigned to the memory of targeted point cloud.
	*  @param[in]  dstPCloudMaskPtr				The mask pointer.
	*  @param[in]  srcPCloudWidth				The total width of the targeted point cloud.
	*  @param[in]  srcPCloudHeight			The total height of the targeted point cloud.
	*  @param[in]  stepPD				The width of a FOV.
	*  @param[in]  offsetMat         The offset matrix for aligning multiple point clouds.
	*/
	RECONSTRUCTION3D_EXPORT bool GeneratePointCloud_multiply(
		const std::vector<std::vector<cv::Mat *> > &patternSet_vec, std::vector<uint32_t> &tiltedViewID_vec, bool is_first_set, bool is_end_set,
		DLP3D_ITER dstPCloudItr, uchar *dstPCloudMaskPtr, uint32_t srcPCloudWidth, uint32_t srcPCloudHeight, uint32_t stepPD, const cv::Mat *offsetMat);

	RECONSTRUCTION3D_EXPORT bool SmoothPointCloud(
		DLP3D_ITER pCloudItr, const uint32_t pdW, const uint32_t pdH, cv::Point3f *dstDepthPtr, cv::Vec3b *dstColorPtr);

	template<typename T> bool GeneratePointCloudIML_multiply(
		const std::vector<std::vector<cv::Mat *> > &patternSet_vec, std::vector<uint32_t> &tiltedViewID_vec, bool is_first_set, bool is_end_set,
		T dstPCloudItr, uchar *dstPCloudMaskPtr, uint32_t srcPCloudWidth, uint32_t srcPCloudHeight, uint32_t stepPD, const cv::Mat *offsetMat)
	{
		return false;
	}

#ifdef RECON_WITH_CUDA
	/**
	 * @brief Copy image from host to GPU device memory
	 * @param srcImg Source image matrix on host
	 * @param viewId View/camera identifier
	 * @param picId Picture/image identifier within the view
	 * 
	 * Copies image data to GPU only when necessary based on system configuration.
	 * Optimized with early exit and validation checks.
	 */
	RECONSTRUCTION3D_EXPORT void CU_CopyImageHost2Dev(cv::Mat *srcImg, uint32_t viewId, uint32_t picId);
#endif

	RECONSTRUCTION3D_EXPORT void DecodeFringeAndCompute3DPerView_multiply(const std::vector<cv::Mat *> *patternSet_vec, cv::Mat *whitePat, const uint32_t tiltedViewID);
	// For getting the reference plane (one at the edge of the working volume and anther in the middle of the calibrating volume)
	RECONSTRUCTION3D_EXPORT bool GlobalConsistency_calibrateOnce(
		const std::vector<std::vector<cv::Mat *> > &patternSet_vec, const std::vector<std::string> &tiltedDeviceID_vec, const uint32_t plane_index);
	RECONSTRUCTION3D_EXPORT bool RestoreGlobalConsistencyData(std::vector<cv::Mat> &varyPlaneUndistVH, const uint32_t planeNum, uint32_t devIdx);
	RECONSTRUCTION3D_EXPORT void DecodeGrayCodeAndPhase_bilateral_multiply(
		std::vector<cv::Mat *> *binCodeSetV, std::vector<cv::Mat *> *binCodeSetH, std::vector<cv::Mat *> *phaseSetV, std::vector<cv::Mat *> *phaseSetH,
		cv::Mat *whitePat, cv::Mat *blackPat, uint32_t viewSetID, uint32_t imageWidth, uint32_t imageHeight, uint32_t order, uint32_t threadCount);

	RECONSTRUCTION3D_EXPORT void DecodeGrayCodeAndPhase_unilateral(
		std::vector<cv::Mat *>   *patternSet_vecGrayCode,
		std::vector<cv::Mat *>   *patternSet_vecPhase,
		const cv::Mat	*whitePat,
		const cv::Mat	*blackPat,
		uint32_t                 viewSetID,
		uint32_t                 imageWidth,
		uint32_t                 imageHeight,
		uint32_t                 order,
		uint32_t                 threadCount,
		bool                     scanDirection);
	RECONSTRUCTION3D_EXPORT void DecodeGrayCodeAndPhase_unilateral_multiply(
		std::vector<cv::Mat *>	*patternSet_vecGrayCode,
		std::vector<cv::Mat *>	*patternSet_vecPhase,
		const cv::Mat					*whitePat,
		const cv::Mat					*blackPat,
		uint32_t				viewSetID,
		uint32_t				imageWidth,
		uint32_t				imageHeight,
		uint32_t				order,
		uint32_t				threadCount,
		bool					scanDirection);

private:
	/**
	 * @brief Initializes views concurrently using multi-threading
	 */
	void InitializeViewsConcurrently(
		const std::vector<std::string> &tiltedDeviceID_vec, const std::vector<uint32_t> &tiltedViewID_vec, ReturnCode &ret);

	/**
	 * @brief Sets the shared view data for the geometry system.
	 * 
	 * This function initializes the shared view parameters including shared device intrinsics,
	 * distortion coefficients, board pose, and processing settings. It also resets
	 * all tilted view data to prepare for new calibration sessions.
	 * 
	 * @param[in] origIntrinsic Original shared device intrinsic matrix (3x3)
	 * @param[in] origDistortion Shared device distortion coefficients (1xN)
	 * @param[in] firstBoardR Rotation matrix from first board to shared device
	 * @param[in] firstBoardT Translation vector from first board to shared device
	 * @param[in] imageSize Size of the input images (width x height)
	 * @param[in] upSampleX Horizontal upsampling factor
	 * @param[in] upSampleY Vertical upsampling factor
	 * @param[in] telecentricMode Flag indicating telecentric lens mode
	 * 
	 * @return ReturnCode Success status or error code
	 * @retval GEOMETRY_CALIBRATION_NOT_COMPLETE if intrinsic matrix is not provided
	 */
	ReturnCode SetSharedViewData(
		const cv::Mat &origIntrinsic, const cv::Mat &origDistortion, const cv::Mat &firstBoardR, 
		const cv::Mat &firstBoardT, const cv::Size &imageSize, const bool &telecentricMode);

	/**
	 * @brief Processes linear inverse phase height for shared device view
	 * @param sourcePoints Input normalized image coordinates
	 * @param undistortedPoints Output undistorted points
	 */
	void ProcessPhaseHeight_sharedView(const std::vector<cv::Point2f>& sourcePoints, std::vector<cv::Point2f>& undistortedPoints);

	/**
	 * @brief Processes geometric intersection for shared device view
	 * @param sourcePoints Input normalized image coordinates
	 * @param undistortedPoints Output undistorted points
	 */
	void ProcessGeometricIntersection_sharedView(const std::vector<cv::Point2f>& sourcePoints, std::vector<cv::Point2f>& undistortedPoints);

	/**
	 * @brief Processes linear inverse phase height for coaxial shared device view
	 * @param sourcePoints Input normalized image coordinates
	 * @param undistortedPoints Output undistorted points
	 * @param shared2CoaxialSharedDevR Projector to coaxial shared camera rotation
	 * @param shared2CoaxialSharedDevT Projector to coaxial shared camera translation
	 * @param originView Output origin point in coaxial shared camera coordinates
	 */
	void ProcessPhaseHeight_coaxSharedView(
		const std::vector<cv::Point2f>& sourcePoints, std::vector<cv::Point2f>& undistortedPoints,
		const cv::Mat& shared2CoaxialSharedDevR, const cv::Mat& shared2CoaxialSharedDevT, cv::Mat& originView);

	/**
	 * @brief Processes geometric intersection for coaxial shared device view
	 * @param sourcePoints Input normalized image coordinates
	 * @param undistortedPoints Output undistorted points
	 * @param shared2CoaxialSharedDevR Projector to coaxial shared camera rotation
	 * @param shared2CoaxialSharedDevT Projector to coaxial shared camera translation
	 * @param originView Output origin point in coaxial shared camera coordinates
	 */
	void ProcessGeometricIntersection_coaxSharedView(
		const std::vector<cv::Point2f>& sourcePoints, std::vector<cv::Point2f>& undistortedPoints,
		const cv::Mat& shared2CoaxialSharedDevR, const cv::Mat& shared2CoaxialSharedDevT, cv::Mat& originView);

	/**
	 * @brief Processes shared device view by generating undistorted point mapping.
	 * 
	 * Creates a grid of points in normalized coordinates and applies appropriate
	 * undistortion based on the camera model and processing method.
	 */
	void ProcessSharedView_sharedView();

	/**
	 * @brief Processes coaxial view with coordinate transformation.
	 */
	void ProcessSharedView_coaxSharedView();

	/**
	 * @brief Initializes the shared view mapping for geometry processing.
	 * 
	 * This function sets up the shared view mapping by generating undistorted point sets
	 * based on the camera model (telecentric or perspective) and processing method.
	 * It handles different configurations including LINEARINVERSEPHASEHEIGHT and GEOMETRICINTERSECTION.
	 */
	void SetSharedView();
	
	/**
	 * @brief Adds tilted view data to the geometry system
	 * @param intrinsic_view Camera intrinsic matrix
	 * @param distortion_view Camera distortion coefficients
	 * @param firstBoardR First board rotation
	 * @param firstBoardT First board translation
	 * @param tiltedViewR View rotation matrix/vector
	 * @param tiltedViewT View translation vector
	 * @param imageSize Image dimensions
	 * @param telecentricMode Telecentric mode flag
	 * @param tiltedViewID Output assigned view ID
	 * @return ReturnCode Success status
	 */
	ReturnCode AddTiltedViewData(
		const cv::Mat &tiltedIntrinsic,	const cv::Mat &tiltedDistortion,	const cv::Mat &firstBoardR,	const cv::Mat &firstBoardT, 
		const cv::Mat &tiltedViewR,		const cv::Mat &tiltedViewT,			const cv::Size &imageSize,	const bool &telecentricMode, 
		uint32_t &tiltedViewID);

	/**
	 * @brief Process geometric intersection for tilted view with telecentric or perspective projection
	 * 
	 * @param tiltedViewID View identifier
	 * @param srcPoints Input 2D points from tilted view
	 * @param undistPoints Output undistorted points
	 * @param tilt2ViewR Rotation from tilted view to reference view
	 * @param tilt2ViewT Translation from tilted view to reference view
	 */
	void ProcessGeometricIntersection_tiltedView(
		const uint32_t tiltedViewID, const std::vector<cv::Point2f>& srcPoints, std::vector<cv::Point2f>& undistPoints,
		const cv::Mat &tilt2ViewR, const cv::Mat &tilt2ViewT);

	/**
	 * @brief Process phase-height mapping for tilted view with telecentric or perspective projection
	 * 
	 * @param tiltedViewID View identifier
	 * @param srcPoints Input 2D points (phase measurements)
	 * @param undistPoints Output undistorted points for height calculation
	 */
	void ProcessPhaseHeight_tiltedView(
		const uint32_t tiltedViewID, const std::vector<cv::Point2f>& srcPoints, std::vector<cv::Point2f>& undistPoints);
	
	/**
	 * @brief Convert triangulation stereo model to phase-height model for measurement
	 * 
	 * @param tiltedViewID ID of the tilted view
	 * @param tiltedDevID Device identifier string for file naming
	 * @param undistP Pointer to undistorted image points
	 * @param other2MasterR Rotation matrix from other view to master view
	 * @param other2MasterT Translation vector from other view to master view
	 */
	void ConvertTriStereoToPHModel(
		const uint32_t tiltedViewID, const cv::Point2f *undistP, const cv::Mat& other2MasterR = cv::Mat(), const cv::Mat& other2MasterT = cv::Mat());

	/**
	 * @brief Prepares full-frame projection coordinate data for global consistency correction
	 * @param tiltedViewID The ID of the tilted view to process
	 * @param sourcePointSet Source point set for undistortion
	 */
	void PrepareFullFrameProjectionCoordinates(
		const uint32_t tiltedViewID, const std::vector<cv::Point2f>& sourcePointSet);

	/**
	 * @brief Adds and configures a tilted view for 3D reconstruction
	 * @param tiltedViewID View identifier
	 * @param tiltedDevID Device identifier
	 */
	void AddTiltedView(const uint32_t tiltedViewID, const std::string tiltedDevID, ReturnCode &ret);

	/**
	 * @brief Adds coaxial shared view data to the geometry system
	 * @param coaxialSharedIntrinsic Camera intrinsic matrix
	 * @param coaxialSharedDistortion Camera distortion coefficients
	 * @param firstBoardR First board rotation
	 * @param firstBoardT First board translation
	 * @param coaxialSharedViewR Coaxial shared view rotation
	 * @param coaxialSharedViewT Coaxial shared view translation
	 * @param imageSize Image dimensions
	 * @param telecentricMode Telecentric mode flag
	 */
	void AddCoaxialSharedViewData(
		const cv::Mat &coaxialSharedIntrinsic, 	const cv::Mat &coaxialSharedDistortion, 
		const cv::Mat &firstBoardR, 			const cv::Mat &firstBoardT, 
		const cv::Mat &coaxialSharedViewR,		const cv::Mat &coaxialSharedViewT,
		const cv::Size &imageSize, 				const bool &telecentricMode);

	/**
	 * @brief Set fringe configuration for multi-camera system
	 * @param[in] projectorID Unique identifier for the projector
	 * @param[in] fringeParams Core fringe pattern parameters
	 * @param[in] clearExisting Whether to clear existing configurations
	 * @return ReturnCode Success or error information
	 */
	ReturnCode SetFringeConfig_MCam(const std::string &projectorID, const FringeCoreParams &fringeParams, bool clearExisting);

	/**
	 * @brief Set fringe configuration for multiple projectors
	 * @param[in] projectorID_vec Vector of unique identifiers for each projector
	 * @param[in] fringeParams_vec Vector of core fringe pattern parameters for each projector
	 * @return ReturnCode Success or error information
	 */
	ReturnCode SetFringeConfig_MProj(
		const std::vector<std::string> &projectorID_vec, const std::vector<FringeCoreParams> &fringeParams_vec);

	/**
	 * @brief Configure fringe patterns based on system arrangement type
	 * @param[in] sharedDeviceID Shared device identifier
	 * @param[in] tiltedDeviceIDs Vector of tilted device identifiers
	 * @param[in] fringeParams Vector of fringe parameters
	 * @return ReturnCode Success or error information
	 */
	ReturnCode ConfigureFringePatterns(
		const std::string& sharedDeviceID, const std::vector<std::string>& tiltedDeviceIDs, const std::vector<FringeCoreParams>& fringeParams);
		
	/**
	 * @brief Initializes the geometry data library by setting up projection coordinate maps,
	 *        point cloud buffers, and precomputing filter coefficients.
	 * 
	 * This function initializes various data structures used for 3D reconstruction including:
	 * - Projection coordinate maps for tilted views
	 * - Point cloud storage for different view configurations
	 * - Bilateral filter lookup tables for image processing
	 */
	void InitDataLibrary();

	/**
	 * @brief Generates point cloud data from pattern images and texture images
	 * @param patternSet_vec Vector of pattern image sets for decoding projection coordinates. Inverted pictures included: true: no pictures;
	 * 		false: two patterns (white and black). Followed by: vertical phase set, vertical binary set, horizontal phase set, horizontal binary set.
	 * @param imgImg_vec Vector of texture images for color information, and texture images.
	 * @param tiltedViewID_vec Vector of tilted view identifiers
	 * @param dstPCData Destination point cloud data structure
	 * @param dstPCloudItr Output iterator for point cloud data
	 * @param dstDepthPtr Pointer to output depth data
	 * @param dstColorPtr Pointer to output color data
	 * @return true if successful, false otherwise
	 */
	template<typename T> bool GeneratePointCloudIML(
		const std::vector<std::vector<cv::Mat *> > &imgSet_vec, const std::vector<uint32_t> &tiltedViewID_vec,
		T dstPCloudItr, cv::Point3f *dstDepthPtr, cv::Vec3b *dstColorPtr, const cv::Mat *offsetMat);

	/**
	 * @brief Generates smoothed point cloud using IML method without pattern input
	 * 
	 * Fallback method that applies smoothing filters to existing point cloud data
	 * when no pattern images are available. Uses Gaussian patch filtering and 
	 * bilateral filtering for noise reduction and hole filling.
	 * 
	 * @tparam T Point cloud iterator type supporting random access
	 * @param[in,out] pCloudItr Point cloud iterator (input data, modified in-place with smoothing results)
	 * @param[in] pdW Width of the point cloud grid
	 * @param[in] pdH Height of the point cloud grid  
	 * @param[out] dstDepthPtr Output depth data pointer
	 * @param[out] dstColorPtr Output color data pointer
	 * @return true if smoothing was successful, false otherwise
	 */
	template<typename T> bool SmoothPointCloudIML(
		T pCloudItr, const uint32_t pdW, const uint32_t pdH, cv::Point3f *dstDepthPtr, cv::Vec3b *dstColorPtr);

#ifdef RECON_WITH_CUDA
	/**
	* @brief Generate point cloud on GPU and transfer to host memory
	* @tparam T Iterator type for point cloud output
	* @param imgSet_vec Vector of image sets for texture generation
	* @param dstPCloudStep Step size for destination point cloud
	* @param dstPCloudItr Iterator to destination point cloud
	* @param dstDepthPtr Pointer to depth data output
	* @param dstColorPtr Pointer to color data output
	* @param offsetMat Optional offset matrix
	* @return true if successful, false otherwise
	*/
	template<typename T> bool CU_GeneratePCloudIML(
		const std::vector<std::vector<cv::Mat *> > &imgSet_vec, T dstPCloudItr,
		cv::Point3f *dstDepthPtr, cv::Vec3b *dstColorPtr, const cv::Mat *offsetMat);
#endif // RECON_WITH_CUDA

private:
	GEOM_SysConfig projectorSystem_config_;
	GEOM_FriDeConfig fringe_decode_config_;

	ModelPhaseHeight modelPH_;

	cv::Size		sharedView_size_;                    /**< raw image size --shared device*/
	cv::Mat			sharedView_intrinsic_;              /**< raw intrinsic matrix --shared device*/
	cv::Mat			sharedView_isotropic_intrinsic_;    /**< raw intrinsic matrix with same aspect ratio --shared device*/
	cv::Mat			sharedView_distortion_;             /**< raw distortion matrix --shared device*/
	cv::Mat			sharedView_fromFirstBoard_R_;
	cv::Mat			sharedView_fromFirstBoard_t_;
	bool			sharedView_telecentric_mode_;        /**< is telecentric mode or not --shared device*/
	cv::Point3f		sharedView_centrePoint_;            /**< optical center, only for pinhole --shared device*/
	cv::Size		*sharedView_mapSize_;
	cv::Mat			sharedView_map_;					/**< undistorted map --shared device*/
	cv::Mat			sharedView_mapUnit_;
	cv::Mat			sharedView_mapZ1_;			/**< undistorted map, which value of z axis equal 1 --shared device*/

	std::vector<cv::Size>		tiltedViewSet_size_;					/**< raw image size --tilted devices*/
	std::vector<cv::Mat>		tiltedViewSet_intrinsic_;				/**< raw intrinsic matrix --tilted devices*/
	std::vector<cv::Mat>		tiltedViewSet_distortion_;				/**< raw distortion matrix --tilted devices*/
	std::vector<cv::Mat>		tiltedViewSet_fromFirstBoard_R_;
	std::vector<cv::Mat>		tiltedViewSet_fromFirstBoard_t_;
	std::vector<cv::Mat>		tiltedViewSet_fromSharedDev_R_;			/**< extrinsic rotation matrix from shared device to each tilted devices --tilted devices*/
	std::vector<cv::Mat>		tiltedViewSet_fromSharedDev_T_;			/**< extrinsic translation matrix from shared device to each tilted devices --tilted devices*/
	std::vector<bool>			tiltedViewSet_telecentric_mode_;		/**< telecentric mode --tilted devices*/
	std::vector<cv::Point3f>	tiltedViewSet_centrePoint_;				/**< optical center of each tilted devices --tilted devices*/
	cv::Size					*tiltedViewSet_mapSize_;
	std::vector<cv::Mat>		tiltedViewSet_map_;						/**< undistorted map --tilted devices*/
	std::vector<cv::Mat>		tiltedViewSet_mapUnit_;				/**< undistorted map unit vector --tilted devices*/

	cv::Size	coaxialSharedView_size_;				/**< raw image size --coaxial shared device*/
	cv::Mat		coaxialSharedView_intrinsic_;			/**< raw intrinsic matrix --coaxial shared device*/
	cv::Mat		coaxialSharedView_distortion_;			/**< raw distortion matrix --coaxial shared device*/
	cv::Mat		coaxialSharedView_fromFirstBoard_R_;
	cv::Mat		coaxialSharedView_fromFirstBoard_t_;
	cv::Mat		coaxialSharedView_fromSharedDev_R_;		/**< extrinsic rotation matrix from shared device to coaxial shared device --coaxial shared device*/
	cv::Mat		coaxialSharedView_fromSharedDev_T_;		/**< extrinsic translation matrix from shared device to coaxial shared device --coaxial shared device*/
#ifdef CALIBCENTRECAMWITHDIST
	cv::Mat		coaxialSharedView_map1_;                        /**< undistorted map1 --coaxial shared device*/
	cv::Mat		coaxialSharedView_map2_;                        /**< undistorted map2 --coaxial shared device*/
#endif
	cv::Point3f	coaxialSharedView_centrePoint_;			/**< optical center, only for pinhole --coaxial shared device*/
	cv::Mat		coaxialSharedView_mapZ1_;		/**< undistorted points map --coaxial shared device*/
	cv::Mat		coaxialSharedView_mapZ1_offset_copy_;
	cv::Mat		coaxialSharedView_intrinsic_offset_;
	cv::Rect	coaxialSharedView_colorMap_offset_;

	std::vector<DisparityUnilateralMap_Camera>	all_view_unilateral_decodedProjCoords_;
	std::vector<std::vector<PlaneEquation>>		all_view_unilateral_projector_planes_;
	std::vector<std::vector<cv::Point3f>>		all_view_unilateral_camera_vector_;
	std::vector<std::vector<bool>>				all_view_unilateral_is_each_cam_valid_;
	std::vector<std::vector<uchar>>				all_view_unilateral_color_vec_;

	std::vector<std::vector<float>>		library_bilateral_filter_;
	std::vector<float>					library_patch_space_;

	Parameters::PointDistanceOffsetZ    distance_offset_z_;
	Parameters::PointDistanceMax        max_distance_z_;
	Parameters::PointDistanceMin        min_distance_z_;
	Parameters::ScaleFactorX        	scale_x_;
	Parameters::ScaleFactorY        	scale_y_;
	Parameters::ScaleFactorZ        	scale_z_;

	Parameters::UpsampleColumns   proj_upsample_columns_;
	Parameters::UpsampleRows      proj_upsample_rows_;
	Parameters::ProjectedSurfaceDiameter      projected_surface_dia_;

	Parameters::SmoothEnable            smooth_enable_;
	Parameters::SmoothFilterRadius      smooth_filter_radius_;
	Parameters::SmoothFilterSpaceSigma  smooth_filter_space_sigma_;
	Parameters::SmoothFilterValueSigma  smooth_filter_value_sigma_;
	Parameters::SmoothPatchRadius       smooth_patch_radius_;
	Parameters::SmoothPatchSpaceSigma   smooth_patch_space_sigma_;
	Parameters::SmoothRemoveMaxDistance smooth_remove_max_distance_;

	Parameters::OCSEpipolarConstraintEnable   	OCS_epipolar_constraint_enable_;
	Parameters::OCSMonotonicityConstraintEnable	OCS_monotonicity_constraint_enable_;
	Parameters::OCSCollisionDetection1DEnable	OCS_collision_detection_1D_enable_;
	Parameters::OCSCollisionDetection2DEnable  	OCS_collision_detection_2D_enable_;
	Parameters::OCSMinResidualThreshold   		OCS_min_residual_threshold_;
	Parameters::OCSAmplitudeConstraintEnable   	OCS_amplitude_constraint_enable_;
	Parameters::OCSMinAmplitudeThresholdEnable  OCS_min_amplitude_threshold_;

	Parameters::OCSOutputEnable			OCS_output_enable_;
	Parameters::GlobalConsistCorrection	global_consist_correct_;

	Parameters::ViewOnProjector     is_view_on_sharedDev_;
	Parameters::CentreViewOffsetX   centre_view_offset_x_;
	Parameters::CentreViewOffsetY   centre_view_offset_y_;
	Parameters::CentreViewOffsetW   centre_view_offset_w_;
	Parameters::CentreViewOffsetH   centre_view_offset_h_;

	Parameters::GRAY_CODE_PARAMETERS_PATTERN_INCLUDE_INVERTED   include_bin_code_inverted_;
	Parameters::GRAY_CODE_PARAMETERS_SEQUENCE_COUNT_HORIZONTAL  num_binary_order_H_;
	Parameters::GRAY_CODE_PARAMETERS_SEQUENCE_COUNT_VERTICAL    num_binary_order_V_;
	Parameters::PHASE_SHIFTE_PARAMETERS_REPEAT_NUM_VERTICAL     num_phase_repeat_V_;
	Parameters::PHASE_SHIFTE_PARAMETERS_REPEAT_NUM_HORIZONTAL   num_phase_repeat_H_;		
	
	Parameters::GRAY_CODE_PARAMETERS_PIXEL_THRESHOLD            binCode_pixel_threshold_;
	

#ifdef USE_PCL
public:
	/**
	 * @brief Generates point cloud using PCL-compatible iterator interface
	 * 
	 * This function provides a PCL-compatible interface for point cloud generation,
	 * routing to either full processing or smoothing-only mode based on input patterns.
	 * It serves as an adapter between the internal geometry processing and PCL ecosystem.
	 * 
	 * @param[in] imgSet_vec Vector of pattern image sets for structured light decoding, and texture images
	 * @param[in] tiltedViewID_vec Vector of identifiers for multi-view geometry
	 * @param[in,out] dstPCData Destination point cloud data structure
	 * @param[out] dstPCloudItr PCL-compatible output iterator for point cloud data
	 * @param[out] dstDepthPtr Pointer to depth data buffer (can be nullptr if not needed)
	 * @param[out] dstColorPtr Pointer to color data buffer (can be nullptr if not needed)
	 * @return true if point cloud generation succeeded, false on error
	 */
	RECONSTRUCTION3D_EXPORT bool GeneratePointCloud_PCL(
		const std::vector<std::vector<cv::Mat *> > &imgSet_vec, const std::vector<uint32_t> &tiltedViewID_vec,
		PCL3D_ITER dstPCloudItr, cv::Point3f *dstDepthPtr, cv::Vec3b *dstColorPtr, const cv::Mat *offsetMat);
	RECONSTRUCTION3D_EXPORT bool GeneratePointCloud_multiply_PCL(
		const std::vector<std::vector<cv::Mat *> > &imgSet_vec, std::vector<uint32_t> &tiltedViewID_vec, bool is_first_set, bool is_end_set,
		PCL3D_ITER dstPCloudItr, uchar *dstPCloudMaskPtr, uint32_t srcPCloudWidth, uint32_t srcPCloudHeight, uint32_t stepPD, const cv::Mat *offsetMat);

	RECONSTRUCTION3D_EXPORT bool SmoothPointCloud_PCL(
		PCL3D_ITER pCloudItr, const uint32_t pdW, const uint32_t pdH, cv::Point3f *dstDepthPtr, cv::Vec3b *dstColorPtr);

#endif
};

} // dlp

#endif  //#ifndef GEOMETRY_HPP
