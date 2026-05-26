#define _CRT_SECURE_NO_WARNINGS		//关闭 C 运行库“不安全函数”的警告，以减少报错
#define BOOST_USE_WINDOWS_H			//让Boost直接使用Window.h的系统定义，以减少冲突
#define NOMINMAX					//关闭Windows.h中的max和min宏定义，以减少和std标准库的冲突

#ifdef _MSC_VER
#pragma warning(disable: 4996)		// 关闭 PCL/Boost/CRT 的弃用警告
#pragma warning(disable: 4819)		// 关闭第三方头文件编码警告
#endif

#include <iostream>
#include <string>
#include <vector>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/io/pcd_io.h>
#include <pcl/visualization/pcl_visualizer.h>	//PCL自带的可视化模块

void showCloud(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud)
{
	pcl::visualization::PCLVisualizer::Ptr viewer(
		new pcl::visualization::PCLVisualizer("PCD Viewer")
	);

	viewer->setBackgroundColor(0, 0, 0);

	viewer->addPointCloud<pcl::PointXYZ>(cloud, "cloud");

	viewer->setPointCloudRenderingProperties(
		pcl::visualization::PCL_VISUALIZER_POINT_SIZE,
		2,
		"cloud"
	);

	viewer->addCoordinateSystem(1.0);
	viewer->initCameraParameters();

	while (!viewer->wasStopped())
	{
		viewer->spinOnce(100);
	}
}

int main(int argc, char **argv)
{
	const std::string blade = "../Resource/blade.pcd";
	const std::string balltest3 = "../Resource/balltest3.pcd";
	std::vector<std::string> filePaths;
	filePaths.push_back(blade);
	filePaths.push_back(balltest3);

	std::vector<pcl::PointCloud<pcl::PointXYZ>::Ptr> clouds;
	for (auto filePath_ : filePaths)
	{
		auto cloud = pcl::make_shared <pcl::PointCloud<pcl::PointXYZ>>();
		if (pcl::io::loadPCDFile(filePath_, *cloud) == -1)
		{
            std::cout << "ERROR: load cloud failed!  -> " << filePath_ << std::endl;
			return -1;
		}

		clouds.push_back(cloud);
	}

	std::cout << "load cloud success!" << std::endl;


	//for (auto cloud_ : clouds)
	//{
	//	std::cout << "cloud size: " << cloud_->size() << std::endl;
	//	std::cout << "cloud width: " << cloud_->width << std::endl;
	//	std::cout << "cloud height: " << cloud_->height << std::endl;
	//	if (cloud_->is_dense)
	//	{
	//		std::cout << "cloud has no invalid points." << std::endl;
	//	}
	//	else
	//	{
	//		std::cout << "cloud may have invalid points." << std::endl;
	//	}
	//	
	//	std::cout << "\n" << std::endl;
	//}

	// 函数：可视化点云
	

	for(auto cloud_ : clouds)
	{
		showCloud(cloud_);
	}

	return 0;
}