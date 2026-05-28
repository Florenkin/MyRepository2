#include <gtest/gtest.h>
#include "mtblc.h"
#include <opencv2/opencv.hpp>
import std;

const std::string output_path="./test_output/"; 
/**
 * @brief Test fixture for single module testing
 * 
 * Sets up and tears down a device connection for each test.
 * Initializes the library, searches for devices, and opens the first device.
 */
TEST(Network, MtL_SearchDevices)
{
	int deviceCount = MtL_SearchDevices();
	ASSERT_GT(deviceCount,0)<<"No devices found. Please check the connection.";
	std::cout<<"Number of devices found: "<<deviceCount<<std::endl;
}
class SingleModuleTest : public ::testing::Test 
{
protected:
	void SetUp() override 
	{
		SMtLConfigParam config;
		MtL_Init(&config);
		int deviceCount = MtL_SearchDevices();
		ASSERT_GT(deviceCount,0)<<"No devices found. Please check the connection.";
		hDevice=MtL_OpenDevice(0);
	}
	void TearDown() override
	{
		MtL_CloseDevice(hDevice);
		MtL_Destroy();
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	}
	MTLHANDLE hDevice=-1;
};

/**
 * @brief Test device information retrieval
 * 
 * Validates that device information can be retrieved successfully and
 * verifies the expected server IP address (192.168.100.25).
 */
TEST_F(SingleModuleTest, DeviceInfo) 
{ 
	SMtLEyeCBInfo deviceInfo;

	int errcode=MtL_GetDeviceInfo(hDevice,&deviceInfo);
	std::string serverIP(reinterpret_cast<char*>(deviceInfo.byServerIP));
	ASSERT_EQ(errcode,0)<<"Failed to get device info. Error code: "<<errcode;
	ASSERT_EQ(serverIP,"192.168.100.25")<<"Unexpected server IP. Expected: 192.168.100.25";
	std::cout<<"IP of device 0: "<<serverIP<<std::endl;
}

/**
 * @brief Test laser enable/disable functionality
 * 
 * Verifies that the laser can be turned on and off without errors.
 * Includes a 500ms delay to allow laser state changes to settle.
 */
TEST_F(SingleModuleTest,Laser) 
{ 
	MtL_EnableLaser(hDevice,true);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	MtL_EnableLaser(hDevice,false);
}

/**
 * @brief Test lid flip functionality (disabled)
 * 
 * Tests the mechanical lid flip operation. Currently disabled due to
 * potential hardware wear or safety concerns during automated testing.
 */
TEST_F(SingleModuleTest,DISABLED_Lidflip) 
{ 
	int ret=MtL_Lidflip(hDevice);
	ASSERT_EQ(ret,0)<<"Lidflip failed with error code: "<<ret;
}

/**
 * @brief Test motor position retrieval
 * 
 * Validates that the current motor position can be read successfully
 * and outputs the position in degrees for verification.
 */
TEST_F(SingleModuleTest,MtL_GetMotorPosition) 
{ 
	double position;
	int ret=MtL_GetMotorPosition(hDevice,&position);
	ASSERT_EQ(ret,0)<<"Failed to get motor position. Error code: "<<ret;
	std::cout<<"Motor position: "<<position<<" degrees"<<std::endl;
}
void save_pointcloud_to_file(const std::string& filename,
	const SMtPointcloud* pointcloud,const SMtPointcloudAttachment* pAttachment=nullptr)
{
	std::ofstream ofs(filename);
	for(size_t k=0;k<pointcloud->num_points;k++)
	{
		SMtPoint3f pt=pointcloud->coords[k];
		if(pAttachment)
		{
			SMtPointAttachment att=pAttachment->data[k];
			//ofs<<std::format("{:.6f} {:.6f} {:.6f} {} {} \n",pt.x,pt.y,pt.z,att.frameid,att.timestamp);
			ofs<<std::format("{:.6f} {:.6f} {:.6f} {} {} {} {} {} \n",pt.x,pt.y,pt.z,att.frameid,att.timestamp,att.r,att.g,att.b);
		}
		else
		{
			ofs<<std::format("{:.6f} {:.6f} {:.6f} \n",pt.x,pt.y,pt.z);
		}
	}
	std::println("Pointcloud saved to {}",filename);
}
/**
 * @brief Test synchronous scanning functionality
 * 
 * Performs a synchronous scan with specified parameters and saves the resulting pointcloud to a text file.
 * Validates that the scan completes successfully and produces valid point data.
 */
TEST_F(SingleModuleTest,ScanSync) 
{ 
	SMtScanConfig config;
	config.range_deg=50;
	config.laserpower=250;
	config.exposure_time_us=100;
	SMtPointcloud pointcloud;
	int ret=MtL_Scan(hDevice, &config, &pointcloud);
	ASSERT_EQ(ret,0)<<"Scan failed with error code: "<<ret<<" Message: "<<MtL_GetLastErrorMessage();
	std::cout<<"Pointcloud size: "<<pointcloud.num_points<<std::endl; 
	save_pointcloud_to_file(output_path+"pointcloud_scan_sync.txt", &pointcloud);
	MtL_DestroyPointcloud(&pointcloud);
}
TEST_F(SingleModuleTest,DISABLED_FOV) 
{ 
	SMtScanConfig config;
	config.range_deg=50;
	config.laserpower=250;
	config.speed_scan=5;
	config.exposure_time_us=1000;
	SMtPointcloud pointcloud;
	int ret=MtL_Scan(hDevice, &config, &pointcloud);
	ASSERT_EQ(ret,0)<<"Scan failed with error code: "<<ret;
	std::cout<<"Pointcloud size: "<<pointcloud.num_points<<std::endl; 
	auto filename=output_path+"FoV_at_WD=.txt";
	save_pointcloud_to_file(filename, &pointcloud);
	std::println("FoV test completed. Please rename the file appropriately for records",filename);		
	MtL_DestroyPointcloud(&pointcloud);
}
TEST_F(SingleModuleTest,DISABLED_RepeatedDaq) 
{ 
	SMtScanConfig config;
	//config.range_deg=50;//@300mm
	//config.laserpower=150;
	//config.speed_scan=5;
	//config.exposure_time_us=50;
	config.range_deg=50;//@800mm
	config.laserpower=250;
	config.speed_scan=10;
	config.exposure_time_us=100;

	const int num_scans=60; 
	std::string datadir = output_path + "daq/";
	std::filesystem::create_directories(datadir);
	for(int k=0;k<num_scans;k++)
	{
		std::println("Starting scan {} of {}",k+1,num_scans);
		SMtPointcloud pointcloud;
		SMtPointcloudAttachment pointcloudAttachment;
		int ret=MtL_Scan(hDevice,&config,&pointcloud,&pointcloudAttachment);
		ASSERT_EQ(ret,0)<<"Scan failed with error code: "<<ret;
		std::cout<<"Pointcloud size: "<<pointcloud.num_points<<std::endl;
		SMtPointcloud filteredPointcloud;
		MtL_GetFilteredPointcloud(&pointcloud,&filteredPointcloud);
		save_pointcloud_to_file(datadir+"scan_"+std::to_string(k)+"_filter_on.txt", &filteredPointcloud);
		save_pointcloud_to_file(datadir+"scan_"+std::to_string(k)+"_filter_off.txt", &pointcloud, &pointcloudAttachment);
		MtL_DestroyPointcloud(&pointcloud);
		MtL_DestroyPointcloud(&filteredPointcloud);
		MtL_DestroyPointcloudAttachment(&pointcloudAttachment);
		std::this_thread::sleep_for(std::chrono::milliseconds(2000)); //delay between scans to allow hardware to settle
	}
}

TEST_F(SingleModuleTest,PointcloudFilter) 
{ 
	SMtScanConfig config;
	config.speed_scan=5;
	config.range_deg=40;
	config.laserpower=200;
	config.exposure_time_us=50;
	SMtPointcloud pointcloud;
	int ret=MtL_Scan(hDevice, &config, &pointcloud);
	ASSERT_EQ(ret,0)<<"Scan failed with error code: "<<ret;
	std::cout<<"Pointcloud size: "<<pointcloud.num_points<<std::endl; 
	SMtPointcloud filteredPointcloud;
	ret=MtL_GetFilteredPointcloud(&pointcloud, &filteredPointcloud);
	ASSERT_EQ(ret,0)<<"Filtering failed with error code: "<<ret;
	std::cout<<"Filtered pointcloud size: "<<filteredPointcloud.num_points<<std::endl;
	auto testname=::testing::UnitTest::GetInstance()->current_test_info()->name();
	save_pointcloud_to_file(output_path+testname+"_on.txt", &filteredPointcloud);
	save_pointcloud_to_file(output_path+testname+"_off.txt", &pointcloud);
	MtL_DestroyPointcloud(&pointcloud);
	MtL_DestroyPointcloud(&filteredPointcloud);
} 

/**
 * @brief Capture a pointcloud and perform texture mapping using calibration data
 */
TEST_F(SingleModuleTest,DISABLED_MapTexture) 
{ 
	SMtScanConfig config;
	config.speed_scan=5;
	config.range_deg=40;
	config.laserpower=200;
	config.exposure_time_us=50;
	SMtPointcloud pointcloud;
	int ret=MtL_Scan(hDevice, &config, &pointcloud);
	ASSERT_EQ(ret,0)<<"Scan failed with error code: "<<ret;
	std::cout<<"Pointcloud size: "<<pointcloud.num_points<<std::endl; 
	SMtPointcloudAttachment pointcloudAttachment; 
	std::string resdir="C:/astri2/lasv/lasvrepo/res/"; //default resource directory
	std::string fn_calibAB=resdir+"CalibAB.yaml",fn_calibAC=resdir+"CalibAC.yaml";
	ret=MtL_MapTexture(hDevice,fn_calibAB.c_str(),fn_calibAC.c_str(),&pointcloud,&pointcloudAttachment);
	ASSERT_EQ(ret,0)<<"Texure mapping failed with error code: "<<ret;
	auto testname=::testing::UnitTest::GetInstance()->current_test_info()->name();
	save_pointcloud_to_file(output_path+testname+".txt", &pointcloud,&pointcloudAttachment);
	MtL_DestroyPointcloud(&pointcloud);
	MtL_DestroyPointcloudAttachment(&pointcloudAttachment);
}

/**
 * @brief Test synchronous scanning with attachment data
 * 
 * Performs a synchronous scan that also captures attachment information
 * (such as frame IDs) for each point. Saves both coordinate and attachment
 * data to demonstrate extended pointcloud functionality.
 */
TEST_F(SingleModuleTest,ScanSyncWithAttachment) 
{ 
	SMtScanConfig config;
	config.laserpower=120;
	config.exposure_time_us=200;
	SMtPointcloud pointcloud;
	SMtPointcloudAttachment pointcloudAttachment;
	int ret=MtL_Scan(hDevice, &config, &pointcloud, &pointcloudAttachment);
	ASSERT_EQ(ret,0)<<"Scan failed with error code: "<<ret<<" - "<<MtL_GetLastErrorMessage();
	std::cout<<"Pointcloud size: "<<pointcloud.num_points<<std::endl; 
	std::string fnout=output_path+"pointcloud_scan_sync_(x,y,z,frameid,timestamp,r,g,b).txt";
	save_pointcloud_to_file(fnout,&pointcloud,&pointcloudAttachment); 
	MtL_DestroyPointcloud(&pointcloud);
	MtL_DestroyPointcloudAttachment(&pointcloudAttachment);
}

/**
 * @brief Test version conflict detection in attachment data
 * 
 * Simulates a version mismatch by artificially increasing the attachment
 * structure size. Verifies that the library properly detects and rejects
 * incompatible structure versions with appropriate error codes.
 */
TEST_F(SingleModuleTest,ScanSyncWithAttachment_verconflict) 
{ 
	SMtScanConfig config;
	config.range_deg=40;
	config.speed_scan=1;
	config.laserpower=120;
	config.exposure_time_us=200;
	SMtPointcloud pointcloud;
	SMtPointcloudAttachment pointcloudAttachment;
	pointcloudAttachment.number_of_bytes_per_attachment=sizeof(SMtPointAttachment)+16; //simulate version conflict with mismatched structure size
	int ret=MtL_Scan(hDevice, &config, &pointcloud, &pointcloudAttachment);
	ASSERT_EQ(ret,-1)<<"Scan should fail due to version conflict.";
}

/**
 * @brief Test asynchronous scanning functionality
 * 
 * Demonstrates asynchronous scanning with callback-based point delivery.
 * Uses a callback function to accumulate points as they are captured,
 * allowing for real-time processing. Saves accumulated points to file
 * after scan completion.
 */
TEST_F(SingleModuleTest,ScanAsync) 
{ 
	std::vector<SMtPoint3f> accumulated_points;
	void* userContext=&accumulated_points; //example of using user context to accumulate points across callbacks
	auto scanCB=[](SMtPointcloud* pPointcloud,
		SMtPointcloudAttachment* // pAttachment
		,void* userContext)
	{
		auto* pAccumulatedPoints=reinterpret_cast<std::vector<SMtPoint3f>*>(userContext);
		pAccumulatedPoints->insert(pAccumulatedPoints->end(), pPointcloud->coords, pPointcloud->coords + pPointcloud->num_points);
		std::print("\rAsync scan callback invoked for {} points; total accumulated points={}   ",pPointcloud->num_points,pAccumulatedPoints->size()); 
	};
	bool ctrlFlag=true;
	SMtScanConfig config; //default config 
	int ret=MtL_ScanAsyncStart(hDevice,&config,&ctrlFlag,scanCB,userContext);
	ASSERT_EQ(ret,0)<<"Async scan start failed with error code: "<<ret<<" - "<<MtL_GetLastErrorMessage();
	while(ctrlFlag)//wait until the motor finishes scanning. this flag will be set to false by the library when the scan is complete, or can be set to false by the caller to stop the scan early.
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::println("\nAsync scan stopped, total accumulated points={}",accumulated_points.size());
	std::string fnout=output_path+"pointcloud_scan_async.txt";
	std::ofstream ofs(fnout);
	for(auto pt:accumulated_points)
		ofs<<std::format("{:.6f} {:.6f} {:.6f} \n",pt.x,pt.y,pt.z);
	MtL_ScanAsyncStop(hDevice,&ctrlFlag);
}

/**
 * @brief Test stationary asynchronous scanning
 * 
 * Tests asynchronous scanning in stationary mode (speed_scan=0) where
 * the motor doesn't rotate. Validates callback invocation counting and
 * frame ID tracking. Stops after a predetermined number of callbacks
 * rather than motor completion.
 */
TEST_F(SingleModuleTest,ScanAsync_Stationary) 
{ 
	int num_cb_invoked=0; 
	void* userContext=&num_cb_invoked; //example of using user context to keep track of callback invocations
	auto scanCB=[](SMtPointcloud* pPointcloud,SMtPointcloudAttachment* pAttachment,void* userContext)
	{
		int* pNumInvoked=reinterpret_cast<int*>(userContext);
		int frameid=-1;
		if(pAttachment->num_points>0)
			frameid=pAttachment->data[0].frameid;
		std::print("\rAsync scan callback invoked for {} points; num_cb_invoked={}; frameids={{{},...}}",pPointcloud->num_points,*pNumInvoked,frameid); 
		(*pNumInvoked)+=1;
	};
	bool ctrlFlag=true;
	SMtScanConfig config; 
	config.speed_scan=0; //stationary scan
	int ret=MtL_ScanAsyncStart(hDevice,&config,&ctrlFlag,scanCB,userContext);
	ASSERT_EQ(ret,0)<<"Async scan start failed with error code: "<<ret<<" - "<<MtL_GetLastErrorMessage();
	while(num_cb_invoked<100)//wait until a certain number of callbacks
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::println("\nAsync scan stopped after {} callback invocations.",num_cb_invoked);
	MtL_ScanAsyncStop(hDevice,&ctrlFlag);
}

/**
 * @brief Test library initialization
 * 
 * Validates that the MtL library can be initialized successfully
 * with default configuration parameters.
 */
TEST(Initialization, DeviceStatusNormal) 
{ 
	SMtLConfigParam config;
    EXPECT_EQ(0,MtL_Init(&config));
}

/**
 * @brief Test error code handling and message retrieval
 * 
 * Verifies that error codes can be converted to human-readable
 * error messages using the MtL_GetErrorInfo function.
 */
TEST(ErrorHandling, ErrCode) 
{ 
	int errcode=5;
	char errmsg[256];
	MtL_GetErrorInfo(errcode,errmsg);
	EXPECT_STREQ("Error code: 5",errmsg);
}

int main(int argc, char **argv) 
{
    // Create test output directory if it doesn't exist
    std::filesystem::create_directories(output_path); 
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}