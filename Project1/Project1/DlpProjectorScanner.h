#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <cstdint>
#include <string>
#include <vector>
#include "DlpUsbI2cTransport.h"

extern "C" {
#include "dlpc_common.h"
#include "dlpc34xx.h"
#include "dlpc347x_internal_patterns.h"
}

/**
 * @brief 光机 USB-I2C 连接参数。
 *
 * 这个结构体只保存设备连接参数，不负责连接设备，也不发送 DLPC 命令。
 * 在 Qt 中可以把这些值放到配置界面、ini 文件或项目参数中，再传给 DlpProjectorScanner。
 */
struct DlpDeviceConfig
{
    /**
     * @brief Cypress USB-I2C 设备 VID。
     *
     * 参数含义：USB 设备的 Vendor ID。
     * 常见 Cypress VID 是 0x04B4，实际项目中应以 Windows 设备管理器中看到的值为准。
     */
    uint16_t cypressVid = 0x04B4;

    /**
     * @brief Cypress USB-I2C 设备 PID。
     *
     * 参数含义：USB 设备的 Product ID。
     * 这里必须设置为设备管理器中的真实 PID，否则无法打开 USB-I2C 设备。
     */
    uint16_t cypressPid = 0x000A;

    /**
     * @brief DLPC 的 7-bit I2C 地址。
     *
     * 参数含义：DLPC 控制芯片在 I2C 总线上的 7-bit 地址。
     * 如果资料中写的是 8-bit 地址 0x36/0x37，一般这里填写 0x1B。
     */
    uint8_t dlpcI2cAddress = 0x1B;
};

/**
 * @brief 一条 Internal Pattern 播放配置。
 *
 * 这个结构体对应 Pattern Order Table 的一行。
 * 一行可以指定从哪个 Pattern Set、哪一张 pattern 开始播放，并设置播放数量、LED 和曝光时间。
 */
struct DlpInternalPatternConfig
{
    /**
     * @brief Flash 中的 Pattern Set 编号。
     *
     * 参数含义：烧录到 Flash 里的 Pattern Set 索引。
     * 通常从 0 开始，例如 0 表示第一组条纹图。
     */
    uint8_t patternSetIndex = 0;

    /**
     * @brief Pattern Set 内的起始 pattern 编号。
     *
     * 参数含义：从指定 Pattern Set 中的第几张图开始播放。
     * 通常填写 0，表示从该组第一张图开始。
     */
    uint8_t patternEntryIndex = 0;

    /**
     * @brief 本条目要播放的 pattern 数量。
     *
     * 参数含义：从 patternEntryIndex 开始，连续播放多少张条纹图。
     * 例如一组有 8 张条纹图，这里可以填写 8。
     */
    uint8_t numberOfPatternsToDisplay = 0;

    /**
     * @brief 是否反相投影。
     *
     * 参数含义：false 表示按原图播放，true 表示黑白反相播放。
     */
    bool invertPatterns = false;

    /**
     * @brief 红色 LED 是否启用。
     *
     * 参数含义：DLPC34XX_IE_ENABLE 表示打开红色 LED，DLPC34XX_IE_DISABLE 表示关闭。
     */
    DLPC34XX_IlluminatorEnable_e redLed = DLPC34XX_IE_DISABLE;

    /**
     * @brief 绿色 LED 是否启用。
     *
     * 参数含义：DLPC34XX_IE_ENABLE 表示打开绿色 LED，DLPC34XX_IE_DISABLE 表示关闭。
     * 结构光条纹常使用单色投影，可以只打开绿色 LED。
     */
    DLPC34XX_IlluminatorEnable_e greenLed = DLPC34XX_IE_ENABLE;

    /**
     * @brief 蓝色 LED 是否启用。
     *
     * 参数含义：DLPC34XX_IE_ENABLE 表示打开蓝色 LED，DLPC34XX_IE_DISABLE 表示关闭。
     */
    DLPC34XX_IlluminatorEnable_e blueLed = DLPC34XX_IE_DISABLE;

    /**
     * @brief 单张 pattern 的点亮时间，单位 us。
     *
     * 参数含义：每张条纹图实际投影亮场持续的时间。
     * 这个值需要和相机曝光时间、触发时序匹配。
     */
    uint32_t illuminationTimeUs = 8000;

    /**
     * @brief 点亮前暗场时间，单位 us。
     *
     * 参数含义：每张 pattern 点亮前预留的黑场时间。
     * 可用于给相机触发和 DMD 状态切换留余量。
     */
    uint32_t preIlluminationDarkTimeUs = 500;

    /**
     * @brief 点亮后暗场时间，单位 us。
     *
     * 参数含义：每张 pattern 点亮后预留的黑场时间。
     * 可用于避免相邻 pattern 的时序互相影响。
     */
    uint32_t postIlluminationDarkTimeUs = 500;
};

/**
 * @brief 一次 Internal Pattern 扫描的参数。
 *
 * 这个结构体只描述扫描参数，不连接设备，也不向光机发送命令。
 * 操作类 DlpProjectorScanner 会读取这个结构体，并按需写入 DLPC。
 */
struct DlpScanConfig
{
    /**
     * @brief Internal Pattern 序列类型。
     *
     * 参数含义：告诉 DLPC 当前 pattern 是 1-bit/8-bit、单色/RGB。
     * 可选值包括 DLPC34XX_ST_ONE_BIT_MONO、DLPC34XX_ST_ONE_BIT_RGB、
     * DLPC34XX_ST_EIGHT_BIT_MONO、DLPC34XX_ST_EIGHT_BIT_RGB。
     */
    DLPC34XX_SequenceType_e sequenceType = DLPC34XX_ST_ONE_BIT_MONO;

    /**
     * @brief 当前 Pattern Set 中的总 pattern 数。
     *
     * 参数含义：Flash 中当前 Pattern Set 实际包含多少张图。
     * 必须和烧录到 Flash 里的 pattern 数量一致。
     */
    uint8_t numberOfPatternsInSet = 8;

    /**
     * @brief 扫描前是否写入 Pattern Configuration。
     *
     * 参数含义：true 表示每次扫描前重新写入全局 pattern 配置；
     * false 表示沿用光机当前配置。
     */
    bool writePatternConfiguration = true;

    /**
     * @brief 扫描前是否写入 Pattern Order Table。
     *
     * 参数含义：true 表示扫描前按 internalPatterns 重写播放顺序；
     * false 表示沿用光机当前 Pattern Order Table。
     */
    bool writePatternOrderTable = true;

    /**
     * @brief 是否写入 Internal Pattern Display Configuration。
     *
     * 参数含义：true 表示配置 DMD block 裁剪；false 表示不改变当前裁剪配置。
     */
    bool writeInternalPatternDisplayConfiguration = false;

    /**
     * @brief DMD block 裁剪起始位置。
     *
     * 参数含义：启用 writeInternalPatternDisplayConfiguration 后生效。
     * 表示从第几个 DMD block 开始显示。
     */
    uint8_t dmdBlockStart = 0;

    /**
     * @brief DMD block 裁剪数量。
     *
     * 参数含义：启用 writeInternalPatternDisplayConfiguration 后生效。
     * 表示连续显示多少个 DMD block。
     */
    uint8_t dmdBlockCount = 0;

    /**
     * @brief 是否配置 TriggerOut1。
     *
     * 参数含义：true 表示启用 TriggerOut1，常用于给相机硬触发；
     * false 表示不修改 TriggerOut1 配置。
     */
    bool writeTriggerOut1 = true;

    /**
     * @brief 是否配置 TriggerOut2。
     *
     * 参数含义：true 表示启用 TriggerOut2；false 表示不修改 TriggerOut2 配置。
     */
    bool writeTriggerOut2 = false;

    /**
     * @brief TriggerOut1 是否反相。
     *
     * 参数含义：DLPC34XX_TI_NOT_INVERTED 表示不反相，
     * DLPC34XX_TI_INVERTED 表示触发信号反相。
     */
    DLPC34XX_TriggerInversion_e triggerOut1Inversion = DLPC34XX_TI_NOT_INVERTED;

    /**
     * @brief TriggerOut2 是否反相。
     *
     * 参数含义：DLPC34XX_TI_NOT_INVERTED 表示不反相，
     * DLPC34XX_TI_INVERTED 表示触发信号反相。
     */
    DLPC34XX_TriggerInversion_e triggerOut2Inversion = DLPC34XX_TI_NOT_INVERTED;

    /**
     * @brief TriggerOut1 延迟时间。
     *
     * 参数含义：TriggerOut1 相对 pattern 时序的延迟，单位由 DLPC API 定义。
     * 当前示例填写 0，表示不增加额外延迟。
     */
    int32_t triggerOut1Delay = 0;

    /**
     * @brief TriggerOut2 延迟时间。
     *
     * 参数含义：TriggerOut2 相对 pattern 时序的延迟，单位由 DLPC API 定义。
     * 当前示例填写 0，表示不增加额外延迟。
     */
    int32_t triggerOut2Delay = 0;

    /**
     * @brief START 命令的重复次数。
     *
     * 参数含义：传给 DLPC34XX_WriteInternalPatternControl(START, repeatCount)。
     * 0 通常表示持续循环播放，非 0 表示按固件定义重复指定次数。
     */
    uint8_t repeatCount = 0;

    /**
     * @brief Pattern Order Table 配置数组。
     *
     * 参数含义：每个元素对应一行 Pattern Order Table。
     * 可以放多行，用于按顺序播放多组条纹图。
     */
    std::vector<DlpInternalPatternConfig> internalPatterns;
};

/**
 * @brief 一张准备写入 Flash 的条纹 pattern 数据。
 *
 * 这个结构体只保存单张 pattern 的一维像素数组，不负责生成或写入。
 * 注意：TI 的 internal pattern 生成器需要一维数组，不是 BMP/PNG 这种二维图片文件。
 */
struct DlpPatternImage
{
    /**
     * @brief 单张 pattern 的一维像素数组。
     *
     * 参数含义：
     * 1-bit pattern 时，每个元素应为 0 或 1；
     * 8-bit pattern 时，每个元素应为 0 到 255。
     * 横向 pattern 时长度一般等于 DMD 高度；纵向 pattern 时长度一般等于 DMD 宽度。
     */
    std::vector<uint8_t> pixels;
};

/**
 * @brief 一组准备写入 Flash 的条纹 pattern 数据。
 *
 * 这个结构体对应一个 Pattern Set。
 * 一个 Pattern Set 可以包含多张相同 bit depth、相同方向的 pattern。
 */
struct DlpPatternSetData
{
    /**
     * @brief Pattern Set 的位深。
     *
     * 参数含义：指定本组 pattern 是 1-bit、2-bit、3-bit、4-bit 还是 8-bit。
     * 常用结构光黑白条纹一般使用 DLPC34XX_INT_PAT_BITDEPTH_ONE。
     */
    DLPC34XX_INT_PAT_BitDepth_e depth = DLPC34XX_INT_PAT_BITDEPTH_EIGHT;

    /**
     * @brief Pattern Set 的方向。
     *
     * 参数含义：DLPC34XX_INT_PAT_DIRECTION_HORIZONTAL 表示横向变化；
     * DLPC34XX_INT_PAT_DIRECTION_VERTICAL 表示纵向变化。
     */
    DLPC34XX_INT_PAT_Direction_e direction = DLPC34XX_INT_PAT_DIRECTION_VERTICAL;

    /**
     * @brief Pattern Set 内的所有 pattern 图片。
     *
     * 参数含义：每个元素是一张条纹 pattern 的一维像素数组。
     */
    std::vector<DlpPatternImage> patterns;
};

/**
 * @brief 生成 Internal Pattern Flash 数据块所需的参数。
 *
 * 这个结构体只描述要生成什么样的条纹数据块，不负责写 Flash。
 * 生成后的二进制数据可以传给 writeInternalPatternDataToFlash() 写入光机。
 */
struct DlpPatternDataBlockConfig
{
    /**
     * @brief DMD 型号。
     *
     * 参数含义：告诉 TI 生成器当前光机使用的 DMD 尺寸。
     * 可选值包括 DLPC34XX_INT_PAT_DMD_DLP2010、DLPC34XX_INT_PAT_DMD_DLP3010、
     * DLPC34XX_INT_PAT_DMD_DLP4710。
     */
    DLPC34XX_INT_PAT_DMD_e dmd = DLPC34XX_INT_PAT_DMD_DLP4710;

    /**
     * @brief 是否进行东西方向翻转。
     *
     * 参数含义：true 表示生成数据时做 East/West flip；false 表示不翻转。
     */
    bool eastWestFlip = false;

    /**
     * @brief 是否沿长轴翻转。
     *
     * 参数含义：true 表示生成数据时沿 DMD 长轴翻转；false 表示不翻转。
     */
    bool longAxisFlip = false;

    /**
     * @brief 要写入 Flash 的 Pattern Set 数组。
     *
     * 参数含义：每个元素是一组条纹 pattern 数据。
     */
    std::vector<DlpPatternSetData> patternSets;

    /**
     * @brief Pattern Order Table 播放顺序。
     *
     * 参数含义：每个元素对应 Pattern Order Table 的一行，用来描述写入 Flash 后如何播放这些 pattern。
     * 这里复用 DlpInternalPatternConfig，便于和现有播放配置保持一致。
     */
    std::vector<DlpInternalPatternConfig> patternOrder;
};

/**
 * @brief DLPC Internal Pattern 扫描控制类。
 *
 * 这个类只负责设备操作：连接光机、初始化 DLPC 命令库、写扫描配置、启动扫描、停止扫描和写入条纹数据。
 * 参数与操作已经分离：所有可调参数放在 DlpDeviceConfig、DlpScanConfig、DlpInternalPatternConfig 等结构体中。
 */
class DlpProjectorScanner
{
public:
    /**
     * @brief 构造扫描控制对象。
     *
     * @param deviceConfig 光机连接参数，包括 Cypress VID、PID、DLPC I2C 地址。
     * @return 构造函数没有返回值。
     *
     * 方法作用：保存设备连接参数，但不会立即连接光机。
     */
    explicit DlpProjectorScanner(const DlpDeviceConfig& deviceConfig);

    /**
     * @brief 析构扫描控制对象。
     *
     * @return 析构函数没有返回值。
     *
     * 方法作用：对象销毁时自动关闭 USB-I2C 连接，避免设备句柄残留。
     */
    ~DlpProjectorScanner();

    DlpProjectorScanner(const DlpProjectorScanner&) = delete;
    DlpProjectorScanner& operator=(const DlpProjectorScanner&) = delete;

    /**
     * @brief 修改设备连接参数。
     *
     * @param deviceConfig 新的光机连接参数，包括 Cypress VID、PID、DLPC I2C 地址。
     * @return 无返回值。
     *
     * 方法作用：更新对象内部保存的设备参数。
     * 注意：如果设备已经连接，应先调用 disconnectProjector()，再修改参数并重新连接。
     */
    void setDeviceConfig(const DlpDeviceConfig& deviceConfig);

    /**
     * @brief 获取当前设备连接参数。
     *
     * @return 返回当前保存的 DlpDeviceConfig。
     *
     * 方法作用：让 Qt 界面或业务代码读取当前连接参数，用于显示、保存或调试。
     */
    DlpDeviceConfig deviceConfig() const;

    /**
     * @brief 判断 USB-I2C 是否已经打开。
     *
     * @return true 表示已连接；false 表示未连接。
     *
     * 方法作用：给 Qt 界面刷新连接状态，例如更新“连接/断开”按钮状态。
     */
    bool isConnected() const;

    /**
     * @brief 连接 USB-I2C 光机。
     *
     * @return true 表示连接成功或之前已经连接；false 表示连接失败。
     *
     * 方法作用：打开 Cypress USB-I2C 设备，并设置 DLPC I2C 地址。
     * 失败原因可通过 lastError() 查看。
     */
    bool connectProjector();

    /**
     * @brief 断开 USB-I2C 光机连接。
     *
     * @return 无返回值。
     *
     * 方法作用：关闭 Cypress USB-I2C 设备句柄。
     */
    void disconnectProjector();

    /**
     * @brief 初始化 DLPC 命令库。
     *
     * @return true 表示初始化完成。
     *
     * 方法作用：向 TI 的 DLPC_COMMON 命令库注册读写回调和收发缓冲区。
     */
    bool initializeCommandLibrary();

    /**
     * @brief 校验扫描参数是否合法。
     *
     * @param config 本次扫描要使用的 DlpScanConfig 参数。
     * @return true 表示参数基本合法；false 表示参数缺失或明显错误。
     *
     * 方法作用：在真正写入 DLPC 前做基础检查，例如条纹数组不能为空、播放数量不能为 0、LED 不能全关。
     */
    bool validateScanConfig(const DlpScanConfig& config);

    /**
     * @brief 把扫描参数写入 DLPC。
     *
     * @param config 本次扫描要使用的 DlpScanConfig 参数。
     * @return true 表示配置写入成功；false 表示参数校验失败或 I2C 命令写入失败。
     *
     * 方法作用：根据 config 写入 Pattern Configuration、触发输出配置和 Pattern Order Table。
     */
    bool configureScan(const DlpScanConfig& config);

    /**
     * @brief 准备 Internal Pattern 扫描。
     *
     * @param config 本次扫描要使用的 DlpScanConfig 参数。
     * @return true 表示光机已进入 Internal Pattern 模式并且 pattern ready；false 表示准备失败。
     *
     * 方法作用：停止上一次播放、重置状态、写入参数、切换模式，并读取状态确认 PatternReadyStatus。
     */
    bool prepareScan(const DlpScanConfig& config);

    /**
     * @brief 启动 Internal Pattern 播放。
     *
     * @param config 本次扫描要使用的 DlpScanConfig 参数。
     * @return true 表示启动命令发送成功；false 表示启动失败。
     *
     * 方法作用：向 DLPC 发送 START 命令，repeatCount 使用 config.repeatCount。
     */
    bool startScan(const DlpScanConfig& config);

    /**
     * @brief 停止 Internal Pattern 播放。
     *
     * @return true 表示停止命令发送成功；false 表示停止失败。
     *
     * 方法作用：向 DLPC 发送 STOP 命令。
     */
    bool stopScan();

    /**
     * @brief 执行一次阻塞式测试扫描。
     *
     * @param config 本次扫描要使用的 DlpScanConfig 参数。
     * @param playTimeMs 光机播放持续时间，单位 ms。
     * @return true 表示完整流程成功；false 表示连接、初始化、准备、启动或停止过程中失败。
     *
     * 方法作用：用于控制台快速测试。Qt 主线程中不要直接调用这个阻塞方法。
     */
    bool runScanFor(const DlpScanConfig& config, uint32_t playTimeMs);

    /**
     * @brief 获取最近一次错误信息。
     *
     * @return 返回错误字符串；如果最近没有错误，返回空字符串。
     *
     * 方法作用：给 Qt 界面显示错误弹窗或写日志。
     */
    std::string lastError() const;

    /**
     * @brief 生成 Internal Pattern 二进制数据块。
     *
     * @param config 数据块生成参数，包括 DMD 型号、Pattern Set 数据、Pattern Order Table 和翻转选项。
     * @param patternDataBlock 输出参数。方法成功后，这里保存可写入 Flash 的 Internal Pattern 二进制数据。
     * @return true 表示生成成功；false 表示参数错误或 TI 生成器返回错误。
     *
     * 方法作用：把一维条纹数组转换成 DLPC 可识别的 Internal Pattern Flash 数据块。
     * 注意：这个方法只生成内存数据，不会写入光机 Flash。
     */
    bool generateInternalPatternDataBlock(
        const DlpPatternDataBlockConfig& config,
        std::vector<uint8_t>* patternDataBlock);

    /**
     * @brief 将 Internal Pattern 二进制数据块写入光机 Flash。
     *
     * @param patternDataBlock 要写入 Flash 的 Internal Pattern 二进制数据。
     * @param chunkSize 每次 FlashStart/FlashContinue 写入的最大字节数。
     * @return true 表示写入命令流程完成；false 表示参数错误、预检查失败、擦除失败或写入失败。
     *
     * 方法作用：把 generateInternalPatternDataBlock() 生成的数据写入
     * DLPC34XX_FDTS_ENTIRE_SENS_PATTERN_DATA 区域。
     * 注意：这是 Flash 写入操作，会覆盖光机中的 Sensing Pattern Data，使用前应确认数据正确。
     */
    bool writeInternalPatternDataToFlash(
        const std::vector<uint8_t>& patternDataBlock,
        uint16_t chunkSize = 1024);

private:
    /**
     * @brief DLPC_COMMON 写命令回调函数。
     *
     * @param writeLength 要写入的字节数。
     * @param writeBuffer 要写入的命令数据缓冲区。
     * @param protocolData DLPC_COMMON 传入的协议上下文，本实现未直接使用。
     * @return 0 表示写入成功；非 0 表示写入失败。
     *
     * 方法作用：把 TI C 接口的写命令转发到当前 DlpProjectorScanner 实例。
     */
    static uint32_t writeCommandCallback(
        uint16_t writeLength,
        uint8_t* writeBuffer,
        DLPC_COMMON_CommandProtocolData_s* protocolData);

    /**
     * @brief DLPC_COMMON 读命令回调函数。
     *
     * @param writeLength 读之前需要先写入的命令字节数。
     * @param writeBuffer 读命令的写入缓冲区。
     * @param readLength 期望读取的字节数。
     * @param readBuffer 接收读取结果的缓冲区。
     * @param protocolData DLPC_COMMON 传入的协议上下文，本实现未直接使用。
     * @return 0 表示读取成功；非 0 表示读取失败。
     *
     * 方法作用：把 TI C 接口的读命令转发到当前 DlpProjectorScanner 实例。
     */
    static uint32_t readCommandCallback(
        uint16_t writeLength,
        uint8_t* writeBuffer,
        uint16_t readLength,
        uint8_t* readBuffer,
        DLPC_COMMON_CommandProtocolData_s* protocolData);

    /**
     * @brief 向 USB-I2C 设备写入命令。
     *
     * @param data 要写入的命令数据。
     * @param length 要写入的字节数。
     * @return true 表示写入成功；false 表示设备未连接或写入失败。
     *
     * 方法作用：封装 transport_ 的写操作，并记录错误信息。
     */
    bool writeCommand(uint8_t* data, uint16_t length);

    /**
     * @brief 通过 USB-I2C 设备执行读命令。
     *
     * @param writeData 读之前先发送的命令数据。
     * @param writeLength 需要先发送的命令字节数。
     * @param readData 接收读取结果的缓冲区。
     * @param readLength 期望读取的字节数。
     * @return true 表示读取成功；false 表示设备未连接或读取失败。
     *
     * 方法作用：封装 transport_ 的读操作，并记录错误信息。
     */
    bool readCommand(
        uint8_t* writeData,
        uint16_t writeLength,
        uint8_t* readData,
        uint16_t readLength);

    /**
     * @brief 检查 DLPC API 返回状态。
     *
     * @param status DLPC API 返回的状态码。
     * @param message 失败时写入 lastError_ 的错误描述前缀。
     * @return true 表示状态正常；false 表示状态为错误。
     *
     * 方法作用：统一处理 DLPC API 的返回值和错误信息。
     */
    bool checkStatus(uint32_t status, const char* message);

    /**
     * @brief 生成 pattern 反相掩码。
     *
     * @param invertPatterns 是否反相播放。
     * @return 返回写入 Pattern Order Table 所需的反相掩码。
     *
     * 方法作用：把布尔参数转换成 DLPC 使用的 bit mask。
     */
    static uint32_t patternInvertMask(bool invertPatterns);

    /**
     * @brief 返回两个 uint32_t 值中的较大值。
     *
     * @param left 左侧比较值。
     * @param right 右侧比较值。
     * @return 返回 left 和 right 中较大的一个。
     *
     * 方法作用：避免 Windows 头文件中的 max 宏和标准库函数冲突。
     */
    static uint32_t maxU32(uint32_t left, uint32_t right);

    /**
     * @brief 计算扫描配置中的最大点亮时间。
     *
     * @param config 扫描配置。
     * @return 返回 internalPatterns 中最大的 illuminationTimeUs。
     *
     * 方法作用：写 Pattern Configuration 时需要给出所有 pattern 的最大亮场时间。
     */
    static uint32_t maxIlluminationTimeFromConfig(const DlpScanConfig& config);

    /**
     * @brief 计算扫描配置中的最大点亮前暗场时间。
     *
     * @param config 扫描配置。
     * @return 返回 internalPatterns 中最大的 preIlluminationDarkTimeUs。
     *
     * 方法作用：写 Pattern Configuration 时需要给出所有 pattern 的最大点亮前暗场时间。
     */
    static uint32_t maxPreDarkTimeFromConfig(const DlpScanConfig& config);

    /**
     * @brief 计算扫描配置中的最大点亮后暗场时间。
     *
     * @param config 扫描配置。
     * @return 返回 internalPatterns 中最大的 postIlluminationDarkTimeUs。
     *
     * 方法作用：写 Pattern Configuration 时需要给出所有 pattern 的最大点亮后暗场时间。
     */
    static uint32_t maxPostDarkTimeFromConfig(const DlpScanConfig& config);

    /**
     * @brief 打印 Internal Pattern 状态。
     *
     * @param patternStatus DLPC 读取到的 Internal Pattern 状态结构体。
     * @return 无返回值。
     *
     * 方法作用：用于控制台调试，查看 PatternReadyStatus、当前 Pattern Set 等状态。
     */
    static void printInternalPatternStatus(const DLPC34XX_InternalPatternStatus_s& patternStatus);

    /**
     * @brief 收集 Internal Pattern 数据生成器输出。
     *
     * @param length 本次回调输出的数据字节数。
     * @param data 本次回调输出的数据指针。
     * @return 无返回值。
     *
     * 方法作用：TI 生成器通过 C 回调分段输出数据，本函数把分段数据追加到 activePatternDataBlock_。
     */
    static void collectPatternDataCallback(uint8_t length, uint8_t* data);

    /**
     * @brief 根据 pattern 配置得到 LED 选择值。
     *
     * @param pattern 单条 Internal Pattern 播放配置。
     * @return 返回 DLPC34XX_INT_PAT_IlluminationSelect_e，用于 Internal Pattern 数据块生成。
     *
     * 方法作用：把 redLed、greenLed、blueLed 三个开关转换为 TI internal pattern 生成器需要的 LED 枚举。
     */
    static DLPC34XX_INT_PAT_IlluminationSelect_e illuminationFromPatternConfig(
        const DlpInternalPatternConfig& pattern);

    /**
     * @brief 设置最近一次错误信息。
     *
     * @param errorMessage 要保存的错误文本。
     * @return 无返回值。
     *
     * 方法作用：统一更新 lastError_，供 lastError() 查询。
     */
    void setLastError(const std::string& errorMessage);

private:
    /**
     * @brief 当前注册到 DLPC_COMMON 的活动实例。
     *
     * 参数含义：TI 的 C 回调接口不能直接传入 this，因此用该静态指针把回调转回类实例。
     * 注意：DLPC_COMMON 是全局命令库，建议一个进程同一时间只用一个 DlpProjectorScanner 实例操作光机。
     */
    static DlpProjectorScanner* activeInstance_;

    /**
     * @brief Internal Pattern 数据生成回调使用的临时输出缓冲区。
     *
     * 参数含义：TI 的 DLPC34XX_INT_PAT_GeneratePatternDataBlock() 使用 C 回调输出数据，
     * 这里用静态指针把回调数据收集到调用者传入的 std::vector<uint8_t> 中。
     */
    static std::vector<uint8_t>* activePatternDataBlock_;

    /**
     * @brief 当前设备连接参数。
     *
     * 参数含义：保存 Cypress VID、PID、DLPC I2C 地址。
     */
    DlpDeviceConfig deviceConfig_;

    /**
     * @brief Cypress USB-I2C 通信对象。
     *
     * 参数含义：封装 USB-I2C 打开、关闭、读写操作。
     */
    DlpUsbI2cTransport transport_;

    /**
     * @brief DLPC 命令库写缓冲区。
     *
     * 参数含义：供 DLPC_COMMON 打包写命令使用。
     */
    uint8_t writeBuffer_[4096] = {};

    /**
     * @brief DLPC 命令库读缓冲区。
     *
     * 参数含义：供 DLPC_COMMON 接收读命令结果使用。
     */
    uint8_t readBuffer_[4096] = {};

    /**
     * @brief 最近一次错误信息。
     *
     * 参数含义：保存连接、读写、配置、启动、停止过程中产生的错误文本。
     */
    std::string lastError_;
};
