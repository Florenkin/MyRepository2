PointCloud 点云可视化项目

一、项目概述

本项目基于 Qt、C++ 和 PCL（Point Cloud Library）开发，用于读取项目资源目录中的 PCD 点云文件，并在桌面端 Qt 界面中完成三维可视化展示。

程序启动后会自动扫描默认点云目录：

D:\code\PointCloud\Resource\PCD

当前示例点云文件包括：

1. 1.pcd
2. blade.pcd
3. balltest3.pcd

二、已实现功能

1. PCD 文件加载
   - 自动扫描 Resource\PCD 目录中的 .pcd 文件。
   - 支持通过界面选择其他 PCD 文件目录。
   - 支持通过界面直接打开指定 .pcd 文件。

2. 点云数据读取
   - 使用 PCL 原生接口读取 PCD 文件。
   - 支持 ASCII 和 binary_compressed 等 PCD 数据格式。
   - 支持包含 x、y、z 字段的点云文件；文件中存在 normal_x、normal_y、normal_z 等额外字段时，也可以提取坐标字段进行显示。
   - 对文件不存在、缺少坐标字段、格式异常、读取失败等情况提供错误提示。

3. 点云可视化
   - 使用 QVTKOpenGLNativeWidget 在 Qt 界面中嵌入 PCLVisualizer。
   - 支持点云三维显示。
   - 支持鼠标旋转、缩放和平移视图。
   - 默认显示坐标轴。
   - 按 Z 轴字段进行点云着色；无法按字段着色时使用统一颜色显示。

4. 界面交互
   - 左侧显示 PCD 文件列表。
   - 双击列表项可加载点云文件。
   - 提供“选择目录”“打开文件”“刷新”“重置视角”等操作。
   - 显示当前文件名、有效点数量和 X/Y/Z 空间范围。

三、项目结构

1. CMakeLists.txt
   - CMake 构建配置。
   - 查找 Qt5、PCL 和 VTK。
   - 设置 Visual Studio 调试运行时 PATH。

2. src/main.cpp
   - Qt 程序入口。
   - 初始化 QVTKOpenGLNativeWidget 所需的 OpenGL 格式。

3. src/MainWindow.h / src/MainWindow.cpp
   - 主窗口和界面交互逻辑。
   - 负责文件列表、按钮、状态栏和点云信息展示。

4. src/PointCloudLoader.h / src/PointCloudLoader.cpp
   - PCD 文件读取与基础校验。
   - 使用 PCLPointCloud2 读取原始点云，再转换为 PointXYZ 进行可视化。

5. src/PointCloudView.h / src/PointCloudView.cpp
   - 点云渲染控件。
   - 封装 QVTKOpenGLNativeWidget 与 PCLVisualizer。

6. scripts/build_release.bat
   - 一键生成并编译 Release 版本。

7. scripts/run_release.bat
   - 设置 Qt、PCL、VTK、OpenNI2 的运行时 DLL 路径并启动程序。

四、开发环境

当前项目已按以下本机环境配置并验证：

1. Qt：C:\Qt\Qt5.15.2\5.15.2\msvc2019_64
2. PCL：D:\library\PCL 1.10.0
3. CMake：Visual Studio 2022 自带 CMake
4. 编译器：Visual Studio 2022 MSVC x64

如本机安装路径不同，需要调整 CMakeLists.txt 中的 PCL_ROOT、QT_RUNTIME_DIR 等路径，或在 CMake 配置时传入对应变量。

五、构建方式

方式一：使用脚本构建

在项目根目录运行：

scripts\build_release.bat

方式二：手动使用 CMake 构建

"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -S D:\code\PointCloud -B D:\code\PointCloud\build -G "Visual Studio 17 2022" -A x64 -DQt5_DIR=C:\Qt\Qt5.15.2\5.15.2\msvc2019_64\lib\cmake\Qt5

"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build D:\code\PointCloud\build --config Release --target PointCloudViewer

构建成功后，可执行文件位于：

D:\code\PointCloud\build\Release\PointCloudViewer.exe

六、运行方式

推荐使用运行脚本启动：

scripts\run_release.bat

该脚本会自动设置 Qt、PCL、VTK、OpenNI2 所需 DLL 路径，然后启动 Release 版本程序。

如果直接运行 exe，需要确保以下目录已加入 PATH：

1. C:\Qt\Qt5.15.2\5.15.2\msvc2019_64\bin
2. D:\library\PCL 1.10.0\bin
3. D:\library\PCL 1.10.0\3rdParty\VTK\bin
4. D:\library\PCL 1.10.0\3rdParty\OpenNI2\Redist

七、验收结果

1. 项目已生成 Qt + C++ + PCL 桌面程序源码。
2. CMake 配置已通过。
3. Release 版本已成功编译。
4. 程序已完成启动烟测，启动后不会立即退出。
5. 程序可读取 Resource\PCD 目录中的 PCD 文件，并在 Qt 界面中进行三维显示。

八、后续可扩展方向

1. 增加点云颜色模式切换。
2. 增加坐标轴、网格、背景色等显示设置。
3. 增加点云滤波、裁剪、降采样等处理功能。
4. 增加加载进度提示，优化大文件读取体验。
5. 增加截图、导出和视角保存功能。
