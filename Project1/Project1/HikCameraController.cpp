#include "HikCameraController.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <algorithm>
#include <cstring>
#include <sstream>

namespace
{
    struct HikMvsApi
    {
        using InitializeFn = int(__stdcall*)();
        using FinalizeFn = int(__stdcall*)();
        using EnumDevicesFn = int(__stdcall*)(unsigned int, MV_CC_DEVICE_INFO_LIST*);
        using CreateHandleFn = int(__stdcall*)(void**, const MV_CC_DEVICE_INFO*);
        using OpenDeviceFn = int(__stdcall*)(void*, unsigned int, unsigned short);
        using CloseDeviceFn = int(__stdcall*)(void*);
        using DestroyHandleFn = int(__stdcall*)(void*);
        using StartGrabbingFn = int(__stdcall*)(void*);
        using StopGrabbingFn = int(__stdcall*)(void*);
        using GetImageBufferFn = int(__stdcall*)(void*, MV_FRAME_OUT*, unsigned int);
        using FreeImageBufferFn = int(__stdcall*)(void*, MV_FRAME_OUT*);
        using SetEnumValueFn = int(__stdcall*)(void*, const char*, unsigned int);
        using SetFloatValueFn = int(__stdcall*)(void*, const char*, float);
        using SetCommandValueFn = int(__stdcall*)(void*, const char*);
        using SaveImageToFileExFn = int(__stdcall*)(void*, MV_SAVE_IMAGE_TO_FILE_PARAM_EX*);

        HMODULE module = nullptr;
        InitializeFn initialize = nullptr;
        FinalizeFn finalize = nullptr;
        EnumDevicesFn enumDevices = nullptr;
        CreateHandleFn createHandle = nullptr;
        OpenDeviceFn openDevice = nullptr;
        CloseDeviceFn closeDevice = nullptr;
        DestroyHandleFn destroyHandle = nullptr;
        StartGrabbingFn startGrabbing = nullptr;
        StopGrabbingFn stopGrabbing = nullptr;
        GetImageBufferFn getImageBuffer = nullptr;
        FreeImageBufferFn freeImageBuffer = nullptr;
        SetEnumValueFn setEnumValue = nullptr;
        SetFloatValueFn setFloatValue = nullptr;
        SetCommandValueFn setCommandValue = nullptr;
        SaveImageToFileExFn saveImageToFileEx = nullptr;
        std::string lastError;

        bool load()
        {
            if (module != nullptr)
            {
                return true;
            }

            module = LoadLibraryA("MvCameraControl.dll");
            if (module == nullptr)
            {
                lastError = "加载 MvCameraControl.dll 失败，请确认已安装海康 MVS 或 DLL 已加入 PATH。";
                return false;
            }

            initialize = loadFunction<InitializeFn>("MV_CC_Initialize");
            finalize = loadFunction<FinalizeFn>("MV_CC_Finalize");
            enumDevices = loadFunction<EnumDevicesFn>("MV_CC_EnumDevices");
            createHandle = loadFunction<CreateHandleFn>("MV_CC_CreateHandle");
            openDevice = loadFunction<OpenDeviceFn>("MV_CC_OpenDevice");
            closeDevice = loadFunction<CloseDeviceFn>("MV_CC_CloseDevice");
            destroyHandle = loadFunction<DestroyHandleFn>("MV_CC_DestroyHandle");
            startGrabbing = loadFunction<StartGrabbingFn>("MV_CC_StartGrabbing");
            stopGrabbing = loadFunction<StopGrabbingFn>("MV_CC_StopGrabbing");
            getImageBuffer = loadFunction<GetImageBufferFn>("MV_CC_GetImageBuffer");
            freeImageBuffer = loadFunction<FreeImageBufferFn>("MV_CC_FreeImageBuffer");
            setEnumValue = loadFunction<SetEnumValueFn>("MV_CC_SetEnumValue");
            setFloatValue = loadFunction<SetFloatValueFn>("MV_CC_SetFloatValue");
            setCommandValue = loadFunction<SetCommandValueFn>("MV_CC_SetCommandValue");
            saveImageToFileEx = loadFunction<SaveImageToFileExFn>("MV_CC_SaveImageToFileEx");

            if (initialize == nullptr || finalize == nullptr || enumDevices == nullptr ||
                createHandle == nullptr || openDevice == nullptr || closeDevice == nullptr ||
                destroyHandle == nullptr || startGrabbing == nullptr || stopGrabbing == nullptr ||
                getImageBuffer == nullptr || freeImageBuffer == nullptr || setEnumValue == nullptr ||
                setFloatValue == nullptr || setCommandValue == nullptr || saveImageToFileEx == nullptr)
            {
                FreeLibrary(module);
                module = nullptr;
                return false;
            }

            return true;
        }

        template <typename FunctionType>
        FunctionType loadFunction(const char* functionName)
        {
            FARPROC functionAddress = GetProcAddress(module, functionName);
            if (functionAddress == nullptr)
            {
                lastError = std::string("加载海康 MVS 函数失败：") + functionName;
                return nullptr;
            }

            return reinterpret_cast<FunctionType>(functionAddress);
        }
    };

    HikMvsApi& hikMvsApi()
    {
        static HikMvsApi api;
        return api;
    }

    std::string formatErrorCode(int errorCode)
    {
        std::ostringstream stream;
        stream << "0x" << std::hex << errorCode;
        return stream.str();
    }
}

HikCameraController::HikCameraController(const HikCameraSystemConfig& config)
    : config_(config)
{
    colorCamera_.config = config_.colorCamera;
    monoCameras_.resize(config_.monoCameras.size());
    for (size_t index = 0; index < config_.monoCameras.size(); ++index)
    {
        monoCameras_[index].config = config_.monoCameras[index];
    }
}

HikCameraController::~HikCameraController()
{
    closeAllCameras();
    finalizeSdk();
}

void HikCameraController::setConfig(const HikCameraSystemConfig& config)
{
    config_ = config;
    colorCamera_.config = config_.colorCamera;
    monoCameras_.resize(config_.monoCameras.size());
    for (size_t index = 0; index < config_.monoCameras.size(); ++index)
    {
        monoCameras_[index].config = config_.monoCameras[index];
    }
}

HikCameraSystemConfig HikCameraController::config() const
{
    return config_;
}

bool HikCameraController::initializeSdk()
{
    if (sdkInitialized_)
    {
        return true;
    }

    HikMvsApi& api = hikMvsApi();
    if (!api.load())
    {
        setLastError(api.lastError);
        return false;
    }

    int status = api.initialize();
    if (status != MV_OK)
    {
        setLastErrorWithCode("初始化海康 MVS SDK 失败", status);
        return false;
    }

    sdkInitialized_ = true;
    lastError_.clear();
    return true;
}

void HikCameraController::finalizeSdk()
{
    if (!sdkInitialized_)
    {
        return;
    }

    hikMvsApi().finalize();
    sdkInitialized_ = false;
}

bool HikCameraController::enumerateDevices(unsigned int transportLayerType)
{
    if (!initializeSdk())
    {
        return false;
    }

    std::memset(&deviceList_, 0, sizeof(deviceList_));
    int status = hikMvsApi().enumDevices(transportLayerType, &deviceList_);
    if (status != MV_OK)
    {
        setLastErrorWithCode("枚举相机设备失败", status);
        return false;
    }

    lastError_.clear();
    return true;
}

uint32_t HikCameraController::deviceCount() const
{
    return deviceList_.nDeviceNum;
}

bool HikCameraController::openAllCameras()
{
    if (!initializeSdk())
    {
        return false;
    }

    if (deviceList_.nDeviceNum == 0)
    {
        setLastError("设备列表为空，请先调用 enumerateDevices() 枚举相机。");
        return false;
    }

    if (!openCamera(config_.colorCamera, &colorCamera_))
    {
        return false;
    }

    monoCameras_.resize(config_.monoCameras.size());
    for (size_t index = 0; index < config_.monoCameras.size(); ++index)
    {
        if (!openCamera(config_.monoCameras[index], &monoCameras_[index]))
        {
            return false;
        }
    }

    lastError_.clear();
    return true;
}

void HikCameraController::closeAllCameras()
{
    auto closeCamera = [](CameraHandle* cameraHandle)
    {
        if (cameraHandle == nullptr || cameraHandle->handle == nullptr)
        {
            return;
        }

        if (cameraHandle->grabbing)
        {
            hikMvsApi().stopGrabbing(cameraHandle->handle);
            cameraHandle->grabbing = false;
        }

        if (cameraHandle->opened)
        {
            hikMvsApi().closeDevice(cameraHandle->handle);
            cameraHandle->opened = false;
        }

        hikMvsApi().destroyHandle(cameraHandle->handle);
        cameraHandle->handle = nullptr;
    };

    closeCamera(&colorCamera_);
    for (CameraHandle& cameraHandle : monoCameras_)
    {
        closeCamera(&cameraHandle);
    }
}

bool HikCameraController::areAllCamerasOpen() const
{
    if (!colorCamera_.opened)
    {
        return false;
    }

    return std::all_of(
        monoCameras_.begin(),
        monoCameras_.end(),
        [](const CameraHandle& cameraHandle)
        {
            return cameraHandle.opened;
        });
}

bool HikCameraController::applyAllCameraSettings()
{
    if (!applyCameraSettings(&colorCamera_))
    {
        return false;
    }

    for (CameraHandle& cameraHandle : monoCameras_)
    {
        if (!applyCameraSettings(&cameraHandle))
        {
            return false;
        }
    }

    lastError_.clear();
    return true;
}

bool HikCameraController::startGrabbing()
{
    auto startCamera = [this](CameraHandle* cameraHandle) -> bool
    {
        if (cameraHandle == nullptr || !cameraHandle->opened)
        {
            setLastError("相机未打开，无法开始采集。");
            return false;
        }

        if (cameraHandle->grabbing)
        {
            return true;
        }

        int status = hikMvsApi().startGrabbing(cameraHandle->handle);
        if (status != MV_OK)
        {
            setLastErrorWithCode(cameraHandle->config.name + " 开始采集失败", status);
            return false;
        }

        cameraHandle->grabbing = true;
        return true;
    };

    if (!startCamera(&colorCamera_))
    {
        return false;
    }

    for (CameraHandle& cameraHandle : monoCameras_)
    {
        if (!startCamera(&cameraHandle))
        {
            return false;
        }
    }

    lastError_.clear();
    return true;
}

bool HikCameraController::stopGrabbing()
{
    bool allOk = true;

    auto stopCamera = [this, &allOk](CameraHandle* cameraHandle)
    {
        if (cameraHandle == nullptr || !cameraHandle->opened || !cameraHandle->grabbing)
        {
            return;
        }

        int status = hikMvsApi().stopGrabbing(cameraHandle->handle);
        if (status != MV_OK)
        {
            setLastErrorWithCode(cameraHandle->config.name + " 停止采集失败", status);
            allOk = false;
            return;
        }

        cameraHandle->grabbing = false;
    };

    stopCamera(&colorCamera_);
    for (CameraHandle& cameraHandle : monoCameras_)
    {
        stopCamera(&cameraHandle);
    }

    if (allOk)
    {
        lastError_.clear();
    }

    return allOk;
}

bool HikCameraController::getOneFrame(uint32_t cameraIndex, HikCameraFrame* frame)
{
    if (frame == nullptr)
    {
        setLastError("取图失败：frame 输出参数为空。");
        return false;
    }

    CameraHandle* cameraHandle = cameraAt(cameraIndex);
    if (cameraHandle == nullptr)
    {
        setLastError("取图失败：相机索引超出范围。");
        return false;
    }

    if (!cameraHandle->opened || !cameraHandle->grabbing)
    {
        setLastError(cameraHandle->config.name + " 未打开或未开始采集。");
        return false;
    }

    MV_FRAME_OUT sdkFrame = {};
    int status = hikMvsApi().getImageBuffer(
        cameraHandle->handle,
        &sdkFrame,
        cameraHandle->config.frameTimeoutMs);

    if (status != MV_OK)
    {
        setLastErrorWithCode(cameraHandle->config.name + " 取图失败", status);
        return false;
    }

    frame->frameInfo = sdkFrame.stFrameInfo;
    frame->data.assign(
        sdkFrame.pBufAddr,
        sdkFrame.pBufAddr + sdkFrame.stFrameInfo.nFrameLen);

    hikMvsApi().freeImageBuffer(cameraHandle->handle, &sdkFrame);
    lastError_.clear();
    return true;
}

bool HikCameraController::getOneFrameFromAll(std::vector<HikCameraFrame>* frames)
{
    if (frames == nullptr)
    {
        setLastError("取图失败：frames 输出参数为空。");
        return false;
    }

    const uint32_t totalCameraCount = 1 + static_cast<uint32_t>(monoCameras_.size());
    frames->clear();
    frames->resize(totalCameraCount);

    for (uint32_t cameraIndex = 0; cameraIndex < totalCameraCount; ++cameraIndex)
    {
        if (!getOneFrame(cameraIndex, &(*frames)[cameraIndex]))
        {
            return false;
        }
    }

    lastError_.clear();
    return true;
}

bool HikCameraController::saveFrameAsPng(
    uint32_t cameraIndex,
    const HikCameraFrame& frame,
    const std::string& filePath)
{
    CameraHandle* cameraHandle = cameraAt(cameraIndex);
    if (cameraHandle == nullptr)
    {
        setLastError("保存 PNG 失败：相机索引超出范围。");
        return false;
    }

    if (!cameraHandle->opened)
    {
        setLastError(cameraHandle->config.name + " 未打开，无法保存 PNG。");
        return false;
    }

    if (frame.data.empty())
    {
        setLastError("保存 PNG 失败：图像数据为空。");
        return false;
    }

    std::vector<char> writablePath(filePath.begin(), filePath.end());
    writablePath.push_back('\0');

    MV_SAVE_IMAGE_TO_FILE_PARAM_EX saveParam = {};
    saveParam.nWidth = frame.frameInfo.nWidth;
    saveParam.nHeight = frame.frameInfo.nHeight;
    saveParam.enPixelType = frame.frameInfo.enPixelType;
    saveParam.pData = const_cast<unsigned char*>(frame.data.data());
    saveParam.nDataLen = static_cast<unsigned int>(frame.data.size());
    saveParam.enImageType = MV_Image_Png;
    saveParam.pcImagePath = writablePath.data();
    saveParam.nQuality = 90;
    saveParam.iMethodValue = 1;

    int status = hikMvsApi().saveImageToFileEx(cameraHandle->handle, &saveParam);
    if (status != MV_OK)
    {
        setLastErrorWithCode(cameraHandle->config.name + " 保存 PNG 失败", status);
        return false;
    }

    lastError_.clear();
    return true;
}

bool HikCameraController::executeSoftwareTrigger(uint32_t cameraIndex)
{
    CameraHandle* cameraHandle = cameraAt(cameraIndex);
    if (cameraHandle == nullptr)
    {
        setLastError("软触发失败：相机索引超出范围。");
        return false;
    }

    if (!cameraHandle->opened)
    {
        setLastError(cameraHandle->config.name + " 未打开，无法软触发。");
        return false;
    }

    return setCommandValue(cameraHandle, "TriggerSoftware");
}

std::string HikCameraController::lastError() const
{
    return lastError_;
}

bool HikCameraController::openCamera(const HikCameraConfig& cameraConfig, CameraHandle* cameraHandle)
{
    if (cameraHandle == nullptr)
    {
        setLastError("打开相机失败：内部相机句柄为空。");
        return false;
    }

    if (cameraConfig.deviceIndex >= deviceList_.nDeviceNum)
    {
        setLastError(cameraConfig.name + " 打开失败：deviceIndex 超出枚举设备数量。");
        return false;
    }

    if (cameraHandle->handle != nullptr)
    {
        if (cameraHandle->grabbing)
        {
            hikMvsApi().stopGrabbing(cameraHandle->handle);
            cameraHandle->grabbing = false;
        }

        if (cameraHandle->opened)
        {
            hikMvsApi().closeDevice(cameraHandle->handle);
            cameraHandle->opened = false;
        }

        hikMvsApi().destroyHandle(cameraHandle->handle);
        cameraHandle->handle = nullptr;
    }

    cameraHandle->config = cameraConfig;
    int status = hikMvsApi().createHandle(
        &cameraHandle->handle,
        deviceList_.pDeviceInfo[cameraConfig.deviceIndex]);

    if (status != MV_OK)
    {
        cameraHandle->handle = nullptr;
        setLastErrorWithCode(cameraConfig.name + " 创建相机句柄失败", status);
        return false;
    }

    status = hikMvsApi().openDevice(cameraHandle->handle, MV_ACCESS_Exclusive, 0);
    if (status != MV_OK)
    {
        setLastErrorWithCode(cameraConfig.name + " 打开相机失败", status);
        hikMvsApi().destroyHandle(cameraHandle->handle);
        cameraHandle->handle = nullptr;
        return false;
    }

    cameraHandle->opened = true;

    if (!applyCameraSettings(cameraHandle))
    {
        return false;
    }

    return true;
}

bool HikCameraController::applyCameraSettings(CameraHandle* cameraHandle)
{
    if (cameraHandle == nullptr || !cameraHandle->opened)
    {
        setLastError("配置相机失败：相机未打开。");
        return false;
    }

    if (!setFloatValue(cameraHandle, "ExposureTime", cameraHandle->config.exposureTimeUs))
    {
        return false;
    }

    if (!setEnumValue(
            cameraHandle,
            "TriggerMode",
            static_cast<unsigned int>(cameraHandle->config.triggerMode)))
    {
        return false;
    }

    if (!setEnumValue(
            cameraHandle,
            "TriggerSource",
            static_cast<unsigned int>(cameraHandle->config.triggerSource)))
    {
        return false;
    }

    return true;
}

bool HikCameraController::setEnumValue(CameraHandle* cameraHandle, const char* key, unsigned int value)
{
    int status = hikMvsApi().setEnumValue(cameraHandle->handle, key, value);
    if (status != MV_OK)
    {
        setLastErrorWithCode(cameraHandle->config.name + " 设置枚举参数 " + key + " 失败", status);
        return false;
    }

    return true;
}

bool HikCameraController::setFloatValue(CameraHandle* cameraHandle, const char* key, float value)
{
    int status = hikMvsApi().setFloatValue(cameraHandle->handle, key, value);
    if (status != MV_OK)
    {
        setLastErrorWithCode(cameraHandle->config.name + " 设置浮点参数 " + key + " 失败", status);
        return false;
    }

    return true;
}

bool HikCameraController::setCommandValue(CameraHandle* cameraHandle, const char* key)
{
    int status = hikMvsApi().setCommandValue(cameraHandle->handle, key);
    if (status != MV_OK)
    {
        setLastErrorWithCode(cameraHandle->config.name + " 执行命令 " + key + " 失败", status);
        return false;
    }

    lastError_.clear();
    return true;
}

HikCameraController::CameraHandle* HikCameraController::cameraAt(uint32_t cameraIndex)
{
    if (cameraIndex == 0)
    {
        return &colorCamera_;
    }

    uint32_t monoIndex = cameraIndex - 1;
    if (monoIndex >= monoCameras_.size())
    {
        return nullptr;
    }

    return &monoCameras_[monoIndex];
}

const HikCameraController::CameraHandle* HikCameraController::cameraAt(uint32_t cameraIndex) const
{
    if (cameraIndex == 0)
    {
        return &colorCamera_;
    }

    uint32_t monoIndex = cameraIndex - 1;
    if (monoIndex >= monoCameras_.size())
    {
        return nullptr;
    }

    return &monoCameras_[monoIndex];
}

void HikCameraController::setLastError(const std::string& errorMessage)
{
    lastError_ = errorMessage;
}

void HikCameraController::setLastErrorWithCode(const std::string& operation, int errorCode)
{
    lastError_ = operation + "，错误码：" + formatErrorCode(errorCode);
}
