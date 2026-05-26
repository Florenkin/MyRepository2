#include "PointCloudLoader.h"

#include <QFile>
#include <QFileInfo>

#include <exception>
#include <vector>

#include <pcl/PCLPointCloud2.h>
#include <pcl/common/common.h>
#include <pcl/common/io.h>
#include <pcl/conversions.h>
#include <pcl/filters/filter.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/pcd_io.h>

// 加载点云函数loadPcdFile
PointCloudLoadResult PointCloudLoader::loadPcdFile(const QString& filePath)
{
    PointCloudLoadResult result;    // 创建返回变量 result
    result.filePath = filePath;
    result.cloud.reset(new pcl::PointCloud<pcl::PointXYZ>()); // 创建空的点云对象

    const QFileInfo fileInfo(filePath);         // 检查文件是否存在
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        result.errorMessage = QStringLiteral("文件不存在：%1").arg(filePath);
        return result;
    }

    // 使用 PCL 读取 PCD文件
    pcl::PCLPointCloud2 cloudBlob;
    const std::string nativePath = QFile::encodeName(fileInfo.absoluteFilePath()).toStdString();
    const int loadStatus = pcl::io::loadPCDFile(nativePath, cloudBlob);
    if (loadStatus < 0) {
        result.errorMessage = QStringLiteral("PCD 文件读取失败，请检查文件格式：%1")
                                  .arg(fileInfo.fileName());
        return result;
    }

    if (pcl::getFieldIndex(cloudBlob, "x") < 0 ||
        pcl::getFieldIndex(cloudBlob, "y") < 0 ||
        pcl::getFieldIndex(cloudBlob, "z") < 0) {
        result.errorMessage = QStringLiteral("PCD 文件缺少 x、y、z 坐标字段：%1")
                                  .arg(fileInfo.fileName());
        return result;
    }

    try {
        pcl::fromPCLPointCloud2(cloudBlob, *result.cloud);
    } catch (const std::exception& ex) {
        result.errorMessage = QStringLiteral("点云格式转换失败：%1").arg(ex.what());
        return result;
    }

    // 存储 result.cloud 的有效点
    std::vector<int> finiteIndices;
    // 去除点云中的无效点
    pcl::removeNaNFromPointCloud(*result.cloud, *result.cloud, finiteIndices);
    if (result.cloud->empty()) {
        result.errorMessage = QStringLiteral("点云文件中没有可显示的有效点：%1")
                                  .arg(fileInfo.fileName());
        return result;
    }

    // 计算点云的边界
    pcl::getMinMax3D(*result.cloud, result.minPoint, result.maxPoint);
    // 计算点云的点数
    result.pointCount = result.cloud->size();
    // 标记读取成功
    result.success = true;
    return result;
}

PointCloudLoadResult PointCloudLoader::applyVoxelGridFilter(
    const pcl::PointCloud<pcl::PointXYZ>::ConstPtr& input,
    float leafSize,
    const QString& sourceName)
{
    PointCloudLoadResult result;
    result.filePath = sourceName;
    result.cloud.reset(new pcl::PointCloud<pcl::PointXYZ>());

    if (!input || input->empty()) {
        result.errorMessage = QStringLiteral("当前没有可滤波的点云数据。");
        return result;
    }

    if (leafSize <= 0.0f) {
        result.errorMessage = QStringLiteral("体素大小必须大于 0。");
        return result;
    }

    pcl::VoxelGrid<pcl::PointXYZ> voxelGrid;
    voxelGrid.setInputCloud(input);
    voxelGrid.setLeafSize(leafSize, leafSize, leafSize);
    voxelGrid.filter(*result.cloud);

    if (result.cloud->empty()) {
        result.errorMessage = QStringLiteral("滤波后没有可显示的有效点，请减小体素大小。");
        return result;
    }

    pcl::getMinMax3D(*result.cloud, result.minPoint, result.maxPoint);
    result.pointCount = result.cloud->size();
    result.success = true;
    return result;
}
