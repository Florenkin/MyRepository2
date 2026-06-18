#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include "DlpProjectorScanner.h"
#include "HikCameraController.h"

#define CAMERA_DLP_SCAN_TEST_MAIN

/**
 * @brief 一次扫描需要采集的条纹数量。
 *
 * 参数含义：光机播放 35 张条纹图，4 个灰度相机对应采集 35 次。
 */
static const uint32_t kScanPatternCount = 35;

/**
 * @brief 创建一份默认扫描参数。
 *
 * @return 返回一份可直接用于测试的 DlpScanConfig。
 *
 * 方法作用：把光机扫描参数集中在一个地方，Qt 中也可以照这个函数组装自己的 DlpScanConfig。
 */
DlpScanConfig CreateDefaultScanConfig()
{
    DlpInternalPatternConfig stripePattern;
    stripePattern.patternSetIndex = 0;
    stripePattern.patternEntryIndex = 0;
    stripePattern.numberOfPatternsToDisplay = static_cast<uint8_t>(kScanPatternCount);
    stripePattern.invertPatterns = false;
    stripePattern.redLed = DLPC34XX_IE_DISABLE;
    stripePattern.greenLed = DLPC34XX_IE_DISABLE;
    stripePattern.blueLed = DLPC34XX_IE_ENABLE;
    stripePattern.illuminationTimeUs = 10000;
    stripePattern.preIlluminationDarkTimeUs = 1000;
    stripePattern.postIlluminationDarkTimeUs = 1000;

    DlpScanConfig scanConfig;
    scanConfig.sequenceType = DLPC34XX_ST_ONE_BIT_MONO;
    scanConfig.numberOfPatternsInSet = static_cast<uint8_t>(kScanPatternCount);
    scanConfig.writePatternConfiguration = true;
    scanConfig.writePatternOrderTable = true;
    scanConfig.writeInternalPatternDisplayConfiguration = false;
    scanConfig.dmdBlockStart = 0;
    scanConfig.dmdBlockCount = 0;
    scanConfig.writeTriggerOut1 = false;
    scanConfig.writeTriggerOut2 = true;
    scanConfig.triggerOut1Inversion = DLPC34XX_TI_NOT_INVERTED;
    scanConfig.triggerOut2Inversion = DLPC34XX_TI_NOT_INVERTED;
    scanConfig.triggerOut1Delay = 0;
    scanConfig.triggerOut2Delay = 0;
    scanConfig.repeatCount = 1;
    scanConfig.internalPatterns.push_back(stripePattern);

    return scanConfig;
}

/**
 * @brief 创建一份默认五相机参数。
 *
 * @return 返回一份可传给 HikCameraController 的 HikCameraSystemConfig。
 *
 * 方法作用：集中配置 1 个彩色网口相机和 4 个 CXP 灰度相机的默认参数。
 * 注意：deviceIndex 需要按现场 MV_CC_EnumDevices() 枚举顺序调整。
 */
HikCameraSystemConfig CreateDefaultCameraSystemConfig()
{
    HikCameraSystemConfig cameraConfig;

    cameraConfig.colorCamera.name = "ColorGigECamera";
    cameraConfig.colorCamera.cameraType = HikCameraType::Color;
    cameraConfig.colorCamera.deviceIndex = 0;
    cameraConfig.colorCamera.exposureTimeUs = 3000.0f;
    cameraConfig.colorCamera.triggerMode = HikTriggerMode::On;
    cameraConfig.colorCamera.triggerSource = HikTriggerSource::Software;
    cameraConfig.colorCamera.frameTimeoutMs = 3000;

    cameraConfig.monoCameras.resize(4);
    for (uint32_t index = 0; index < 4; ++index)
    {
        HikCameraConfig monoCamera;
        monoCamera.name = "MonoCxpCamera" + std::to_string(index + 1);
        monoCamera.cameraType = HikCameraType::Mono;
        monoCamera.deviceIndex = index + 1;
        monoCamera.exposureTimeUs = 2500.0f;
        monoCamera.triggerMode = HikTriggerMode::On;
        monoCamera.triggerSource = HikTriggerSource::LinkTrigger0;
        monoCamera.frameTimeoutMs = 3000;

        cameraConfig.monoCameras[index] = monoCamera;
    }

    return cameraConfig;
}

/**
 * @brief 保存彩色相机图像。
 *
 * @param cameraController 相机控制对象。
 * @param frame 彩色相机图像。
 * @return true 表示保存成功；false 表示保存失败。
 *
 * 方法作用：彩色相机只软触发拍摄一张，并保存到 camera_output 目录。
 */
bool SaveColorCameraFrame(HikCameraController& cameraController, const HikCameraFrame& frame)
{
    CreateDirectoryA("camera_output", nullptr);
    return cameraController.saveFrameAsPng(0, frame, "camera_output\\color_camera.png");
}

/**
 * @brief 保存一轮灰度相机图像。
 *
 * @param cameraController 相机控制对象。
 * @param frames 四个灰度相机的图像。
 * @param patternIndex 当前条纹序号。
 * @return true 表示本轮所有图像保存成功；false 表示至少一张图保存失败。
 *
 * 方法作用：把一次 DLP 条纹触发得到的 4 个灰度相机图像分别保存到 camera_output 目录。
 */
bool SaveMonoCameraFrames(
    HikCameraController& cameraController,
    const std::vector<HikCameraFrame>& frames,
    uint32_t patternIndex)
{
    CreateDirectoryA("camera_output", nullptr);

    for (uint32_t cameraIndex = 0; cameraIndex < frames.size(); ++cameraIndex)
    {
        char filePath[256] = {};
        sprintf_s(
            filePath,
            "camera_output\\pattern_%02u_mono_%u.png",
            patternIndex,
            cameraIndex + 1);

        if (!cameraController.saveFrameAsPng(cameraIndex + 1, frames[cameraIndex], filePath))
        {
            printf("[CAMERA TEST] save failed: %s, %s\n", filePath, cameraController.lastError().c_str());
            return false;
        }
    }

    return true;
}

/**
 * @brief 执行一次光机和五相机同步扫描测试。
 *
 * @return true 表示完整流程成功；false 表示相机或光机流程失败。
 *
 * 方法作用：彩色相机按网口枚举并软触发拍摄 1 张；4 个灰度相机按 CXP 采集卡枚举，
 * 启动取流后由 DLP 条纹触发采集 35 轮灰度图像。
 */
bool RunCameraDlpScanTest()
{
    HikCameraSystemConfig cameraConfig = CreateDefaultCameraSystemConfig();
    HikCameraController cameraController(cameraConfig);

    const unsigned int cameraTransportLayer = MV_GIGE_DEVICE | MV_GENTL_CXP_DEVICE;
    if (!cameraController.enumerateDevices(cameraTransportLayer))
    {
        printf("[CAMERA TEST] enumerate failed: %s\n", cameraController.lastError().c_str());
        return false;
    }

    printf("[CAMERA TEST] found %u camera devices\n", cameraController.deviceCount());
    if (cameraController.deviceCount() < 5)
    {
        printf("[CAMERA TEST] need 5 devices: 1 GigE color camera + 4 CXP mono cameras\n");
        return false;
    }

    if (!cameraController.openAllCameras())
    {
        printf("[CAMERA TEST] open failed: %s\n", cameraController.lastError().c_str());
        return false;
    }

    if (!cameraController.applyAllCameraSettings())
    {
        printf("[CAMERA TEST] apply settings failed: %s\n", cameraController.lastError().c_str());
        return false;
    }

    if (!cameraController.startGrabbing())
    {
        printf("[CAMERA TEST] start grabbing failed: %s\n", cameraController.lastError().c_str());
        return false;
    }

    if (!cameraController.executeSoftwareTrigger(0))
    {
        printf("[CAMERA TEST] color software trigger failed: %s\n", cameraController.lastError().c_str());
        cameraController.stopGrabbing();
        return false;
    }

    HikCameraFrame colorFrame;
    if (!cameraController.getOneFrame(0, &colorFrame))
    {
        printf("[CAMERA TEST] get color frame failed: %s\n", cameraController.lastError().c_str());
        cameraController.stopGrabbing();
        return false;
    }

    if (!SaveColorCameraFrame(cameraController, colorFrame))
    {
        printf("[CAMERA TEST] save color frame failed: %s\n", cameraController.lastError().c_str());
        cameraController.stopGrabbing();
        return false;
    }

    printf("[CAMERA TEST] saved one color camera frame\n");

    DlpDeviceConfig deviceConfig;
    deviceConfig.cypressVid = 0x04B4;
    deviceConfig.cypressPid = 0x000A;
    deviceConfig.dlpcI2cAddress = 0x1B;

    DlpScanConfig scanConfig = CreateDefaultScanConfig();
    DlpProjectorScanner scanner(deviceConfig);

    if (!scanner.connectProjector())
    {
        printf("[DLP TEST] connect failed: %s\n", scanner.lastError().c_str());
        cameraController.stopGrabbing();
        return false;
    }

    if (!scanner.initializeCommandLibrary())
    {
        printf("[DLP TEST] initialize failed: %s\n", scanner.lastError().c_str());
        cameraController.stopGrabbing();
        return false;
    }

    if (!scanner.prepareScan(scanConfig))
    {
        printf("[DLP TEST] prepare failed: %s\n", scanner.lastError().c_str());
        cameraController.stopGrabbing();
        return false;
    }

    if (!scanner.startScan(scanConfig))
    {
        printf("[DLP TEST] start failed: %s\n", scanner.lastError().c_str());
        cameraController.stopGrabbing();
        return false;
    }

    for (uint32_t patternIndex = 0; patternIndex < kScanPatternCount; ++patternIndex)
    {
        std::vector<HikCameraFrame> monoFrames(4);
        for (uint32_t monoIndex = 0; monoIndex < 4; ++monoIndex)
        {
            const uint32_t cameraIndex = monoIndex + 1;
            if (!cameraController.getOneFrame(cameraIndex, &monoFrames[monoIndex]))
            {
                printf("[CAMERA TEST] get mono frame failed: %s\n", cameraController.lastError().c_str());
                scanner.stopScan();
                cameraController.stopGrabbing();
                return false;
            }
        }

        if (!SaveMonoCameraFrames(cameraController, monoFrames, patternIndex))
        {
            scanner.stopScan();
            cameraController.stopGrabbing();
            return false;
        }

        printf("[CAMERA TEST] saved pattern %u mono frames\n", patternIndex);
    }

    scanner.stopScan();
    cameraController.stopGrabbing();
    cameraController.closeAllCameras();

    printf("[CAMERA TEST] scan success, saved one color PNG and %u groups of mono PNG images\n", kScanPatternCount);
    return true;
}

#ifdef CAMERA_DLP_SCAN_TEST_MAIN

/**
 * @brief 控制台测试入口。
 *
 * @return 0 表示测试成功；-1 表示测试失败。
 *
 * 方法作用：在非 Qt 环境下快速验证彩色网口相机、CXP 灰度相机和 DLP 扫描流程。
 */
int main()
{
    if (!RunCameraDlpScanTest())
    {
        return -1;
    }

    return 0;
}

#endif
