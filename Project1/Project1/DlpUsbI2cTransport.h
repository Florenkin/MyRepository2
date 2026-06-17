#pragma once

#include <cstdint>
#include <string>

extern "C" {
#include "CyUSBSerial.h"
}

/**
 * @brief Cypress USB-Serial 转 I2C 通信层，不依赖 Qt
 *
 * 典型链路：
 * PC -> USB -> Cypress USB-Serial -> I2C -> DLPC34xx 光机控制器
 *
 * 这个类只负责底层 USB-I2C 读写。
 * 上层需要把 write()/read() 接到 DLPC_COMMON_InitCommandLibrary() 的回调中。
 */
class DlpUsbI2cTransport
{
public:
    DlpUsbI2cTransport();
    ~DlpUsbI2cTransport();

    DlpUsbI2cTransport(const DlpUsbI2cTransport&) = delete;
    DlpUsbI2cTransport& operator=(const DlpUsbI2cTransport&) = delete;

    /**
     * @brief 打开 Cypress USB-Serial 设备并配置 I2C
     * @param vid Cypress 设备 VID，例如 0x04B4
     * @param pid Cypress 设备 PID，需要在设备管理器中查看
     * @param i2cSlaveAddress DLPC 的 7-bit I2C 地址，常见为 0x1B
     */
    bool open(uint16_t vid, uint16_t pid, uint8_t i2cSlaveAddress);

    /**
     * @brief 关闭设备
     */
    void close();

    /**
     * @brief 是否已经打开
     */
    bool isOpen() const;

    /**
     * @brief I2C 写
     */
    bool write(const uint8_t* data, uint16_t length);

    /**
     * @brief I2C 读
     *
     * DLPC 读命令一般流程：
     * 1. 先写入读命令 opcode
     * 2. 再读取返回数据
     */
    bool read(const uint8_t* writeData,
              uint16_t writeLength,
              uint8_t* readData,
              uint16_t readLength);

    /**
     * @brief 获取最后一次错误信息
     */
    const std::string& lastError() const;

private:
    bool configureI2c();
    void setLastError(const std::string& error);
    static std::string cyStatusToString(CY_RETURN_STATUS status);
    static std::string toHex16(uint16_t value);

private:
    CY_HANDLE m_handle = nullptr;
    uint8_t m_slaveAddress = 0x1B;
    std::string m_lastError;
};
