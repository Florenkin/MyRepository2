/** @file       calibration.hpp
 *  @ingroup    group_Calibration
 *  @brief      Contains definitions for calibrating tiltedDevices and sharedDevices
 *  @copyright  Multi-view_IpS
 *
 *  The calibration.hpp file defines the tiltedDevice and sharedDevice calibration
 *  models for use in 3D reconstruction applications using projecting pattern
 *  technology. The following objects are defined:
 *
 *  - dlp::Calibration::Data
 *  - dlp::Calibration::TiltedDevice
 *  - dlp::Calibration::SharedDevice
 *
 */

#ifndef CALIBRATION_HPP
#define CALIBRATION_HPP

#include <reconstruction3d_global.h>

// DLP Structured Light SDK header files
#include <common/debug.hpp>                     // Adds dlp::Debug
#include <common/module.hpp>                    // Adds dlp::Module
#include <common/returncode.hpp>                // Adds dlp::ReturnCode
#include <common/parameters.hpp>                // Adds dlp::Parameter

#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

// OpenCV header files
#include <opencv2/opencv.hpp>                   // Adds OpenCV image container

// C++ standard header files
#include <vector>                               // Adds std::vector
#include <string>                               // Adds std::string
#include <thread>

#include<iostream>
#include<fstream>

#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif

#include <calibration/multicalib.hpp>

// Shortcut cv::Mat definitions for calibration matrices
// when using cv::Mat::create()  rows, columns, data format
#define DLP_CV_INTRINSIC_SETUP          3, 3, CV_64FC1
#define DLP_CV_EXTRINSIC_SETUP          2, 3, CV_64FC1
#define DLP_CV_DISTORTION_SETUP         14, 1, CV_64FC1
#define DLP_CV_HOMOGRAPHY_SETUP         3, 3, CV_64FC1
#define DLP_CV_OFFSET_TRANSFORM_SETUP   4, 4, CV_64FC1

// DLP Structured Light error messages for dlp::ReturnCodes
#define CALIBRATION_DATA_NULL_POINTER_INTRINSIC             "CALIBRATION_DATA_NULL_POINTER_INTRINSIC"
#define CALIBRATION_DATA_NULL_POINTER_EXTRINSIC             "CALIBRATION_DATA_NULL_POINTER_EXTRINSIC"
#define CALIBRATION_DATA_NULL_POINTER_DISTORTION            "CALIBRATION_DATA_NULL_POINTER_DISTORTION"
#define CALIBRATION_DATA_NULL_POINTER_REPROJECTION_ERROR    "CALIBRATION_DATA_NULL_POINTER_REPROJECTION_ERROR"
#define CALIBRATION_DATA_NULL_POINTER_OFFSET_TRANSFORM      "CALIBRATION_DATA_NULL_POINTER_OFFSET_TRANSFORM"

#define CALIBRATION_DATA_NULL_POINTER_COLUMNS               "CALIBRATION_DATA_NULL_POINTER_COLUMNS"
#define CALIBRATION_DATA_NULL_POINTER_ROWS                  "CALIBRATION_DATA_NULL_POINTER_ROWS"
#define CALIBRATION_DATA_NOT_COMPLETE                       "CALIBRATION_DATA_NOT_COMPLETE"

#define CALIBRATION_DATA_FILE_EXTENSION_INVALID             "CALIBRATION_DATA_FILE_EXTENSION_INVALID"
#define CALIBRATION_DATA_FILE_SAVE_FAILED                   "CALIBRATION_DATA_FILE_SAVE_FAILED"
#define CALIBRATION_DATA_FILE_LOAD_FAILED                   "CALIBRATION_DATA_FILE_LOAD_FAILED"
#define CALIBRATION_DATA_FILE_INVALID                       "CALIBRATION_DATA_FILE_INVALID"

#define CALIBRATION_NOT_SETUP                               "CALIBRATION_NOT_SETUP"
#define CALIBRATION_NOT_COMPLETE                            "CALIBRATION_NOT_COMPLETE"
#define CALIBRATION_NOT_COAXIAL_SHAREDDEVICE                "CALIBRATION_NOT_COAXIAL_SHAREDDEVICE"
#define CALIBRATION_NOT_FROM_TILTEDDEVICE                   "CALIBRATION_NOT_FROM_TILTEDDEVICE"
#define CALIBRATION_NULL_POINTER_SETTINGS                   "CALIBRATION_NULL_POINTER_SETTINGS"
#define CALIBRATION_NULL_POINTER_SUCCESS                    "CALIBRATION_NULL_POINTER_SUCCESS"
#define CALIBRATION_NULL_POINTER_SUCCESSFUL                 "CALIBRATION_NULL_POINTER_SUCCESSFUL"
#define CALIBRATION_NULL_POINTER_TOTAL_REQUIRED             "CALIBRATION_NULL_POINTER_TOTAL_REQUIRED"
#define CALIBRATION_NULL_POINTER_DATA                       "CALIBRATION_NULL_POINTER_DATA"
#define CALIBRATION_NULL_POINTER_CALIBRATION_IMAGE          "CALIBRATION_NULL_POINTER_CALIBRATION_IMAGE"
#define CALIBRATION_NULL_POINTER_REPROJECTION_ERROR         "CALIBRATION_NULL_POINTER_REPROJECTION_ERROR"
#define CALIBRATION_NULL_POINTER_PROJECTED_BOARD            "CALIBRATION_NULL_POINTER_PROJECTED_BOARD"

#define CALIBRATION_PARAMETERS_IMAGE_SIZE_MISSING                         "CALIBRATION_PARAMETERS_IMAGE_SIZE_MISSING"
#define CALIBRATION_PARAMETERS_FOCAL_LENGTH_MISSING                       "CALIBRATION_PARAMETERS_FOCAL_LENGTH_MISSING"
#define CALIBRATION_PARAMETERS_PIXEL_SIZE_MISSING                         "CALIBRATION_PARAMETERS_PIXEL_SIZE_MISSING"

#define CALIBRATION_PARAMETERS_BOARD_TYPE_MISSING                         "CALIBRATION_PARAMETERS_BOARD_TYPE_MISSING"
#define CALIBRATION_PARAMETERS_NUMBER_BOARDS_MISSING                      "CALIBRATION_PARAMETERS_NUMBER_BOARDS_MISSING"

#define CALIBRATION_PARAMETERS_BOARD_TYPE_MISSING                         "CALIBRATION_PARAMETERS_BOARD_TYPE_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_TYPE_INVALID                         "CALIBRATION_PARAMETERS_BOARD_TYPE_INVALID"
#define CALIBRATION_PARAMETERS_BOARD_FEATURE_SIZE_MISSING                 "CALIBRATION_PARAMETERS_BOARD_FEATURE_SIZE_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_FEATURE_SIZE_INVALID                 "CALIBRATION_PARAMETERS_BOARD_FEATURE_SIZE_INVALID"
#define CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_MISSING             "CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_INVALID             "CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_INVALID"
#define CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_IN_PIXELS_MISSING   "CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_IN_PIXELS_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_IN_PIXELS_INVALID   "CALIBRATION_PARAMETERS_BOARD_FEATURE_DISTANCE_IN_PIXELS_INVALID"
#define CALIBRATION_PARAMETERS_PATTERN_TYPE_NOT_SUPPORTED                 "CALIBRATION_PARAMETERS_PATTERN_TYPE_NOT_SUPPORTED"
#define CALIBRATION_PARAMETERS_PATTERN_SIZE_MISSING                       "CALIBRATION_PARAMETERS_PATTERN_SIZE_MISSING"
#define CALIBRATION_PARAMETERS_PATTERN_SIZE_INVALID                       "CALIBRATION_PARAMETERS_PATTERN_SIZE_INVALID"
#define CALIBRATION_PARAMETERS_PATTERN_POINT_DISTANCE_MISSING             "CALIBRATION_PARAMETERS_PATTERN_POINT_DISTANCE_MISSING"
#define CALIBRATION_PARAMETERS_PATTERN_POINT_DISTANCE_INVALID             "CALIBRATION_PARAMETERS_PATTERN_POINT_DISTANCE_INVALID"
#define CALIBRATION_PARAMETERS_PATTERN_POINT_LOCATION_OUT_OF_RANGE        "CALIBRATION_PARAMETERS_PATTERN_POINT_LOCATION_OUT_OF_RANGE"
#define CALIBRATION_PARAMETERS_PATTERN_BORDER_DISTANCE_MISSING            "CALIBRATION_PARAMETERS_PATTERN_BORDER_DISTANCE_MISSING"
#define CALIBRATION_PARAMETERS_PATTERN_BORDER_DISTANCE_INVALID            "CALIBRATION_PARAMETERS_PATTERN_BORDER_DISTANCE_INVALID"
#define CALIBRATION_PARAMETERS_TANGENT_DISTORTION_MISSING                 "CALIBRATION_PARAMETERS_TANGENT_DISTORTION_MISSING"
#define CALIBRATION_PARAMETERS_TILTED_MODEL_DISTORTION_MISSING            "CALIBRATION_PARAMETERS_TILTED_MODEL_DISTORTION_MISSING"
#define CALIBRATION_PARAMETERS_SIXTH_ORDER_DISTORTION_MISSING             "CALIBRATION_PARAMETERS_SIXTH_ORDER_DISTORTION_MISSING"
#define CALIBRATION_PARAMETERS_FIX_ASPECT_RATIO_MISSING                   "CALIBRATION_PARAMETERS_FIX_ASPECT_RATIO_MISSING"
#define CALIBRATION_PARAMETERS_FOURTH_ORDER_DISTORTION_MISSING            "CALIBRATION_PARAMETERS_FOURTH_ORDER_DISTORTION_MISSING"
#define CALIBRATION_PARAMETERS_SECOND_ORDER_DISTORTION_MISSING            "CALIBRATION_PARAMETERS_SECOND_ORDER_DISTORTION_MISSING"
#define CALIBRATION_PARAMETERS_USE_INTRISIC_GUESS_MISSING                 "CALIBRATION_PARAMETERS_USE_INTRISIC_GUESS_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_CLUSTERING_MISSING      "CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_CLUSTERING_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_SYMMETRIC_MISSING       "CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_SYMMETRIC_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_FILTER_MIN_AREA_MISSING "CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_FILTER_MIN_AREA_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_FILTER_MAX_AREA_MISSING "CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_FILTER_MAX_AREA_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_MIN_THRESH_MISSING      "CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_MIN_THRESH_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_MAX_THRESH_MISSING      "CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_MAX_THRESH_MISSING"
#define CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_BLOB_COLOR_MISSING      "CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_BLOB_COLOR_MISSING"

#define CALIBRATION_TILTEDDEVICE_CALIBRATION_MISSING                  "CALIBRATION_TILTEDDEVICE_CALIBRATION_MISSING"
#define CALIBRATION_TILTEDDEVICE_CALIBRATION_HOMOGRAPHIES_MISSING     "CALIBRATION_TILTEDDEVICE_CALIBRATION_HOMOGRAPHIES_MISSING"

#define CALIB_PHASE_STEPS    4
//#define CALIBCENTRECAMWITHDIST

/** @brief  Contains all reconstruction 3D library classes, functions, etc. */
namespace dlp{

/**
 * @enum       SystemArrangementType  
 * @ingroup    group_Calibration
 * @brief      Definition of system arrangement types combining camera/projector count and DMD mirror type
 */
enum SystemArrangementType {
	INVALID,             /*!< 0 - Invalid system configuration */

    // MCSP = Multi-Camera Single-Projector
    MCSP_ORTHOGONAL,     /*!< 1 - Multi-camera single-projector with orthogonal DMD */
    MCSP_DIAMOND,        /*!< 2 - Multi-camera single-projector with diamond DMD    */
    
    // SCMP = Single-Camera Multi-Projector  
    SCMP_ORTHOGONAL,     /*!< 3 - Single-camera multi-projector with orthogonal DMD */
    SCMP_DIAMOND         /*!< 4 - Single-camera multi-projector with diamond DMD    */
};

/**
 * @enum CalibrationBoardType
 * @brief Enumeration defining different types of calibration targets
 * @ingroup group_Calibration
 */
enum CalibrationBoardType {
    NOPATTERN,		/*!< 0: Only monochromic pattern */

    CHESSBOARD,     /*!< 1: Chessboard pattern with alternating squares */
    CIRCLE,         /*!< 2: Grid of circular markers */
    RING            /*!< 3: Concentric ring pattern */
};

/** @class      Calibration
 *  @ingroup    group_Calibration
 *  @brief      Umbrella class for calibration data, tiltedDevice, and sharedDevice routines
 *              using the OpenCV.
 *
 *  The Calibration class contains classes used for system calibration
 *  including: TiltedDevice and SharedDevice models, Settings, Errors, and Data container.
 *
 *  @warning No instances of the Calibration class should be made. The class exists
 *           so that the private calibration data could be modified by the nested
 *           tiltedDevice and sharedDevice calibration classes.
 */
class RECONSTRUCTION3D_EXPORT Calibration{
public:

	/** @class      Parameters
	 *  @ingroup    group_Calibration
	 *  @brief      Contains the dlp::Parameters::Entry objects for the Setup() routines
	 *              for \ref dlp::Calibration::TiltedDevice and \ref dlp::Calibration::SharedDevice
	 */
	class RECONSTRUCTION3D_EXPORT Parameters{
	public:
		//for tiltedDevices and sharedDevices
		DLP_NEW_PARAMETERS_ENTRY(FixImageDistance,   "CALIBRATION_PARAMETERS_FIX_IMAGE_DISTANCE",		bool,  false);
		DLP_NEW_PARAMETERS_ENTRY(FixPrincipalPoint,  "CALIBRATION_PARAMETERS_FIX_PRINCIPAL_POINT",		bool,  false);
		DLP_NEW_PARAMETERS_ENTRY(SetTangentDistZero, "CALIBRATION_PARAMETERS_SET_TANGENT_DIST_TO_ZERO",	bool,   true);
		DLP_NEW_PARAMETERS_ENTRY(FixSixthOrderDist,  "CALIBRATION_PARAMETERS_FIX_SIXTH_ORDER_DIST",		bool,  false);
		DLP_NEW_PARAMETERS_ENTRY(FixFourthOrderDist, "CALIBRATION_PARAMETERS_FIX_FOURTH_ORDER_DIST",	bool,  false);
		DLP_NEW_PARAMETERS_ENTRY(FixSecondOrderDist, "CALIBRATION_PARAMETERS_FIX_SECOND_ORDER_DIST",	bool,  false);
		DLP_NEW_PARAMETERS_ENTRY(FixAspectRatio,     "CALIBRATION_PARAMETERS_FIX_ASPECT_RATIO",			bool,   true);
		DLP_NEW_PARAMETERS_ENTRY(ThinPrismModel,     "CALIBRATION_PARAMETERS_THIN_PRISM_MODEL",			bool,  false);		
		DLP_NEW_PARAMETERS_ENTRY(TelecentricModel,   "CALIBRATION_PARAMETERS_TELECENTRIC_MODEL",		bool,  false);
		DLP_NEW_PARAMETERS_ENTRY(TelecentricModelAngleX,   "CALIBRATION_PARAMETERS_TELECENTRIC_ANGLE_X",	float,  0.0f);
		DLP_NEW_PARAMETERS_ENTRY(TelecentricModelAngleY,   "CALIBRATION_PARAMETERS_TELECENTRIC_ANGLE_Y",    float,  0.0f);
		DLP_NEW_PARAMETERS_ENTRY(TiltedModel,           "CALIBRATION_PARAMETERS_TILTED_MODEL",              bool,  false);
		DLP_NEW_PARAMETERS_ENTRY(FixTauXTauYTilted,     "CALIBRATION_PARAMETERS_FIX_TAUX_TAUY_TILTED",      bool,  false);
		DLP_NEW_PARAMETERS_ENTRY(TiltedAngleVertical,   "CALIBRATION_PARAMETERS_TIlTED_ANGLE_VERTICAL",		float,  0.0f);
		DLP_NEW_PARAMETERS_ENTRY(TiltedAngleHorizontal, "CALIBRATION_PARAMETERS_TIlTED_ANGLE_HORIZONTAL",	float,  0.0f);
		DLP_NEW_PARAMETERS_ENTRY(FixAngleInPlane,       "CALIBRATION_PARAMETERS_FIX_ANGAL_IN_PLANE",        bool,  false);
		DLP_NEW_PARAMETERS_ENTRY(AmongPlaneThreshCx,	"CALIBRATION_PARAMETERS_AMONG_PLANES_THRESHOLD_CX",     float, 20.f);
		DLP_NEW_PARAMETERS_ENTRY(AmongPlaneThreshCy,	"CALIBRATION_PARAMETERS_AMONG_PLANES_THRESHOLD_CY",     float, 20.f);
		DLP_NEW_PARAMETERS_ENTRY(AmongPlaneThreshAngle,	"CALIBRATION_PARAMETERS_AMONG_PLANES_THRESHOLD_ANGLE",	float, 0.1f);
		DLP_NEW_PARAMETERS_ENTRY(UseIntrinsicGuess,		"CALIBRATION_PARAMETERS_USE_INTRISIC_GUESS",			bool,  true);
		DLP_NEW_PARAMETERS_ENTRY(IterationCount,		"CALIBRATION_PARAMETERS_ITERATION_MAXIMUM_COUNT",		uint32_t, 30);
		DLP_NEW_PARAMETERS_ENTRY(IterDesiredAccuracy,	"CALIBRATION_PARAMETERS_ITERATION_DESIRED_ACCURACY",	float, DBL_EPSILON);		

		DLP_NEW_PARAMETERS_ENTRY(OffAxisVertical,       "CALIBRATION_PARAMETERS_OFF_AXIS_VERTICAL",        float,      0.0f);
		DLP_NEW_PARAMETERS_ENTRY(OffAxisHorizontal,     "CALIBRATION_PARAMETERS_OFF_AXIS_HORIZONTAL",      float,      0.0f);		
		
		DLP_NEW_PARAMETERS_ENTRY(ImageColumns,          "CALIBRATION_PARAMETERS_IMAGE_COLUMNS",         uint32_t,      2448);
		DLP_NEW_PARAMETERS_ENTRY(ImageRows,             "CALIBRATION_PARAMETERS_IMAGE_ROWS",            uint32_t,      2048);
		DLP_NEW_PARAMETERS_ENTRY(PixelSize,             "CALIBRATION_PARAMETERS_PIXEL_SIZE",               float,     3.45f);
		DLP_NEW_PARAMETERS_ENTRY(FocalLength,           "CALIBRATION_PARAMETERS_FOCAL_LENGTH",             float,        25);
		DLP_NEW_PARAMETERS_ENTRY(ImageDistance,         "CALIBRATION_PARAMETERS_IMAGE_DISTANCE",           float,        30);
		DLP_NEW_PARAMETERS_ENTRY(ImageDistanceYAxis,	"CALIBRATION_PARAMETERS_IMAGE_DISTANCE_Y",         float,         0);		

		//for sharedDevices
		DLP_NEW_PARAMETERS_ENTRY(ProjectorMirrorType,	"CALIBRATION_PARAMETERS_PROJECTOR_MIRROR_TYPE",		uint32_t,	0);
		DLP_NEW_PARAMETERS_ENTRY(PhaseRepeatNum,		"CALIBRATION_PARAMETERS_PHASE_REPEAT_NUM",			uint32_t,	1);
		DLP_NEW_PARAMETERS_ENTRY(GrayCodeVerticalNum,	"CALIBRATION_PARAMETERS_GRAY_CODE_VERTICAL_NUM",	uint32_t,	9);
		DLP_NEW_PARAMETERS_ENTRY(GrayCodeHorizontalNum,	"CALIBRATION_PARAMETERS_GRAY_CODE_HORIZONTAL_NUM",	uint32_t,	9);		

		DLP_NEW_PARAMETERS_ENTRY(CompensateAngleX,	"CALIBRATION_PARAMETERS_COMPENSATE_ANGLE_X",	float,	0.0f);
		DLP_NEW_PARAMETERS_ENTRY(CompensateAngleY,	"CALIBRATION_PARAMETERS_COMPENSATE_ANGLE_Y",	float,	0.0f);
		DLP_NEW_PARAMETERS_ENTRY(CompensateOffsetX,	"CALIBRATION_PARAMETERS_COMPENSATE_OFFSET_X",	float,	0.0f);
		DLP_NEW_PARAMETERS_ENTRY(CompensateOffsetY,	"CALIBRATION_PARAMETERS_COMPENSATE_OFFSET_Y",	float,	0.0f);
		DLP_NEW_PARAMETERS_ENTRY(CompensateScale,	"CALIBRATION_PARAMETERS_COMPENSATE_SCALE",		float,	1.0f);

		//for calibration targets
		DLP_NEW_PARAMETERS_ENTRY(CorrespondPointsMinDist,		"CALIBRATION_PARAMETERS_CORRESPOND_POINTS_MIN_DIST",		float, 	  0.2f);
		DLP_NEW_PARAMETERS_ENTRY(BoardType,						"CALIBRATION_PARAMETERS_BOARD_TYPE",						uint32_t,    1);
		DLP_NEW_PARAMETERS_ENTRY(BoardCount,					"CALIBRATION_PARAMETERS_BOARD_COUNT",						uint32_t,    5);
		DLP_NEW_PARAMETERS_ENTRY(BoardFeatureColumns,			"CALIBRATION_PARAMETERS_BOARD_FEATURE_COLUMNS",				uint32_t,    9);
		DLP_NEW_PARAMETERS_ENTRY(BoardFeatureRows,				"CALIBRATION_PARAMETERS_BOARD_FEATURE_ROWS",				uint32_t,    8);
		DLP_NEW_PARAMETERS_ENTRY(BoardFeatureColumnDistance,	"CALIBRATION_PARAMETERS_BOARD_FEATURE_COLUMN_DISTANCE",		double,   1000);
		DLP_NEW_PARAMETERS_ENTRY(BoardFeatureRowDistance,		"CALIBRATION_PARAMETERS_BOARD_FEATURE_ROW_DISTANCE",		double,   1000);
		DLP_NEW_PARAMETERS_ENTRY(BoardCirclesGridClustering,	"CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_CLUSTERING",		bool,    false);
		DLP_NEW_PARAMETERS_ENTRY(BoardCirclesGridSymmetric,		"CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_SYMMETRIC",		bool,     true);
		DLP_NEW_PARAMETERS_ENTRY(BoardCirclesGridMinThresh,		"CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_MIN_THRESH",		uint32_t,  100);
		DLP_NEW_PARAMETERS_ENTRY(BoardCirclesGridMaxThresh,		"CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_MAX_THRESH",		uint32_t,  200);
		DLP_NEW_PARAMETERS_ENTRY(BoardCirclesGridBlobColor,		"CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_BLOB_COLOR",		uchar,     255);
		DLP_NEW_PARAMETERS_ENTRY(BoardCirclesGridFilterMinArea,	"CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_FILTER_MIN_AREA",	uint32_t,   20);
		DLP_NEW_PARAMETERS_ENTRY(BoardCirclesGridFilterMaxArea,	"CALIBRATION_PARAMETERS_BOARD_CIRCLES_GRID_FILTER_MAX_AREA",    uint32_t, 5000);

		DLP_NEW_PARAMETERS_ENTRY(VirtualAngleX,         "CALIBRATION_VIRTUAL_EXROTATION_ANGLE_X",	double,	0.0);
		DLP_NEW_PARAMETERS_ENTRY(VirtualAngleY,         "CALIBRATION_VIRTUAL_EXROTATION_ANGLE_Y",	double,	0.0);
		DLP_NEW_PARAMETERS_ENTRY(VirtualAngleZ,         "CALIBRATION_VIRTUAL_EXROTATION_ANGLE_Z",	double,	0.0);
		DLP_NEW_PARAMETERS_ENTRY(VirtualTranslationX,    "CALIBRATION_VIRTUAL_EXTRANSLATION_X",		double,	0.0);
		DLP_NEW_PARAMETERS_ENTRY(VirtualTranslationY,    "CALIBRATION_VIRTUAL_EXTRANSLATION_Y",		double,	0.0);
		DLP_NEW_PARAMETERS_ENTRY(VirtualTranslationZ,    "CALIBRATION_VIRTUAL_EXTRANSLATION_Z",		double,	0.0);
		DLP_NEW_PARAMETERS_ENTRY(VirtualSecondOrderDist,   "CALIBRATION_VIRTUAL_SECOND_ORDER_DIST",	double,	0.0);
		DLP_NEW_PARAMETERS_ENTRY(VirtualFourthOrderDist,   "CALIBRATION_VIRTUAL_FOURTH_ORDER_DIST",	double,	0.0);
		DLP_NEW_PARAMETERS_ENTRY(VirtualSixthOrderDist,    "CALIBRATION_VIRTUAL_SIXTH_ORDER_DIST",	double,	0.0);
		DLP_NEW_PARAMETERS_ENTRY(VirtualRandomOffset,	   "CALIBRATION_VIRTUAL_RANDOM_OFFSET",		double,	0.0);

	};

	/** @class      Data
	 *  @brief      Container for calibration model distortion coefficients and
	 *              intrinsic/extrinsic parameters.
	 *  @ingroup    group_Calibration
	 *
	 *  In addition to containing calibration data, this class can save and load the
	 *  data using XML files using \ref Data::Save() and \ref Data::Load().
	 *
	 *  This calibration data object contains the information required to generate
	 *  the geometrical rays of a model for the system geometry. See \ref dlp::Geometry for
	 *  more information.
	 */
	class Data{
		friend class Calibration; /**< Allows \ref Calibration::TiltedDevice and \ref Calibration::SharedDevice to modify private calibration data */
	public:
		/** @brief Enumeration to access rotation and translation rows from cv::Mat extrinsic object */
		enum ExtrinsicRow{
			EXTRINSIC_ROW_ROTATION    = 0,      /**< Row location of rotation vector in extrinsic cv::Mat object    */
			EXTRINSIC_ROW_TRANSLATION = 1       /**< Row location of translation vector in extrinsic cv::Mat object */
		};

		RECONSTRUCTION3D_EXPORT Data();
		RECONSTRUCTION3D_EXPORT ~Data();
		RECONSTRUCTION3D_EXPORT Data(const Data &data);
		RECONSTRUCTION3D_EXPORT Data & operator=(const Data &data);

		RECONSTRUCTION3D_EXPORT void Clear();

		RECONSTRUCTION3D_EXPORT bool IsComplete() const;
		RECONSTRUCTION3D_EXPORT bool IsTiltedDevice() const;

		RECONSTRUCTION3D_EXPORT ReturnCode GetData(cv::Mat *intrinsic, cv::Mat *extrinsic, cv::Mat *distortion, double *reprojection_error)const;
		RECONSTRUCTION3D_EXPORT ReturnCode GetOffsetTransform(cv::Mat *offsetTransform)const;
		RECONSTRUCTION3D_EXPORT void SetOffsetTransform(cv::Mat &offsetTransform);

		RECONSTRUCTION3D_EXPORT ReturnCode GetResolution(uint32_t *columns, uint32_t *rows)const;

		RECONSTRUCTION3D_EXPORT ReturnCode Save(const std::string &filename);
		RECONSTRUCTION3D_EXPORT ReturnCode Load(const std::string &filename);

	private:
		bool calibration_complete_;         /**< Boolean flag to mark that calibration object is completed calibration data */
		bool calibration_of_camera_;        /**< Boolean flag to mark if the calibration object is a camera. If false the data is for a sharedDevice */

		// Model resolution
		uint32_t image_columns_;        /**< Number of pixel columns the calibration model contains */
		uint32_t image_rows_;           /**< Number of pixel rows the calibration model contains */

		// Final calibration data
		cv::Mat intrinsic_;                 /**< OpenCV matrix which contains the focal length and point of calibrated model */
		cv::Mat distortion_;                /**< OpenCV matrix which contains the lens distortion coefficients */
		cv::Mat extrinsic_;                 /**< OpenCV matrix which contains the translation and rotation vectors of the calibration model relative to the calibration board */

		// offset transform between two cameras
		cv::Mat offset_transform_;          /**< OpenCV matrix which contains the offset transforms to convert current camera's location to standard camera */
	};

	/** @class      TiltedDevice
	 *  @brief      Contains methods for calibrating a tiltedDevice.
	 *  @ingroup    group_Calibration
	 *
	 *  To calibrate a TiltedDevice, the following steps should be performed:
	 *  -# Create \ref dlp::Calibration::TiltedDevice object
	 *  -# Setup the module with \ref dlp::Calibration::TiltedDevice::Setup()
	 *
	 */
	class TiltedDevice: public dlp::Module{
	public:
		/** 
		 * @brief  Default constructor - creates empty Calibration::TiltedDevice object
		 * 
		 * Initializes the calibration object with empty device name and sets up
		 * debug logging with appropriate naming convention.
		 */
		RECONSTRUCTION3D_EXPORT TiltedDevice();
		/**
		 * @brief  Parameterized constructor - creates Calibration::TiltedDevice object with specified device name
		 * @param[in] deviceName  Name identifier for the tilted device calibration instance
		 * 
		 * Initializes the calibration object with the provided device name and sets up
		 * debug logging with device-specific naming convention.
		 */
		RECONSTRUCTION3D_EXPORT TiltedDevice(std::string deviceName);
		/**
		 * @brief  Destructor - destroys Calibration::TiltedDevice object and releases all allocated memory
		 * 
		 * Safely deinitializes the object by clearing all calibration data and resources
		 * before destruction.
		 */
		RECONSTRUCTION3D_EXPORT ~TiltedDevice();
		
		/**
		 * @brief Enables or disables debug output for the reconstruction 3D module
		 * 
		 * Controls debug output for both the DLP framework and multi-camera calibration
		 * components. When enabled, sets debug level to verbose (3), when disabled
		 * sets level to warnings and errors only (2).
		 * 
		 * @param[in] enable  Boolean flag to enable (true) or disable (false) debug output
		 */
		RECONSTRUCTION3D_EXPORT void SetDebugEnable(const bool &enable) {
			dlp::Module::SetDebugEnable(enable);
			
			// Ensure multi-calibration debug is always enabled but control level
			cv::MultiCalib::multiCalib_debug_.SetEnable(true);
			
			if (enable) {
				// Set verbose debug level for detailed output
				cv::MultiCalib::multiCalib_debug_.SetLevel(3);
			} else {
				// Set minimal debug level for warnings and errors only
				cv::MultiCalib::multiCalib_debug_.SetLevel(2);
			}
		}

		/**
		 * @brief  Creates or reinitializes the tilted device calibration object
		 * @param[in] deviceName  Name identifier for the tilted device calibration instance
		 * 
		 * This method allows dynamic creation or reinitialization of the calibration object
		 * with a new device name. Useful for reusing object instances with different configurations.
		 */
		RECONSTRUCTION3D_EXPORT void Create(std::string deviceName);
		/**
		 * @brief Clears all calibration settings, parameters, and data
		 * 
		 * Resets all calibration parameters to zero values, clears calibration data structures,
		 * and removes all image points. This method provides a complete reset of the 
		 * tilted device calibration state to its initial condition.
		 */
		RECONSTRUCTION3D_EXPORT void ClearAll();
		/**
		 * @brief Clears calibration data structures and resets calibration state
		 * 
		 * Resets the internal calibration data container and prepares the system
		 * for a new calibration procedure.
		 */
		RECONSTRUCTION3D_EXPORT void ClearCalibrationData();
		/**
		 * @brief Clears and resizes the calibration image points container
		 * @param[in] num  Number of elements to resize the points container to
		 * 
		 * Clears all existing image points and resizes the container to the specified
		 * number of elements. Passing 0 completely clears the container.
		 */
		RECONSTRUCTION3D_EXPORT void ClearCalibrationImagePoints(uint32_t num);
		
		/**
		 * @brief Sets up the tilted device calibration object with provided parameters
		 * 
		 * Initializes all calibration parameters from the provided settings. If settings are empty,
		 * loads default values for all parameters. Configures distortion models, camera parameters,
		 * and computes the compensation matrix for image correction.
		 * 
		 * @param[in] settings Configuration parameters for tilted device calibration
		 * @return ReturnCode indicating success or specific error conditions
		 */
		RECONSTRUCTION3D_EXPORT ReturnCode Setup(const dlp::Parameters &settings);
		/** 
		 * @brief      Save all defined parameters with default values to a text file for TiltedDevice calibration
		 * @param[in]  filename    Output file name
		 * @retval     RETURN_OK   File saved successfully
		 * @retval     Other error codes File save failed
		 */
		RECONSTRUCTION3D_EXPORT ReturnCode SaveDefaultParameters(const std::string &filename);
		/**
		 * @brief      Retrieves calibration settings
		 * 
		 * Exports all current calibration parameters to a dlp::Parameters object.
		 * This includes distortion model settings, camera parameters, compensation
		 * values, and virtual calibration parameters.
		 * 
		 * @param[out] settings    Pointer to \ref dlp::Parameters object to store settings in
		 * @warning    This method clears the \ref dlp::Parameters object before adding any settings
		 * @retval     CALIBRATION_NOT_SETUP               Calibration has not been setup
		 * @retval     CALIBRATION_NULL_POINTER_SETTINGS   Input argument is NULL
		 * @retval     RETURN_OK                           Settings successfully retrieved
		 */
		RECONSTRUCTION3D_EXPORT ReturnCode GetSetup(dlp::Parameters *settings) const;

		/**
		 * @brief Returns true if calibration has been successfully completed
		 * 
		 * Checks the internal calibration state to determine if the calibration
		 * process has been successfully finished and valid calibration parameters
		 * are available for use.
		 * 
		 * @return bool True if calibration is complete and valid, false otherwise
		 */
		RECONSTRUCTION3D_EXPORT bool IsCalibrationComplete() const;

		/**
		 * @brief Sets the image resolution for calibration
		 * 
		 * Configures the image dimensions used for calibration calculations.
		 * This affects the camera matrix and compensation matrix computations.
		 * 
		 * @param[in] width Image width in pixels
		 * @param[in] height Image height in pixels
		 */
		RECONSTRUCTION3D_EXPORT void SetResolution(uint32_t width,uint32_t height);
		
		/**
		 * @brief Retrieves the image resolution as cv::Size object
		 * 
		 * @return cv::Size containing image width and height in pixels
		 */
		RECONSTRUCTION3D_EXPORT cv::Size Resolution();		
		
		/**
		 * @brief Sets the pixel size for calibration
		 * 
		 * Configures the physical pixel size used for converting between pixel
		 * coordinates and physical measurements.
		 * 
		 * @param[in] length Pixel size in micrometers (μm)
		 */
		RECONSTRUCTION3D_EXPORT void SetPixelSize(float length);

		/**
		 * @brief Generates calibration flags based on current configuration settings
		 * 
		 * Constructs a bitmask of calibration flags by combining various calibration options
		 * such as fixed parameters, distortion models, and optimization constraints. Each enabled
		 * setting contributes a specific flag to the final calibration configuration.
		 * 
		 * @return int Bitmask of calibration flags for use in multi-camera calibration
		 */
		RECONSTRUCTION3D_EXPORT int GetCalibrationFlag();

		/**
		 * @brief Sets the complete set of image points for calibration
		 * 
		 * Replaces all existing image points with the provided set of 2D image coordinates.
		 * Each inner vector represents points from a single calibration image or pose.
		 * 
		 * @param[in] imageP Vector of vectors containing 2D image points for calibration
		 */
		RECONSTRUCTION3D_EXPORT void SetImagePoints(std::vector<std::vector<cv::Point2f>> &imageP);
		/**
		 * @brief Sets image points for a specific picture set index
		 * 
		 * Assigns 2D image points to a particular calibration image set. If the index
		 * exceeds the current container size, the container is automatically expanded
		 * with empty vectors until the requested index is available.
		 * 
		 * @param[in] indexPicSet Index of the picture set to update
		 * @param[in] imageP Vector of 2D image points for the specified picture set
		 */
		RECONSTRUCTION3D_EXPORT void SetImagePoints(uint32_t indexPicSet,std::vector<cv::Point2f> &imageP);
		/**
		 * @brief Retrieves all non-empty image points for calibration
		 * 
		 * Copies all image point sets that contain actual data (non-empty vectors)
		 * to the output parameter. Empty sets are filtered out during the copy process.
		 * 
		 * @param[out] imageP Output vector that will contain all non-empty image point sets
		 */
		RECONSTRUCTION3D_EXPORT void GetImagePoints(std::vector<std::vector<cv::Point2f>> &imageP);

		/** 
		 * @brief Copies calibration data into the dlp::Calibration::Data object
		 * 
		 * Updates the internal calibration data with externally provided calibration results.
		 * Useful for loading previously computed calibration parameters or updating calibration
		 * from external sources. The method validates that the provided data represents a
		 * complete calibration before applying it.
		 * 
		 * @param[in] data dlp::Calibration::Data object to copy data from
		 * @retval CALIBRATION_NOT_COMPLETE Supplied calibration data is NOT complete
		 * @retval RETURN_OK Calibration data successfully set
		 * 
		 * @note Useful for updating calibration data if previously completed
		 * @warning Only complete calibration data can be set. Incomplete data will be rejected.
		 */
		RECONSTRUCTION3D_EXPORT ReturnCode SetCalibrationData(Data &data );
		/** 
		 * @brief Retrieves calibration data
		 * 
		 * Returns the complete calibration data including camera matrix, distortion coefficients,
		 * and calibration status. Requires that calibration has been both setup and completed
		 * successfully before data can be retrieved.
		 * 
		 * @param[out] data Pointer to dlp::Calibration::Data object to store calibration results
		 * @retval CALIBRATION_NOT_SETUP Calibration has not been setup
		 * @retval CALIBRATION_NOT_COMPLETE Calibration has not been completed
		 * @retval CALIBRATION_NULL_POINTER_DATA Input argument is NULL
		 * @retval RETURN_OK Calibration data successfully retrieved
		 */
		RECONSTRUCTION3D_EXPORT ReturnCode GetCalibrationData(Data *data ) const;
		
		/**
		 * @brief Sets calibration data from OpenCV matrices
		 * 
		 * Updates the internal calibration data with provided camera intrinsic, distortion,
		 * and extrinsic matrices. Marks the calibration as complete and sets the image
		 * resolution from the current configuration.
		 * 
		 * @param[in] intrinsicM Camera intrinsic matrix (3x3)
		 * @param[in] distortionM Camera distortion coefficients matrix
		 * @param[in] extrinsicM Camera extrinsic matrix (rotation and translation)
		 */
		RECONSTRUCTION3D_EXPORT void SetCalibrationData(cv::Mat &intrinsicM,cv::Mat &distortionM,cv::Mat &extrinsicM);
		/**
		 * @brief Retrieves calibration parameters as OpenCV matrices and physical values
		 * 
		 * Returns camera intrinsic and distortion matrices along with physical camera parameters.
		 * If calibration is not complete, provides estimated values based on current configuration.
		 * 
		 * @param[out] intrinsicM Camera intrinsic matrix (3x3)
		 * @param[out] distortionM Camera distortion coefficients matrix
		 * @param[out] focalLengthUm Focal length in micrometers
		 * @param[out] pixelSizeUm Pixel size in micrometers
		 * @return bool True if calibration data is available, false if not setup
		 */
		RECONSTRUCTION3D_EXPORT bool GetCalibrationData(cv::Mat &intrinsicM,cv::Mat &distortionM, float &focalLengthUm, float &pixelSizeUm);
		/**
		 * @brief Retrieves complete calibration data including extrinsic parameters
		 * 
		 * Returns camera intrinsic, distortion, and extrinsic matrices. If calibration is not 
		 * complete, provides estimated values based on current configuration including tilted
		 * angles in the distortion model.
		 * 
		 * @param[out] intrinsicM Camera intrinsic matrix (3x3)
		 * @param[out] distortionM Camera distortion coefficients matrix (14x1)
		 * @param[out] extrinsicM Camera extrinsic matrix (rotation and translation combined)
		 * @return bool True if calibration data is available, false if not setup
		 */
		RECONSTRUCTION3D_EXPORT bool GetCalibrationData(cv::Mat &intrinsicM,cv::Mat &distortionM,cv::Mat &extrinsicM);

		/**
		 * @brief Generates virtual calibration data for simulation purposes
		 * 
		 * Creates synthetic calibration parameters using virtual configuration values.
		 * This is useful for testing and simulation when real calibration data is not available.
		 * Includes custom distortion coefficients and virtual extrinsic transformations.
		 * 
		 * @param[out] intrinsicM Virtual camera intrinsic matrix (3x3)
		 * @param[out] distortionM Virtual distortion coefficients matrix (14x1)
		 * @param[out] extrinsicR Virtual rotation matrix (3x3)
		 * @param[out] extrinsicT Virtual translation vector (3x1)
		 * @return bool True if virtual data generated successfully, false if not setup
		 */
		RECONSTRUCTION3D_EXPORT bool GetVirtualCalibrationData(cv::Mat &intrinsicM,cv::Mat &distortionM,cv::Mat &extrinsicR,cv::Mat &extrinsicT);
		
		/**
		 * @brief Outputs debug information at specified log level
		 * 
		 * Provides a convenient interface for logging debug messages with configurable
		 * severity levels through the internal debug system.
		 * 
		 * @param[in] level Debug level (1=Error, 2=Warning, 3=Info, 4=Debug)
		 * @param[in] info Debug message to output
		 */
		RECONSTRUCTION3D_EXPORT void DebugOutputInfo(const uint32_t &level, const std::string &info);
	public:
		Parameters::FixImageDistance    fix_image_distance_;    			/**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::FixPrincipalPoint   fix_principal_point_;   			/**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::SetTangentDistZero  zero_tangent_distortion_;       	/**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::FixSixthOrderDist   fix_sixth_order_distortion_;    	/**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::FixFourthOrderDist  fix_fourth_order_distortion_;   	/**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::FixSecondOrderDist  fix_second_order_distortion_;   	/**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::TiltedModel         tilted_model_distortion_;       	/**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::ThinPrismModel      thin_prism_model_;              	/**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::FixAspectRatio      fix_aspect_ratio_distortion_;   	/**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::UseIntrinsicGuess	use_intrinsic_guess_distortion_;	/**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::TelecentricModel    telecentric_model_;             	/**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::TelecentricModelAngleX	telecentric_model_angle_x_;		/**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::TelecentricModelAngleY	telecentric_model_angle_y_;		/**< See \ref dlp::Calibration::TiltedDevice::Setup() */

		Parameters::OffAxisVertical         off_axis_vertical_;             /**< vertical offset in tiltedDevices*/
		Parameters::OffAxisHorizontal       off_axis_horizontal_;           /**< horizontal offset in tiltedDevices*/
		Parameters::TiltedAngleVertical     tilted_angle_vertical_;         /**< vertical tilted angle in tiltedDevices*/
		Parameters::TiltedAngleHorizontal   tilted_angle_horizontal_;       /**< horizontal tilted angle in tiltedDevices*/
		Parameters::FixTauXTauYTilted       fix_taux_tauy_tilted_;          /**< fix tilted parameters in tiltedDevices*/
		Parameters::FixAngleInPlane         fix_angle_in_plane_;            /**< fix tilted parameters in tiltedDevices*/
		Parameters::ImageDistance           image_distance_mm_;             /**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::ImageDistanceYAxis      image_distance_y_mm_;			/**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::FocalLength             focal_length_mm_;               /**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::PixelSize               pixel_size_um_;                 /**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::ImageRows               image_rows_;                    /**< See \ref dlp::Calibration::TiltedDevice::Setup() */
		Parameters::ImageColumns            image_columns_;                 /**< See \ref dlp::Calibration::TiltedDevice::Setup() */

		Parameters::CompensateAngleX        compensate_angle_x_;
		Parameters::CompensateAngleY        compensate_angle_y_;
		Parameters::CompensateOffsetX       compensate_offset_x_;
		Parameters::CompensateOffsetY       compensate_offset_y_;
		Parameters::CompensateScale         compensate_scale_;

		Parameters::VirtualAngleX           virtual_angle_x_;
		Parameters::VirtualAngleY           virtual_angle_y_;
		Parameters::VirtualAngleZ           virtual_angle_z_;
		Parameters::VirtualTranslationX     virtual_translation_x_;
		Parameters::VirtualTranslationY     virtual_translation_y_;
		Parameters::VirtualTranslationZ     virtual_translation_z_;
		Parameters::VirtualSecondOrderDist  virtual_second_order_dist_;
		Parameters::VirtualFourthOrderDist  virtual_fourth_order_dist_;
		Parameters::VirtualSixthOrderDist   virtual_sixth_order_dist_;
		Parameters::VirtualRandomOffset		virtual_random_offset_;

		Data        calibration_data_;     /**< Member to store calibration data for model     */

		// image_points_xy_ stores the x, y location of the features in the calibration image
		std::vector<std::vector<cv::Point2f>>   image_points_xy_;
		std::string                             device_name_;
		cv::Mat                                 compensate_matrix_;
		cv::Mat                                 compensate_matrix_inv_;
		//DISALLOW_COPY_AND_ASSIGN(TiltedDevice);
	}; // class TiltedDevice

	/** @class      SharedDevice
	 *  @brief      Contains methods for calibrating a SharedDevice.
	 *
	 *  SharedDevice calibration must be performed concurrently with a tiltedDevice calibration.
	 *
	 *  To calibrate a sharedDevice, the following steps should be performed:
	 *  -# Create \ref dlp::Calibration::SharedDevice object
	 *  -# Setup the module with \ref dlp::Calibration::SharedDevice::Setup()
	 *  -# Add images using \ref dlp::Calibration::SharedDevice::AddPointCorrespondences_MCam()
	 *  -# Add images until \ref dlp::Calibration::SharedDevice::GetCalibrationProgress() shows that enough images have been successfully added
	 *  -# After adding the number of images requires, call dlp::Calibration::SharedDevice::Calibrate() to perform the calibration
	 *  -# Retrieve the calibration data using \ref dlp::Calibration::SharedDevice::RestoreCalibrationData()
	 *     - This calibration data can be saved for later use
	 *
	 *  @ingroup    group_Calibration
	 */
	class SharedDevice : public Calibration::TiltedDevice{
	public:
		/**
		 * @brief Default constructor - constructs an empty SharedDevice object
		 * 
		 * Initializes all member variables to default values and sets up debug logging.
		 * The coaxial shared device pointer is initialized to nullptr.
		 */
		RECONSTRUCTION3D_EXPORT SharedDevice();
		/**
		 * @brief Parameterized constructor - constructs SharedDevice with specified name
		 * 
		 * @param deviceName The name identifier for this shared device
		 * 
		 * Initializes the device with the given name and sets up all calibration parameters
		 * to their default values. Inherits from TiltedDevice base class.
		 */
		RECONSTRUCTION3D_EXPORT SharedDevice(std::string deviceName);
		/**
		 * @brief Destructor - destroys object and releases all allocated memory
		 * 
		 * Ensures proper cleanup by calling ClearAll() to release all dynamically
		 * allocated resources and reset all member variables.
		 */
		RECONSTRUCTION3D_EXPORT ~SharedDevice();

		/**
		 * @brief Resets all settings and calibration data to zero/default values
		 * 
		 * This comprehensive reset function clears all calibration parameters, settings,
		 * optical system configuration, and deallocates device objects. It maintains
		 * the debug name setting but resets everything else to initial state.
		 * 
		 * @note Memory for fundamental structures is not deallocated, only reset to zero/default values
		 * @see ClearCalibrationData() for a less comprehensive reset option
		 */
		RECONSTRUCTION3D_EXPORT void ClearAll();
		/**
		 * @brief Clears all calibration data while preserving configuration settings
		 * 
		 * This function resets the actual calibration data (intrinsic/extrinsic parameters,
		 * distortion coefficients, etc.) while maintaining the current configuration
		 * settings. Useful for starting a new calibration without reconfiguring all parameters.
		 * 
		 * @note This is a less comprehensive reset compared to ClearAll()
		 * @see ClearAll() for complete reset of both data and configuration
		 */
		RECONSTRUCTION3D_EXPORT void ClearCalibrationData();

		/**
		 * @brief Sets all required parameters for sharedDevice calibration
		 * 
		 * Overrides the tiltedDevice Setup() method. This function initializes all calibration parameters
		 * from the provided settings, using default values for optional parameters and returning errors for 
		 * missing required parameters.
		 * 
		 * @param[in] settings dlp::Parameters object to retrieve settings from
		 * @retval PARAMETERS_EMPTY Supplied dlp::Parameters object is empty
		 * @retval CALIBRATION_PARAMETERS_IMAGE_SIZE_MISSING The image resolution settings are missing 
		 *         (Calibration::TiltedDevice::image_rows_ and/or Calibration::TiltedDevice::image_columns_)
		 * @retval CALIBRATION_PARAMETERS_TANGENT_DISTORTION_MISSING The tangent distortion setting is missing 
		 *         (Calibration::TiltedDevice::zero_tangent_distortion_)
		 * @retval CALIBRATION_PARAMETERS_SIXTH_ORDER_DISTORTION_MISSING The sixth order distortion setting is missing 
		 *         (Calibration::TiltedDevice::fix_sixth_order_distortion_)
		 * @retval SUCCESS All parameters were successfully set
		 */
		RECONSTRUCTION3D_EXPORT ReturnCode Setup(const dlp::Parameters &settings);

		/** @brief      Save all defined parameters with default values to a text file
		 * 
		 *  @ingroup 	group_Calibration
		 *  @param[in]  filename    Output file name
		 *  @retval     RETURN_OK   File saved successfully
		 *  @retval     Other error codes File save failed
		 */
		RECONSTRUCTION3D_EXPORT ReturnCode SaveDefaultParameters(const std::string &filename);

		/**
		 * @brief Adds tilted camera devices to the shared device object
		 * 
		 * @ingroup group_Calibration
		 * @param[in] tiltedDevices Vector of TiltedDevice objects to add
		 * @param[in] tiltedDeviceNames Vector of device names corresponding to the tilted devices
		 * @return true if all devices were added successfully, false otherwise
		 * 
		 * @note This function performs the following operations:
		 * - Validates input parameters
		 * - Clears existing tilted device objects
		 * - Creates new tilted device objects with the provided parameters
		 * - Cleans up coaxial shared device object
		 */
		RECONSTRUCTION3D_EXPORT bool AddTiltedDevices(std::vector<TiltedDevice> &tiltedDevices, std::vector<std::string> &tiltedDeviceNames);

		/**
		 * @brief Adds tilted devices and a coaxial shared device to the shared device object
		 * 
		 * @ingroup group_Calibration
		 * @param[in] tiltedDevices Vector of TiltedDevice objects to add
		 * @param[in] tiltedDeviceNames Vector of device names corresponding to the tilted devices
		 * @param[in] coaxialSharedDevice Coaxial shared device object to add
		 * @param[in] coaxialSharedDeviceName Name of the coaxial shared device
		 * @return true if all devices were added successfully, false otherwise
		 * 
		 * @note This is an extended version that includes both tilted devices and a coaxial device
		 */
		RECONSTRUCTION3D_EXPORT bool AddTiltedDevices(
			std::vector<TiltedDevice>	&tiltedDevices, 		std::vector<std::string>	&tiltedDeviceNames,
			TiltedDevice				&coaxialSharedDevice, 	std::string					coaxialSharedDeviceName);

		/**
		 * @brief Checks if the system has a coaxial shared device (coaxial shared camera)
		 * @ingroup group_Calibration
		 * @return true if coaxial shared device exists, false otherwise
		 */
		RECONSTRUCTION3D_EXPORT bool HasCoaxialSharedDevice();
		
		/**
		 * @brief Checks if patterned calibration board should be used
		 * 
		 * @ingroup group_Calibration
		 * @return true if patterned calibration board is required, false for no pattern mode
		 */
		RECONSTRUCTION3D_EXPORT bool IsUsePatternedCalibBoard();
		/**
		 * @brief Sets the orientation angles for telecentric model
		 * 
		 * @ingroup group_Calibration
		 * @param[in] angleX X-axis orientation angle in degrees
		 * @param[in] angleY Y-axis orientation angle in degrees
		 */
		RECONSTRUCTION3D_EXPORT void SetOrientAngle(double angleX, double angleY);
		/**
		 * @brief Gets the current orientation angles for telecentric model
		 * 
		 * @ingroup group_Calibration
		 * @param[out] angleX X-axis orientation angle in degrees
		 * @param[out] angleY Y-axis orientation angle in degrees
		 */
		RECONSTRUCTION3D_EXPORT void GetOrientAngle(double &angleX, double &angleY);
		/**
		 * @brief Gets the calibration board size in pattern points
		 * 
		 * @ingroup group_Calibration
		 * @return cv::Size containing columns and rows of the calibration board
		 */
		RECONSTRUCTION3D_EXPORT cv::Size GetCalibBoardSize();
		
		/**
		 * @brief Sets the system calibration board type based on the provided type identifier
		 * 
		 * This function maps integer type values to the corresponding CalibrationBoardType enum
		 * and updates the system calibration board configuration accordingly. It supports
		 * chessboard, circle grid, and ring grid patterns for different calibration scenarios.
		 * 
		 * @param type Integer representing the calibration board type:
		 *             - 1: CHESSBOARD (Standard chessboard pattern)
		 *             - 2: CIRCLE (Circular grid pattern)
		 *             - 3: RING (Concentric ring pattern)
		 *             - 0 or others: NOPATTERN (Monochromic calibration targets)
		 */
		RECONSTRUCTION3D_EXPORT void SetSystemCalibBoardType(int type);

		/**
		 * @brief Initializes calibration parameters and data structures before calibration process		 
		 * 
		 * @details This function prepares the system for calibration by:
		 * - Setting up phase shifting and Gray code parameters
		 * - Calculating effective projector resolution based on mirror type
		 * - Generating 3D calibration board points in real space
		 * - Clearing and initializing data structures for calibration data storage
		 * 
		 * @ingroup group_Calibration
		 */
		RECONSTRUCTION3D_EXPORT void InitBeforeCalibration();

		RECONSTRUCTION3D_EXPORT uint8_t GetNumberOfBinaryCodeOrderVertical();
		RECONSTRUCTION3D_EXPORT uint8_t GetNumberOfBinaryCodeOrderHorizontal();
		RECONSTRUCTION3D_EXPORT uint8_t GetNumberOfPhaseRepeatVertical();
		RECONSTRUCTION3D_EXPORT uint8_t GetNumberOfPhaseRepeatHorizontal();
		RECONSTRUCTION3D_EXPORT bool IsBinaryCodeInvertIncluded();
		RECONSTRUCTION3D_EXPORT uint8_t GetNumberOfPicturesBinarySupp(const bool &hasRingLight);
		RECONSTRUCTION3D_EXPORT uint8_t GetNumberOfPicturesBinaryCodeVertical();
		RECONSTRUCTION3D_EXPORT uint8_t GetNumberOfPicturesBinaryCodeHorizontal();
		RECONSTRUCTION3D_EXPORT uint8_t GetNumberOfPicturesPhaseVertical();
		RECONSTRUCTION3D_EXPORT uint8_t GetNumberOfPicturesPhaseHorizontal();
		RECONSTRUCTION3D_EXPORT uint8_t GetNumberOfPicturesTotal(const bool &hasRingLight);
		RECONSTRUCTION3D_EXPORT TiltedDevice* GetCoaxialSharedDevice_MCam();

		RECONSTRUCTION3D_EXPORT double GetFinalReprojectionError();
		
		/**
		 * @brief Adds corresponding points between cameras and projector for hybrid calibration.
		 *        Handles both rectified (using intrinsic and extrinsic parameters) and unrectified images.
		 *        Supports telecentric models and coaxial shared devices. Uses multi-threading for efficiency.
		 * 
		 * @ingroup group_Calibration
		 * @param indexPicSet Index of the current picture set.
		 * @param eachCamImageSet Vector of image sets for each camera. If coaxial device exists, its images are last.
		 * @param tiltedDevIntrinsicM_vec Intrinsic matrices for side cameras (optional, for rectification).
		 * @param tiltedDevDistCoefsM_vec Distortion coefficients for side cameras (optional, for rectification).
		 * @param tiltedCam_R_vec Rotation vectors for side cameras (optional, for rectification).
		 * @param tiltedCam_T_vec Translation vectors for side cameras (optional, for rectification).
		 * @return true if corresponding points were successfully found and processed, false otherwise.
		 */
		RECONSTRUCTION3D_EXPORT bool AddPointCorrespondences_MCam(	
			uint32_t                             indexPicSet,
			std::vector<std::vector<cv::Mat *>>  &eachCamImageSet,
			std::vector<cv::Mat>                 *tiltedDevIntrinsicM_vec = nullptr,
			std::vector<cv::Mat>                 *tiltedDevDistCoefsM_vec = nullptr,
			std::vector<std::vector<cv::Mat>>    *tiltedCam_R_vec = nullptr,
			std::vector<std::vector<cv::Mat>>    *tiltedCam_T_vec = nullptr);

		/**
		 * @brief Adds corresponding points for hybrid multi-projector calibration
		 * 
		 * This function processes image sets from multiple tilted projectors to establish
		 * point correspondences between shared camera and projectors. It handles both
		 * patterned targets and telecentric models, including 3D point generation and
		 * validation for telecentric systems.
		 * 
		 * @ingroup group_Calibration 
		 * @param indexPicSet Index of the current picture set
		 * @param eachDevImageSet Vector of image sets for each projector device
		 * @return true if corresponding points were successfully added and processed
		 * @return false if point correspondence finding failed for any projector
		 */
		RECONSTRUCTION3D_EXPORT bool AddPointCorrespondences_MProj(uint32_t indexPicSet, std::vector<std::vector<cv::Mat *>> &eachDevImageSet);

		/**
		 * @brief Stores all corresponding calibration points to a file for future use.
		 * 
		 * This function saves all detected corner points from both projector and cameras
		 * to a specified file. This allows for quick recalibration without needing to
		 * re-capture images. Use \ref dlp::Calibration::SharedDevice::ReStoreProjAndCamPoints
		 * to load these points later.
		 * 
		 * @ingroup group_Calibration
		 * @param[in] filename The file path where the corner points will be stored
		 */
		RECONSTRUCTION3D_EXPORT void StoreProjAndCamPoints(std::string filename);
		/**
		 * @brief Restores corresponding calibration points from a previously saved file.
		 * 
		 * This function loads corner points for projectors and cameras from a file that was 
		 * previously saved by \ref dlp::Calibration::SharedDevice::StoreProjAndCamPoints. 
		 * This enables quick recalibration without the need to re-detect corners from images.
		 * 
		 * @ingroup group_Calibration
		 * @param[in] filename The file path containing the stored corner points
		 * @return true if all points were successfully restored, false otherwise
		 */
		RECONSTRUCTION3D_EXPORT bool ReStoreProjAndCamPoints(std::string filename);

		RECONSTRUCTION3D_EXPORT bool GenerateCorrespondingPoints(uint32_t indexPicSet, cv::Mat &extrinsicRDiff, cv::Mat &extrinsicTDiff);

		/**
		 * @brief Retrieves the progress of calibration image acquisition.
		 * 
		 * This function checks how many calibration image sets have been successfully added
		 * to the system. When \ref dlp::Calibration::SharedDevice::AddPointCorrespondences 
		 * returns true, it indicates that one calibration image set has been successfully 
		 * processed and stored.
		 * 
		 * @ingroup group_Calibration
		 * @param[out] isFinished Indicates whether all required image sets have been successfully processed
		 * @param[out] stateVec Optional vector containing the success state of each individual image set
		 * @return ReturnCode Success status or error information if the calibration is not properly setup
		 */
		RECONSTRUCTION3D_EXPORT ReturnCode GetCalibrationProgress(bool &isFinished, std::vector<bool> *stateVec=nullptr);
		RECONSTRUCTION3D_EXPORT void GetThreshForCalibrateProjector(float &threshCx, float &threshCy, float &threshAngle);

		/**
		 * @brief Calibration interface after getting enough calibration images through
		 *        \ref dlp::Calibration::SharedDevice::AddPointCorrespondences_MCam
		 * 
		 * This function executes a two-stage calibration process:
		 * 1. Calibrates all tilted devices separately
		 * 2. Performs shared device calibration including coaxial devices if present
		 * 
		 * @ingroup    group_Calibration
		 * @param[out] devicesIntrinsicM_vec Intrinsic matrices for all devices
		 * @param[out] devicesDistCoefsM_vec Distortion coefficients for all devices  
		 * @param[out] dev2dev_R_vec Rotation matrices between devices
		 * @param[out] dev2dev_T_vec Translation vectors between devices
		 * @param[out] reprojectionErr Final reprojection error of the calibration
		 * @return ReturnCode Success status or detailed error information
		 */
		RECONSTRUCTION3D_EXPORT ReturnCode Calibrate(
			std::vector<cv::Mat> &devicesIntrinsicM_vec, 	std::vector<cv::Mat> &devicesDistCoefsM_vec,
			std::vector<cv::Mat> &dev2dev_R_vec, 			std::vector<cv::Mat> &dev2dev_T_vec,
			double &reprojectionErr);
		/**
		 * @brief Calibrates all tilted devices in the shared device system.
		 * 
		 * This function performs a multi-stage calibration process for tilted devices:
		 * 1. Individual calibration of each tilted device
		 * 2. Pairwise calibration between the first device and other devices  
		 * 3. Global optimization of all devices together (if 3+ devices)
		 * 
		 * @ingroup    group_Calibration
		 * @param[out] tiltedDevIntrinsicM_vec Intrinsic matrices for tilted devices
		 * @param[out] tiltedDevDistCoefsM_vec Distortion coefficients for tilted devices
		 * @param[out] tiltedDevFocalLengthUm_vec Focal lengths in micrometers for tilted devices
		 * @param[out] tiltedDevPixelSizeUm_vec Pixel sizes in micrometers for tilted devices
		 * @param[out] tiltedDevOriginR_vec Rotation matrices from world origin to each device
		 * @param[out] tiltedDevOriginT_vec Translation vectors from world origin to each device
		 * @param[out] tiltedDevEachR_vec Relative rotation matrices between devices
		 * @param[out] tiltedDevEachT_vec Relative translation vectors between devices
		 * @return ReturnCode Success status or detailed error information
		 */
		RECONSTRUCTION3D_EXPORT ReturnCode CalibrateTiltedDevices(
			std::vector<cv::Mat> &tiltedDevIntrinsicM_vec, 			std::vector<cv::Mat> &tiltedDevDistCoefsM_vec,
			std::vector<float> &tiltedDevFocalLengthUm_vec, 		std::vector<float> &tiltedDevPixelSizeUm_vec,
			std::vector<cv::Mat> &tiltedDevOriginR_vec, 			std::vector<cv::Mat> &tiltedDevOriginT_vec,
			std::vector<std::vector<cv::Mat>> &tiltedDevEachR_vec, 	std::vector<std::vector<cv::Mat>> &tiltedDevEachT_vec);
		RECONSTRUCTION3D_EXPORT bool NeedToRefineSharedDevice();
		/**
		 * @brief Calibrates the shared device system including both tilted devices and the main shared device.
		 * 
		 * This function performs the calibration of the shared device (typically a projector) in relation
		 * to the already calibrated tilted devices. It establishes the spatial relationships between
		 * all devices in the system.
		 * 
		 * @ingroup group_Calibration 
		 * @param[in,out] devicesIntrinsicM_vec Intrinsic matrices for all devices (shared device at index 0)
		 * @param[in,out] devicesDistCoefsM_vec Distortion coefficients for all devices
		 * @param[in,out] devicesFocalLengthUm_vec Focal lengths in micrometers for all devices
		 * @param[in,out] devicesPixelSizeUm_vec Pixel sizes in micrometers for all devices
		 * @param[in] tiltedDevEachR_vec Relative rotation matrices for tilted devices
		 * @param[in] tiltedDevEachT_vec Relative translation vectors for tilted devices
		 * @param[out] dev2dev_R_vec Rotation matrices between all devices
		 * @param[out] dev2dev_T_vec Translation vectors between all devices
		 * @param[out] reprojectionErr Final reprojection error of the calibration
		 * @param[in] isFirst Flag indicating if this is the first calibration iteration
		 * @return ReturnCode Success status or detailed error information
		 */
		RECONSTRUCTION3D_EXPORT ReturnCode CalibrateSharedDevice(
			std::vector<cv::Mat> &devicesIntrinsicM_vec, 			std::vector<cv::Mat> &devicesDistCoefsM_vec,
			std::vector<float>	&devicesFocalLengthUm_vec, 			std::vector<float> &devicesPixelSizeUm_vec,
			std::vector<std::vector<cv::Mat> > &tiltedDevEachR_vec, std::vector<std::vector<cv::Mat> > &tiltedDevEachT_vec,
			std::vector<cv::Mat> &dev2dev_R_vec, 					std::vector<cv::Mat> &dev2dev_T_vec,
			double	&reprojectionErr, 								bool isFirst);
		
		/**
		 * @brief Calibrates the coaxial shared device and integrates it into the existing device system.
		 * 
		 * This function performs calibration for a coaxial shared device (typically a center camera)
		 * and establishes its spatial relationship with the already calibrated devices in the system.
		 * The coaxial device is calibrated relative to the existing tilted devices and shared device.
		 * 
		 * @ingroup group_Calibration 
		 * @param[in,out] devicesIntrinsicM_vec Intrinsic matrices for all devices (coaxial device appended at end)
		 * @param[in,out] devicesDistCoefsM_vec Distortion coefficients for all devices
		 * @param[in,out] devicesFocalLengthUm_vec Focal lengths in micrometers for all devices
		 * @param[in,out] devicesPixelSizeUm_vec Pixel sizes in micrometers for all devices
		 * @param[out] dev2dev_R_vec Updated rotation matrices between all devices including coaxial device
		 * @param[out] dev2dev_T_vec Updated translation vectors between all devices including coaxial device
		 * @param[out] reprojectionErr Final reprojection error of the coaxial device calibration
		 * @return ReturnCode Success status or detailed error information
		 */
		RECONSTRUCTION3D_EXPORT ReturnCode CalibrateCoaxialSharedDevice(
			std::vector<cv::Mat> &devicesIntrinsicM_vec, 	std::vector<cv::Mat> &devicesDistCoefsM_vec,
			std::vector<float> &devicesFocalLengthUm_vec, 	std::vector<float> &devicesPixelSizeUm_vec,
			std::vector<cv::Mat> &dev2dev_R_vec, 			std::vector<cv::Mat> &dev2dev_T_vec,
			double &reprojectionErr);

		/**
		 * @brief Retrieves the calibration data including intrinsic matrix, distortion coefficients, focal length, and pixel size
		 * 
		 * This function returns the current calibration parameters. If calibration is not complete,
		 * it generates estimated values based on the current setup parameters including image distance,
		 * pixel size, off-axis adjustments, and tilted angles.
		 * 
		 * @param[out] intrinsicM Output intrinsic camera matrix (3x3 CV_64F)
		 * @param[out] distortionM Output distortion coefficients (1x14 CV_64F)
		 * @param[out] focalLengthUm Output focal length in micrometers
		 * @param[out] pixelSizeUm Output pixel size in micrometers
		 * @return true if calibration data was successfully retrieved, false if the device is not setup
		 * 
		 * @note If calibration is not complete, estimated values are calculated based on current parameters
		 * @see Setup() for initializing calibration parameters
		 */
		RECONSTRUCTION3D_EXPORT bool GetCalibrationData(cv::Mat &intrinsicM, cv::Mat &distortionM, float &focalLengthUm, float &pixelSizeUm);

		/**
		 * @brief Saves calibration data to a file
		 * 
		 * This function saves the complete calibration data including intrinsic parameters,
		 * distortion coefficients, and extrinsic transformations between devices to a file.
		 * 
		 * @param devicesIntrinsicM_vec Vector of intrinsic matrices for all devices
		 * @param devicesDistCoefsM_vec Vector of distortion coefficients for all devices
		 * @param dev2dev_R_vec Vector of rotation matrices between devices
		 * @param dev2dev_T_vec Vector of translation vectors between devices
		 * @param reprojectionErr Final reprojection error from calibration
		 * @param fileName Output filename for saving calibration data
		 * @return true if saving was successful, false otherwise
		 */
		RECONSTRUCTION3D_EXPORT bool SaveCalibrationData(
			std::vector<cv::Mat> &devicesIntrinsicM_vec,	std::vector<cv::Mat> &devicesDistCoefsM_vec,
			std::vector<cv::Mat> &dev2dev_R_vec, 			std::vector<cv::Mat> &dev2dev_T_vec,
			double &reprojectionErr, 						std::string fileName);

		/**
		 * @brief Restores calibration data from file including coaxial shared camera
		 * 
		 * This function loads calibration data from a file that includes coaxial shared camera information.
		 * It's an overload that handles systems with coaxial cameras.
		 * 
		 * @param fileName Input filename containing calibration data
		 * @param sharedDevID Identifier for the shared device
		 * @param coaxialSharedCamID Identifier for the coaxial shared camera
		 * @param tiltedDevID_vec Vector of identifiers for tilted devices
		 * @param telecentricMode_vec Output vector of telecentric mode flags
		 * @param intrinsic_vec Output vector of intrinsic matrices
		 * @param distortion_vec Output vector of distortion coefficients
		 * @param size_vec Output vector of image sizes
		 * @param firstBoardR_vec Output vector of rotation matrices from first board to devices
		 * @param firstBoardT_vec Output vector of translation vectors from first board to devices
		 * @param relativeR_vec Output vector of relative rotation matrices between devices
		 * @param relativeT_vec Output vector of relative translation vectors between devices
		 * @param systemOpticConfig Output system optical configuration
		 * @return true if restoration was successful, false otherwise
		 */
		static RECONSTRUCTION3D_EXPORT bool RestoreCalibrationData(
			const std::string &fileName,
			const std::string &sharedDevID,						const std::string &coaxialSharedCamID,
			const std::vector<std::string> &tiltedDevID_vec, 	std::vector<bool> &telecentricMode_vec,
			std::vector<cv::Mat> &intrinsic_vec, 				std::vector<cv::Mat> &distortion_vec,
			std::vector<cv::Size> &size_vec,
			std::vector<cv::Mat> &firstBoardR_vec, 				std::vector<cv::Mat> &firstBoardT_vec,
			std::vector<cv::Mat> &relativeR_vec, 				std::vector<cv::Mat> &relativeT_vec,
			SystemArrangementType &systemOpticConfig);
		/**
		 * @brief Restores calibration data from file (base version without coaxial camera)
		 * 
		 * This function loads calibration data from a file for systems without coaxial cameras.
		 * 
		 * @param fileName Input filename containing calibration data
		 * @param sharedDevID Identifier for the shared device
		 * @param tiltedDevID_vec Vector of identifiers for tilted devices
		 * @param telecentricMode_vec Output vector of telecentric mode flags
		 * @param intrinsic_vec Output vector of intrinsic matrices
		 * @param distortion_vec Output vector of distortion coefficients
		 * @param size_vec Output vector of image sizes
		 * @param firstBoardR_vec Output vector of rotation matrices from first board to devices
		 * @param firstBoardT_vec Output vector of translation vectors from first board to devices
		 * @param relativeR_vec Output vector of relative rotation matrices between devices
		 * @param relativeT_vec Output vector of relative translation vectors between devices
		 * @param systemOpticConfig Output system optical configuration
		 * @return true if restoration was successful, false otherwise
		 */
		static RECONSTRUCTION3D_EXPORT bool RestoreCalibrationData(
			const std::string &fileName, 						const std::string &sharedDevID,
			const std::vector<std::string> &tiltedDevID_vec,	std::vector<bool> &telecentricMode_vec,
			std::vector<cv::Mat> &intrinsic_vec, 				std::vector<cv::Mat> &distortion_vec,
			std::vector<cv::Size> &size_vec,
			std::vector<cv::Mat> &firstBoardR_vec, 				std::vector<cv::Mat> &firstBoardT_vec,
			std::vector<cv::Mat> &relativeR_vec, 				std::vector<cv::Mat> &relativeT_vec,
			SystemArrangementType &systemOpticConfig);
		
		/**
		 * @brief Checks if the system configuration uses multiple cameras.
		 * 
		 * Multi-camera systems are identified by the MCSP (Multi-camera Single projector) configurations.
		 * 
		 * @param systemOpticConfig[in] The system optical configuration to check
		 * @return true if the system uses multiple cameras (MCSP configurations)
		 * @return false if the system uses single camera or invalid configuration
		 */
		static RECONSTRUCTION3D_EXPORT bool IsMultiCameras(const SystemArrangementType &systemOpticConfig);
		/**
		 * @brief Checks if the system configuration uses diamond mirror arrangement.
		 * 
		 * Diamond arrangement refers to a specific optical path configuration used in 
		 * both single and multi-camera systems.
		 * 
		 * @param systemOpticConfig[in] The system optical configuration to check
		 * @return true if the system uses diamond mirror arrangement
		 * @return false if the system uses orthogonal arrangement or invalid configuration
		 */
		static RECONSTRUCTION3D_EXPORT bool IsSystemDiamond(const SystemArrangementType &systemOpticConfig);
	private:
		/**
		 * @brief Finds point correspondences between camera and projector coordinates
		 * 
		 * Establishes correspondence between camera image points and projector coordinates
		 * using phase-shifting and binary code patterns. Supports multiple operation modes:
		 * - Patterned calibration board mode: Uses physical calibration pattern detection
		 * - Tele mode without patterns: Uses phase-based correspondence
		 *   - Multi-camera case: Single projector with multiple cameras
		 *   - Multi-projector case: Single camera with multiple projectors
		 * 
		 * @ingroup group_Calibration 
		 * @param[in] srcImageSet Set of phase-shifting and binary code pattern images
		 * @param[out] cornerImagePoints Detected camera image coordinates
		 * @param[out] cornerProjectorPoints Corresponding projector coordinates  
		 * @param[out] disparityVH Vertical and horizontal disparity maps
		 * @param[in] devObject Device object for debug output
		 * @return bool True if correspondence established successfully, false otherwise
		 */
		bool FindCameraProjectorCorrespondences(
			std::vector<cv::Mat *>   &srcImageSet,
			std::vector<cv::Point2f> &cornerImagePoints,
			std::vector<cv::Point2f> &cornerProjectorPoints,
			std::vector<cv::Mat>     &disparityVH,
			TiltedDevice*            devObject);
		/**
		 * @brief Detects calibration board control points in camera image
		 * 
		 * Performs feature detection on calibration board images to extract corner points.
		 * Supports multiple board types including chessboard, circle grid, and ring grid.
		 * For circle/ring patterns, includes advanced post-processing to refine detection
		 * and determine optimal corner ordering.
		 * 
		 * @ingroup group_Calibration 
		 * @param[in] srcImage Input camera image containing calibration board
		 * @param[out] cornerImagePoints Detected corner points in image coordinates
		 * @param[in] camObject Camera device object for debug output
		 * @return bool True if detection successful, false otherwise
		 */
		bool DetectCalibrationBoardCorners(cv::Mat *srcImage, std::vector<cv::Point2f> &cornerImagePoints, TiltedDevice* camObject);

		/**
		 * @brief Retrieves calibration data from all devices in the system
		 * 
		 * Collects camera calibration parameters (intrinsic, extrinsic, etc.) from all
		 * tilted devices and optionally from the coaxial shared device if present.
		 * Performs validation checks to ensure calibration is properly setup and completed.
		 * 
		 * @ingroup group_Calibration
		 * @param[out] data_vec Vector to store calibration data for all devices
		 * @return ReturnCode Success status or error codes if validation fails
		 */
		ReturnCode GetDevicesCalibrationData(std::vector<Data> &data_vec);

		/**
		 * @brief Retrieves filtered 3D object corner points with valid data
		 * 
		 * Returns a collection of 3D object points from calibration data, 
		 * excluding any empty point sets to ensure only valid data is processed.
		 * This is typically used for camera calibration where we need only
		 * images that successfully detected chessboard corners.
		 * 
		 * @ingroup group_Calibration 
		 * @return std::vector<std::vector<cv::Point3f>> Filtered 3D object points 
		 *         where each inner vector contains corner points from one calibration image
		 *         and only non-empty point sets are included
		 */
		std::vector<std::vector<cv::Point3f>> GetObjectFilteredCornerPointsXYZ();

	protected:
		//for tilted devices
		std::vector<TiltedDevice* > tiltedDeviceObjects_;
		TiltedDevice*               coaxialSharedDeviceObject_;

		// for projector (the parameters of all projectors are regarded as the same)
		uint32_t projector_width_;
		uint32_t projector_height_;
		float effective_projector_width_;       /**< Member to store the width of DMD array in pixels */
		float effective_projector_height_;      /**< Member to store the height of DMD array @note For diamond array DMDs the effective height is half the number of pixels */

		SystemArrangementType   system_opticConfig_; /**< Member to track DLP_Platform mirror type for calibration_board_feature_points_xyz_ generation */
		CalibrationBoardType	system_board_type_;

		std::vector<uint32_t>       phase_pixels_per_period_v_h_;   /**< number of pixels per period in phase shifting patterns*/
		std::vector<uint32_t>       phase_repeat_num_v_h_;          /**< number of repeat time of phase shifting patterns*/
		std::vector<uint32_t>       gray_code_pic_num_v_h_;         /**< number of binary patterns*/

		// for calibration
		// the filtered object points.
		std::vector<std::vector<cv::Point3f>>   object_filtered_corner_points_xyz_;
		// the object points.
		std::vector<cv::Point3f>                object_points_xyz_;

		std::vector<bool>                       state_image_set_;

		std::vector<std::vector<cv::Mat>>       corner_image_show_set_;

		std::vector<cv::Mat>      all_relative_R_;      /**< storing all relative rotation matrix of all devices*/
		std::vector<cv::Mat>      all_relative_T_;      /**< storing all relative translation matrix of all devices*/

		std::vector<std::vector<std::string>>	data_excel_;

		//PixelArrayConfiguration pixel_array_config_;
		double reprojection_error_; /**< Sum of errors between actual calibration board feature locations and the reprojection estimations */

		//for tiltedDevices and sharedDevices
		Parameters::IterationCount      			iteration_maximum_counter_;
		Parameters::IterDesiredAccuracy 			iteration_desired_accuracy_;

		//for projectors (regarding as the same)
		Parameters::ProjectorMirrorType     projector_mirror_type_;         /**< projector DMD arrangement type*/
		Parameters::PhaseRepeatNum          phase_repeat_num_;              /**< the number of projecting time for phase shifting patterns*/
		Parameters::GrayCodeVerticalNum     gray_code_vertical_num_;        /**< the number of vertical binary patterns*/
		Parameters::GrayCodeHorizontalNum   gray_code_horizontal_num_;      /**< the number of horizontal binary patterns*/

		//for sharedDevices
		Parameters::CorrespondPointsMinDist         points_minimum_distance_;       /**< minimum distance among correspondences of sharedDevices*/
		Parameters::AmongPlaneThreshCx				thresh_cx_among_planes_;
		Parameters::AmongPlaneThreshCy				thresh_cy_among_planes_;
		Parameters::AmongPlaneThreshAngle			thresh_angle_among_planes_;

		//for boards
		Parameters::BoardType                       board_type_;                /**< calibration board type:chessboard, circle or ring*/
		Parameters::BoardCount                      board_number_required_;     /**< total number of calibration images required*/
		Parameters::BoardFeatureColumns             board_columns_;             /**< columns of corners in calibration board*/
		Parameters::BoardFeatureRows                board_rows_;                /**< rows of corners in calibration board*/
		Parameters::BoardFeatureColumnDistance      board_column_distance_;     /**< real distance between two corners in a column*/
		Parameters::BoardFeatureRowDistance         board_row_distance_;        /**< real distance between two corners in a row*/
		Parameters::BoardCirclesGridClustering      board_circles_clustering_;
		Parameters::BoardCirclesGridSymmetric       board_circles_symmetric_;
		Parameters::BoardCirclesGridFilterMinArea   board_circles_filter_min_area_;
		Parameters::BoardCirclesGridFilterMaxArea   board_circles_filter_max_area_;
		Parameters::BoardCirclesGridMinThresh       board_circles_min_thresh_;
		Parameters::BoardCirclesGridMaxThresh       board_circles_max_thresh_;
		Parameters::BoardCirclesGridBlobColor       board_circles_blob_color_;

		DISALLOW_COPY_AND_ASSIGN(SharedDevice);
	};	// class SharedDevice
};  // class calibration
} 	// namespace dlp

#endif  //#ifndef CALIBRATION_HPP

