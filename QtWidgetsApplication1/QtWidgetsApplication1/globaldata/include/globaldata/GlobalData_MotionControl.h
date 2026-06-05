#pragma once

#ifndef MOTIONCONTROLGLOBALDATA_H
#define MOTIONCONTROLGLOBALDATA_H

#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

#include <QObject>
#include <QRectF>
#include <QList>
#include <QColor>
#include <QDataStream>
#include <QDebug>
#include <QPixmap>

#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif

/**
 *全局数据结构，用于各个子模块的之间的数据交互
 */
namespace DeviceGlobalData
{
    enum DeviceRunMode{
        em_ContinuousMode,
        em_RequestMode
    };

    namespace MotionControlCommon{
        //scale factor between inch and pixel, 1 inch = 25.4*100 pixels; 1 mm = 100 pixels;
        //运动控制器指令返回值
        const short INST_RES_EXEC_OK             = 0;           // 指令执行成功
        const short INST_RES_EXEC_ERROR          = 1;           // 指令执行失败        *检测当前指令的指令条件是否满足
        const short INST_RES_LICENSE_ERROR       = 2;           // Lincense不支持     *如果需要此功能，请与生产商联系
        const short INST_RES_PARAM_ERROR         = 7;           // 指令参数错误        *检测当前指令输入参数取值
        const short INST_RES_INST_ERROR          = 8;           // 不支持gai该指令     *DSP固件不支持该指令对应的功能

        //主机和运动控制器通讯失败
        const short INST_RES_CHECK_DRIVER        = -1;          // 是否正确安装运动控制器驱动程序
        const short INST_RES_CHECK_CON           = -2;          // 检查运动控制器是否接插牢靠
        const short INST_RES_CHANGE_HOST         = -3;          // 更换主机
        const short INST_RES_CHANGE_CONTROLLER   = -4;          // 更换控制器
        const short INST_RES_CHECK_GOLDFINGER    = -5;          // 运动控制器的金手指是否干净

        // 打开控制器失败
        const short INST_RES_OPEN_ERROR          = -6;          //1.是否正确安装运动控制器驱动程序
                                                                //2.是否调用了2此 GT_Open指令
                                                                //3.其程序是否已经打开运动控制器，或进程中是否流驻着打开的程序

        const short INST_RES_NO_RESPOND          = -7;          // 运动控制器没响应
        const short INST_RES_THREAD_BUSY         = -8;          // 多线程资源忙  检查PCI通讯是否正常

        //运动控制器指令返回值对应说明
        const char STR_RES_EXEC_OK[]            = "指令执行成功";

        const char STR_RES_EXEC_ERROR[]         = "指令执行失败, 检测当前指令的指令条件是否满足";
        const char STR_RES_LICENSE_ERROR[]      = "Lincense不支持, 如果需要此功能，请与生产商联系";
        const char STR_RES_PARAM_ERROR[]        = "指令参数错误, 检测当前指令输入参数取值";
        const char STR_RES_INST_ERROR[]         = "DSP固件不支持该指令的功能";

        const char STR_RES_CHECK_DRIVER[]       = "主机和运动控制器通讯失败,检查安装运动控制器驱动程序是否正确";
        const char STR_RES_CHECK_CON[]          = "主机和运动控制器通讯失败,检查运动控制器是否接插牢靠";
        const char STR_RES_CHANGE_HOST[]        = "主机和运动控制器通讯失败,更换主机";
        const char STR_RES_CHANGE_CONTROLLER[]  = "主机和运动控制器通讯失败,更换控制器";
        const char STR_RES_CHECK_GOLDFINGER[]   = "主机和运动控制器通讯失败,检查运动控制器的金手指是否干净";

        const char STR_RES_OPEN_ERROR[]         = "打开控制器失败";
        const char STR_RES_NO_RESPOND[]         = "运动控制器没响应";
        const char STR_RES_THREAD_BUSY[]        = "多线程资源忙";

        enum MotionMode
        {
            Trap            = 0,            //点位运动，控制器上电后，默认的模式
            Jog             = 1,            //Jog模式(以一定的速度，沿着某个方向运动)
            PT              = 2,            //
            Gear            = 3,            //电子齿轮模式
            Follow          = 4,            //
            Coordinate      = 5,            //插补模式
            PVT             = 6             //
        };

        //Home方向轴
        enum HomeAxis
        {
            HomeAxis_X = 1,
            HomeAxis_Y,
            HomeAxis_XAndY
        };

        //内存对齐
        struct AxisStatusFlag
        {
            AxisStatusFlag() {

            }

            bool bFlagAlarm         = false;		// 伺服报警标志

            bool bFlagMError        = false;		// 跟随误差越限标志

            bool bFlagPosLimit      = false;        // 正限位触发标志
            bool bFlagNegLimit      = false;        // 负限位触发标志

            bool bFlagSmoothStop    = false;		// 平滑停止标志
            bool bFlagAbruptStop    = false;		// 紧急停止标志

            bool bFlagServoOn       = false;        // 伺服使能标志
            bool bFlagMotion        = false;		// 规划器运动标志
        };

        struct AxisStatus
        {
            AxisStatus() {}

            int axisNo;
            //关键标志
            AxisStatusFlag statusFlag;

            //规划运动数据
            double profilePos;                         // 规划位置
            double profileVel;                         // 规划速度
            double profileAcc;                         // 规划加速度

            //编码器/实际 运动数据
            double encoderPos;                         // 实际位置
            double encoderVel;                         // 实际速度
            double encoderAcc;                         // 实际加速度

            //运动模式
            MotionMode motionMode;
        };

        enum GearType
        {
            Gear_Master_Profile,    //规划轴
            Gear_Master_Encoder     //编码器
        };
        //对外提供接口，不仅提供控制接口, 可提供单独调试的面板

        enum ResType
        {
            Res_OK,                 //指令执行正常
            Res_Not_Connection,     //驱动器未连接
            Res_Axis_Disable,       //轴未使能
            Res_Axis_Error          //轴报警
        };

        //停止方式
        enum StopType
        {
            Smooth,      //平滑停止
            Abrupt       //紧急停止
        };

//Device01  高工运动平台
//DeviceAOI AOI机器, 直线电机, Scale为1000
/*
    const unsigned long pulsesPerTurn   = 10000; //电机每旋转 1 圈的指令脉冲数
    const unsigned int  distancePerTurn = 10;    //mm

    // 1000个脉冲1mm,  100个脉冲1um
    // pulses/mm  一个毫米多少脉冲
    const double disUnit = (double)pulsesPerTurn/(double)distancePerTurn;
*/
        const double disUnit = 1000;

        const short X_Axis_Driving  = 1;    //X轴 主动轴
        const short Y_Axis_Driving  = 2;    //Y轴主动轴，龙门轴
        const short Y_Axis_Driven   = 3;    //Y轴从动轴

        const short Z_Axis_Driving               = 3;   //X轴 主动轴
        const short Conveyer_Width_Axis_Driving  = 4;   //pcb载板，调宽
        const short Conveyer_Axis_Driving        = 5;   //传送带

        //单位mm
        static const double X_Axis_MaxValue = 630;   //平台X方向最大值
        static const double X_Axis_MinValue = 0;

        static const double Y_Axis_MaxValue = 550;   //平台Y方向最大值
        static const double Y_Axis_MinValue = 0;

        //不回零，怎么判断绝对位置，但是实际情况是，调整好了，需要每次回零
        //手动回零，清空基准
        //阻塞回零，非阻塞回零

        //Z轴
        static const uint Z_Axis_MaxValue = 99999;
        static const uint Z_Axis_MinValue = 0;

        //传送带宽度
        static const uint Conveyer_MaxWidth = 99999;
        static const uint Conveyer_MinWidth = 0;

        //如果是绝对值传感器，则可以手动拖动，找到第一个点到位置

        //General IO(EXO) - Output control
        #define EXO_LED_Yellow   1
        #define EXO_LED_Green    2
        #define EXO_LED_Red      3
        #define EXO_AlarmBuzzer  4
        #define EXO_PCB_Lock     5
        #define EXO_PCB_Stop     6

        //General IO(EXI) - Input signals
        #define EXI_EmergencyStop    1     //急停
        #define EXI_LockDoor         2     //关门
        #define EXI_Undefine01       3
        #define EXI_Conveyer_In      4     //PCB到上料位置
        #define EXI_Conveyer_Out     5     //PCB到出料位置

        #define EXI_PCB_OnPosition   6     //PCB到检测位置
        #define EXI_PCB_Locked       7     //顶板气缸锁定
        #define EXI_PCB_Stoper_Down  9     //定位气缸 下降(Down Home)
        #define EXI_PCB_Stoper_Up    10    //定位气缸 上升

        #define EXI_UpDoorLock       13    //上门关闭
        #define EXI_DownDoorLock     14    //下门关闭

        //extern input
        struct GPIO_EXI_Status
        {
            GPIO_EXI_Status() = default;

            bool EmergencyStopFlag{false};         //急停标志
            bool UpDoorLockFlag{false};            //上门关闭标志
            bool DownDoorLockFlag{false};          //下门关闭标志

            bool ConveyerInSignalFlag{false};      //PCB到上料位置
            bool ConveyerOutSignalFlag{false};     //PCB到出料位置

            bool PcbOnInspectPosFlag{false};       //PCB到检测位置
            bool PcbLockFlag{false};               //顶板气缸锁定
            bool PcbStoperUpFlag{false};           //定位气缸 Home
            bool PcbStoperDownFlag{false};         //定位气缸 上升

            bool LoadPCBRequrestFlag{false};       //请求上料 上位机
            bool AllowUnLoadPCBFlag{false};        //允许下料 下位机
        };

        //本机状态，输出 extern output
        struct GPIO_EXO_Status
        {
            GPIO_EXO_Status() = default;

            bool LED_Yellow_EnableFlag{false};     //黄色LED
            bool LED_Green_EnableFlag{false};
            bool LED_Red_EnableFlag{false};

            bool AlarmBuzzerEnableFlag{false};     //蜂鸣器
            bool PCBLockedFlag{false};             //顶板
            bool PCBStopperUpFlag{false};          //档板

            bool LoadPCBRequestFlag{false};        //上料请求 本机
            bool UnloadPCBRequestFlag{false};      //卸料请求 本机

            bool ConveyerMotionFlag{false};
            bool ConveyerInverseFlag{false};
        };

        enum GPIOEnableStatus{
            EM_EXI_Status_Enable,    //输入
            EM_EXI_Status_Disable,
            EM_EXI_Status_Unknow,

            EM_EXO_Status_Enable,    //输出
            EM_EXO_Status_Disable,
            EM_EXO_Status_Unknow
        };

    }
};

/*
Q_DECLARE_METATYPE(GlobalData::Rect3D);
Q_DECLARE_METATYPE(GlobalData::ROIData);
Q_DECLARE_METATYPE(GlobalData::PackageData);
*/

#endif // MOTIONCONTROLGLOBALDATA_H
