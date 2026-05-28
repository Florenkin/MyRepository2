/**
 * @file mtblc.h
 * @brief Motic SDK common definitions and APIs
 * 
 * This file includes data structures and functions of Motic SDK for laser scanning
 * 
 * @code
 * MtL_Init();
 * // ... your code here ...
 * MtL_Destroy();
 * @endcode
 * @date   
 */

#define MOTICSDK_EXPORTS
#pragma warning( disable : 4201 ) //disable warning on nameless struct/union because they are used widely in vizum headers, which we have to simulate here
#include "inc/MTL_Export.h"
typedef int MTLHANDLE;
typedef struct __SMtPoint3f { float x; float y; float z; } SMtPoint3f;
typedef struct __SMtLConfigParam{ unsigned int        nDeviceTimeOut=65535;  }    SMtLConfigParam;
typedef struct __SMtLEyeCBInfo { unsigned int		nSize; char byServerIP[128];} SMtLEyeCBInfo;
#pragma warning( default : 4201 )

/**
 * @brief Configuration parameters for laser scanning operations
 * 
 * This structure contains all the parameters needed to configure a laser scan,
 * including motor speed, scanning range, exposure settings, and laser power.
 */
struct SMtScanConfig
{
	double speed_scan=10;        ///< Scanning speed in units per second
	double range_deg=50;         ///< Scanning range in degrees
	uint16_t exposure_time_us=100; ///< Camera exposure time in microseconds
	uint8_t laserpower=200;      ///< Laser power level [1,250]
	bool color_camera=true;           ///< Whether to capture color information with the point cloud
};

/**
* @brief Store point cloud data 
*/
struct SMtPointcloud
{
	SMtPoint3f* coords=nullptr;  ///< Array of 3D point coordinates
	size_t num_points=0;   ///< Number of points in the point cloud
};

/**
 * @brief Attachment data for individual points in a point cloud
 * 
 * Contains additional metadata for each point including frame information and color data.
 */
struct SMtPointAttachment
{
	int timestamp; ///< Timestamp of when the point was captured
	int frameid;    ///< Frame ID of the point cloud
	float r,g,b;    ///< RGB color values for the point cloud
};

/**
 * @brief Collection of attachment data for all points in a point cloud
 * 
 * Contains an array of attachment data corresponding to points in the point cloud,
 * along with size information for verification and extensibility.
 */
struct SMtPointcloudAttachment
{
	SMtPointAttachment* data=nullptr;  ///< Array of attachment data for each point
	size_t num_points=0;         ///< Number of attachment entries
	size_t number_of_bytes_per_attachment=sizeof(SMtPointAttachment); ///< Size of each attachment entry for future extensibility and version verification
};
/**
 * @brief Initialize the Motic laser scanning library
 * @param [in] pConfigParam Configuration parameters for initializing the library. If nullptr, default parameters will be used.
 * @return An integer error code. 0 indicates success, while non-zero indicates failure
 */
MTLAPI int MtL_Init(const SMtLConfigParam* pConfigParam);

/**
 * @brief Search for devices
 * @return number of devices found
 */
MTLAPI int MtL_SearchDevices();	

/**
 * @brief Open device
 * @param [in] hDevice Device handle
 * @param [out] pOpenDeviceParam Device open information
 * @param [out] pErrorCode Returns error code, does not return when NULL
 * @return Returns 0 on success, otherwise error code
 */
MTLAPI MTLHANDLE MtL_OpenDevice(int device_id);

/**
 * @brief Close device
 * @param [in] hDevice Device handle
 * @return Returns 0 on successful close, otherwise error code
 */
MTLAPI int MtL_CloseDevice(MTLHANDLE hDevice);

/**
 * @brief Get device information
 * @param [in] hDevice Device handle
 * @param [in] pInfo Device information
 * @return Returns 0 on success, otherwise error code
 */
MTLAPI int MtL_GetDeviceInfo(MTLHANDLE hDevice, SMtLEyeCBInfo* pInfo);
/**
* @brief Get a filtered version of a point cloud
*
* This function applies a filtering algorithm to the input point cloud and
* produces a new point cloud containing only the filtered points.
*
* @param [in] pPointcloud_in Pointer to the input point cloud
* @param [out] pPointcloud_out Pointer to the output point cloud that will receive the filtered points
* @return An integer error code. 0 indicates success, while non-zero indicates failure
* @retval 0 Indicates success
* @retval Non-zero Indicates failure, you can use MtL_GetErrorInfo to get more information
**/
MTLAPI int MtL_GetFilteredPointcloud(SMtPointcloud* pPointcloud_in,SMtPointcloud* pPointcloud_out);
/**
 * @brief Destroy and free memory allocated for a point cloud structure
 * 
 * This function releases all memory associated with a point cloud, including
 * the coordinate array. After calling this function, the point cloud structure
 * should not be used until it is re-initialized.
 * 
 * @param [in,out] pPointcloud Pointer to the point cloud structure to destroy
 * @return An integer error code. 0 indicates success, while non-zero indicates failure
 * @retval 0 Indicates success
 * @retval Non-zero Indicates failure, you can use MtL_GetErrorInfo to get more information
 */
MTLAPI int MtL_DestroyPointcloud(SMtPointcloud* pPointcloud);

/**
 * @brief Destroy and free memory allocated for point cloud attachment data
 * 
 * This function releases all memory associated with point cloud attachment data,
 * including the attachment array. After calling this function, the attachment
 * structure should not be used until it is re-initialized.
 * 
 * @param [in,out] pPointcloudAttachement Pointer to the point cloud attachment structure to destroy
 * @return An integer error code. 0 indicates success, while non-zero indicates failure
 * @retval 0 Indicates success
 * @retval Non-zero Indicates failure, you can use MtL_GetErrorInfo to get more information
 */
MTLAPI int MtL_DestroyPointcloudAttachment(SMtPointcloudAttachment* pPointcloudAttachment);

/**
 * @brief Flip or toggle the laser scanning device lid
 * 
 * This function controls the mechanical lid of the laser scanning device,
 * typically used to open or close the scanning chamber for sample placement
 * or removal.
 * 
 * @param [in] hDevice Device handle for the laser scanning device
 * @return An integer error code. 0 indicates success, while non-zero indicates failure
 * @retval 0 Indicates success
 * @retval Non-zero Indicates failure, you can use MtL_GetErrorInfo to get more information
 */
MTLAPI int MtL_Lidflip(MTLHANDLE hDevice);

/**
 * @brief Perform a synchronous laser scan operation
 * 
 * This function executes a complete laser scan using the specified configuration
 * parameters and returns the resulting point cloud data. The function blocks
 * until the scan is complete. Optionally, attachment data with additional
 * metadata can also be retrieved.
 * 
 * @param [in] hDevice Device handle for the laser scanning device
 * @param [in] pScanConfig Pointer to scan configuration parameters
 * @param [out] pPointcloud Pointer to structure that will receive the point cloud data
 * @param [out] pPointcloudAttachment Optional pointer to structure that will receive attachment data (can be nullptr)
 * @return An integer error code. 0 indicates success, while non-zero indicates failure
 * @retval 0 Indicates success
 * @retval Non-zero Indicates failure, you can use MtL_GetErrorInfo to get more information
 */
MTLAPI int MtL_Scan(MTLHANDLE hDevice,SMtScanConfig* pScanConfig,SMtPointcloud* pPointcloud, SMtPointcloudAttachment* pPointcloudAttachment=nullptr);

/**
 * @brief Start an asynchronous laser scan operation
 * 
 * This function initiates a non-blocking laser scan that runs in the background.
 * Point cloud data is delivered through a callback function as it becomes available.
 * The scan continues until stopped via MtL_ScanAsyncStop or the control flag is set to false.
 * 
 * @param [in] hDevice Device handle for the laser scanning device
 * @param [in] pScanConfig Pointer to scan configuration parameters
 * @param [in,out] pCtrlFlag Pointer to boolean flag for controlling scan execution (set to false to stop)
 * @param [in] pScanCB Callback function that receives point cloud data as it's generated
 * @param [in] userContext User-defined context pointer passed to the callback function
 * @return An integer error code. 0 indicates success, while non-zero indicates failure
 * @retval 0 Indicates success
 * @retval Non-zero Indicates failure, you can use MtL_GetErrorInfo to get more information
 */
MTLAPI int MtL_ScanAsyncStart(MTLHANDLE hDevice,SMtScanConfig* pScanConfig,bool* pCtrlFlag,void(*pScanCB)(SMtPointcloud* pPointcloud,SMtPointcloudAttachment* pAttachment, void* ctx), void* userContext);

/**
 * @brief Stop an ongoing asynchronous laser scan operation
 * 
 * This function terminates an active asynchronous scan that was started with
 * MtL_ScanAsyncStart. It sets the control flag to false and waits for the
 * scan thread to complete gracefully.
 * 
 * @param [in] hDevice Device handle for the laser scanning device
 * @param [in,out] pCtrlFlag Pointer to the same control flag used in MtL_ScanAsyncStart
 * @return An integer error code. 0 indicates success, while non-zero indicates failure
 * @retval 0 Indicates success
 * @retval Non-zero Indicates failure, you can use MtL_GetErrorInfo to get more information
 */
MTLAPI int MtL_ScanAsyncStop(MTLHANDLE hDevice,bool* pCtrlFlag);

/**
 * @brief Map texture from the color camera (C) onto a point cloud
 *
 * @param[in] calib_AB: path to calibration file for monochrome cameras
 * @param[in] calib_AC: path to calibration file for the color camera (with respect to the first monochrome camera)
 * @param[in] pPointcloud: point cloud obtained from a previous scan
 * @param[inout] pPointcloudAttachments: additional point cloud data or attachments
 * @retval Non-zero Indicates failure, you can use MtL_GetErrorInfo to get more information
 */
MTLAPI int MtL_MapTexture(MTLHANDLE hDevice,const char* path_calibAB,const char* path_calibAC,SMtPointcloud* pPointcloud,SMtPointcloudAttachment* pPointcloudAttachment);

/*!
* @brief Obtain the current position of the motor in degrees. The position is measured in degrees, where 0 degrees corresponds to the zero position of the motor, and positive values indicate rotation in the forward direction. This function can be used to monitor the motor's position during scanning or to verify that the motor has reached a specific position after a move command.
* @param [in] hDevice Device handle
* @param [out] pPosition Pointer to a double where the current motor position in degrees will be stored
* @return An integer error code. 0 indicates success, while non-zero indicates failure. In case of failure, you can use the MtL_GetErrorInfo function to get more information about the error.
* @retval 0 Indicates success
* @retval Non-zero Indicates failure, you can use MtL_GetErrorInfo to get more information
*/
MTLAPI int MtL_GetMotorPosition(MTLHANDLE hDevice,double* pPosition);

/**
* @brief Enable or disable the laser. Enabling the laser will turn it on if the motor is moving, and keep it on until the motor stops. Disabling the laser will turn it off immediately. This function can be used to control whether the laser is on during motor scanning, or to manually turn on the laser for other purposes (e.g. for visual inspection) without starting a scan.
* @param [in] hDevice Device handle
* @param [in] bEnable Enable or disable the laser
* @return An integer error code. 0 indicates success, while non-zero indicates failure. In case of failure, you can use the MtL_GetErrorInfo function to get more information about the error.
* @retval 0 Indicates success
* @retval Non-zero Indicates failure, you can use MtL_GetErrorInfo to get more information
*/
MTLAPI int MtL_EnableLaser(MTLHANDLE hDevice, bool bEnable);
MTLAPI char* MtL_GetLastErrorMessage();
/*
* @brief Get error information corresponding to an error code. This function can be used to convert an error code returned by other functions into a human-readable error message for debugging or logging purposes.
* @param [in] nErrorCode error code returned by a function in case of failure
* @param [out] szError  textual description of the error corresponding to the error code, stored in a character array of size 256. The caller should ensure that the array is allocated and has enough space to hold the error message.
*/
MTLAPI void MtL_GetErrorInfo(int nErrorCode, char szError[256]);
/**
 * @brief Destroy and cleanup the SDK
 */
MTLAPI void MtL_Destroy();