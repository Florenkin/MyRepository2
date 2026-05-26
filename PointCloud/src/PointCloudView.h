#pragma once

#include <QWidget>

#include <pcl/visualization/pcl_visualizer.h>

class QVTKOpenGLNativeWidget;

class PointCloudView : public QWidget
{
    Q_OBJECT

public:
    explicit PointCloudView(QWidget* parent = nullptr);

    void showCloud(const pcl::PointCloud<pcl::PointXYZ>::ConstPtr& cloud);
    void clearCloud();
    void resetCamera();

private:
    void initializeViewer();
    void renderNow();

    QVTKOpenGLNativeWidget* vtkWidget_ = nullptr;
    pcl::visualization::PCLVisualizer::Ptr viewer_;
};
