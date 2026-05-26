# 项目结构

```text
SF4710DC_ControlQt/
├── CMakeLists.txt
├── CMakePresets.json
├── CMakeUserPresets.json
├── README.md
├── STRUCTURE.md
├── SF4710DC_ControlQt.pro
├── src/
│   ├── main.cpp
│   ├── DeviceController.h
│   ├── DeviceController.cpp
│   ├── MainWindow.h
│   └── MainWindow.cpp
├── third_party/
│   └── SF_dlp/
│       ├── inc/
│       │   ├── dlpdefine.h
│       │   ├── dlphandle.h
│       │   ├── dlphandle_global.h
│       │   └── simpleQtLogger.h
│       ├── lib_win/
│       │   ├── MSVC2015_64/
│       │   │   ├── dlphandle.dll
│       │   │   ├── dlphandle.lib
│       │   │   ├── dlphandled.dll
│       │   │   └── dlphandled.lib
│       │   └── MinGW_32/
│       │       ├── dlphandle.dll
│       │       ├── dlphandled.dll
│       │       ├── libdlphandle.a
│       │       └── libdlphandled.a
│       └── lib_linux/
│           └── lib_linux.tar.xz
└── out/
    └── build/
        └── release/
            ├── SF4710DC_ControlQt.exe
            ├── dlphandle.dll
            ├── Qt5Core.dll
            ├── Qt5Gui.dll
            ├── Qt5SerialPort.dll
            ├── Qt5Widgets.dll
            ├── platforms/
            │   └── qwindows.dll
            └── ...
```

## 主要文件说明

`CMakeLists.txt`

配置 Qt5 Widgets、Qt5 SerialPort、官方 `dlphandle.lib` 链接和运行库部署。构建后自动复制 `dlphandle.dll` 并执行 `windeployqt`。

`SF4710DC_ControlQt.pro`

qmake/Qt Creator 工程文件。保留给不使用 CMake 的构建方式。

`src/main.cpp`

Qt 应用入口，创建并显示主窗口。

`src/DeviceController.h/.cpp`

设备控制封装层。持有唯一 `DlpHandle` 实例，负责 SDK 初始化、串口枚举、打开/关闭、就绪检测、MCU 通信验证、SDK 错误文本、收发日志和进度信号转发。

`src/MainWindow.h/.cpp`

Qt 主界面。保留“连接 / 操作 / 高级 / 日志”分区，负责采集参数、调用 `DeviceController` 和显示结果。所有光机命令在发送前会确认设备已经就绪。

`third_party/SF_dlp`

官方 SDK 文件。程序只通过 SDK 访问光机，不在界面层手写串口协议。

`out/build/release`

CMake Release 输出目录。包含 exe、SDK DLL 和 Qt 运行库。

## 控制流程

```text
MainWindow
    ↓ 采集参数、显示结果
DeviceController
    ↓ 串口连接、设备就绪检测、MCU 版本验证、日志转发
DlpHandle SDK
    ↓ 官方串口协议实现
SF4710DC 光机
```

## 连接状态定义

```text
未连接
    没有打开串口

串口已连接，等待光机响应
    串口打开成功，但还没有通过就绪检测

已连接，光机就绪
    串口打开成功，并且 SF_deviceCheckReady() 与 SF_getVersion() 都返回成功
```

只有“已连接，光机就绪”时，其他功能模块才会继续向光机发送命令。
