/****************************************************************************
**
**  File Name:          dlphandle.h
**  Description:        Declaration of the ExampleClass, a Qt-based class providing [briefly state core functionality, e.g., "data processing and UI interaction features"].
**  Author:             [zhangya/853728579@qq.com]
**  Created:            [Date, e.g., 2024-05-20]
**  Last Modified:      [Date, e.g., 2024-05-21]
**  Version:            1.0.0
**  Qt Version:         Tested with Qt 5.30
**  License:            [e.g., MIT License, GNU GPL v3]
**
****************************************************************************/

#ifndef DLPHANDLE_H
#define DLPHANDLE_H

#include <QObject>

#include "dlphandle_global.h"
#include "dlpdefine.h"

#define DEF_TIMEOUT     (10)

class McuUart;
class DlpcSamples;
class QTimer;

class DLPHANDLE_LIBRARY_EXPORT DlpHandle : public QObject
{
    Q_OBJECT

private:
    struct uartPack_t{
        bool rxFlag;
        bool errFlag;
        quint8 cmd;
        QByteArray data;
    };

    enum eventFlag{
        EF_NONE = 0,
        EF_START,
        EF_PROCESSING,
        EF_END
    };

    struct eventPack_t{
        eventFlag status    = EF_NONE;
        bool onceFlag       = false;
        bool successFlag    = false;
    };

public:
    enum dataDire_e{
        DD_RECEIVE,
        DD_TRANSMIT
    };
    Q_ENUM(dataDire_e)

    enum transmitStatus_e{
        TS_NONE = 0,
        TS_START,
        TS_FINISH,
        TS_ERROR,
        TS_EXIT
    };
    Q_ENUM(transmitStatus_e)

public:
    explicit DlpHandle(QObject *parent = nullptr);

    //初始化库
    int SF_initLibrary(void);
    //获取库的版本号
    int SF_getLibraryVersion(int & mainVer, int & subVer);

    //uart配置
    int SF_openUart(const QString & com);
    int SF_closeUart(void);
    //检测设备是否就绪
    int SF_deviceCheckReady(void);

    //设置DLP电源
    int SF_setDlpPower(const dlpStatus_e status);
    //设置软触发
    int SF_softTrigger(const quint8 triCnt, const quint16 interval);
    //设置蓝灯亮度
    int SF_setBlueBright(const quint16 bright);
    //获取蓝灯亮度
    int SF_getBlueBright(quint16 & outBright);
    //复位MCU
    int SF_resetMCU(void);
    //获取MCU程序的版本号
    int SF_getVersion(QString & outVersion);
    //恢复出厂设置
    int SF_resetFactory(void);
    //获取MCU程序的构建时间
    int SF_getBuildTime(QString & outBuildTime);
    //更新触发输出时间
    int SF_updateTriggerTime(quint32 & outTime);
    //控制MCU硬件接口
    int SF_ctlHardware(const ctlStatus_e status);
    //设置用户数据
    int SF_setUserData(const QByteArray & data);
    //获取用户数据
    int SF_getUserData(QByteArray & outData);
    //获取设置ID号
    int SF_getDeviceID(QByteArray & outDeviceID);
    //获取告警温度值
    int SF_getAlarmTemp(quint8 & outMcuTemp, quint8 & outDlpTemp);
    //获取运行状态
    int SF_getRunStatus(runStatus_t & outStatus);
    //设置设备启动加载的IIC命令集
    int SF_setStartIIC(const int index, const quint8 cmd, const QByteArray & data);
    //获取设备启动加载的IIC命令集
    int SF_getStartIIC(const int index, quint8 & outCmd, QByteArray & outData);
    //删除设备启动加载的IIC命令集
    int SF_delStartIIC(const int index);
    //清除设备启动所有加载的IIC命令集
    int SF_clrStartIIC(void);
    //设置设备启动时自动配置的RGB电流大小
    int SF_setStartCurren(const rgbCurrent_t & current);
    //获取设备启动时自配置的RGB电流大小
    int SF_getStartCurrent(rgbCurrent_t & outCurrent);
    //删除设备启动时自动配置的RGB电流大小
    int SF_delStartCurrent(void);
    //设置触发输入参数
    int SF_setTriggerInPara(const triggerIn_t & triIn);
    //获取解发输入参数
    int SF_getTriggerInPara(triggerIn_t & outTriIn);
    //设置触发结束参数
    int SF_setTriggerEndPara(const triggerEnd_t & triEnd);
    //获取触发结束参数
    int SF_getTriggerEndPara(triggerEnd_t & outTriEnd);
    //脉冲信号输出
    int SF_outPulseSignal(const int time);
    //获取触发输入计数
    int SF_getTriggerCount(quint32 & outCount);
    //清除触发输入计数
    int SF_clrTriggerCount(void);
    //获取设备启动配置pattern time文件名
    int SF_getStartupPT(QString & outFileName);

    //设置测试图案
    int SF_setTestPattern(const testPattern_e pattern);
    //设置splash画面
    int SF_setSplashImages(const quint8 splash);
    //获取操作模式
    int SF_getOperationMode(operatMode_e & outMode, QString & outModeStr);
    //设置操作模式
    int SF_setOperationMode(const operatMode_e mode);
    //切换到外部视频
    int SF_switchExternalVideo(void);
    //设置色温
    int SF_setColorTemp(const quint8 colorTemp);
    //获取色温
    int SF_getColorTemp(quint8 & outColorTemp);
    //设置图像镜像
    int SF_setImageMirror(const imageMirror_e mirror);
    //获取图像镜像
    int SF_getImageMirror(imageMirror_e & outMirror);
    //设置RGB灯使能
    int SF_setRGBEnable(bool red, bool green, bool blue);
    //获取RGB灯使用
    int SF_getRGBEnable(bool & red, bool & green, bool & blue);
    //设置RGB灯电流大小
    int SF_setRGBCurrent(quint16 red, quint16 green, quint16 blue);
    //获取RGB灯电流大小
    int SF_getRGBCurrent(quint16 & red, quint16 & green, quint16 & blue);
    //获取RGB灯最大电流
    int SF_getRGBMaxCurrent(quint16 & maxRed, quint16 & maxGreen, quint16 & maxBlue);
    //设置触发和模式就绪信号
    int SF_setTriggerAndPatternSign(const readySignal_t & readySignal);
    //获取触发和模式就绪信号
    int SF_getTriggerAndPatternSign(readySignal_t & outReadySignal);
    //显示和控制图案
    int SF_setPatternControl(patternCtl_e pattern, quint8 repeatCount);
    int SF_getSequenceError(bool & error);
    //通过曝光时间获取最小predrak time和postdrak time值
    int SF_readValidateExposureTime(patternMode_e PatternMode, sequenceType_e BitDepth, uint32_t ExposureTime, validateExposureTime_t &ValidateExposureTime);
    //写入曝光时间
    int SF_writePatternOrderTableEntry(writeControl_e WriteControl, patternOrderTableEntry_t &PatternOrderTableEntry);
    //从flash中加载时序表到DLP中
    int SF_loadPatternOrderTableEntryFromFlash(void);

    //向DLP中写入数据
    int SF_writeDataToDLP(const quint8 cmd, const QByteArray & writeData);
    //从DLP中读取数据
    int SF_readDataFromDLP(const quint8 cmd, const QByteArray & writeData, const quint8 readLen, QByteArray & outReadData);

    //从flash中加载pattern time到DLP中
    int SF_loadPTtoDLPfromFlash(bool save, QString & fileName);
    //升级pattern time到DLP中
    int SF_updatePTtoDLPfromAPP(const QByteArray & ptData, quint8 timeout = DEF_TIMEOUT);
    //从DLP中获取pattern time数据
    int SF_getPTtoAPPfromDLP(QByteArray & outPtData, quint8 timeout = DEF_TIMEOUT);
    //生成pattern data
    int SF_generatePatternData(const std::vector<INT_PAT_PatternSet_t> & PatternSets, const std::vector<INT_PAT_PatternOrderTableEntry_t> & PatternOrderTableEntries, bool eastWestFlip, bool longAxisFlip, QByteArray & outPatternData);
    //升级pattern bin数据到DLP中
    //这个代码处理的时间比较长，需要等待时间长点，timeout这个参数不需要配置
    int SF_updatePBtoDLPfromAPP(const QByteArray & pbData, quint8 timeout = DEF_TIMEOUT);
    //从DLP中获取pattern bin数据
    int SF_getPBtoAPPfromDLP(QByteArray & outPbData, quint8 timeout = DEF_TIMEOUT);

    //mcu flash
    //升级pattern time数据到MCU flash
    //flashName:MCU flash中存储的pattern time数据文件名，以.pt结尾，例如: 0.pt, 范围: <0 - DEF_PATT_TIME_MAX_NUM>
    int SF_updatePTtoFlashFromAPP(const QString & flashName, const QByteArray & ptData, quint8 timeout = DEF_TIMEOUT);
    //从MCU flash中获取pattern time数据
    //flashName:MCU flash中存储的pattern time数据文件名，以.pt结尾，例如: 0.pt, 范围: <0 - DEF_PATT_TIME_MAX_NUM>
    int SF_getPTtoAPPFromFlash(const QString & flashName, QByteArray & outPtData, quint8 timeout = DEF_TIMEOUT);

    //从MCU flash获取所有保存的文件名(pattern time, pattern bin, DLP升级文件)
    int SF_getFileNameFromFlash(int & flashUsed, QStringList & fileName);
    //从MCU flash中删除文件
    int SF_delFileFromFlash(const QStringList & fileName);
    //清除MCU flash中所有文件
    int SF_clrAllFileFromFlash(void);

    //升级DLP程序到  MCU flash 或 DLP 中
    int SF_updateDLPProgram(const updateDir_e dir, const QString & fileName, quint8 timeout = DEF_TIMEOUT);
    //从MCU flash中升级DLP程序
    int SF_updateDLPFromFlash(void);
    //从DLP中获取主DLP flash中所有程序
    int SF_getDLPProgramFromDLP(QByteArray & outData, quint8 timeout = DEF_TIMEOUT);
    //升级MCU程序
    int SF_updateMCUProgram(const QString & fileName, quint8 timeout = DEF_TIMEOUT);

signals:
    //uart
    //串口错误信号,参考 enum dlpError_e 错误码
    //这里主要是串口硬件断开，数据发送超时等错误
    void SF_uartErrorSign(int errCode);
    //串口完整数据包
    void SF_uartPackSign(dataDire_e dir, const QByteArray & pack);
    //串口命令数据包
    void SF_uartCmdDataSign(dataDire_e dir, quint8 cmd, const QByteArray & data);
    //处理串口上报命令
    void SF_handleRxReportSign(quint8 cmd, const QByteArray & data);
    //处理串口接收错误，参考enum cmdErrCode
    //这里主要是底层MCU处理串口数据返回的错误
    void SF_handleRxErrorSign(quint8 cmd, int errCode);
    //传输状态
    void SF_transmitStatusSign(transmitStatus_e status);
    //传输进度
    void SF_transmitProgressSign(int progress);

private:
    void parameterInit(void);

    int sendData(quint8 cmd, const QByteArray & data = QByteArray());
    void clrUartPack(void);
    bool waitReceptionComplete(const uartPack_t & pack, int timeout = 100);
    void clrEventPack(void);
    int waitEventComplete(eventPack_t & event, int firstTimeout = 200, int onecTimeout = 200);
    int checkPTandPB(const QList<pattSets_t>& psList, const QList<pattOrder_t> & ptList);
    quint32 crc32(quint8 *pData, int len);

private:
    McuUart * m_mcuUart = nullptr;
    DlpcSamples * m_dlpcSamples = nullptr;

    uartPack_t  m_uartPack;
    eventPack_t m_eventPack;

    QTimer * m_deviceTimer = nullptr;
    int m_deviceBusyCnt = -1;
    bool m_deviceBusy = false;
};

#endif // DLPHANDLE_H
