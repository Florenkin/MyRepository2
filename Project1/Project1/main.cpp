#include <cstdint>
#include <cstdio>
#include <thread>
#include <chrono>
//#define WIN32
#include "DlpUsbI2cTransport.h"
#define DLP_SCAN_TEST_MAIN
extern "C" {

#include "dlpc_common.h"
#include "dlpc34xx.h"
}

// ======================================================
// 1. DLPC 命令库缓冲区
// ======================================================

static uint8_t g_writeBuffer[4096];
static uint8_t g_readBuffer[4096];


// ======================================================
// 2. Cypress USB-I2C 通信对象
// ======================================================

static DlpUsbI2cTransport g_transport;


// ======================================================
// 3. 设备参数，需要你根据设备管理器修改
// ======================================================

// Cypress 常见 VID，具体以设备管理器为准
static constexpr uint16_t CYPRESS_VID = 0x04B4;

// TODO：必须改成你设备管理器里的真实 PID
static constexpr uint16_t CYPRESS_PID = 0x000A;

// DLPC 的 7-bit I2C 地址。
// 如果资料写 0x36/0x37，一般这里填 0x1B。
static constexpr uint8_t DLPC_I2C_ADDR = 0x1B;


// ======================================================
// 4. 连接 USB-I2C 光机
// ======================================================

bool ConnectProjector()
{
    if (g_transport.isOpen())
    {
        return true;
    }

    bool ok = g_transport.open(
        CYPRESS_VID,
        CYPRESS_PID,
        DLPC_I2C_ADDR
    );

    if (!ok)
    {
        printf("[DLP USB ERROR] %s\n", g_transport.lastError().c_str());
        return false;
    }

    printf("[DLP USB OK] Projector connected\n");
    return true;
}

void DisconnectProjector()
{
    g_transport.close();
    printf("[DLP USB OK] Projector disconnected\n");
}


// ======================================================
// 5. 真实光机写接口
// ======================================================

bool YourDlpTransport_Write(uint8_t* data, uint16_t length)
{
    if (!g_transport.isOpen())
    {
        printf("[DLP WRITE ERROR] transport not open\n");
        return false;
    }

    bool ok = g_transport.write(data, length);

    if (!ok)
    {
        printf("[DLP WRITE ERROR] %s\n", g_transport.lastError().c_str());
        return false;
    }

    return true;
}


// ======================================================
// 6. 真实光机读接口
// ======================================================

bool YourDlpTransport_Read(
    uint8_t* writeData,
    uint16_t writeLength,
    uint8_t* readData,
    uint16_t readLength)
{
    if (!g_transport.isOpen())
    {
        printf("[DLP READ ERROR] transport not open\n");
        return false;
    }

    bool ok = g_transport.read(
        writeData,
        writeLength,
        readData,
        readLength
    );

    if (!ok)
    {
        printf("[DLP READ ERROR] %s\n", g_transport.lastError().c_str());
        return false;
    }

    return true;
}


// ======================================================
// 7. DLPC_COMMON 回调
// ======================================================

uint32_t DlpWriteCommandCallback(
    uint16_t writeLength,
    uint8_t* writeBuffer,
    DLPC_COMMON_CommandProtocolData_s* protocolData)
{
    (void)protocolData;

    bool ok = YourDlpTransport_Write(writeBuffer, writeLength);
    return ok ? DLPC_SUCCESS : FAIL;
}

uint32_t DlpReadCommandCallback(
    uint16_t writeLength,
    uint8_t* writeBuffer,
    uint16_t readLength,
    uint8_t* readBuffer,
    DLPC_COMMON_CommandProtocolData_s* protocolData)
{
    bool ok = YourDlpTransport_Read(
        writeBuffer,
        writeLength,
        readBuffer,
        readLength
    );

    if (ok && protocolData)
    {
        protocolData->BytesRead = readLength;
    }

    return ok ? DLPC_SUCCESS : FAIL;
}


// ======================================================
// 8. 工具函数
// ======================================================

bool CheckStatus(uint32_t status, const char* msg)
{
    if (status != DLPC_SUCCESS)
    {
        printf("[DLP ERROR] %s failed, status = %u\n", msg, status);

        if (!g_transport.lastError().empty())
        {
            printf("[DLP TRANSPORT ERROR] %s\n", g_transport.lastError().c_str());
        }

        return false;
    }

    printf("[DLP OK] %s\n", msg);
    return true;
}


// ======================================================
// 9. 初始化 DLPC 命令库
// ======================================================

bool InitDlpCommandLibrary()
{
    DLPC_COMMON_InitCommandLibrary(
        g_writeBuffer,
        sizeof(g_writeBuffer),
        g_readBuffer,
        sizeof(g_readBuffer),
        DlpWriteCommandCallback,
        DlpReadCommandCallback
    );

    printf("[DLP OK] DLPC command library initialized\n");
    return true;
}


// ======================================================
// 10. 不配置参数，直接准备扫描
// ======================================================

bool PrepareDlpScanWithoutConfig()
{
    uint32_t status = 0;

    // 1. 停止上一次可能残留的播放
    status = DLPC34XX_WriteInternalPatternControl(
        DLPC34XX_PC_STOP,
        0
    );

    // 第一次调用可能本来就没播放，失败不一定直接退出
    CheckStatus(status, "Stop previous internal pattern");

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 2. Reset Pattern 状态
    //status = DLPC34XX_WriteInternalPatternControl(
    //    DLPC34XX_PC_RESET,
    //    0
    //);

    if (!CheckStatus(status, "Reset internal pattern"))
    {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 3. 切换到 Internal Pattern Streaming 模式
    status = DLPC34XX_WriteOperatingModeSelect(
        DLPC34XX_OM_SENS_INTERNAL_PATTERN
    );

    if (!CheckStatus(status, "Set internal pattern mode"))
    {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 4. 读取 Pattern 状态
    DLPC34XX_InternalPatternStatus_s patternStatus = {};
    status = DLPC34XX_ReadInternalPatternStatus(&patternStatus);

    if (!CheckStatus(status, "Read internal pattern status"))
    {
        return false;
    }

    printf("[DLP INFO] PatternReadyStatus = %d\n",
        static_cast<int>(patternStatus.PatternReadyStatus));

    printf("[DLP INFO] CurrentPatOrderEntryIndex = %d\n",
        static_cast<int>(patternStatus.CurrentPatOrderEntryIndex));

    printf("[DLP INFO] CurrentPatSetIndex = %d\n",
        static_cast<int>(patternStatus.CurrentPatSetIndex));

    printf("[DLP INFO] NumPatInCurrentPatSet = %d\n",
        static_cast<int>(patternStatus.NumPatInCurrentPatSet));

    printf("[DLP INFO] NumPatDisplayedFromPatSet = %d\n",
        static_cast<int>(patternStatus.NumPatDisplayedFromPatSet));

    if (patternStatus.PatternReadyStatus != DLPC34XX_PRS_READY)
    {
        printf("[DLP ERROR] Pattern not ready. Flash 内可能没有烧录 Pattern 数据。\n");
        return false;
    }
    printf("[DLP OK] Pattern ready\n");
    return true;
}


// ======================================================
// 11. 直接开始扫描
// ======================================================

bool StartDlpScanWithoutConfig()
{
    uint32_t status = DLPC34XX_WriteInternalPatternControl(
        DLPC34XX_PC_START,
        0
    );


    return CheckStatus(status, "Start internal pattern");
}


// ======================================================
// 12. 停止扫描
// ======================================================

bool StopDlpScan()
{
    uint32_t status = DLPC34XX_WriteInternalPatternControl(
        DLPC34XX_PC_STOP,
        0
    );

    return CheckStatus(status, "Stop internal pattern");
}


// ======================================================
// 13. 一次完整调用
// ======================================================

bool RunDlpScanOnceWithoutConfig()
{
    // 1. 打开 USB-I2C 光机
    if (!ConnectProjector())
    {
        return false;
    }

    // 2. 初始化 DLPC 命令库
    InitDlpCommandLibrary();

    // 3. 不配置 Pattern，只准备扫描
    if (!PrepareDlpScanWithoutConfig())
    {
        return false;
    }

    // 4. 左右相机先进入硬触发采集
    // cameraLeft->startHardwareTrigger();
    // cameraRight->startHardwareTrigger();

    // 5. 启动光机 Pattern
    if (!StartDlpScanWithoutConfig())
    {
        return false;
    }

    // 6. 这里等待相机采集完成
    // 建议不要用固定 sleep，最好等左右相机采到指定张数。
    //
    // while (cameraLeft->imageCount() < patternCount ||
    //        cameraRight->imageCount() < patternCount)
    // {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(5));
    // }

    // 临时测试：只让光机播放 2 秒
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 7. 停止光机
    StopDlpScan();

    return true;
}


// ======================================================
// 14. 控制台测试 main，可选
// ======================================================

#ifdef DLP_SCAN_TEST_MAIN

int main()
{
    bool ok = RunDlpScanOnceWithoutConfig();

    if (!ok)
    {
        printf("[DLP TEST] scan failed\n");
        DisconnectProjector();
        return -1;
    }

    printf("[DLP TEST] scan success\n");
    DisconnectProjector();
    return 0;
}

#endif