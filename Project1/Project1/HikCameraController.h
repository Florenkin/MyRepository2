#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "MVS/MvCameraControl.h"

/**
 * @brief 相机类型。
 *
 * 参数含义：用于区分彩色相机和灰度相机，方便 Qt 界面按不同用途显示和管理。
 */
enum class HikCameraType
{
    Color,
    Mono
};

/**
 * @brief 相机触发模式。
 *
 * 参数含义：对应海康 MVS 节点 TriggerMode。
 */
enum class HikTriggerMode
{
    Off = MV_TRIGGER_MODE_OFF,
    On = MV_TRIGGER_MODE_ON
};

/**
 * @brief 相机触发源。
 *
 * 参数含义：对应海康 MVS 节点 TriggerSource。
 */
enum class HikTriggerSource
{
    Line0 = MV_TRIGGER_SOURCE_LINE0,
    Line1 = MV_TRIGGER_SOURCE_LINE1,
    Line2 = MV_TRIGGER_SOURCE_LINE2,
    Line3 = MV_TRIGGER_SOURCE_LINE3,
    Counter0 = MV_TRIGGER_SOURCE_COUNTER0,
    Software = MV_TRIGGER_SOURCE_SOFTWARE,
    FrequencyConverter = MV_TRIGGER_SOURCE_FrequencyConverter,
    LinkTrigger0 = 21
};

/**
 * @brief 单个相机的配置参数。
 *
 * 这个结构体只保存相机参数，不负责打开相机，也不执行采集。
 * Qt 中可以把这些值放到参数界面或配置文件中，再传给 HikCameraController。
 */
struct HikCameraConfig
{
    /**
     * @brief 相机逻辑名称。
     *
     * 参数含义：给 Qt 或日志使用的名称，例如 ColorCamera、MonoCamera0。
     */
    std::string name;

    /**
     * @brief 相机类型。
     *
     * 参数含义：Color 表示彩色相机，Mono 表示灰度相机。
     */
    HikCameraType cameraType = HikCameraType::Mono;

    /**
     * @brief 枚举设备列表中的相机索引。
     *
     * 参数含义：调用 MV_CC_EnumDevices() 后，用该索引选择要打开的设备。
     * 如果现场相机顺序固定，可以先用索引快速绑定；正式项目建议后续扩展为序列号绑定。
     */
    uint32_t deviceIndex = 0;

    /**
     * @brief 曝光时间，单位 us。
     *
     * 参数含义：写入海康 MVS 的 ExposureTime 节点。
     */
    float exposureTimeUs = 10000.0f;

    /**
     * @brief 触发模式。
     *
     * 参数含义：Off 表示连续采集，On 表示等待外部触发或软触发。
     */
    HikTriggerMode triggerMode = HikTriggerMode::On;

    /**
     * @brief 触发源。
     *
     * 参数含义：选择触发信号来自 Line0/Line1/Line2/Line3/软件触发等。
     */
    HikTriggerSource triggerSource = HikTriggerSource::Line0;

    /**
     * @brief 单帧取图超时时间，单位 ms。
     *
     * 参数含义：调用 getOneFrame() 等待图像时最多等待多久。
     */
    uint32_t frameTimeoutMs = 1000;
};

/**
 * @brief 五相机系统配置。
 *
 * 这个结构体只保存整套相机参数：1 个彩色相机和 4 个灰度相机。
 */
struct HikCameraSystemConfig
{
    /**
     * @brief 彩色相机配置。
     *
     * 参数含义：系统中的 1 个彩色相机参数。
     */
    HikCameraConfig colorCamera;

    /**
     * @brief 灰度相机配置数组。
     *
     * 参数含义：系统中的 4 个灰度相机参数，灰度相机由海康采集卡控制。
     */
    std::vector<HikCameraConfig> monoCameras;
};

/**
 * @brief 单帧图像数据。
 *
 * 这个结构体保存从海康 SDK 取到的一帧图像数据和帧信息。
 */
struct HikCameraFrame
{
    /**
     * @brief 图像字节数据。
     *
     * 参数含义：从 SDK 图像缓存复制出来的原始图像数据。
     */
    std::vector<uint8_t> data;

    /**
     * @brief 图像帧信息。
     *
     * 参数含义：包含宽、高、像素格式、帧号、曝光时间等信息。
     */
    MV_FRAME_OUT_INFO_EX frameInfo = {};
};

/**
 * @brief 海康五相机控制类。
 *
 * 这个类负责相机操作：初始化 SDK、枚举设备、打开相机、写入参数、开始采集、停止采集、取图和关闭相机。
 * 参数与操作分离：所有可调参数放在 HikCameraConfig 和 HikCameraSystemConfig 中。
 */
class HikCameraController
{
public:
    /**
     * @brief 构造相机控制对象。
     *
     * @param config 五相机系统配置，包括 1 个彩色相机和 4 个灰度相机参数。
     * @return 构造函数没有返回值。
     *
     * 方法作用：保存配置参数，但不会立即打开相机。
     */
    explicit HikCameraController(const HikCameraSystemConfig& config);

    /**
     * @brief 析构相机控制对象。
     *
     * @return 析构函数没有返回值。
     *
     * 方法作用：对象销毁时自动停止采集并关闭所有已打开的相机。
     */
    ~HikCameraController();

    HikCameraController(const HikCameraController&) = delete;
    HikCameraController& operator=(const HikCameraController&) = delete;

    /**
     * @brief 修改五相机系统配置。
     *
     * @param config 新的五相机系统配置。
     * @return 无返回值。
     *
     * 方法作用：更新内部保存的参数。若相机已经打开，建议先 closeAllCameras() 再修改。
     */
    void setConfig(const HikCameraSystemConfig& config);

    /**
     * @brief 获取当前五相机系统配置。
     *
     * @return 返回当前保存的 HikCameraSystemConfig。
     *
     * 方法作用：给 Qt 界面读取当前参数，用于显示、保存或调试。
     */
    HikCameraSystemConfig config() const;

    /**
     * @brief 初始化海康 MVS SDK。
     *
     * @return true 表示初始化成功或已经初始化；false 表示初始化失败。
     *
     * 方法作用：调用 MV_CC_Initialize()，在枚举和打开相机前执行。
     */
    bool initializeSdk();

    /**
     * @brief 释放海康 MVS SDK。
     *
     * @return 无返回值。
     *
     * 方法作用：关闭所有相机后调用 MV_CC_Finalize() 释放 SDK 资源。
     */
    void finalizeSdk();

    /**
     * @brief 枚举当前可用相机设备。
     *
     * @param transportLayerType 海康传输层类型，例如 MV_GIGE_DEVICE、MV_USB_DEVICE 或组合值。
     * @return true 表示枚举成功；false 表示枚举失败。
     *
     * 方法作用：刷新内部设备列表，后续 openAllCameras() 会按 deviceIndex 从该列表选择设备。
     */
    bool enumerateDevices(unsigned int transportLayerType);

    /**
     * @brief 获取枚举到的设备数量。
     *
     * @return 返回最近一次 enumerateDevices() 枚举到的设备数量。
     *
     * 方法作用：给 Qt 界面显示当前可用相机数量。
     */
    uint32_t deviceCount() const;

    /**
     * @brief 打开所有配置中的相机。
     *
     * @return true 表示所有相机都打开成功；false 表示至少一个相机打开失败。
     *
     * 方法作用：按 HikCameraSystemConfig 中的彩色相机和灰度相机配置创建句柄并打开设备。
     */
    bool openAllCameras();

    /**
     * @brief 关闭所有相机。
     *
     * @return 无返回值。
     *
     * 方法作用：停止采集、关闭设备并销毁海康 SDK 句柄。
     */
    void closeAllCameras();

    /**
     * @brief 判断所有配置中的相机是否都已打开。
     *
     * @return true 表示所有相机都已打开；false 表示至少一个相机未打开。
     *
     * 方法作用：给 Qt 界面刷新连接状态。
     */
    bool areAllCamerasOpen() const;

    /**
     * @brief 给所有已打开相机写入配置参数。
     *
     * @return true 表示所有相机配置成功；false 表示至少一个相机配置失败。
     *
     * 方法作用：写入 ExposureTime、TriggerMode 和 TriggerSource。
     */
    bool applyAllCameraSettings();

    /**
     * @brief 开始所有相机采集。
     *
     * @return true 表示所有相机启动采集成功；false 表示至少一个相机启动失败。
     *
     * 方法作用：对所有已打开相机调用 MV_CC_StartGrabbing()。
     */
    bool startGrabbing();

    /**
     * @brief 停止所有相机采集。
     *
     * @return true 表示所有相机停止采集成功；false 表示至少一个相机停止失败。
     *
     * 方法作用：对所有已打开相机调用 MV_CC_StopGrabbing()。
     */
    bool stopGrabbing();

    /**
     * @brief 获取指定相机的一帧图像。
     *
     * @param cameraIndex 相机索引。0 表示彩色相机，1 到 4 表示第 1 到第 4 个灰度相机。
     * @param frame 输出参数。成功后保存图像数据和图像帧信息。
     * @return true 表示取图成功；false 表示索引错误、相机未打开或等待超时。
     *
     * 方法作用：从指定相机主动取一帧图像，并把 SDK 缓存复制到 frame.data。
     */
    bool getOneFrame(uint32_t cameraIndex, HikCameraFrame* frame);

    /**
     * @brief 获取五个相机各一帧图像。
     *
     * @param frames 输出参数。成功后依次保存彩色相机和 4 个灰度相机的图像。
     * @return true 表示五个相机都取图成功；false 表示至少一个相机取图失败。
     *
     * 方法作用：用于一次触发后收集所有相机图像。
     */
    bool getOneFrameFromAll(std::vector<HikCameraFrame>* frames);

    /**
     * @brief 将指定相机的一帧图像保存为 PNG 文件。
     *
     * @param cameraIndex 相机索引。0 表示彩色相机，1 到 4 表示第 1 到第 4 个灰度相机。
     * @param frame 要保存的图像帧。
     * @param filePath 输出 PNG 文件路径。
     * @return true 表示保存成功；false 表示索引错误、相机未打开或 SDK 保存失败。
     *
     * 方法作用：使用海康 MVS 的 MV_CC_SaveImageToFileEx() 把原始帧数据转换并保存为 PNG。
     */
    bool saveFrameAsPng(
        uint32_t cameraIndex,
        const HikCameraFrame& frame,
        const std::string& filePath);

    /**
     * @brief 发送软件触发命令。
     *
     * @param cameraIndex 相机索引。0 表示彩色相机，1 到 4 表示第 1 到第 4 个灰度相机。
     * @return true 表示软触发命令发送成功；false 表示索引错误、相机未打开或命令失败。
     *
     * 方法作用：当 TriggerSource 设置为 Software 时，用该方法主动触发一帧。
     */
    bool executeSoftwareTrigger(uint32_t cameraIndex);

    /**
     * @brief 获取最近一次错误信息。
     *
     * @return 返回错误字符串；如果最近没有错误，返回空字符串。
     *
     * 方法作用：给 Qt 界面显示错误弹窗或写日志。
     */
    std::string lastError() const;

private:
    struct CameraHandle
    {
        HikCameraConfig config;
        void* handle = nullptr;
        bool opened = false;
        bool grabbing = false;
    };

    bool openCamera(const HikCameraConfig& cameraConfig, CameraHandle* cameraHandle);
    bool applyCameraSettings(CameraHandle* cameraHandle);
    bool setEnumValue(CameraHandle* cameraHandle, const char* key, unsigned int value);
    bool setFloatValue(CameraHandle* cameraHandle, const char* key, float value);
    bool setCommandValue(CameraHandle* cameraHandle, const char* key);
    CameraHandle* cameraAt(uint32_t cameraIndex);
    const CameraHandle* cameraAt(uint32_t cameraIndex) const;
    void setLastError(const std::string& errorMessage);
    void setLastErrorWithCode(const std::string& operation, int errorCode);

private:
    HikCameraSystemConfig config_;
    MV_CC_DEVICE_INFO_LIST deviceList_ = {};
    CameraHandle colorCamera_;
    std::vector<CameraHandle> monoCameras_;
    bool sdkInitialized_ = false;
    std::string lastError_;
};
