#include "MainWindow.h"

#include <QApplication>     // 创建QT窗口
#include <QSurfaceFormat>   // 配置OpenGL渲染格式

#include <QVTKOpenGLNativeWidget.h>     // VTK 嵌入 Qt 界面的控件，本项目用它显示 PCL 点云。

int main(int argc, char* argv[])
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat()); // 在创建QT窗口之前，设置VTK

    QApplication app(argc, argv);   // 创建QT窗口变量app
    MainWindow window;              // 创建主窗口变量window
    window.show();                  // 显示主窗口
    return app.exec();              // 开始运行app
}   
