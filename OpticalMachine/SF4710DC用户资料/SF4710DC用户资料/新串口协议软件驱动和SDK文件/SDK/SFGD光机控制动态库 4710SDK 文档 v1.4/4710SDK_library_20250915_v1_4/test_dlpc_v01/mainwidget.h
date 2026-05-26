#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QSerialPortInfo>

#include "dlphandle.h"
#include "countdowndialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWidget; }
QT_END_NAMESPACE

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private:
    void layoutInit(void);
    void refreshComboxCom(void);
    QByteArray strToHex(const QString &str);

    static bool comparePortNames(const QSerialPortInfo &port1, const QSerialPortInfo &port2);

private slots:
    void on_pushButton_com_connect_clicked();
    void on_comboBox_com_currentIndexChanged(int index);
    void on_pushButton_version_get_clicked();
    void on_pushButton_operation_mode_get_clicked();
    void on_pushButton_color_temp_get_clicked();
    void on_pushButton_color_temp_set_clicked();
    void on_radioButton_single_clicked();
    void on_radioButton_multiple_clicked();
    void on_radioButton_unlimited_clicked();
    void on_pushButton_soft_trigger_start_clicked();
    void on_pushButton_soft_trigger_stop_clicked();
    //startup iic set按钮clicked
    void on_pushButton_startup_iic_set_clicked();
    //startup iic get按钮clicked
    void on_pushButton_startup_iic_get_clicked();
    //startup iic del按钮clicked
    void on_pushButton_startup_iic_del_clicked();
    //startup iic clear按钮clicked
    void on_pushButton_startup_iic_clear_clicked();
    //读取运行状态
    void on_pushButton_run_state_get_clicked();
    //读取镜像
    void on_pushButton_mirror_image_get_clicked();
    //设置镜像
    void on_pushButton_mirror_image_set_clicked();
    //设置test pattern
    void on_pushButton_test_patterns_set_clicked();
    //设置RGB led使能
    void on_pushButton_rgb_led_enable_get_clicked();
    void on_pushButton_rgb_led_enable_set_clicked();
    void on_pushButton_rgb_led_current_get_clicked();
    void on_pushButton_rgb_led_current_set_clicked();
    void on_pushButton_rgb_led_max_current_get_clicked();
    void on_pushButton_trigger_pattern_get_clicked();
    void on_pushButton_trigger_pattern_set_clicked();
    void on_pushButton_display_control_run_once_clicked();
    void on_pushButton_display_control_run_continue_clicked();
    void on_pushButton_display_control_pause_clicked();
    void on_pushButton_display_control_step_clicked();
    void on_pushButton_display_control_restart_clicked();
    void on_pushButton_display_control_stop_clicked();
    void on_pushButton_iic_common_set_clicked();
    void on_pushButton_iic_common_get_clicked();
    void on_pushButton_mcu_reboot_clicked();
    void on_pushButton_dlp_reboot_clicked();
    void on_pushButton_factory_reset_clicked();

    void on_pushButton_flash_file_refesh_clicked();
    void on_pushButton_flash_file_add_clicked();
    void on_pushButton_flash_file_sub_clicked();
    void on_pushButton_flash_file_clear_clicked();

    //Generate pattern data
    void on_pushButton_generate_patt_bin_clicked();
    //MCU update
    void on_pushButton_mcu_update_clicked();
    //DLP update
    void on_pushButton_get_dlp_from_dlp_clicked();
    void on_pushButton_dlp_update_clicked();
    //pattern time
    void on_pushButton_pattern_time_clicked();
    //pattern bin
    void on_pushButton_pattern_bin_clicked();

private:
    //没有选择pt文件
    void selectPatternTimeFile(const QString &fileName);
    //升级pt文件到flash中
    void updatePTfileToFlash(void);
    //升级pt文件到dlp中
    void updatePTfileToDlp(void);
    //升级pt文件到dlp中从flash中
    void updatePTfileToDlpFromFlash(void);
    //获取pt文件从flash中
    void getPTfileFromFlash(void);
    void getPTfileFromDlp(void);

    //升级pb文件到dlp中
    void updatePBfileToDlp(void);
    //获取pb文件从dlp中
    void getPBfileFromDlp(void);

    //生成pattern数据
    bool createPatternSetAndOrder(std::vector<INT_PAT_PatternSet_t> & patternSets, std::vector<INT_PAT_PatternOrderTableEntry_t> & patternOrderTableEntries);
    //释放pattern数据
    void releasePatternData(std::vector<INT_PAT_PatternSet_t> & patternSets, std::vector<INT_PAT_PatternOrderTableEntry_t> & patternOrderTableEntries);
    void populateOneBitPatternData(uint16_t Length, uint8_t* Data, uint16_t NumBars);
    void populateEightBitPatternData(uint16_t Length, uint8_t* Data, uint16_t NumBars);

private:
    Ui::MainWidget *ui;

    DlpHandle * m_dlpHandle = nullptr;
    CountDownDialog * m_countDownDialog = nullptr;
};
#endif // MAINWIDGET_H
