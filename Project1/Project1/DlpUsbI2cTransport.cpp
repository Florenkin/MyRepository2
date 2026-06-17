#include "DlpUsbI2cTransport.h"

#include <cstring>
#include <iomanip>
#include <sstream>

DlpUsbI2cTransport::DlpUsbI2cTransport()
{
}

DlpUsbI2cTransport::~DlpUsbI2cTransport()
{
    close();
}

bool DlpUsbI2cTransport::open(uint16_t vid, uint16_t pid, uint8_t i2cSlaveAddress)
{
    close();

    m_slaveAddress = i2cSlaveAddress;
    m_lastError.clear();

#ifndef WIN32
    CyLibraryInit();
#endif

    UINT8 deviceCount = 0;
    CY_RETURN_STATUS status = CyGetListofDevices(&deviceCount);

    if (status != CY_SUCCESS)
    {
        setLastError("CyGetListofDevices failed: " + cyStatusToString(status));
        return false;
    }

    for (UINT8 i = 0; i < deviceCount; ++i)
    {
        CY_DEVICE_INFO info;
        std::memset(&info, 0, sizeof(info));

        status = CyGetDeviceInfo(i, &info);
        if (status != CY_SUCCESS)
        {
            continue;
        }

        if (info.vidPid.vid != vid || info.vidPid.pid != pid)
        {
            continue;
        }

#ifdef WIN32
        // CyUSBSerial.h 注释说明：Windows 下 interfaceNum 应设置为 0
        UINT8 interfaceNum = 0;
#else
        UINT8 interfaceNum = 0;
#endif

        status = CyOpen(i, interfaceNum, &m_handle);

        if (status != CY_SUCCESS)
        {
            setLastError("CyOpen failed: " + cyStatusToString(status));
            m_handle = nullptr;
            return false;
        }

        if (!configureI2c())
        {
            close();
            return false;
        }

        return true;
    }

    setLastError("未找到 Cypress USB-Serial 设备，VID=0x" + toHex16(vid) +
                 " PID=0x" + toHex16(pid));
    return false;
}

void DlpUsbI2cTransport::close()
{
    if (m_handle)
    {
        CyClose(m_handle);
        m_handle = nullptr;
    }

#ifndef WIN32
    CyLibraryExit();
#endif
}

bool DlpUsbI2cTransport::isOpen() const
{
    return m_handle != nullptr;
}

bool DlpUsbI2cTransport::configureI2c()
{
    if (!m_handle)
    {
        setLastError("I2C configure failed: handle is null");
        return false;
    }

    CY_I2C_CONFIG config;
    std::memset(&config, 0, sizeof(config));

    config.frequency = 400000;       // 400KHz；如果不稳定，可改成 100000
    config.slaveAddress = 0;         // 主机模式下这里不是 DLPC 地址
    config.isMaster = true;
    config.isClockStretch = false;

    CY_RETURN_STATUS status = CySetI2cConfig(m_handle, &config);

    if (status != CY_SUCCESS)
    {
        setLastError("CySetI2cConfig failed: " + cyStatusToString(status));
        return false;
    }

    return true;
}

bool DlpUsbI2cTransport::write(const uint8_t* data, uint16_t length)
{
    if (!m_handle)
    {
        setLastError("I2C write failed: handle is null");
        return false;
    }

    if (!data || length == 0)
    {
        setLastError("I2C write failed: invalid buffer");
        return false;
    }

    CY_I2C_DATA_CONFIG dataConfig;
    std::memset(&dataConfig, 0, sizeof(dataConfig));

    dataConfig.slaveAddress = m_slaveAddress;
    dataConfig.isStopBit = true;
    dataConfig.isNakBit = false;

    CY_DATA_BUFFER writeBuffer;
    std::memset(&writeBuffer, 0, sizeof(writeBuffer));

    // Cypress API 的 buffer 类型不是 const，所以这里 const_cast。
    // 这里不会修改原数据，只是为了适配 API。
    writeBuffer.buffer = const_cast<UCHAR*>(reinterpret_cast<const UCHAR*>(data));
    writeBuffer.length = length;
    writeBuffer.transferCount = 0;

    constexpr UINT32 timeoutMs = 1000;

    CY_RETURN_STATUS status = CyI2cWrite(
        m_handle,
        &dataConfig,
        &writeBuffer,
        timeoutMs
    );

    if (status != CY_SUCCESS)
    {
        setLastError("CyI2cWrite failed: " + cyStatusToString(status));
        return false;
    }

    if (writeBuffer.transferCount != length)
    {
        std::ostringstream oss;
        oss << "CyI2cWrite length mismatch, expect=" << length
            << " actual=" << writeBuffer.transferCount;
        setLastError(oss.str());
        return false;
    }

    return true;
}

bool DlpUsbI2cTransport::read(const uint8_t* writeData,
                              uint16_t writeLength,
                              uint8_t* readData,
                              uint16_t readLength)
{
    if (!m_handle)
    {
        setLastError("I2C read failed: handle is null");
        return false;
    }

    if (!writeData || writeLength == 0 || !readData || readLength == 0)
    {
        setLastError("I2C read failed: invalid buffer");
        return false;
    }

    constexpr UINT32 timeoutMs = 1000;

    // 1. 先写入 DLPC 的读命令 opcode
    CY_I2C_DATA_CONFIG writeConfig;
    std::memset(&writeConfig, 0, sizeof(writeConfig));

    writeConfig.slaveAddress = m_slaveAddress;

    // DLPC 读命令通常需要 repeated-start，所以这里默认不发 stop。
    // 如果你的硬件读失败，可以尝试改成 true。
    writeConfig.isStopBit = false;
    writeConfig.isNakBit = false;

    CY_DATA_BUFFER cmdBuffer;
    std::memset(&cmdBuffer, 0, sizeof(cmdBuffer));

    cmdBuffer.buffer = const_cast<UCHAR*>(reinterpret_cast<const UCHAR*>(writeData));
    cmdBuffer.length = writeLength;
    cmdBuffer.transferCount = 0;

    CY_RETURN_STATUS status = CyI2cWrite(
        m_handle,
        &writeConfig,
        &cmdBuffer,
        timeoutMs
    );

    if (status != CY_SUCCESS)
    {
        setLastError("CyI2cWrite before read failed: " + cyStatusToString(status));
        return false;
    }

    // 2. 再读取返回数据
    CY_I2C_DATA_CONFIG readConfig;
    std::memset(&readConfig, 0, sizeof(readConfig));

    readConfig.slaveAddress = m_slaveAddress;
    readConfig.isStopBit = true;
    readConfig.isNakBit = true;

    CY_DATA_BUFFER readBuffer;
    std::memset(&readBuffer, 0, sizeof(readBuffer));

    readBuffer.buffer = reinterpret_cast<UCHAR*>(readData);
    readBuffer.length = readLength;
    readBuffer.transferCount = 0;

    status = CyI2cRead(
        m_handle,
        &readConfig,
        &readBuffer,
        timeoutMs
    );

    if (status != CY_SUCCESS)
    {
        setLastError("CyI2cRead failed: " + cyStatusToString(status));
        return false;
    }

    if (readBuffer.transferCount != readLength)
    {
        std::ostringstream oss;
        oss << "CyI2cRead length mismatch, expect=" << readLength
            << " actual=" << readBuffer.transferCount;
        setLastError(oss.str());
        return false;
    }

    return true;
}

const std::string& DlpUsbI2cTransport::lastError() const
{
    return m_lastError;
}

void DlpUsbI2cTransport::setLastError(const std::string& error)
{
    m_lastError = error;
}

std::string DlpUsbI2cTransport::cyStatusToString(CY_RETURN_STATUS status)
{
    switch (status)
    {
    case CY_SUCCESS: return "CY_SUCCESS";
    case CY_ERROR_ACCESS_DENIED: return "CY_ERROR_ACCESS_DENIED";
    case CY_ERROR_DRIVER_INIT_FAILED: return "CY_ERROR_DRIVER_INIT_FAILED";
    case CY_ERROR_DEVICE_INFO_FETCH_FAILED: return "CY_ERROR_DEVICE_INFO_FETCH_FAILED";
    case CY_ERROR_DRIVER_OPEN_FAILED: return "CY_ERROR_DRIVER_OPEN_FAILED";
    case CY_ERROR_INVALID_PARAMETER: return "CY_ERROR_INVALID_PARAMETER";
    case CY_ERROR_REQUEST_FAILED: return "CY_ERROR_REQUEST_FAILED";
    case CY_ERROR_DOWNLOAD_FAILED: return "CY_ERROR_DOWNLOAD_FAILED";
    case CY_ERROR_FIRMWARE_INVALID_SIGNATURE: return "CY_ERROR_FIRMWARE_INVALID_SIGNATURE";
    case CY_ERROR_INVALID_FIRMWARE: return "CY_ERROR_INVALID_FIRMWARE";
    case CY_ERROR_DEVICE_NOT_FOUND: return "CY_ERROR_DEVICE_NOT_FOUND";
    case CY_ERROR_IO_TIMEOUT: return "CY_ERROR_IO_TIMEOUT";
    case CY_ERROR_PIPE_HALTED: return "CY_ERROR_PIPE_HALTED";
    case CY_ERROR_BUFFER_OVERFLOW: return "CY_ERROR_BUFFER_OVERFLOW";
    case CY_ERROR_INVALID_HANDLE: return "CY_ERROR_INVALID_HANDLE";
    case CY_ERROR_ALLOCATION_FAILED: return "CY_ERROR_ALLOCATION_FAILED";
    case CY_ERROR_I2C_DEVICE_BUSY: return "CY_ERROR_I2C_DEVICE_BUSY";
    case CY_ERROR_I2C_NAK_ERROR: return "CY_ERROR_I2C_NAK_ERROR";
    case CY_ERROR_I2C_ARBITRATION_ERROR: return "CY_ERROR_I2C_ARBITRATION_ERROR";
    case CY_ERROR_I2C_BUS_ERROR: return "CY_ERROR_I2C_BUS_ERROR";
    case CY_ERROR_I2C_BUS_BUSY: return "CY_ERROR_I2C_BUS_BUSY";
    case CY_ERROR_I2C_STOP_BIT_SET: return "CY_ERROR_I2C_STOP_BIT_SET";
    case CY_ERROR_STATUS_MONITOR_EXIST: return "CY_ERROR_STATUS_MONITOR_EXIST";
    default:
    {
        std::ostringstream oss;
        oss << "CY_ERROR_UNKNOWN(" << static_cast<int>(status) << ")";
        return oss.str();
    }
    }
}

std::string DlpUsbI2cTransport::toHex16(uint16_t value)
{
    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << value;
    return oss.str();
}
