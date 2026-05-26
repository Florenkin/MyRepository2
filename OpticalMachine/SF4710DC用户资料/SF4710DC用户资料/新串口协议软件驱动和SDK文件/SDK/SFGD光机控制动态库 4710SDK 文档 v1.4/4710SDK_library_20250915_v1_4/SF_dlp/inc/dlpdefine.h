#ifndef DLPDEFINE_H
#define DLPDEFINE_H

#include <QObject>

#define DEF_PATT_TIME_MAX_NUM		(100)	//MCU flash中存储最大的pattern time的个数
#define DEF_PATT_BIN_MAX_NUM		(100)	//MCU flash中存储最大的pattern bin的个数

#define DEF_USER_DATA_MAX_NUM       (23)

#define DEF_START_IIC_MIN_INDEX     (1)
#define DEF_START_IIC_MAX_INDEX     (9)
#define DEF_START_IIC_MAX_LEN       (115)

enum dlpError_e{
    DE_SUCCESS                  = 0,
    DE_FAIL,
    DE_OPEN_FILE_ERR,               //打开文件失败
    DE_PARAMETER_ERR,               //参数错误
    DE_DEVICE_BUSY,                 //设备忙

    /* start uart */
    DE_UART_ERR                 = 50,
    DE_UART_OPEN_FAIL,              //串口打开失败
    DE_UART_TIMEOUT,                //串口通信超时
    DE_UART_DISCONNECT,             //串口被强制断开连接
    DE_UART_NOT_OPEN,               //串口未打开
    DE_UART_BUSY,                   //串口忙
    /* end uart   */

    /* start cmd */
    DE_CMD_ERR                  = 100,
    DE_CMD_DATA_LEN_ERR,            //命令数据长度错误
    DE_CMD_TIMEOUT,                 //命令超时
    DE_CMD_RX_DATA,                 //命令数据错误
    DE_CMD_RX_DATA_LEN,             //命令数据长度错误
    /* end cmd */

    /*start pt and pb data*/
    DE_PT_AND_PB_ERR            = 150,
    DE_PACK_DATA_ERR,               //pattern bin数据包有问题，包首没有添加 PATN 字符
    DE_FIRST_TRANSMIT,              //pattern time或者pattern bin首包传输过程中出现错误
    DE_TRANSMITING,                 //pattern time或者pattern bin传输过程中出现错误
    DE_TIME_ERR,                    //pattern time显示时间太小
    DE_PRE_TIME_ERR,                //pattern time前曝光时间太小
    DE_POST_TIME_ERR,               //pattern time后曝光时间太小
    DE_IMAGE_OVER_LIMIT,            //pattern bin 图案数量超过限制
    DE_PT_INDEX_ERR,                //pattern time中索引超过了pattern sets
    DE_PT_NUM_ERR,                  //pattern time中的选择索引超过了最大图片数量
    DE_IMAGE_SIZE_ERR,              //pattern bin中选择图像大小错误
    /*end pt and pb data*/

    /*start update file */
    DE_UPDATE_ERR               = 200,
    DE_UPDATE_FILE_ERR,              //升级文件有误
    /*end update file*/
};

enum cmdErrCode{
    CEC_OK = 0x00,
    CEC_FAIL,
    CEC_CMD_CHECK,                  /* 命令校验错误*/
    CEC_CMD_INVALID,                /* 命令无校,没有查打到该命令*/
    CEC_CMD_DATA_LEN,               /* 命令数据错误,超出范围*/
    CEC_CMD_DATA,                   //数据错误
    CEC_CMD_PASSWORD,               //密码错误
    CEC_CMD_DATA_HANDLE,            //数据处理错误
    CEC_CMD_DLP_PWR_OFF,            //DLP电源关闭
    CEC_CMD_STORAGE_IS_FULL,        //MCU flash存储空间已满
    CEC_CMD_NOT_FILE,               //没有该文件
    CEC_CMD_OPERATE,                //该命令操作失败
    CEC_CMD_DATA_EMPTY,             //读取命令中数据为空
    CEC_CMD_CRC,                    //校验CRC错误，这里主要是指传输大文件时CRC校验错误
    CEC_CMD_12V_OFF                 //DLP的12V关闭
};

enum dlpStatus_e{
    DS_REBOOT   = 0x01,     //重启DLP
    DS_START,               //启动DLP
    DS_STOP                 //关闭DLP
};

enum ctlStatus_e{
    CS_OPEN_12V = 0x01,		//打开12V电源 false:关闭12V电源,true:打开12V电源
    CS_CLOSE_12V,			//关闭12V电源
    CS_SPI_TO_MCU,			//SPI连接到MCU
    CS_SPI_TO_USB,			//SPI连接到USB
    CS_OPEN_PARKZ_PIN,      //打开PARKZ引脚
    CS_CLOSE_PARKZ_PIN,     //关闭PARKZ引脚
    CS_OPEN_DLPCM_PIN,      //打开DLPCM引脚
    CS_CLOSE_DLPCM_PIN      //关闭DLPCM引脚
};

struct runStatus_t{
    bool dlpStatus;         //DLP状态false:关机, true:开机
    quint8 mcuTemp;         //MCU温度
    quint8 dlpTemp;         //DLP温度
    bool mcuTempAlarm;      //MCU告警温度
    bool ledTempAlarm;      //DLP告警温度
    bool power12Vstate;     //12V电源状态   false:关闭12V电源,true:打开12V电源
    bool spiPinState;       //SPI引脚状态   false:USB spi, true:MCU spi
    bool parkzPinState;     //parkz引脚状态
    bool dlpcmPinState;     //dlpcm引脚状态
};

struct rgbCurrent_t{
    quint16 redCurrent;
    quint16 greenCurrent;
    quint16 blueCurrent;
};

struct triggerIn_t{
    bool enable;        //触发输入使能，0：关闭输入触发，1：打开输入触发
    bool polar;         //触发输入极性，0：低电平有效，1：高电平有效
    int in1HoldTime;    //输入1保持时间[1-100]ms
    int in2HoldTime;    //输入2保持时间[1-100]ms
};

struct triggerEnd_t{
    bool enable;        //使能，0：关闭输出触发，1：打开输出触发
    bool polar;         //极性，0：低电平有效，1：高电平有效
    int outDelay;       //输出延时，数据范围[-999, 999]ms
    int outHoldTime;    //输出极性保持时间
};

enum testPattern_e{
    TP_CHECKERBOARD,                //棋盘
    TP_WHITE_COLOR,                 //白图
    TP_BLACK_COLOR,                 //黑图
    TP_GRID,                        //网格
    TP_COLOR_RAMP                   //色阶
};

enum operatMode_e{
    OM_EXTERNAL_VIDEO_PORT = 0,		// 0x0-External Video Port
    OM_TEST_PATTERN_GENERATOR,		// 0x1-Test Pattern Generator
    OM_SPLASH_SCREEN,				// 0x2-Splash Screen
    OM_SENS_EXTERNAL_PATTERN,		// 0x3-External Pattern Streaming
    OM_SENS_INTERNAL_PATTERN,		// 0x4-Internal Pattern Streaming
    OM_SENS_SPLASH_PATTERN,			// 0x5-Splash Pattern Streaming
    OM_STANDBY=0xFF					// 0xFF-Standby
};

enum patternMode_e
{
    PM_EXTERNAL = 0,                //External
    PM_INTERNAL,                    //Internal
    PM_SPLASH,                      //Splash
};

enum sequenceType_e
{
    ST_ONE_BIT_MONO = 0x0,          //1 Bit Mono
    ST_ONE_BIT_RGB,                 //1 Bit RGB
    ST_EIGHT_BIT_MONO,              //8 Bit Mono
    ST_EIGHT_BIT_RGB,               //8 Bit RGB
};

enum exposureTimeSupported_e
{
    ETS_NO = 0x0,                   //No
    ETS_YES,                        //Yes
};

enum zeroDarkTimeSupported_e
{
    ZDTS_NO = 0x0,                 //No
    ZDTS_YES,                      //Yes
};

struct validateExposureTime_t
{
    exposureTimeSupported_e ExposureTimeSupported;
    zeroDarkTimeSupported_e ZeroDarkTimeSupported;
    uint32_t MinimumExposureTime;
    uint32_t PreExposureDarkTime;
    uint32_t PostExposureDarkTime;
};

enum writeControl_e
{
    WC_CONTINUE = 0x0,             //Continue
    WC_START,                      //Start
    WC_RELOAD_FROM_FLASH,          //Reload from Flash
};

enum illuminatorEnable_e
{
    IE_DISABLE = 0x0,              //Disable
    IE_ENABLE,                      //Enable
};

struct patternOrderTableEntry_t
{
    uint8_t PatSetIndex;
    uint8_t NumberOfPatternsToDisplay;
    illuminatorEnable_e RedIlluminator;
    illuminatorEnable_e GreenIlluminator;
    illuminatorEnable_e BlueIlluminator;
    uint32_t PatternInvertLsword;
    uint32_t PatternInvertMsword;
    uint32_t IlluminationTime;
    uint32_t PreIlluminationDarkTime;
    uint32_t PostIlluminationDarkTime;
};

enum imageMirror_e{
    IF_MIRROR_NONE,                 //无镜像
    IF_MIRROR_VERTICAL,             //长轴镜像
    IF_MIRROR_HORIZONTAL,           //短轴镜像
    IF_MIRROR_ROTATE                //长轴和短轴镜像
};

struct readySignal_t{
    bool triggerOut1Enable;         //DLP触发输出1使能
    bool triggerOut1Invert;         //DLP触发输出1极性
    int triggerOut1Delay;           //DLP触发输出1延时
    bool triggerOut2Enable;         //DLP触发输出2使能
    bool triggerOut2Invert;         //DLP触发输出2极性
    int triggerOut2Delay;           //DLP触发输出2延时
    bool triggerInEnable;           //DLP触发输入使能
    bool triggerInPolarity;         //DLP触发输入极性
    bool patternReadyEnable;        //DLP图案就绪使能
    bool patternReadyPolarity;      //DLP图案就绪极性
};

enum patternCtl_e{
    PC_START = 0,               //开始
    PC_STOP,                    //停止
    PC_PAUSE,                   //暂停
    PC_STEP,                    //单步
    PC_RESUME,                  //继续
    PC_RESET                    //重新开始
};

enum updateDir_e{
    UD_TO_DLP,                  //将DLP程序升级到DLP中
    UD_TO_FLASH                 //将DLP程序升级到MCU flash中
};

/*
 * ---------------  pattern time
 */
#define PAT_ORDER_TABLE_SIZE            (23)

enum illuminator_e{
    ILL_RED     = 0x01,
    ILL_GREEN   = 0x02,
    ILL_BLUE    = 0x04,
    ILL_RGB     = 0x07,
};

struct pattOrder_t{
    int pattSetsIndex;
    int pattTotalNum;       //图案的总数量
    illuminator_e illuminator;
    bool invert;
    quint32 time;
    quint32 preDarkTime;
    quint32 postDarkTime;
};

/*
 * ---------------  pattern bin
 */
//添加最大图片数量
#define DEF_1BIT_H_PIC_MAX_NUM          (60)
#define DEF_1BIT_V_PIC_MAX_NUM          (64)
#define DEF_8BIT_H_PIC_MAX_NUM          (7)
#define DEF_8BIT_V_PIC_MAX_NUM          (8)

enum pattType_e{
    PT_ONE_HORIZONTAL = 0x01,
    PT_ONE_VERTICAL,
    PT_EIGHT_HORIZONTAL,
    PT_EIGHT_VERTICAL
};
struct pattSets_t{
    pattType_e type;
    QStringList pattPaths;
};



enum INT_PAT_BitDepth_e
{
    INT_PAT_BITDEPTH_ONE = 1,
    INT_PAT_BITDEPTH_EIGHT = 8
};

enum INT_PAT_Direction_e
{
    INT_PAT_DIRECTION_HORIZONTAL,
    INT_PAT_DIRECTION_VERTICAL
};

enum INT_PAT_IlluminationSelect_e
{
    INT_PAT_ILLUMINATION_NONE = 0,
    INT_PAT_ILLUMINATION_RED = 1,
    INT_PAT_ILLUMINATION_GREEN = 2,
    INT_PAT_ILLUMINATION_BLUE = 4,
    INT_PAT_ILLUMINATION_RGB = 7
};

struct INT_PAT_PatternData_t
{
    uint32_t PixelArrayCount;
    uint8_t* PixelArray;
};

struct INT_PAT_PatternSet_t
{
    INT_PAT_BitDepth_e     BitDepth;
    INT_PAT_Direction_e    Direction;
    uint32_t               PatternCount;
    INT_PAT_PatternData_t* PatternArray;
};

struct INT_PAT_PatternOrderTableEntry_t
{
    uint8_t                               PatternSetIndex;
    uint8_t                               NumDisplayPatterns;
    INT_PAT_IlluminationSelect_e          IlluminationSelect;
    bool                                  InvertPatterns;
    uint32_t                              IlluminationTimeInMicroseconds;
    uint32_t                              PreIlluminationDarkTimeInMicroseconds;
    uint32_t                              PostIlluminationDarkTimeInMicroseconds;
};

#endif // DLPDEFINE_H
