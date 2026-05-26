#include "mainwidget.h"
#include "countdowndialog.h"
#include "ui_mainwidget.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QButtonGroup>
#include <QDebug>
#include <math.h>

#include "cintvalidator.h"
#include "simpleQtLogger.h"
#include "tabledelegates.h"

#define APP_STARTUP_IIC_DATA_SIZE (64) //startup iic data size

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    m_dlpHandle = new DlpHandle(this);
    m_dlpHandle->SF_initLibrary();

    layoutInit();

    m_countDownDialog = new CountDownDialog();
    m_countDownDialog->stopCount();
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::layoutInit()
{
    refreshComboxCom();
    connect(ui->comboBox_com, &MyComboBox::clicked, this, &MainWidget::refreshComboxCom);
    ui->pushButton_com_connect->setProperty("ComStatus", false);

    //串口错误处理
    connect(m_dlpHandle, &DlpHandle::SF_uartErrorSign, this, [this](int errCode){
        if (errCode == DE_SUCCESS){
            return ;
        }
        if (ui->pushButton_com_connect->property("ComStatus").toBool()){
            //disconnect
            ui->pushButton_com_connect->setProperty("ComStatus", false);
            ui->pushButton_com_connect->setText(tr("Connect"));
        }
        QMessageBox::warning(this, tr("Error"), QString("Uart error code : 0x%1 ").arg(errCode, 4, 16, QLatin1Char('0')));
    });

    //低层MCU处理命令错误
    connect(m_dlpHandle, &DlpHandle::SF_handleRxErrorSign, this, [this](quint8 cmd, int errCode){
        QMessageBox::warning(this, tr("Error"), QString("Uart cmd : 0x%1,error code : 0x%2 ").arg(cmd, 2, 16).arg(errCode&0xff, 4, 16, QLatin1Char('0')));
    });

    //菜单按键
    QButtonGroup * btnGroup = new QButtonGroup(this);
    ui->pushButton_operation->setCheckable(true);
    btnGroup->addButton(ui->pushButton_operation, 0);
    ui->pushButton_update->setCheckable(true);
    btnGroup->addButton(ui->pushButton_update, 1);
    ui->pushButton_log->setCheckable(true);
    btnGroup->addButton(ui->pushButton_log, 2);
    connect(btnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), ui->stackedWidget, &QStackedWidget::setCurrentIndex);
    btnGroup->button(0)->click();

    //----------------------操作类
    ui->lineEdit_color_temp->setMaxLength(4);
    ui->lineEdit_color_temp->setPlaceholderText("0-255");
    ui->lineEdit_color_temp->setValidator(new CIntValidator(0, 255, ui->lineEdit_color_temp));

    //soft trigger
    ui->radioButton_single->setChecked(true);
    ui->label_number->hide();
    ui->lineEdit_number->hide();
    ui->label_interval->hide();
    ui->lineEdit_interval->hide();

    //startup iic command
    ui->lineEdit_startup_iic_index->setMaxLength(1);
    ui->lineEdit_startup_iic_index->setPlaceholderText("1-9");
    ui->lineEdit_startup_iic_index->setValidator(new CIntValidator(1, 9, ui->lineEdit_startup_iic_index));
    //startup iic cmd只能输入16进制数
    ui->lineEdit_startup_iic_cmd->setMaxLength(2);
    QRegExp regx("^[0-9a-fA-F ]+$");
    ui->lineEdit_startup_iic_cmd->setValidator(new QRegExpValidator(regx, this));
    ui->lineEdit_startup_iic_data->setMaxLength(APP_STARTUP_IIC_DATA_SIZE * 3 + 3);
    ui->lineEdit_startup_iic_data->setValidator(new QRegExpValidator(regx, this));

    //RGB led current
    ui->lineEdit_red_led_current->setMaxLength(4);
    ui->lineEdit_red_led_current->setPlaceholderText("91-1000");
    ui->lineEdit_red_led_current->setValidator(new CIntValidator(91, 1000, ui->lineEdit_red_led_current));
    ui->lineEdit_green_led_current->setMaxLength(4);
    ui->lineEdit_green_led_current->setPlaceholderText("91-1000");
    ui->lineEdit_green_led_current->setValidator(new CIntValidator(91, 1000, ui->lineEdit_green_led_current));
    ui->lineEdit_blue_led_current->setMaxLength(4);
    ui->lineEdit_blue_led_current->setPlaceholderText("91-1000");
    ui->lineEdit_blue_led_current->setValidator(new CIntValidator(91, 1000, ui->lineEdit_blue_led_current));

    ui->pushButton_display_control_pause->setEnabled(false);
    ui->pushButton_display_control_step->setEnabled(false);
    ui->pushButton_display_control_restart->setEnabled(false);
    ui->pushButton_display_control_stop->setEnabled(false);
    ui->pushButton_display_control_pause->setProperty("PauseStatus", false);

    ui->lineEdit_iic_common->setMaxLength(10 * 3 + 3);
    ui->lineEdit_iic_common->setValidator(new QRegExpValidator(regx, this));
    ui->lineEdit_iic_data->setMaxLength(APP_STARTUP_IIC_DATA_SIZE * 3 + 3);
    ui->lineEdit_iic_data->setValidator(new QRegExpValidator(regx, this));

    //----------------------升级类
    ui->progressBar_flash_space->setValue(0);
    ui->progressBar_flash_space->setRange(0, 100);

    ui->tableWidget_flash_file->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget_flash_file->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableWidget_flash_file->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_flash_file->setAlternatingRowColors(true);
    //tableWidget_flash_file单击响应
    connect(ui->tableWidget_flash_file, &QTableWidget::itemClicked, this, [this](QTableWidgetItem * item){
        //qDebug() << "clicked item" << item->row() << item->column();
        //获取当前选中文本
        selectPatternTimeFile(item->text());
    });

    //MCU升级
    ui->progressBar_mcu_update->setValue(0);
    ui->progressBar_mcu_update->setRange(0, 100);

    ui->progressBar_dlp_update->setValue(0);
    ui->progressBar_dlp_update->setRange(0, 100);
    ui->progressBar_get_dlp_from_dlp->setValue(0);
    ui->progressBar_get_dlp_from_dlp->setRange(0, 100);
    //DLP升级按钮组
    QButtonGroup * btnGroupUpdate = new QButtonGroup(this);
    btnGroupUpdate->addButton(ui->radioButton_load_dlp_to_dlp, 0);
    btnGroupUpdate->addButton(ui->radioButton_load_dlp_to_flash, 1);
    btnGroupUpdate->addButton(ui->radioButton_load_dlp_to_dlp_from_flash, 2);
    connect(btnGroupUpdate, QOverload<int, bool>::of(&QButtonGroup::buttonToggled), this, [this](int id, bool checked){
        if (!checked){
            return;
        }
        switch (id)
        {
        case 0:
        case 1: ui->pushButton_dlp_update->setText(tr("DLP firmware")); break;
        default: ui->pushButton_dlp_update->setText(tr("DLP update")); break;
        }
    });
    
    //pattern time
    ui->progressBar_pattern_time->setValue(0);
    ui->progressBar_pattern_time->setRange(0, 100);
    ui->checkBox_pt_save->hide();
    ui->label_pt_file_name->hide();
    ui->lineEdit_pt_file_name->hide();
    selectPatternTimeFile(QString());

    //PT升级按钮组
    QButtonGroup * btnGroupUpdatePt = new QButtonGroup(this);
    btnGroupUpdatePt->addButton(ui->radioButton_load_pt_to_flash, 0);
    btnGroupUpdatePt->addButton(ui->radioButton_load_pt_to_dlp, 1);
    btnGroupUpdatePt->addButton(ui->radioButton_load_pt_to_dlp_from_flash, 2);
    btnGroupUpdatePt->addButton(ui->radioButton_get_pt_from_flash, 3);
    btnGroupUpdatePt->addButton(ui->radioButton_get_pt_from_dlp, 4);
    connect(btnGroupUpdatePt, QOverload<int, bool>::of(&QButtonGroup::buttonToggled), this, [this](int id, bool checked){
        if (!checked){
            return;
        }

        ui->label_pt_file_name->show();
        ui->lineEdit_pt_file_name->show();
        switch (id)
        {
        case 0:
            ui->checkBox_pt_save->hide();
            ui->pushButton_pattern_time->setText(tr("PT file"));
            break;
        case 1:
            ui->label_pt_file_name->hide();
            ui->lineEdit_pt_file_name->hide();
            ui->checkBox_pt_save->hide();
            ui->pushButton_pattern_time->setText(tr("PT file"));
            break;
        case 2: 
            ui->checkBox_pt_save->show();
            ui->pushButton_pattern_time->setText(tr("PT update"));
            break;
        case 3:
            ui->checkBox_pt_save->hide();
            ui->pushButton_pattern_time->setText(tr("Save path"));
            break;
        case 4: 
            ui->label_pt_file_name->hide();
            ui->lineEdit_pt_file_name->hide();
            ui->checkBox_pt_save->hide();
            ui->pushButton_pattern_time->setText(tr("Save path"));
            break;
        default: break;
        }
    });

    //pattern bin升级
    ui->progressBar_pattern_bin->setValue(0);
    ui->progressBar_pattern_bin->setRange(0, 100);
    //pattern bin升级按钮组
    QButtonGroup * btnGroupUpdateBin = new QButtonGroup(this);
    btnGroupUpdateBin->addButton(ui->radioButton_load_pb_to_dlp, 0);
    btnGroupUpdateBin->addButton(ui->radioButton_get_pb_from_dlp, 1);
    connect(btnGroupUpdateBin, QOverload<int, bool>::of(&QButtonGroup::buttonToggled), this, [this](int id, bool checked){
        if (!checked){
            return;
        }
        switch (id)
        {
        case 0: ui->pushButton_pattern_bin->setText(tr("PB file")); break;
        case 1: ui->pushButton_pattern_bin->setText(tr("Save path")); break;
        default: break;
        }
    });

    //------------------------日志类
    connect(m_dlpHandle, &DlpHandle::SF_uartPackSign, this, [this](DlpHandle::dataDire_e dir, const QByteArray & pack){
        QString log = (dir == DlpHandle::DD_RECEIVE)?("rx = "):("tx = ");
        log.append(pack.toHex(' '));
        ui->textEdit_log->append(log);
    });
}

//刷新串口列表
void MainWidget::refreshComboxCom()
{
    //清空串口列表
    ui->comboBox_com->blockSignals(true);
    ui->comboBox_com->clear();
    //添加串口列表
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    std::sort(ports.begin(), ports.end(), comparePortNames);
    foreach(auto com, ports) {
        ui->comboBox_com->addItem(com.portName());
    }
    ui->comboBox_com->blockSignals(false);
}

//十六进制字符串转化为字节数组
QByteArray MainWidget::strToHex(const QString &str)
{
    QByteArray byteArray;
    QStringList strList = str.split(" ");
    foreach (QString str, strList) {
        bool ok = false;
        int value = str.toInt(&ok, 16);
        if (ok){
            byteArray.append(char(value & 0xFF));
        }
    }
    return byteArray;
}

//比较串口名称后面的数字的大小，从小到大排序
bool MainWidget::comparePortNames(const QSerialPortInfo &port1, const QSerialPortInfo &port2)
{
    bool ok = false;
    int p1 = port1.portName().remove("COM").toInt(&ok);
    if (ok == false){
        return false;
    }
    int p2 = port2.portName().remove("COM").toInt(&ok);
    if (ok == false){
        return false;
    }

    return p1 < p2;
}

void MainWidget::on_pushButton_com_connect_clicked()
{
    if (ui->pushButton_com_connect->property("ComStatus").toBool()){
        //断开串口连接
        int ret = m_dlpHandle->SF_closeUart();
        if (ret == DE_SUCCESS){
            ui->pushButton_com_connect->setProperty("ComStatus", false);
            ui->pushButton_com_connect->setText(tr("Connect"));
        }

        return ;
    }
    //打开串口连接
    QString currentCom = ui->comboBox_com->currentText();
    int ret = m_dlpHandle->SF_openUart(currentCom);
    if (ret == DE_SUCCESS) {
        ui->pushButton_com_connect->setProperty("ComStatus", true);
        ui->pushButton_com_connect->setText(tr("Disconnect"));
    }
}

void MainWidget::on_comboBox_com_currentIndexChanged(int index)
{
    if (index < 0){
        return ;
    }
}

void MainWidget::on_pushButton_version_get_clicked()
{
    //失能按钮，防止多次点击
    ui->pushButton_version_get->setEnabled(false);
    QString version;
    int ret = m_dlpHandle->SF_getVersion(version);
    if (ret != DE_SUCCESS){
        ui->pushButton_version_get->setEnabled(true);
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
        return ;
    }
    ui->lineEdit_version->setText(version);
    ui->pushButton_version_get->setEnabled(true);
}

void MainWidget::on_pushButton_operation_mode_get_clicked()
{
    //失能按钮，防止多次点击
    ui->pushButton_operation_mode_get->setEnabled(false);
    operatMode_e operatMode;
    QString operatModeStr;
    int ret = m_dlpHandle->SF_getOperationMode(operatMode, operatModeStr);
    if (ret != DE_SUCCESS){
        ui->pushButton_operation_mode_get->setEnabled(true);
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
        return ;

    }
    ui->lineEdit_operation_mode->setText(operatModeStr);
    ui->pushButton_operation_mode_get->setEnabled(true);
}

void MainWidget::on_pushButton_color_temp_get_clicked()
{
    //失能按钮，防止多次点击
    ui->pushButton_color_temp_get->setEnabled(false);
    quint8 colorTemp;
    int ret = m_dlpHandle->SF_getColorTemp(colorTemp);
    if (ret != DE_SUCCESS){
        ui->pushButton_color_temp_get->setEnabled(true);
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
        return ;
    }
    ui->lineEdit_color_temp->setText(QString::number(colorTemp));
    ui->pushButton_color_temp_get->setEnabled(true);
}

void MainWidget::on_pushButton_color_temp_set_clicked()
{
    //失能按钮，防止多次点击
    ui->pushButton_color_temp_set->setEnabled(false);
    bool ok = false;
    quint8 colorTemp = quint8(ui->lineEdit_color_temp->text().toUInt(&ok) & 0xFF);
    int ret = m_dlpHandle->SF_setColorTemp(colorTemp);
    if (ret != DE_SUCCESS){
        L_ERROR("set color temperature error!");
        //显示错误信息
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_color_temp_set->setEnabled(true);
}
//读取运行状态
void MainWidget::on_pushButton_run_state_get_clicked()
{
    //失能按钮，防止多次点击
    ui->pushButton_run_state_get->setEnabled(false);

    runStatus_t runStatus;
    int ret = m_dlpHandle->SF_getRunStatus(runStatus);
    if (ret != DE_SUCCESS){
        ui->pushButton_run_state_get->setEnabled(true);
        L_ERROR("get run state error!");
        //显示错误信息
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));

        return ;
    }

/*
    bool dlpStatus;         //DLP状态false:关机, true:开机
    quint8 mcuTemp;         //MCU温度
    quint8 dlpTemp;         //DLP温度
    bool mcuTempAlarm;      //MCU告警温度
    bool ledTempAlarm;      //DLP告警温度
    bool power12Vstate;     //12V电源状态   false:关闭12V电源,true:打开12V电源
    bool spiPinState;       //SPI引脚状态   false:USB spi, true:MCU spi
    bool parkzPinState;     //parkz引脚状态
    bool dlpcmPinState;     //dlpcm引脚状态
*/
    QString runStatusStr;
    runStatusStr.append(QString("DLP status: %1;").arg((runStatus.dlpStatus)?("run"):("stop")));
    runStatusStr.append(QString("MCU temp: %1;DLP temp: %2;").arg(runStatus.mcuTemp).arg(runStatus.dlpTemp));
    runStatusStr.append(QString("MCU temp alarm: %1;DLP temp alarm: %2;").arg((runStatus.mcuTempAlarm)?("warining"):("normal")).arg((runStatus.ledTempAlarm)?("warining"):("normal")));
    runStatusStr.append(QString("12V: %1;").arg((runStatus.power12Vstate)?("close"):("open")));
    runStatusStr.append(QString("SPI pin: %1;").arg((runStatus.dlpStatus)?("usb"):("mcu")));
    runStatusStr.append(QString("Parkz pin: %1;").arg((runStatus.parkzPinState)?("close"):("open")));
    runStatusStr.append(QString("DLPcm pin: %1;").arg((runStatus.dlpcmPinState)?("close"):("open")));

    ui->lineEdit_run_state->setText(runStatusStr);
    ui->pushButton_run_state_get->setEnabled(true);
}

//读取镜像
void MainWidget::on_pushButton_mirror_image_get_clicked()
{
    //失能按钮，防止多次点击
    ui->pushButton_mirror_image_get->setEnabled(false);

    imageMirror_e mirrorImage;
    int ret = m_dlpHandle->SF_getImageMirror(mirrorImage);
    if (ret != DE_SUCCESS){
        ui->pushButton_mirror_image_get->setEnabled(true);
        L_ERROR("get mirror image error!");
        //显示错误信息
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));

        return ;
    }

/*
    IF_MIRROR_NONE,                 //无镜像
    IF_MIRROR_VERTICAL,             //长轴镜像
    IF_MIRROR_HORIZONTAL,           //短轴镜像
    IF_MIRROR_ROTATE                //长轴和短轴镜像
*/
    if (mirrorImage == IF_MIRROR_NONE)
    {
        ui->checkBox_vertical->setChecked(false);
        ui->checkBox_horizontal->setChecked(false);
    } else if (mirrorImage == IF_MIRROR_VERTICAL)
    {
        ui->checkBox_vertical->setChecked(true);
        ui->checkBox_horizontal->setChecked(false);
    } else if (mirrorImage == IF_MIRROR_HORIZONTAL)
    {
        ui->checkBox_vertical->setChecked(false);
        ui->checkBox_horizontal->setChecked(true);
    } else if (mirrorImage == IF_MIRROR_ROTATE)
    {
        ui->checkBox_vertical->setChecked(true);
        ui->checkBox_horizontal->setChecked(true);
    }

    ui->pushButton_mirror_image_get->setEnabled(true);
}

//设置镜像
void MainWidget::on_pushButton_mirror_image_set_clicked()
{
    //失能按钮，防止多次点击
    ui->pushButton_mirror_image_set->setEnabled(false);
    imageMirror_e mirrorImage;
    if (ui->checkBox_vertical->isChecked() && ui->checkBox_horizontal->isChecked())
    {
        mirrorImage = IF_MIRROR_ROTATE;
    } else if (ui->checkBox_vertical->isChecked())
    {
        mirrorImage = IF_MIRROR_VERTICAL;
    } else if (ui->checkBox_horizontal->isChecked())
    {
        mirrorImage = IF_MIRROR_HORIZONTAL;
    } else
    {
        mirrorImage = IF_MIRROR_NONE;
    }
    int ret = m_dlpHandle->SF_setImageMirror(mirrorImage);
    if (ret != DE_SUCCESS){
        
        L_ERROR("set mirror image error!");
        //显示错误信息
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_mirror_image_set->setEnabled(true);
}

void MainWidget::on_radioButton_single_clicked()
{
    //隐藏数量输入框
    ui->label_number->hide();
    ui->lineEdit_number->hide();
    //隐藏间隔输入框
    ui->label_interval->hide();
    ui->lineEdit_interval->hide();
}

void MainWidget::on_radioButton_multiple_clicked()
{
    //显示数量输入框
    ui->label_number->show();
    ui->lineEdit_number->show();
    //显示间隔输入框
    ui->label_interval->show();
    ui->lineEdit_interval->show();
    //设置数量输入框范围
    ui->lineEdit_number->setMaxLength(3);
    ui->lineEdit_number->setPlaceholderText("2-254");
    ui->lineEdit_number->setValidator(new CIntValidator(2, 254, ui->lineEdit_number));
    //设置间隔输入框范围
    ui->lineEdit_interval->setMaxLength(5);
    ui->lineEdit_interval->setPlaceholderText("15-10000");
    ui->lineEdit_interval->setValidator(new CIntValidator(15, 10000, ui->lineEdit_interval));
}

void MainWidget::on_radioButton_unlimited_clicked()
{
    //隐藏数量输入框
    ui->label_number->hide();
    ui->lineEdit_number->hide();
    //显示间隔输入框
    ui->label_interval->show();
    ui->lineEdit_interval->show();
    //设置间隔输入框范围
    ui->lineEdit_interval->setMaxLength(5);
    ui->lineEdit_interval->setPlaceholderText("15-10000");
    ui->lineEdit_interval->setValidator(new CIntValidator(15, 10000, ui->lineEdit_interval));
}

void MainWidget::on_pushButton_soft_trigger_start_clicked()
{
    //失能按钮，防止多次点击
    ui->pushButton_soft_trigger_start->setEnabled(false);

    int number = 0;
    int interval = 0;
    //判断单次或多次或无限
    if (ui->radioButton_single->isChecked()){
        number = 1;
        interval = 0;
    }else if (ui->radioButton_multiple->isChecked()){
        bool ok = false;
        number = ui->lineEdit_number->text().toInt(&ok);
        if (ok == false){
            ui->pushButton_soft_trigger_start->setEnabled(true);
            QMessageBox::warning(this, "Error", QString("Number error!"));
            return ;
        }
        interval = ui->lineEdit_interval->text().toInt(&ok);
        if (ok == false){
            ui->pushButton_soft_trigger_start->setEnabled(true);
            QMessageBox::warning(this, "Error", QString("Interval error!"));
            return ;
        }

        if (number < 2 || number > 254){
            QMessageBox::warning(this, "Error", QString("Number error!"));
            ui->pushButton_soft_trigger_start->setEnabled(true);
            return ;
        }
        if (interval < 15 || interval > 10000){
            QMessageBox::warning(this, "Error", QString("Interval error!"));
            ui->pushButton_soft_trigger_start->setEnabled(true);
            return ;
        }
    }else if (ui->radioButton_unlimited->isChecked()){
        bool ok = false;
        interval = ui->lineEdit_interval->text().toInt(&ok);
        if (ok == false){
            ui->pushButton_soft_trigger_start->setEnabled(true);
            QMessageBox::warning(this, "Error", QString("Interval error!"));
            return ;
        }
        if (interval < 15 || interval > 10000){
            QMessageBox::warning(this, "Error", QString("Interval error!"));
            ui->pushButton_soft_trigger_start->setEnabled(true);
            return ;
        }
        number = 0xFF;
    }else{
        QMessageBox::warning(this, "Error", QString("Please select trigger mode!"));
        ui->pushButton_soft_trigger_start->setEnabled(true);
        return ;
    }

    //发送软触发命令
    int ret = m_dlpHandle->SF_softTrigger(quint8(number & 0xFF), quint16(interval & 0xFFFF));
    if (ret != DE_SUCCESS){
        L_ERROR("soft trigger start error!");
        //显示错误信息
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_soft_trigger_start->setEnabled(true);
}

void MainWidget::on_pushButton_soft_trigger_stop_clicked()
{
    //失能按钮，防止多次点击
    ui->pushButton_soft_trigger_stop->setEnabled(false);
    //发送软触发停止命令
    int ret = m_dlpHandle->SF_softTrigger(0, 0);
    if (ret != DE_SUCCESS){
        L_ERROR("soft trigger stop error!");
        //显示错误信息
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_soft_trigger_stop->setEnabled(true);
}

//startup iic set按钮clicked
void MainWidget::on_pushButton_startup_iic_set_clicked()
{
    //失能按钮，防止多次点击
    ui->pushButton_startup_iic_set->setEnabled(false);
    //获取startup iic index
    bool ok = false;
    quint8 startupIicIndex = quint8(ui->lineEdit_startup_iic_index->text().toUInt(&ok) & 0xFF);
    if (ok == false){
        QMessageBox::warning(this, "Error", QString("Startup IIC index error!"));
        ui->pushButton_startup_iic_set->setEnabled(true);
        return ;
    }
    if (ui->lineEdit_startup_iic_cmd->text().isEmpty()){
        QMessageBox::warning(this, "Error", QString("Startup IIC cmd error!"));
        ui->pushButton_startup_iic_set->setEnabled(true);
        return ;
    }
    //获取startup iic cmd
    int startupIicCmd = ui->lineEdit_startup_iic_cmd->text().toInt(&ok, 16);
    if (ok == false){
        QMessageBox::warning(this, "Error", QString("Startup IIC cmd error!"));
        ui->pushButton_startup_iic_set->setEnabled(true);
        return ;
    }
    //获取startup iic data
    QByteArray startupIicData = strToHex(ui->lineEdit_startup_iic_data->text().trimmed());
    if (startupIicData.length() > APP_STARTUP_IIC_DATA_SIZE)
    {
        QMessageBox::warning(this, "Error", QString("Startup IIC data error!"));
        ui->pushButton_startup_iic_set->setEnabled(true);
        return ;
    }

    //发送startup iic命令
    int ret = m_dlpHandle->SF_setStartIIC(startupIicIndex, quint8(startupIicCmd & 0xFF), startupIicData);
    if (ret != DE_SUCCESS){
        L_ERROR("startup iic set error!");
        //显示错误信息
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_startup_iic_set->setEnabled(true);
}

//startup iic get按钮clicked
void MainWidget::on_pushButton_startup_iic_get_clicked()
{
    //失能按钮，防止多次点击
    ui->pushButton_startup_iic_get->setEnabled(false);
    //获取startup iic index
    bool ok = false;
    quint8 startupIicIndex = quint8(ui->lineEdit_startup_iic_index->text().toUInt(&ok) & 0xFF);
    if (ok == false){
        QMessageBox::warning(this, "Error", QString("Startup IIC index error!"));
        ui->pushButton_startup_iic_get->setEnabled(true);
        return ;
    }
    //发送startup iic命令
    quint8 startupIicCmd = 0;
    QByteArray startupIicData;
    int ret = m_dlpHandle->SF_getStartIIC(startupIicIndex, startupIicCmd, startupIicData);
    if (ret != DE_SUCCESS){
        L_ERROR("startup iic get error!");
        //显示错误信息
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
        ui->pushButton_startup_iic_get->setEnabled(true);
        return ;
    }
    //显示startup iic命令
    ui->lineEdit_startup_iic_cmd->setText(QString::number(startupIicCmd, 16));
    //显示startup iic数据
    ui->lineEdit_startup_iic_data->setText(QString(startupIicData.toHex(' ')));

    ui->pushButton_startup_iic_get->setEnabled(true);
}

//startup iic del按钮clicked
void MainWidget::on_pushButton_startup_iic_del_clicked()
{
    //失能按钮，防止多次点击
    ui->pushButton_startup_iic_del->setEnabled(false);
    //获取startup iic index
    bool ok = false;
    quint8 startupIicIndex = quint8(ui->lineEdit_startup_iic_index->text().toUInt(&ok) & 0xFF);
    if (ok == false){
        QMessageBox::warning(this, "Error", QString("Startup IIC index error!"));
        ui->pushButton_startup_iic_del->setEnabled(true);
        return ;
    }

    //提示用户是否删除
    QMessageBox::StandardButton btn = QMessageBox::information(this, tr("Warning"), tr("Delete startup IIC"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btn == QMessageBox::No){
        ui->pushButton_startup_iic_del->setEnabled(true);
        return ;
    }

    //发送startup iic命令
    int ret = m_dlpHandle->SF_delStartIIC(startupIicIndex);
    if (ret != DE_SUCCESS){
        L_ERROR("startup iic del error!");
        //显示错误信息
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_startup_iic_del->setEnabled(true);
}

//startup iic clear按钮clicked
void MainWidget::on_pushButton_startup_iic_clear_clicked()
{
    //失能按钮，防止多次点击
    ui->pushButton_startup_iic_clear->setEnabled(false);

    //提示用户是否清除
    QMessageBox::StandardButton btn = QMessageBox::information(this, tr("Warning"), tr("Clear startup IIC"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btn == QMessageBox::No){
        ui->pushButton_startup_iic_clear->setEnabled(true);
        return ;
    }

    //发送startup iic命令
    int ret = m_dlpHandle->SF_clrStartIIC();
    if (ret != DE_SUCCESS){
        L_ERROR("startup iic clear error!");
        //显示错误信息
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_startup_iic_clear->setEnabled(true);
}
//设置test pattern
void MainWidget::on_pushButton_test_patterns_set_clicked()
{
    testPattern_e testPattern;
    if (ui->radioButton_checkerboard->isChecked()){
        testPattern = TP_CHECKERBOARD;
    } else if (ui->radioButton_color_ramp->isChecked()){
        testPattern = TP_COLOR_RAMP;
    } else if (ui->radioButton_grid->isChecked()){
        testPattern = TP_GRID;
    } else if (ui->radioButton_white_color->isChecked()){
        testPattern = TP_WHITE_COLOR;
    } else if (ui->radioButton_black_color->isChecked()){
        testPattern = TP_BLACK_COLOR;
    } else {
        QMessageBox::warning(this, "Error", QString("Test pattern error!"));
        return ;
    }
    ui->pushButton_test_patterns_set->setEnabled(false);
    int ret = m_dlpHandle->SF_setTestPattern(testPattern);
    if (ret != DE_SUCCESS){
        L_ERROR("test pattern set error!");
        //显示错误信息
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_test_patterns_set->setEnabled(true);
}

//读取RGB led使能
void MainWidget::on_pushButton_rgb_led_enable_get_clicked()
{
    ui->pushButton_rgb_led_enable_get->setEnabled(false);
    bool redEnable = false, greenEnable = false, blueEnable = false;
    int ret = m_dlpHandle->SF_getRGBEnable(redEnable, greenEnable, blueEnable);
    if (ret != DE_SUCCESS){
        ui->pushButton_rgb_led_enable_get->setEnabled(true);
        L_ERROR("RGB led enable get error!");
        //显示错误信息
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
        return;
    }

    ui->checkBox_red_led_enable->setChecked(redEnable);
    ui->checkBox_green_led_enable->setChecked(greenEnable);
    ui->checkBox_blue_led_enable->setChecked(blueEnable);

    ui->pushButton_rgb_led_enable_get->setEnabled(true);
}
void MainWidget::on_pushButton_rgb_led_enable_set_clicked()
{
    ui->pushButton_rgb_led_enable_set->setEnabled(false);
    bool redEnable = ui->checkBox_red_led_enable->isChecked();
    bool greenEnable = ui->checkBox_green_led_enable->isChecked();
    bool blueEnable = ui->checkBox_blue_led_enable->isChecked();
    int ret = m_dlpHandle->SF_setRGBEnable(redEnable, greenEnable, blueEnable);
    if (ret != DE_SUCCESS){
        L_ERROR("RGB led enable set error!");
        //显示错误信息
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_rgb_led_enable_set->setEnabled(true);
}

void MainWidget::on_pushButton_rgb_led_current_get_clicked()
{
    ui->pushButton_rgb_led_current_get->setEnabled(false);
    quint16 redCurrent = 0, greenCurrent = 0, blueCurrent = 0;
    int ret = m_dlpHandle->SF_getRGBCurrent(redCurrent, greenCurrent, blueCurrent);
    if (ret != DE_SUCCESS){ 
        L_ERROR("RGB led current get error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
        ui->pushButton_rgb_led_current_get->setEnabled(true);
        return;
    }
    ui->lineEdit_red_led_current->setText(QString::number(redCurrent));
    ui->lineEdit_green_led_current->setText(QString::number(greenCurrent));
    ui->lineEdit_blue_led_current->setText(QString::number(blueCurrent));

    ui->pushButton_rgb_led_current_get->setEnabled(true);
}

void MainWidget::on_pushButton_rgb_led_current_set_clicked()
{
    if (ui->lineEdit_red_led_current->text().isEmpty() || ui->lineEdit_green_led_current->text().isEmpty() || ui->lineEdit_blue_led_current->text().isEmpty()){
        L_ERROR("RGB led current set error!");
        QMessageBox::warning(this, "Error", QString("RGB led current set error!"));
        return ;
    }

    ui->pushButton_rgb_led_current_set->setEnabled(false);
    quint16 redCurrent = ui->lineEdit_red_led_current->text().trimmed().toUShort();
    quint16 greenCurrent = ui->lineEdit_green_led_current->text().trimmed().toUShort();
    quint16 blueCurrent = ui->lineEdit_blue_led_current->text().trimmed().toUShort();
    int ret = m_dlpHandle->SF_setRGBCurrent(redCurrent, greenCurrent, blueCurrent);
    if (ret != DE_SUCCESS){
        L_ERROR("RGB led current set error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_rgb_led_current_set->setEnabled(true);
}

void MainWidget::on_pushButton_rgb_led_max_current_get_clicked()
{
    ui->pushButton_rgb_led_max_current_get->setEnabled(false);
    quint16 redMaxCurrent = 0, greenMaxCurrent = 0, blueMaxCurrent = 0;
    int ret = m_dlpHandle->SF_getRGBMaxCurrent(redMaxCurrent, greenMaxCurrent, blueMaxCurrent);
    if (ret != DE_SUCCESS){
        L_ERROR("RGB led max current get error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
        ui->pushButton_rgb_led_max_current_get->setEnabled(true);
        return;
    }
    ui->lineEdit_max_red_led_current->setText(QString::number(redMaxCurrent));
    ui->lineEdit_max_green_led_current->setText(QString::number(greenMaxCurrent));
    ui->lineEdit_max_blue_led_current->setText(QString::number(blueMaxCurrent));

    ui->pushButton_rgb_led_max_current_get->setEnabled(true);
}

void MainWidget::on_pushButton_trigger_pattern_get_clicked()
{
    ui->pushButton_trigger_pattern_get->setEnabled(false);
    readySignal_t readySignal;
    int ret = m_dlpHandle->SF_getTriggerAndPatternSign(readySignal);
    if (ret != DE_SUCCESS){
        L_ERROR("trigger pattern get error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
        ui->pushButton_trigger_pattern_get->setEnabled(true);
        return ;
    }
    ui->checkBox_trigger_out1_enable->setChecked(readySignal.triggerOut1Enable);
    ui->checkBox_trigger_out1_invert->setChecked(readySignal.triggerOut1Invert);
    ui->lineEdit_trigger_out1_delay->setText(QString::number(readySignal.triggerOut1Delay));
    ui->checkBox_trigger_out2_enable->setChecked(readySignal.triggerOut2Enable);
    ui->checkBox_trigger_out2_invert->setChecked(readySignal.triggerOut2Invert);
    ui->lineEdit_trigger_out2_delay->setText(QString::number(readySignal.triggerOut2Delay));
    ui->checkBox_trigger_in_enable->setChecked(readySignal.triggerInEnable);
    ui->comboBox_trigger_in->setCurrentIndex((readySignal.triggerInPolarity)?(1):(0));
    ui->checkBox_pattern_ready_enable->setChecked(readySignal.patternReadyEnable);
    ui->comboBox_pattern_ready->setCurrentIndex((readySignal.patternReadyPolarity)?(1):(0));

    ui->pushButton_trigger_pattern_get->setEnabled(true);
}

void MainWidget::on_pushButton_trigger_pattern_set_clicked()
{
    if (ui->lineEdit_trigger_out1_delay->text().isEmpty() || ui->lineEdit_trigger_out2_delay->text().isEmpty()){
        L_ERROR("trigger pattern set error!");
        QMessageBox::warning(this, "Error", QString("trigger pattern set error!"));
        return ;
    }

    readySignal_t readySignal;
    readySignal.triggerOut1Enable = ui->checkBox_trigger_out1_enable->isChecked();
    readySignal.triggerOut1Invert = ui->checkBox_trigger_out1_invert->isChecked();
    readySignal.triggerOut1Delay = ui->lineEdit_trigger_out1_delay->text().trimmed().toUShort();
    readySignal.triggerOut2Enable = ui->checkBox_trigger_out2_enable->isChecked();
    readySignal.triggerOut2Invert = ui->checkBox_trigger_out2_invert->isChecked();
    readySignal.triggerOut2Delay = ui->lineEdit_trigger_out2_delay->text().trimmed().toUShort();
    readySignal.triggerInEnable = ui->checkBox_trigger_in_enable->isChecked();
    readySignal.triggerInPolarity = (ui->comboBox_trigger_in->currentIndex() == 1)?(true):(false);
    readySignal.patternReadyEnable = ui->checkBox_pattern_ready_enable->isChecked();
    readySignal.patternReadyPolarity = (ui->comboBox_pattern_ready->currentIndex() == 1)?(true):(false);
    int ret = m_dlpHandle->SF_setTriggerAndPatternSign(readySignal);
    if (ret != DE_SUCCESS){
        L_ERROR("trigger pattern set error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_trigger_pattern_set->setEnabled(true);
}

void MainWidget::on_pushButton_display_control_run_once_clicked()
{
    ui->pushButton_display_control_run_once->setEnabled(false);

    int ret = m_dlpHandle->SF_setPatternControl(PC_START, 0);
    if (ret != DE_SUCCESS){
        L_ERROR("display control run once error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }

    ui->pushButton_display_control_run_once->setEnabled(true);
}

void MainWidget::on_pushButton_display_control_run_continue_clicked()
{
    ui->pushButton_display_control_run_continue->setEnabled(false);

    int ret = m_dlpHandle->SF_setPatternControl(PC_START, 0xFF);
    if (ret != DE_SUCCESS){
        L_ERROR("display control run once error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_display_control_pause->setEnabled(true);
    ui->pushButton_display_control_stop->setEnabled(true);
    ui->pushButton_display_control_run_once->setEnabled(false);

    ui->pushButton_display_control_run_continue->setEnabled(true);
}

void MainWidget::on_pushButton_display_control_pause_clicked()
{
    ui->pushButton_display_control_pause->setEnabled(false);

    bool state = ui->pushButton_display_control_pause->property("PauseStatus").toBool();
    if (state){
        state = false;
        ui->pushButton_display_control_pause->setText("Pause");
        ui->pushButton_display_control_step->setEnabled(false);
        ui->pushButton_display_control_restart->setEnabled(false);
        int ret = m_dlpHandle->SF_setPatternControl(PC_RESUME, 0);
        if (ret != DE_SUCCESS){
            L_ERROR("display control pause error!");
            QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
        }
    }else{
        state = true;
        ui->pushButton_display_control_pause->setText("Resume");
        ui->pushButton_display_control_step->setEnabled(true);
        ui->pushButton_display_control_restart->setEnabled(true);
        int ret = m_dlpHandle->SF_setPatternControl(PC_PAUSE, 0);
        if (ret != DE_SUCCESS){
            L_ERROR("display control pause error!");
            QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
        }
    }

    ui->pushButton_display_control_pause->setEnabled(true);
}

void MainWidget::on_pushButton_display_control_step_clicked()
{
    ui->pushButton_display_control_step->setEnabled(false);
    int ret = m_dlpHandle->SF_setPatternControl(PC_STEP, 0);
    if (ret != DE_SUCCESS){
        L_ERROR("display control step error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_display_control_step->setEnabled(true);
}

void MainWidget::on_pushButton_display_control_restart_clicked()
{
    ui->pushButton_display_control_restart->setEnabled(false);
    int ret = m_dlpHandle->SF_setPatternControl(PC_RESET, 0);
    if (ret != DE_SUCCESS){
        L_ERROR("display control restart error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_display_control_run_once->setEnabled(true);
}

void MainWidget::on_pushButton_display_control_stop_clicked()
{
    ui->pushButton_display_control_pause->setEnabled(false);
    ui->pushButton_display_control_step->setEnabled(false);
    ui->pushButton_display_control_restart->setEnabled(false);

    //设置显示
    ui->pushButton_display_control_pause->setText("Pause");
    ui->pushButton_display_control_pause->setProperty("PauseStatus", false);

    int ret = m_dlpHandle->SF_setPatternControl(PC_STOP, 0);
    if (ret != DE_SUCCESS){
        L_ERROR("display control stop error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
}

//iic common
void MainWidget::on_pushButton_iic_common_set_clicked()
{
    ui->pushButton_iic_common_set->setEnabled(false);
    //获取参数
    QByteArray cmdTemp = strToHex(ui->lineEdit_iic_common->text().trimmed());
    QByteArray data = strToHex(ui->lineEdit_iic_data->text().trimmed());
    quint8 cmd = static_cast<quint8>(cmdTemp.at(0) & 0xFF);
    int ret = m_dlpHandle->SF_writeDataToDLP(cmd, data);
    if (ret != DE_SUCCESS){
        L_ERROR("iic common set error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_iic_common_set->setEnabled(true);
}

void MainWidget::on_pushButton_iic_common_get_clicked()
{
    ui->pushButton_iic_common_get->setEnabled(false);
    QByteArray cmdTemp = strToHex(ui->lineEdit_iic_common->text().trimmed());
    if (cmdTemp.length() < 2){
        L_ERROR("iic common get error!");
        QMessageBox::warning(this, "Error", tr("Please input the correct command!"));
        ui->pushButton_iic_common_get->setEnabled(true);
        return ;
    }

    quint8 cmd      = static_cast<quint8>(cmdTemp.at(0));
    quint8 readLed  = static_cast<quint8>(cmdTemp.right(1).at(0));
    QByteArray writeData;
    writeData.append(cmdTemp.mid(1, cmdTemp.length() - 2));

    QByteArray readData;
    int ret = m_dlpHandle->SF_readDataFromDLP(cmd, writeData, readLed, readData);
    if (ret != DE_SUCCESS){
        L_ERROR("iic common get error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
        ui->pushButton_iic_common_get->setEnabled(true);
        return ;
    }
    ui->lineEdit_iic_data->setText(readData.toHex(' '));
    ui->pushButton_iic_common_get->setEnabled(true);
}

void MainWidget::on_pushButton_mcu_reboot_clicked()
{
    ui->pushButton_mcu_reboot->setEnabled(false);
    int ret = m_dlpHandle->SF_resetMCU();
    if (ret != DE_SUCCESS){
        L_ERROR("mcu reboot error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_mcu_reboot->setEnabled(true);
}

void MainWidget::on_pushButton_dlp_reboot_clicked()
{
    ui->pushButton_dlp_reboot->setEnabled(false);
    int ret = m_dlpHandle->SF_setDlpPower(DS_REBOOT);
    if (ret != DE_SUCCESS){
        L_ERROR("dlp reboot error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_dlp_reboot->setEnabled(true);
}

void MainWidget::on_pushButton_factory_reset_clicked()
{
    ui->pushButton_factory_reset->setEnabled(false);
    int ret = m_dlpHandle->SF_resetFactory();
    if (ret != DE_SUCCESS){
        L_ERROR("factory reset error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_factory_reset->setEnabled(true);
}

/*====================================================================================================
 *======================      升级类
 */
void MainWidget::on_pushButton_flash_file_refesh_clicked()
{
    ui->pushButton_flash_file_refesh->setEnabled(false);
    int flashUsed = 0;
    QStringList fileNameList;
    int ret = m_dlpHandle->SF_getFileNameFromFlash(flashUsed, fileNameList);
    if (ret != DE_SUCCESS){
        L_ERROR("get file name from flash error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }

    ui->progressBar_flash_space->setValue(flashUsed);

    ui->tableWidget_flash_file->clearContents();
    ui->tableWidget_flash_file->setRowCount(0);

    for (const auto &fileName : fileNameList){
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignCenter);
        item->setText(fileName);
        ui->tableWidget_flash_file->insertRow(ui->tableWidget_flash_file->rowCount());
        ui->tableWidget_flash_file->setItem(ui->tableWidget_flash_file->rowCount() - 1, 0, item);
    }
    ui->pushButton_flash_file_refesh->setEnabled(true);

    selectPatternTimeFile(QString());
}

void MainWidget::on_pushButton_flash_file_add_clicked()
{
    ui->pushButton_flash_file_add->setEnabled(false);

    QStringList ptFiles;
    for (int row = 0; row < ui->tableWidget_flash_file->rowCount(); row++){
        QString fileName = ui->tableWidget_flash_file->item(row, 0)->text();
        //打印fileName
        if (fileName.endsWith(".pt")){
            ptFiles.append(fileName);
        }
    }

    L_INFO(QString("ptFiles: %1").arg(ptFiles.join(",").toStdString().c_str()));

    QString fileName;
    for (int count = 0; count < DEF_PATT_TIME_MAX_NUM; count++){
        QString tempName = QString("%1.pt").arg(count);
        if (!ptFiles.contains(tempName)){
            fileName = tempName;
            break;
        }
    }
    if (fileName.isEmpty()){
        QMessageBox::warning(this, "Error", "Please add a pattern file.");
        ui->pushButton_flash_file_add->setEnabled(true);
        return ;
    }
    int row = ui->tableWidget_flash_file->rowCount();
    ui->tableWidget_flash_file->insertRow(row);
    QTableWidgetItem *item = new QTableWidgetItem();
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(fileName);
    ui->tableWidget_flash_file->setItem(row, 0, item);
    ui->pushButton_flash_file_add->setEnabled(true);
}

void MainWidget::on_pushButton_flash_file_sub_clicked()
{
    auto selected = ui->tableWidget_flash_file->selectionModel()->selectedRows();
    if (selected.isEmpty()){
        QMessageBox::warning(this, "Error", "Please select a row to delete.");
        return;
    }
    ui->pushButton_flash_file_sub->setEnabled(false);
    QStringList fileNameList;
    for (const auto &index : selected)
    {
        fileNameList.append(ui->tableWidget_flash_file->item(index.row(), 0)->text());
    }
    int ret = m_dlpHandle->SF_delFileFromFlash(fileNameList);
    if (ret != DE_SUCCESS){
        L_ERROR("delete file from flash error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    on_pushButton_flash_file_refesh_clicked();
    ui->pushButton_flash_file_sub->setEnabled(true);
}

void MainWidget::on_pushButton_flash_file_clear_clicked()
{
    ui->pushButton_flash_file_clear->setEnabled(false);
    int ret = m_dlpHandle->SF_clrAllFileFromFlash();
    if (ret != DE_SUCCESS){
        L_ERROR("clear flash error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    on_pushButton_flash_file_refesh_clicked();
    ui->pushButton_flash_file_clear->setEnabled(true);
}

//-------------------------MCU 升级
void MainWidget::on_pushButton_generate_patt_bin_clicked()
{
    ui->pushButton_generate_patt_bin->setEnabled(false);

    std::vector<INT_PAT_PatternSet_t> patternSets;
    std::vector<INT_PAT_PatternOrderTableEntry_t> patternOrderTableEntries;

    QByteArray patternData;
    createPatternSetAndOrder(patternSets, patternOrderTableEntries);
    int ret = m_dlpHandle->SF_generatePatternData(patternSets, patternOrderTableEntries, true, true, patternData);
    releasePatternData(patternSets, patternOrderTableEntries);

    if (ret != DE_SUCCESS){
        ui->pushButton_generate_patt_bin->setEnabled(true);
        L_ERROR("generate pattern data error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));

        return ;
    }

    //这个位置处理需要比较久的时间
    m_countDownDialog->setInfor(tr("Initiating the transfer, please wait!"));
    m_countDownDialog->setStartTime(2 * 60);
    m_countDownDialog->startCount();
    ret = m_dlpHandle->SF_updatePBtoDLPfromAPP(patternData);
    if (ret != DE_SUCCESS){
        L_ERROR("update pattern data to DLP error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    m_countDownDialog->stopCount();

    ui->pushButton_generate_patt_bin->setEnabled(true);
}

void MainWidget::on_pushButton_mcu_update_clicked()
{
    //提示用户是否需要升级MCU程序
    QMessageBox::StandardButton btn = QMessageBox::information(this, tr("Warning"), tr("Update MCU"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btn == QMessageBox::No){
        return ;
    }
    //选择mcu升级文件，bin文件
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select MCU updata file"), "", "file(*.bin)");
    if (fileName.isEmpty()){
        //提示用户选择文件
        QMessageBox::warning(this, "Error", "Please select a file.");
        return ;
    }
    ui->pushButton_mcu_update->setEnabled(false);
    ui->progressBar_mcu_update->setValue(0);

    connect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_mcu_update, &QProgressBar::setValue, Qt::UniqueConnection);
    int ret = m_dlpHandle->SF_updateMCUProgram(fileName);
    disconnect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_mcu_update, &QProgressBar::setValue);
    if (ret != DE_SUCCESS){
        L_ERROR("update mcu error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_mcu_update->setEnabled(true);
}

//---------------------DLP 升级或获取
void MainWidget::on_pushButton_get_dlp_from_dlp_clicked()
{
    ui->pushButton_get_dlp_from_dlp->setEnabled(false);
    //提示需要保存的文件名
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"), "", "file (*.img)");
    if (fileName.isNull()){
        L_ERROR("Please select file name");
        return ;
    }

    ui->progressBar_get_dlp_from_dlp->setValue(0);

    connect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_get_dlp_from_dlp, &QProgressBar::setValue, Qt::UniqueConnection);

    QByteArray dlpPack;
    int ret = m_dlpHandle->SF_getDLPProgramFromDLP(dlpPack);
    if (ret == DE_SUCCESS){
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
            file.write(dlpPack);
            file.close();
        }
    }else{
        L_ERROR("Get dlp program error!");
    }

    disconnect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_get_dlp_from_dlp, &QProgressBar::setValue);

    ui->pushButton_get_dlp_from_dlp->setEnabled(true);
}

void MainWidget::on_pushButton_dlp_update_clicked()
{
    //提示用户是否升级DLP程序
    QMessageBox::StandardButton btn = QMessageBox::information(this, tr("Warning"), tr("Update DLP"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btn == QMessageBox::No){
        return ;
    }
    ui->pushButton_dlp_update->setEnabled(false);
    if (ui->radioButton_load_dlp_to_dlp_from_flash->isChecked())
    {
        ui->progressBar_dlp_update->setValue(0);
        connect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_dlp_update, &QProgressBar::setValue, Qt::UniqueConnection);
        int ret = m_dlpHandle->SF_updateDLPFromFlash();
        disconnect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_dlp_update, &QProgressBar::setValue);
        if (ret != DE_SUCCESS){
            L_ERROR("update dlp error!");
            QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
        }
        ui->pushButton_dlp_update->setEnabled(true);
        return ;
    }

    //选择dlp升级文件，img文件
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select DLP updata file"), "", "file(*.img)");
    if (fileName.isEmpty()){
        //提示用户选择文件
        QMessageBox::warning(this, "Error", "Please select a file.");
        ui->pushButton_dlp_update->setEnabled(true);
        return ;
    }
    updateDir_e updateDir = UD_TO_FLASH;
    if (ui->radioButton_load_dlp_to_dlp->isChecked()){
        updateDir = UD_TO_DLP;
    }
    ui->progressBar_dlp_update->setValue(0);
    connect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_dlp_update, &QProgressBar::setValue, Qt::UniqueConnection);
    int ret = m_dlpHandle->SF_updateDLPProgram(updateDir, fileName);
    disconnect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_dlp_update, &QProgressBar::setValue);
    if (ret != DE_SUCCESS){
        L_ERROR("update dlp error!");
        QMessageBox::warning(this, "Error", QString("Error code : 0x%1 ").arg(ret & 0xFFFF, 4, 16, QLatin1Char('0')));
    }
    ui->pushButton_dlp_update->setEnabled(true);
}

//pattern time
void MainWidget::on_pushButton_pattern_time_clicked()
{
    //提示用户是否需要升级
    QMessageBox::StandardButton ret = QMessageBox::question(this, "Update", "Do you want to update dlp?", QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::No){
        return;
    }

    ui->pushButton_pattern_time->setEnabled(false);
    if (ui->radioButton_load_pt_to_flash->isChecked()){
        updatePTfileToFlash();
    } else if (ui->radioButton_load_pt_to_dlp->isChecked()) {
        updatePTfileToDlp();
    } else if (ui->radioButton_load_pt_to_dlp_from_flash->isChecked()) {
        updatePTfileToDlpFromFlash();
    } else if (ui->radioButton_get_pt_from_flash->isChecked()) {
        getPTfileFromFlash();
    } else if (ui->radioButton_get_pt_from_dlp->isChecked()){
        getPTfileFromDlp();
    }

    ui->pushButton_pattern_time->setEnabled(true);
}

//pattern bin
void MainWidget::on_pushButton_pattern_bin_clicked()
{
    //提示用户是否需要升级
    QMessageBox::StandardButton ret = QMessageBox::question(this, "Update", "Do you want to update dlp?", QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::No){
        return;
    }

    ui->pushButton_pattern_bin->setEnabled(false);
    if (ui->radioButton_load_pb_to_dlp->isChecked()){
        updatePBfileToDlp();
    } else if (ui->radioButton_get_pb_from_dlp->isChecked()) {
        getPBfileFromDlp();
    }
    ui->pushButton_pattern_bin->setEnabled(true);
}

void MainWidget::selectPatternTimeFile(const QString &fileName)
{
    if (fileName.isEmpty()){
        ui->radioButton_load_pt_to_flash->setEnabled(false);
        ui->radioButton_load_pt_to_dlp_from_flash->setEnabled(false);
        ui->radioButton_get_pt_from_flash->setEnabled(false);
        ui->lineEdit_pt_file_name->setText("");
        ui->radioButton_load_pt_to_dlp->click();
        return ;
    }
    ui->lineEdit_pt_file_name->setText(fileName);
    ui->radioButton_load_pt_to_flash->setEnabled(true);
    ui->radioButton_load_pt_to_dlp_from_flash->setEnabled(true);
    ui->radioButton_get_pt_from_flash->setEnabled(true);
}

void MainWidget::updatePTfileToFlash()
{
    //打开pt文件
    QString ptFile = QFileDialog::getOpenFileName(this, "Open pt file", ".", "*.pt *.img *.bin");
    if (ptFile.isEmpty()){
        //打印提示信息
        L_ERROR("Open pt file canceled.");
        return;
    }
    //读取文件数据
    QFile file(ptFile);
    if (!file.open(QIODevice::ReadOnly)){
        //打印提示信息
        L_ERROR("Open pt file failed.");
        return;
    }
    QByteArray ptData = file.readAll();
    file.close();
    if (ptData.isEmpty()){
        L_ERROR("Read pt file failed.");
        return;
    }
    ui->progressBar_pattern_time->setValue(0);
    connect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_pattern_time, &QProgressBar::setValue, Qt::UniqueConnection);
    int ret = m_dlpHandle->SF_updatePTtoFlashFromAPP(ui->lineEdit_pt_file_name->text(), ptData);
    disconnect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_pattern_time, &QProgressBar::setValue);
    if (ret != DE_SUCCESS)
    {
        LS_ERROR("Update PT to flash failed.");
        QMessageBox::warning(this, "Error", "Update PT to flash failed.");
    }
}

void MainWidget::updatePTfileToDlp()
{
    //打开pt文件
    QString ptFile = QFileDialog::getOpenFileName(this, "Open pt file", ".", "*.pt *.img *.bin");
    if (ptFile.isEmpty()){
        //打印提示信息
        L_ERROR("Open pt file canceled.");
        return;
    }
    //读取文件数据
    QFile file(ptFile);
    if (!file.open(QIODevice::ReadOnly)){
        //打印提示信息
        L_ERROR("Open pt file failed.");
        return;
    }
    QByteArray ptData = file.readAll();
    file.close();
    if (ptData.isEmpty()){
        L_ERROR("Read pt file failed.");
        return;
    }
    ui->progressBar_pattern_time->setValue(0);
    connect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_pattern_time, &QProgressBar::setValue, Qt::UniqueConnection);
    int ret = m_dlpHandle->SF_updatePTtoDLPfromAPP(ptData);
    disconnect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_pattern_time, &QProgressBar::setValue);
    if (ret != DE_SUCCESS)
    {
        L_ERROR("Update pt to dlp failed.");
        QMessageBox::warning(this, "Error", "Update pt to dlp failed.");
    }
}

void MainWidget::updatePTfileToDlpFromFlash()
{
    if (ui->lineEdit_pt_file_name->text().isEmpty()){
        return ;
    }

    QString fileName = ui->lineEdit_pt_file_name->text().trimmed();
    ui->progressBar_pattern_time->setValue(0);
    connect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_pattern_time, &QProgressBar::setValue, Qt::UniqueConnection);
    int ret = m_dlpHandle->SF_loadPTtoDLPfromFlash(ui->checkBox_pt_save->isChecked(), fileName);
    disconnect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_pattern_time, &QProgressBar::setValue);
    if (ret != DE_SUCCESS)
    {
        L_ERROR("Update pt to dlp failed.");
        QMessageBox::warning(this, "Error", "Update pt to dlp failed.");
    }
}

void MainWidget::getPTfileFromFlash()
{
    QString ptName = ui->lineEdit_pt_file_name->text().trimmed();
    if (ptName.isEmpty()){
        L_ERROR("Please input pt file name.");
        QMessageBox::warning(this, "Error", "Please input pt file name.");
        return ;
    }

    QString saveName = QFileDialog::getSaveFileName(this, tr("Select DLP updata file"), "", "file(*.pt)");
    if (saveName.isEmpty()){
        L_ERROR("Select DLP updata file failed.");
        QMessageBox::warning(this, "Error", "Select DLP updata file failed.");
        return ;
    }

    QByteArray ptData;
    ui->progressBar_pattern_time->setValue(0);
    connect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_pattern_time, &QProgressBar::setValue, Qt::UniqueConnection);
    int ret = m_dlpHandle->SF_getPTtoAPPFromFlash(ptName, ptData);
    disconnect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_pattern_time, &QProgressBar::setValue);
    if (ret != DE_SUCCESS){
        L_ERROR(QString("SF_getPTtoAPPFromFlash failed with error code %1").arg(ret));
        QMessageBox::warning(this, tr("Warning"), tr("Get PT file from DLP failed!"));
        return;
    }
    if (ptData.isEmpty()){
        L_ERROR(QString("SF_getPTtoAPPFromFlash failed with error code %1").arg(ret));
        QMessageBox::warning(this, tr("Warning"), tr("Get PT file from DLP failed!"));
        return;
    }

    QFile file(saveName);
    if (!file.open(QIODevice::WriteOnly)){
        L_ERROR(QString("SF_getPTtoAPPFromFlash failed with error code %1").arg(ret));
        QMessageBox::warning(this, tr("Warning"), tr("Get PT file from DLP failed!"));
        return;
    }
    file.write(ptData);
    file.close();
}

void MainWidget::getPTfileFromDlp()
{
    QString saveName = QFileDialog::getSaveFileName(this, tr("Select DLP updata file"), "", "file(*.pt)");
    if (saveName.isEmpty()){
        L_ERROR("Select DLP updata file failed.");
        QMessageBox::warning(this, "Error", "Select DLP updata file failed.");
        return ;
    }

    QByteArray ptData;
    ui->progressBar_pattern_time->setValue(0);
    connect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_pattern_time, &QProgressBar::setValue, Qt::UniqueConnection);
    int ret = m_dlpHandle->SF_getPTtoAPPfromDLP(ptData);
    disconnect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_pattern_time, &QProgressBar::setValue);
    if (ret != DE_SUCCESS)
    {
        LS_ERROR(tr("Error: %1").arg(ret));
        QMessageBox::warning(this, tr("Error"), tr("Error: %1").arg(ret));
        return ;
    }
    if (ptData.isEmpty())
    {
        LS_ERROR(tr("Error: No data received"));
        QMessageBox::warning(this, tr("Error"), tr("Error: No data received"));
        return ;
    }
    QFile file(saveName);
    if (!file.open(QIODevice::WriteOnly))
    {
        LS_ERROR(tr("Error: Can't open file %1").arg(file.fileName()));
        QMessageBox::warning(this, tr("Error"), tr("Error: Can't open file %1").arg(file.fileName()));
        return; 
    }
    file.write(ptData);
    file.close();
}

void MainWidget::updatePBfileToDlp(void)
{
//打开pt文件
    QString pbFile = QFileDialog::getOpenFileName(this, "Open pt file", ".", "*.pb *.img");
    if (pbFile.isEmpty()){
        //打印提示信息
        L_ERROR("Open pb file canceled.");
        return;
    }
    //读取文件数据
    QFile file(pbFile);
    if (!file.open(QIODevice::ReadOnly)){
        //打印提示信息
        L_ERROR("Open pb file failed.");
        return;
    }
    QByteArray pbData = file.readAll();
    file.close();
    if (pbData.isEmpty()){
        L_ERROR("Read pb file failed.");
        return;
    }
    ui->progressBar_pattern_bin->setValue(0);
    connect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_pattern_bin, &QProgressBar::setValue, Qt::UniqueConnection);
    int ret = m_dlpHandle->SF_updatePBtoDLPfromAPP(pbData);
    disconnect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_pattern_bin, &QProgressBar::setValue);
    if (ret != DE_SUCCESS)
    {
        L_ERROR("Error in SF_updatePBtoDLPfromAPP");
        QMessageBox::warning(this, "Error", "Error in SF_updatePBtoDLPfromAPP");
    }
}

void MainWidget::getPBfileFromDlp(void)
{
    QString saveName = QFileDialog::getSaveFileName(this, tr("Select DLP updata file"), "", "file(*.pb)");
    if (saveName.isEmpty()){
        L_ERROR("Select DLP updata file failed.");
        QMessageBox::warning(this, "Error", "Select DLP updata file failed.");
        return ;
    }
    QByteArray pbData;
    ui->progressBar_pattern_bin->setValue(0);
    connect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_pattern_bin, &QProgressBar::setValue, Qt::UniqueConnection);
    int ret = m_dlpHandle->SF_getPBtoAPPfromDLP(pbData);
    disconnect(m_dlpHandle, &DlpHandle::SF_transmitProgressSign, ui->progressBar_pattern_bin, &QProgressBar::setValue);
    if (ret != DE_SUCCESS)
    {
        L_ERROR("SF_getPBtoAPPfromDLP failed");
        QMessageBox::warning(this, "Error", "SF_getPBtoAPPfromDLP failed");
        return;
    }
    if (pbData.isEmpty()){
        L_ERROR("SF_getPBtoAPPfromDLP returned empty data");
        QMessageBox::warning(this, "Error", "SF_getPBtoAPPfromDLP returned empty data");
        return;
    }
    QFile file(saveName);
    if (!file.open(QIODevice::WriteOnly)){
        L_ERROR("Failed to open file for writing");
        QMessageBox::warning(this, "Error", "Failed to open file for writing");
        return;
    }
    file.write(pbData);
    file.close();
}

#define MAX_WIDTH                           1920
#define MAX_HEIGHT                          1080
#define NUM_ONE_BIT_HORIZONTAL_PATTERNS     4
#define NUM_ONE_BIT_VERTICAL_PATTERNS       4
#define NUM_EIGHT_BIT_HORIZONTAL_PATTERNS   4
#define NUM_EIGHT_BIT_VERTICAL_PATTERNS     4

bool MainWidget::createPatternSetAndOrder(std::vector<INT_PAT_PatternSet_t> & patternSets, std::vector<INT_PAT_PatternOrderTableEntry_t> & patternOrderTableEntries)
{
    uint16_t NumBars;

    INT_PAT_PatternSet_t pattSet;
    pattSet.BitDepth = INT_PAT_BITDEPTH_ONE;
    pattSet.Direction = INT_PAT_DIRECTION_HORIZONTAL;
    pattSet.PatternCount = NUM_ONE_BIT_HORIZONTAL_PATTERNS;
    pattSet.PatternArray = new INT_PAT_PatternData_t[NUM_ONE_BIT_HORIZONTAL_PATTERNS];
    for (int index = 0; index < NUM_ONE_BIT_HORIZONTAL_PATTERNS; index++){
        NumBars = static_cast<uint16_t>(2 * (index + 1));
        uint8_t * pixelArray = new uint8_t[MAX_HEIGHT];
        populateOneBitPatternData(MAX_HEIGHT, pixelArray, NumBars);
        pattSet.PatternArray[index].PixelArray      = pixelArray;
        pattSet.PatternArray[index].PixelArrayCount = MAX_HEIGHT;
    }
    patternSets.push_back(pattSet);

    pattSet.BitDepth = INT_PAT_BITDEPTH_ONE;
    pattSet.Direction = INT_PAT_DIRECTION_VERTICAL;
    pattSet.PatternCount = NUM_ONE_BIT_VERTICAL_PATTERNS;
    pattSet.PatternArray = new INT_PAT_PatternData_t[NUM_ONE_BIT_VERTICAL_PATTERNS];
    for (int index = 0; index < NUM_ONE_BIT_VERTICAL_PATTERNS; index++){
        NumBars = static_cast<uint16_t>(2 * (index + 1));
        uint8_t * pixelArray = new uint8_t[MAX_WIDTH];
        populateOneBitPatternData(MAX_WIDTH, pixelArray, NumBars);
        pattSet.PatternArray[index].PixelArray      = pixelArray;
        pattSet.PatternArray[index].PixelArrayCount = MAX_WIDTH;
    }
    patternSets.push_back(pattSet);

    pattSet.BitDepth = INT_PAT_BITDEPTH_EIGHT;
    pattSet.Direction = INT_PAT_DIRECTION_HORIZONTAL;
    pattSet.PatternCount = NUM_EIGHT_BIT_HORIZONTAL_PATTERNS;
    pattSet.PatternArray = new INT_PAT_PatternData_t[NUM_EIGHT_BIT_HORIZONTAL_PATTERNS];
    for (int index = 0; index < NUM_EIGHT_BIT_HORIZONTAL_PATTERNS; index++){
        NumBars = static_cast<uint16_t>(2 * (index + 1));
        uint8_t * pixelArray = new uint8_t[MAX_HEIGHT];
        populateEightBitPatternData(MAX_HEIGHT, pixelArray, NumBars);
        pattSet.PatternArray[index].PixelArray      = pixelArray;
        pattSet.PatternArray[index].PixelArrayCount = MAX_HEIGHT;
    }
    patternSets.push_back(pattSet);

    pattSet.BitDepth = INT_PAT_BITDEPTH_EIGHT;
    pattSet.Direction = INT_PAT_DIRECTION_VERTICAL;
    pattSet.PatternCount = NUM_EIGHT_BIT_VERTICAL_PATTERNS;
    pattSet.PatternArray = new INT_PAT_PatternData_t[NUM_EIGHT_BIT_VERTICAL_PATTERNS];
    for (int index = 0; index < NUM_EIGHT_BIT_VERTICAL_PATTERNS; index++){
        NumBars = static_cast<uint16_t>(2 * (index + 1));
        uint8_t * pixelArray = new uint8_t[MAX_WIDTH];
        populateEightBitPatternData(MAX_WIDTH, pixelArray, NumBars);
        pattSet.PatternArray[index].PixelArray      = pixelArray;
        pattSet.PatternArray[index].PixelArrayCount = MAX_WIDTH;
    }
    patternSets.push_back(pattSet);


    uint8_t PatternSetIdx        = 0;
    INT_PAT_PatternOrderTableEntry_t pattOrderTableEntry;

    pattOrderTableEntry.PatternSetIndex                        = PatternSetIdx;
    pattOrderTableEntry.NumDisplayPatterns                     = static_cast<uint8_t>(patternSets[PatternSetIdx++].PatternCount);
    pattOrderTableEntry.IlluminationSelect                     = INT_PAT_ILLUMINATION_RED;
    pattOrderTableEntry.InvertPatterns                         = false;
    pattOrderTableEntry.IlluminationTimeInMicroseconds         = 5000;
    pattOrderTableEntry.PreIlluminationDarkTimeInMicroseconds  = 250;
    pattOrderTableEntry.PostIlluminationDarkTimeInMicroseconds = 1000;
    patternOrderTableEntries.push_back(pattOrderTableEntry);

    pattOrderTableEntry.PatternSetIndex                        = PatternSetIdx;
    pattOrderTableEntry.NumDisplayPatterns                     = static_cast<uint8_t>(patternSets[PatternSetIdx++].PatternCount);
    pattOrderTableEntry.IlluminationSelect                     = INT_PAT_ILLUMINATION_GREEN;
    pattOrderTableEntry.InvertPatterns                         = false;
    pattOrderTableEntry.IlluminationTimeInMicroseconds         = 5000;
    pattOrderTableEntry.PreIlluminationDarkTimeInMicroseconds  = 250;
    pattOrderTableEntry.PostIlluminationDarkTimeInMicroseconds = 1000;
    patternOrderTableEntries.push_back(pattOrderTableEntry);

    pattOrderTableEntry.PatternSetIndex                        = PatternSetIdx;
    pattOrderTableEntry.NumDisplayPatterns                     = static_cast<uint8_t>(patternSets[PatternSetIdx++].PatternCount);
    pattOrderTableEntry.IlluminationSelect                     = INT_PAT_ILLUMINATION_BLUE;
    pattOrderTableEntry.InvertPatterns                         = false;
    pattOrderTableEntry.IlluminationTimeInMicroseconds         = 5000;
    pattOrderTableEntry.PreIlluminationDarkTimeInMicroseconds  = 250;
    pattOrderTableEntry.PostIlluminationDarkTimeInMicroseconds = 1000;
    patternOrderTableEntries.push_back(pattOrderTableEntry);

    pattOrderTableEntry.PatternSetIndex                        = PatternSetIdx;
    pattOrderTableEntry.NumDisplayPatterns                     = static_cast<uint8_t>(patternSets[PatternSetIdx++].PatternCount);
    pattOrderTableEntry.IlluminationSelect                     = INT_PAT_ILLUMINATION_RGB;
    pattOrderTableEntry.InvertPatterns                         = false;
    pattOrderTableEntry.IlluminationTimeInMicroseconds         = 11000;
    pattOrderTableEntry.PreIlluminationDarkTimeInMicroseconds  = 250;
    pattOrderTableEntry.PostIlluminationDarkTimeInMicroseconds = 1000;
    patternOrderTableEntries.push_back(pattOrderTableEntry);

    return true;
}

void MainWidget::releasePatternData(std::vector<INT_PAT_PatternSet_t> & patternSets, std::vector<INT_PAT_PatternOrderTableEntry_t> & patternOrderTableEntries)
{
    for (auto &pattSet : patternSets)
    {
        if (pattSet.PatternArray != nullptr)
        {
            for (uint32_t index = 0; index < pattSet.PatternCount; index++)
            {
                if (pattSet.PatternArray[index].PixelArray != nullptr)
                {
                    delete [] pattSet.PatternArray[index].PixelArray;
                    pattSet.PatternArray[index].PixelArray      = nullptr;
                    pattSet.PatternArray[index].PixelArrayCount = 0;
                }
            }
            delete [] pattSet.PatternArray;
            pattSet.PatternArray = nullptr;
        }
        pattSet.PatternCount = 0;
    }
    patternSets.clear();
    patternOrderTableEntries.clear();
}

void MainWidget::populateOneBitPatternData(uint16_t Length, uint8_t* Data, uint16_t NumBars)
{
    uint16_t PixelPos  = 0;
    uint16_t BarPos    = 0;
    uint16_t BarWidth  = Length / NumBars;
    uint8_t  PixelData = 0;

    for (; PixelPos < Length; PixelPos++)
    {
        Data[PixelPos] = PixelData;

        BarPos++;
        if (BarPos >= BarWidth)
        {
            BarPos = 0;
            PixelData = (PixelData == 0 ? 1 : 0);
        }
    }
}

void MainWidget::populateEightBitPatternData(uint16_t Length, uint8_t* Data, uint16_t NumBars)
{
    uint16_t PixelPos     = 0;
    uint16_t BarPos       = 0;
    uint16_t BarWidth     = Length / (2 * NumBars);
    uint8_t  PixelData    = 0;
    int16_t  PixelDataInc = static_cast<int16_t>(ceil(255.0 / BarWidth));

    for (; PixelPos < Length; PixelPos++)
    {
        Data[PixelPos] = PixelData;

        BarPos++;
        if (BarPos >= BarWidth)
        {
            BarPos    = 0;
            PixelDataInc = -PixelDataInc;
        }

        PixelData = static_cast<uint8_t>(PixelData + PixelDataInc);
    }
}
