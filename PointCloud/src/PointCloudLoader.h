#pragma once

#include <QString>

#include <cstddef>

#include <Eigen/Core>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

struct PointCloudLoadResult
{
    bool success = false;
    QString errorMessage;
    QString filePath;
    std::size_t pointCount = 0;
    Eigen::Vector4f minPoint = Eigen::Vector4f::Zero();
    Eigen::Vector4f maxPoint = Eigen::Vector4f::Zero();
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud;
};

class PointCloudLoader
{
public:
    static PointCloudLoadResult loadPcdFile(const QString& filePath);
    static PointCloudLoadResult applyVoxelGridFilter(
        const pcl::PointCloud<pcl::PointXYZ>::ConstPtr& input,
        float leafSize,
        const QString& sourceName = QString());
};
