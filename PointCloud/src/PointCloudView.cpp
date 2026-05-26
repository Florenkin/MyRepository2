#include "PointCloudView.h"

#include <QVBoxLayout>

#include <QVTKOpenGLNativeWidget.h>

#include <pcl/visualization/point_cloud_color_handlers.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

// vtkWidget_ ：创建QT显示控件
PointCloudView::PointCloudView(QWidget* parent)
    : QWidget(parent)
{
    vtkWidget_ = new QVTKOpenGLNativeWidget(this);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(vtkWidget_);

    initializeViewer();
}

// initializeViewer ：初始化 VTK + PCL 显示环境
void PointCloudView::initializeViewer()
{
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow =
        vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renderWindow->AddRenderer(renderer);

    viewer_.reset(new pcl::visualization::PCLVisualizer(
        renderer, renderWindow, "PointCloudViewer", false));
    viewer_->setBackgroundColor(0.04, 0.05, 0.07);
    viewer_->addCoordinateSystem(1.0);
    viewer_->initCameraParameters();

    vtkWidget_->SetRenderWindow(renderWindow);
    viewer_->setupInteractor(vtkWidget_->GetInteractor(), vtkWidget_->GetRenderWindow());
    renderNow();
}

// showCloud ：显示点云函数 
void PointCloudView::showCloud(const pcl::PointCloud<pcl::PointXYZ>::ConstPtr& cloud)
{
    if (!cloud || cloud->empty()) {
        clearCloud();
        return;
    }

    viewer_->removeAllPointClouds();
    viewer_->removeAllShapes();
    viewer_->addCoordinateSystem(1.0);

    pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> cloudColor(
        cloud, 120, 210, 255);
    viewer_->addPointCloud<pcl::PointXYZ>(cloud, cloudColor, "loaded-cloud");

    viewer_->setPointCloudRenderingProperties(
        pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "loaded-cloud");
    viewer_->resetCameraViewpoint("loaded-cloud");
    viewer_->resetCamera();
    if (vtkWidget_->GetRenderWindow()) {
        vtkWidget_->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->ResetCameraClippingRange();
    }
    renderNow();
}

// clearCloud ：清空并刷新点云函数 
void PointCloudView::clearCloud()
{
    viewer_->removeAllPointClouds();
    viewer_->removeAllShapes();
    viewer_->addCoordinateSystem(1.0);
    renderNow();
}

// resetCamera ：重置相机函数
void PointCloudView::resetCamera()
{
    viewer_->resetCamera();
    if (vtkWidget_->GetRenderWindow()) {
        vtkWidget_->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->ResetCameraClippingRange();
    }
    renderNow();
}

// renderNow ：刷新渲染函数
void PointCloudView::renderNow()
{
    if (!viewer_ || !vtkWidget_) {
        return;
    }

    viewer_->spinOnce(1, true);

    if (vtkWidget_->GetRenderWindow()) {
        vtkWidget_->GetRenderWindow()->Render();
    }

    vtkWidget_->update();
}
