#include "DeviceController.h"

#include <QDateTime>
#include <QElapsedTimer>
#include <QSerialPortInfo>

namespace
{
int comNumber(const QString &name)
{
    QString copy = name;
    copy.remove(QStringLiteral("COM"), Qt::CaseInsensitive);
    bool ok = false;
    const int number = copy.toInt(&ok);
    return ok ? number : 100000;
}

QString timestamp()
{
    return QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
}
}

DeviceController::DeviceController(QObject *parent)
    : QObject(parent),
      m_handle(new DlpHandle(this))
{
    const int ret = m_handle->SF_initLibrary();
    emit logLine(QStringLiteral("[%1] SDK 初始化: %2")
                 .arg(timestamp(), errorText(ret)));

    connect(m_handle, &DlpHandle::SF_uartErrorSign, this, [this](int errCode) {
        if (errCode == DE_SUCCESS) {
            return;
        }
        m_connected = false;
        m_deviceReady = false;
        m_connectedPort.clear();
        emit connectionChanged(false);
        emit sdkError(QStringLiteral("串口错误: %1").arg(errorText(errCode)));
    });

    connect(m_handle, &DlpHandle::SF_handleRxErrorSign, this, [this](quint8 cmd, int errCode) {
        emit sdkError(QStringLiteral("命令 0x%1 失败: %2")
                      .arg(cmd, 2, 16, QLatin1Char('0'))
                      .arg(commandErrorText(errCode & 0xff)));
    });

    connect(m_handle, &DlpHandle::SF_uartPackSign, this,
            [this](DlpHandle::dataDire_e dir, const QByteArray &pack) {
        const QString prefix = (dir == DlpHandle::DD_RECEIVE) ? QStringLiteral("RX") : QStringLiteral("TX");
        emit logLine(QStringLiteral("[%1] %2: %3").arg(timestamp(), prefix, hex(pack)));
    });

    connect(m_handle, &DlpHandle::SF_transmitProgressSign, this, &DeviceController::progressChanged);

    connect(m_handle, &DlpHandle::SF_transmitStatusSign, this, [this](DlpHandle::transmitStatus_e status) {
        QString text = QStringLiteral("无");
        switch (status) {
        case DlpHandle::TS_START: text = QStringLiteral("开始"); break;
        case DlpHandle::TS_FINISH: text = QStringLiteral("完成"); break;
        case DlpHandle::TS_ERROR: text = QStringLiteral("错误"); break;
        case DlpHandle::TS_EXIT: text = QStringLiteral("退出"); break;
        case DlpHandle::TS_NONE:
        default: break;
        }
        emit logLine(QStringLiteral("[%1] 传输状态: %2").arg(timestamp(), text));
    });
}

DeviceController::~DeviceController()
{
    if (m_connected) {
        m_handle->SF_closeUart();
    }
}

DlpHandle *DeviceController::handle() const
{
    return m_handle;
}

bool DeviceController::isConnected() const
{
    return m_connected;
}

bool DeviceController::isDeviceReady() const
{
    return m_deviceReady;
}

QString DeviceController::connectedPort() const
{
    return m_connectedPort;
}

QStringList DeviceController::availablePorts() const
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    std::sort(ports.begin(), ports.end(), [](const QSerialPortInfo &left, const QSerialPortInfo &right) {
        const int leftNo = comNumber(left.portName());
        const int rightNo = comNumber(right.portName());
        if (leftNo == rightNo) {
            return left.portName() < right.portName();
        }
        return leftNo < rightNo;
    });

    QStringList names;
    for (const QSerialPortInfo &port : ports) {
        names << port.portName();
    }
    return names;
}

int DeviceController::openPort(const QString &portName)
{
    const int ret = m_handle->SF_openUart(portName);
    if (ret == DE_SUCCESS) {
        m_connected = true;
        m_deviceReady = false;
        m_connectedPort = portName;
        emit connectionChanged(true);
        emit logLine(QStringLiteral("[%1] 已连接: %2").arg(timestamp(), portName));

        const int readyRet = checkDeviceReady();
        if (readyRet != DE_SUCCESS) {
            emit logLine(QStringLiteral("[%1] 光机未响应，已关闭串口: %2")
                         .arg(timestamp(), errorText(readyRet)));
            m_handle->SF_closeUart();
            m_connected = false;
            m_deviceReady = false;
            m_connectedPort.clear();
            emit connectionChanged(false);
            return readyRet;
        }
    }
    return ret;
}

int DeviceController::closePort()
{
    const int ret = m_handle->SF_closeUart();
    if (ret == DE_SUCCESS) {
        m_connected = false;
        m_deviceReady = false;
        m_connectedPort.clear();
        emit connectionChanged(false);
        emit logLine(QStringLiteral("[%1] 已断开").arg(timestamp()));
    }
    return ret;
}

int DeviceController::checkDeviceReady()
{
    if (!m_connected) {
        m_deviceReady = false;
        return DE_UART_NOT_OPEN;
    }

    const int ret = m_handle->SF_deviceCheckReady();
    emit logLine(QStringLiteral("[%1] 设备就绪检测: %2")
                 .arg(timestamp(), errorText(ret)));
    if (ret != DE_SUCCESS) {
        m_deviceReady = false;
        emit connectionChanged(m_connected);
        return ret;
    }

    QString version;
    const int versionRet = m_handle->SF_getVersion(version);
    emit logLine(QStringLiteral("[%1] MCU 通信验证: %2%3")
                 .arg(timestamp(), errorText(versionRet),
                      versionRet == DE_SUCCESS ? QStringLiteral("，版本 %1").arg(version) : QString()));
    m_deviceReady = (versionRet == DE_SUCCESS);
    emit connectionChanged(m_connected);
    return versionRet;
}

QString DeviceController::errorText(int code)
{
    switch (code) {
    case DE_SUCCESS: return QStringLiteral("成功");
    case DE_FAIL: return QStringLiteral("失败");
    case DE_OPEN_FILE_ERR: return QStringLiteral("打开文件失败");
    case DE_PARAMETER_ERR: return QStringLiteral("参数错误");
    case DE_DEVICE_BUSY: return QStringLiteral("设备忙");
    case DE_UART_ERR: return QStringLiteral("串口错误");
    case DE_UART_OPEN_FAIL: return QStringLiteral("串口打开失败");
    case DE_UART_TIMEOUT: return QStringLiteral("串口通信超时");
    case DE_UART_DISCONNECT: return QStringLiteral("串口已断开");
    case DE_UART_NOT_OPEN: return QStringLiteral("串口未打开");
    case DE_UART_BUSY: return QStringLiteral("串口忙");
    case DE_CMD_ERR: return QStringLiteral("命令错误");
    case DE_CMD_DATA_LEN_ERR: return QStringLiteral("命令数据长度错误");
    case DE_CMD_TIMEOUT: return QStringLiteral("命令超时");
    case DE_CMD_RX_DATA: return QStringLiteral("命令接收数据错误");
    case DE_CMD_RX_DATA_LEN: return QStringLiteral("命令接收长度错误");
    case DE_PT_AND_PB_ERR: return QStringLiteral("PT/PB 错误");
    case DE_PACK_DATA_ERR: return QStringLiteral("图案数据包错误");
    case DE_FIRST_TRANSMIT: return QStringLiteral("首包传输失败");
    case DE_TRANSMITING: return QStringLiteral("传输失败");
    case DE_TIME_ERR: return QStringLiteral("Pattern Time 显示时间过小");
    case DE_PRE_TIME_ERR: return QStringLiteral("前暗场时间过小");
    case DE_POST_TIME_ERR: return QStringLiteral("后暗场时间过小");
    case DE_IMAGE_OVER_LIMIT: return QStringLiteral("图案数量超过限制");
    case DE_PT_INDEX_ERR: return QStringLiteral("Pattern Time 索引错误");
    case DE_PT_NUM_ERR: return QStringLiteral("图案数量错误");
    case DE_IMAGE_SIZE_ERR: return QStringLiteral("图像大小错误");
    case DE_UPDATE_ERR: return QStringLiteral("升级错误");
    case DE_UPDATE_FILE_ERR: return QStringLiteral("升级文件无效");
    default: return QStringLiteral("错误 0x%1").arg(code & 0xffff, 4, 16, QLatin1Char('0'));
    }
}

QString DeviceController::commandErrorText(int code)
{
    switch (code) {
    case CEC_OK: return QStringLiteral("正常");
    case CEC_FAIL: return QStringLiteral("失败");
    case CEC_CMD_CHECK: return QStringLiteral("命令校验错误");
    case CEC_CMD_INVALID: return QStringLiteral("无效命令");
    case CEC_CMD_DATA_LEN: return QStringLiteral("数据长度超出范围");
    case CEC_CMD_DATA: return QStringLiteral("数据错误");
    case CEC_CMD_PASSWORD: return QStringLiteral("密码错误");
    case CEC_CMD_DATA_HANDLE: return QStringLiteral("数据处理错误");
    case CEC_CMD_DLP_PWR_OFF: return QStringLiteral("DLP 电源已关闭");
    case CEC_CMD_STORAGE_IS_FULL: return QStringLiteral("MCU Flash 已满");
    case CEC_CMD_NOT_FILE: return QStringLiteral("文件不存在");
    case CEC_CMD_OPERATE: return QStringLiteral("操作失败");
    case CEC_CMD_DATA_EMPTY: return QStringLiteral("读取数据为空");
    case CEC_CMD_CRC: return QStringLiteral("CRC 校验错误");
    case CEC_CMD_12V_OFF: return QStringLiteral("12V 电源已关闭");
    default: return QStringLiteral("命令错误 0x%1").arg(code & 0xff, 2, 16, QLatin1Char('0'));
    }
}

QString DeviceController::hex(const QByteArray &data)
{
    return QString::fromLatin1(data.toHex(' '));
}
