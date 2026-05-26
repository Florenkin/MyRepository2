# SF4710DC 光机 Qt 控制程序

这是一个 Windows 桌面控制程序，使用 Qt5 Widgets 和官方 `DlpHandle` SDK 控制 SF4710DC 光机。程序不直接重写串口协议，所有设备命令都通过 SDK 调用完成。

## 重要说明

`读取 SDK` 读取的是本地 `dlphandle.dll` 的版本，不需要光机响应。因此它成功不代表光机已经通信正常。

判断光机是否真正连接成功，请看连接栏状态是否显示：

```text
已连接，光机就绪
```

或在“操作 -> 设备状态”中点击“检测就绪”，日志显示：

```text
检测设备就绪: 成功
```

如果只打开了串口但光机没有响应，程序会自动关闭串口并提示更换 COM 口。双通道 USB-Serial 设备通常会出现多个 COM 口，请逐个尝试，直到光机就绪检测成功。

## 运行

已构建的 Release 程序位于：

```text
D:\code\OpticalMachine\SF4710DC_ControlQt\out\build\release\SF4710DC_ControlQt.exe
```

运行前确认同目录下存在：

```text
dlphandle.dll
Qt5Core.dll
Qt5Gui.dll
Qt5Widgets.dll
Qt5SerialPort.dll
platforms\qwindows.dll
```

## 连接步骤

1. 给光机上电。
2. 使用数据线连接光机和电脑。
3. 在设备管理器确认出现 `USB-Serial` 对应的 COM 口。
4. 启动程序，点击“刷新”。
5. 推荐先点击“自动查找”，程序会逐个尝试 COM 口。
6. 找到光机后，状态会显示“已连接，光机就绪”。
7. 如果手动连接，在串口下拉框选择 `USB-Serial` 对应的 COM 口后点击“连接”。
8. 如果提示光机未响应，请换另一个 `USB-Serial Dual Channel` 的 COM 口。
9. 状态显示“已连接，光机就绪”后再使用其他功能。

## 功能分区

### 连接

- 枚举电脑上的串口。
- 下拉框显示 COM 口、设备描述和厂商信息。
- 提供“自动查找”，逐个尝试串口，直到光机通过通信验证。
- 连接后自动调用 `SF_deviceCheckReady()`，并继续读取 MCU 版本验证控制通道。
- 连接失败或选错双通道 COM 口时给出明确提示。

### 操作

- 读取 MCU 版本、SDK 版本、操作模式、运行状态。
- 设置 DLP 电源、操作模式、色温、Splash 画面。
- 软触发启动/停止。
- 图像镜像、测试图案。
- RGB 使能、电流、最大电流。
- 触发输出/输入、图案就绪信号。
- 图案控制和 IIC 读写。

### 高级

- MCU Flash 文件列表、删除、清空。
- PT/PB 文件上传、保存、生成示例 PB。
- MCU/DLP 固件升级。
- DLP 镜像导出。
- MCU 复位、DLP 重启、恢复出厂、12V 控制。

高风险动作均有确认弹窗。

## 编译环境

当前工程按 Windows Qt5 + MSVC 构建。

已验证环境：

```text
Qt: C:\Qt\Qt5.15.2\5.15.2\msvc2019_64
MSVC: Visual Studio 2022 x64
CMake: Visual Studio bundled CMake
SDK: third_party\SF_dlp
```

## CMake 构建

在 Visual Studio Developer PowerShell 或 Developer Command Prompt 中执行：

```powershell
cmake -S D:\code\OpticalMachine\SF4710DC_ControlQt `
      -B D:\code\OpticalMachine\SF4710DC_ControlQt\out\build\release `
      -G Ninja `
      -DCMAKE_BUILD_TYPE=Release `
      -DCMAKE_PREFIX_PATH=C:\Qt\Qt5.15.2\5.15.2\msvc2019_64

cmake --build D:\code\OpticalMachine\SF4710DC_ControlQt\out\build\release --config Release
```

构建后会自动复制 `dlphandle.dll`，并调用 `windeployqt` 部署 Qt 运行库。

## Qt Creator / qmake 构建

也可以打开：

```text
SF4710DC_ControlQt.pro
```

选择 Qt5 MSVC 或 MinGW 套件构建。MSVC 构建已加入 `/utf-8`，避免中文界面乱码。

## 常见问题

### 只有“读取 SDK”能用，其他功能超时

说明本地 SDK DLL 正常，但光机命令没有响应。读取 SDK 不会访问光机。请检查：

- 是否选错了 USB-Serial 双通道中的 COM 口。
- 光机是否上电。
- COM 口是否被官方 GUI、串口助手或其他软件占用。
- 设备管理器中驱动是否正常。
- 点击“自动查找”，让程序逐个尝试 COM 口。

### 连接后立即提示光机未响应

这是程序主动做的保护。串口能打开，但 `SF_deviceCheckReady()` 失败。请更换另一个 COM 口或检查线缆、电源、驱动。

### 设备管理器没有“端口 COM”

说明系统没有把设备识别成串口。请检查 USB 数据线、光机供电、驱动安装和 USB 接口。

## 安全提示

固件升级、Flash 清空、恢复出厂、重启、电源控制会影响设备状态。操作前请确认当前连接的是目标光机，并避免中途断电或拔线。
