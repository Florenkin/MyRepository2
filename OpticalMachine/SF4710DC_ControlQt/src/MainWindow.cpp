#include "MainWindow.h"

#include <algorithm>
#include <cmath>
#include <functional>

#include <QAbstractItemView>
#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QModelIndex>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScrollArea>
#include <QSerialPortInfo>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>

namespace
{
const int kStartupIicMaxBytes = 64;
const int kIicMaxBytes = 64;
const int kPatternWidth = 1920;
const int kPatternHeight = 1080;
const int kOneBitHorizontalPatterns = 4;
const int kOneBitVerticalPatterns = 4;
const int kEightBitHorizontalPatterns = 4;
const int kEightBitVerticalPatterns = 4;

QString onOffText(bool value)
{
    return value ? QStringLiteral("开") : QStringLiteral("关");
}

QString normalAlarmText(bool value)
{
    return value ? QStringLiteral("告警") : QStringLiteral("正常");
}

template <typename T>
T enumAt(const QComboBox *combo)
{
    return static_cast<T>(combo->currentData().toInt());
}

void addLine(QVBoxLayout *layout)
{
    QLabel *line = new QLabel;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_device(new DeviceController(this))
{
    setWindowTitle(QStringLiteral("SF4710DC 光机控制"));
    resize(1180, 780);

    QWidget *root = new QWidget(this);
    QVBoxLayout *rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(10);
    rootLayout->addWidget(buildConnectionBar());

    QTabWidget *tabs = new QTabWidget(root);
    tabs->addTab(buildOperationPage(), QStringLiteral("操作"));
    tabs->addTab(buildAdvancedPage(), QStringLiteral("高级"));
    tabs->addTab(buildLogPage(), QStringLiteral("日志"));
    rootLayout->addWidget(tabs, 1);

    setCentralWidget(root);

    connect(m_device, &DeviceController::connectionChanged, this, &MainWindow::updateConnectedUi);
    connect(m_device, &DeviceController::logLine, this, &MainWindow::appendLog);
    connect(m_device, &DeviceController::sdkError, this, [this](const QString &message) {
        appendLog(QStringLiteral("[SDK] %1").arg(message));
        QMessageBox::warning(this, QStringLiteral("设备错误"), message);
    });

    refreshPorts();
    updateConnectedUi(false);
}

QWidget *MainWindow::buildConnectionBar()
{
    QGroupBox *box = new QGroupBox(QStringLiteral("连接"), this);
    QHBoxLayout *layout = new QHBoxLayout(box);

    m_portCombo = new QComboBox(box);
    m_refreshPortsButton = button(QStringLiteral("刷新"));
    m_connectButton = button(QStringLiteral("连接"));
    m_autoDetectButton = button(QStringLiteral("自动查找"));
    m_connectionStatus = statusLabel(QStringLiteral("未连接"));

    layout->addWidget(new QLabel(QStringLiteral("串口:"), box));
    layout->addWidget(m_portCombo, 1);
    layout->addWidget(m_refreshPortsButton);
    layout->addWidget(m_autoDetectButton);
    layout->addWidget(m_connectButton);
    layout->addSpacing(12);
    layout->addWidget(m_connectionStatus);

    connect(m_refreshPortsButton, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    connect(m_autoDetectButton, &QPushButton::clicked, this, &MainWindow::autoDetectDevice);
    connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::toggleConnection);

    return box;
}

QWidget *MainWindow::buildOperationPage()
{
    QScrollArea *area = new QScrollArea(this);
    area->setWidgetResizable(true);

    QWidget *page = new QWidget(area);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(10);

    QHBoxLayout *top = new QHBoxLayout();
    top->addWidget(buildInfoGroup(), 1);
    top->addWidget(buildPowerModeGroup(), 1);
    layout->addLayout(top);

    QHBoxLayout *mid = new QHBoxLayout();
    mid->addWidget(buildSoftTriggerGroup(), 1);
    mid->addWidget(buildImageGroup(), 1);
    mid->addWidget(buildRgbGroup(), 1);
    layout->addLayout(mid);

    QHBoxLayout *low = new QHBoxLayout();
    low->addWidget(buildTriggerGroup(), 2);
    low->addWidget(buildPatternControlGroup(), 1);
    layout->addLayout(low);

    layout->addWidget(buildIicGroup());
    layout->addStretch();

    area->setWidget(page);
    return area;
}

QWidget *MainWindow::buildAdvancedPage()
{
    QScrollArea *area = new QScrollArea(this);
    area->setWidgetResizable(true);

    QWidget *page = new QWidget(area);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(10);

    QHBoxLayout *top = new QHBoxLayout();
    top->addWidget(buildFlashGroup(), 1);
    top->addWidget(buildFirmwareGroup(), 1);
    layout->addLayout(top);
    layout->addWidget(buildPatternFileGroup());
    layout->addWidget(buildDangerGroup());
    layout->addStretch();

    area->setWidget(page);
    return area;
}

QWidget *MainWindow::buildLogPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);

    m_logEdit = new QPlainTextEdit(page);
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumBlockCount(5000);
    layout->addWidget(m_logEdit, 1);

    QHBoxLayout *buttons = new QHBoxLayout();
    QPushButton *clear = button(QStringLiteral("清空日志"));
    QPushButton *save = button(QStringLiteral("保存日志"));
    buttons->addStretch();
    buttons->addWidget(clear);
    buttons->addWidget(save);
    layout->addLayout(buttons);

    connect(clear, &QPushButton::clicked, m_logEdit, &QPlainTextEdit::clear);
    connect(save, &QPushButton::clicked, this, [this]() {
        const QString fileName = selectSaveFile(QStringLiteral("保存日志"),
                                                QStringLiteral("日志文件 (*.log);;文本文件 (*.txt);;所有文件 (*.*)"));
        if (!fileName.isEmpty()) {
            writeFile(fileName, m_logEdit->toPlainText().toUtf8());
        }
    });

    return page;
}

QGroupBox *MainWindow::buildInfoGroup()
{
    QGroupBox *box = new QGroupBox(QStringLiteral("设备状态"), this);
    QVBoxLayout *layout = new QVBoxLayout(box);

    QFormLayout *form = new QFormLayout();
    m_versionLabel = statusLabel(QStringLiteral("-"));
    m_libraryLabel = statusLabel(QStringLiteral("-"));
    m_modeLabel = statusLabel(QStringLiteral("-"));
    form->addRow(QStringLiteral("MCU 版本:"), m_versionLabel);
    form->addRow(QStringLiteral("SDK 版本:"), m_libraryLabel);
    form->addRow(QStringLiteral("当前模式:"), m_modeLabel);
    layout->addLayout(form);

    m_statusText = new QTextEdit(box);
    m_statusText->setReadOnly(true);
    m_statusText->setMinimumHeight(96);
    layout->addWidget(m_statusText);

    QHBoxLayout *buttons = new QHBoxLayout();
    QPushButton *getVersion = button(QStringLiteral("读取版本"));
    QPushButton *getLibrary = button(QStringLiteral("读取 SDK"));
    QPushButton *checkReady = button(QStringLiteral("检测就绪"));
    QPushButton *getMode = button(QStringLiteral("读取模式"));
    QPushButton *getStatus = button(QStringLiteral("读取状态"));
    buttons->addWidget(getVersion);
    buttons->addWidget(getLibrary);
    buttons->addWidget(checkReady);
    buttons->addWidget(getMode);
    buttons->addWidget(getStatus);
    layout->addLayout(buttons);

    connect(getVersion, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        QString version;
        const int ret = m_device->handle()->SF_getVersion(version);
        if (ret == DE_SUCCESS) m_versionLabel->setText(version);
        showResult(QStringLiteral("读取 MCU 版本"), ret);
    });
    connect(getLibrary, &QPushButton::clicked, this, [this]() {
        int mainVer = 0;
        int subVer = 0;
        const int ret = m_device->handle()->SF_getLibraryVersion(mainVer, subVer);
        if (ret == DE_SUCCESS) m_libraryLabel->setText(QStringLiteral("%1.%2").arg(mainVer).arg(subVer));
        showResult(QStringLiteral("读取 SDK 版本"), ret);
    });
    connect(checkReady, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        showResult(QStringLiteral("检测设备就绪"), m_device->handle()->SF_deviceCheckReady());
    });
    connect(getMode, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        operatMode_e mode;
        QString modeText;
        const int ret = m_device->handle()->SF_getOperationMode(mode, modeText);
        if (ret == DE_SUCCESS) m_modeLabel->setText(modeText);
        showResult(QStringLiteral("读取操作模式"), ret);
    });
    connect(getStatus, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        runStatus_t status;
        const int ret = m_device->handle()->SF_getRunStatus(status);
        if (ret == DE_SUCCESS) m_statusText->setText(statusText(status));
        showResult(QStringLiteral("读取运行状态"), ret);
    });

    return box;
}

QGroupBox *MainWindow::buildPowerModeGroup()
{
    QGroupBox *box = new QGroupBox(QStringLiteral("电源与模式"), this);
    QFormLayout *form = new QFormLayout(box);

    m_powerCombo = new QComboBox(box);
    m_powerCombo->addItem(QStringLiteral("启动 DLP"), DS_START);
    m_powerCombo->addItem(QStringLiteral("关闭 DLP"), DS_STOP);
    m_powerCombo->addItem(QStringLiteral("重启 DLP"), DS_REBOOT);
    QPushButton *applyPower = button(QStringLiteral("执行"));

    m_operationModeCombo = new QComboBox(box);
    m_operationModeCombo->addItem(QStringLiteral("外部视频"), OM_EXTERNAL_VIDEO_PORT);
    m_operationModeCombo->addItem(QStringLiteral("测试图案"), OM_TEST_PATTERN_GENERATOR);
    m_operationModeCombo->addItem(QStringLiteral("Splash 画面"), OM_SPLASH_SCREEN);
    m_operationModeCombo->addItem(QStringLiteral("外部图案流"), OM_SENS_EXTERNAL_PATTERN);
    m_operationModeCombo->addItem(QStringLiteral("内部图案流"), OM_SENS_INTERNAL_PATTERN);
    m_operationModeCombo->addItem(QStringLiteral("Splash 图案流"), OM_SENS_SPLASH_PATTERN);
    QPushButton *applyMode = button(QStringLiteral("设置模式"));

    m_colorTempSpin = spinBox(0, 255, 128);
    QPushButton *getColor = button(QStringLiteral("读取"));
    QPushButton *setColor = button(QStringLiteral("设置"));

    m_splashCombo = new QComboBox(box);
    for (int i = 0; i <= 31; ++i) {
        m_splashCombo->addItem(QStringLiteral("画面 %1").arg(i), i);
    }
    QPushButton *setSplash = button(QStringLiteral("设置画面"));
    QPushButton *externalVideo = button(QStringLiteral("切到外部视频"));

    QHBoxLayout *powerLayout = new QHBoxLayout();
    powerLayout->addWidget(m_powerCombo);
    powerLayout->addWidget(applyPower);
    form->addRow(QStringLiteral("DLP 电源:"), powerLayout);

    QHBoxLayout *modeLayout = new QHBoxLayout();
    modeLayout->addWidget(m_operationModeCombo);
    modeLayout->addWidget(applyMode);
    form->addRow(QStringLiteral("操作模式:"), modeLayout);

    QHBoxLayout *colorLayout = new QHBoxLayout();
    colorLayout->addWidget(m_colorTempSpin);
    colorLayout->addWidget(getColor);
    colorLayout->addWidget(setColor);
    form->addRow(QStringLiteral("色温:"), colorLayout);

    QHBoxLayout *splashLayout = new QHBoxLayout();
    splashLayout->addWidget(m_splashCombo);
    splashLayout->addWidget(setSplash);
    splashLayout->addWidget(externalVideo);
    form->addRow(QStringLiteral("Splash / 视频:"), splashLayout);

    connect(applyPower, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        showResult(QStringLiteral("设置 DLP 电源"), m_device->handle()->SF_setDlpPower(enumAt<dlpStatus_e>(m_powerCombo)));
    });
    connect(applyMode, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        showResult(QStringLiteral("设置操作模式"), m_device->handle()->SF_setOperationMode(enumAt<operatMode_e>(m_operationModeCombo)));
    });
    connect(getColor, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        quint8 color = 0;
        const int ret = m_device->handle()->SF_getColorTemp(color);
        if (ret == DE_SUCCESS) m_colorTempSpin->setValue(color);
        showResult(QStringLiteral("读取色温"), ret);
    });
    connect(setColor, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        showResult(QStringLiteral("设置色温"),
                   m_device->handle()->SF_setColorTemp(static_cast<quint8>(m_colorTempSpin->value())));
    });
    connect(setSplash, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        showResult(QStringLiteral("设置 Splash 画面"),
                   m_device->handle()->SF_setSplashImages(static_cast<quint8>(m_splashCombo->currentData().toInt())));
    });
    connect(externalVideo, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        showResult(QStringLiteral("切换外部视频"), m_device->handle()->SF_switchExternalVideo());
    });

    return box;
}

QGroupBox *MainWindow::buildSoftTriggerGroup()
{
    QGroupBox *box = new QGroupBox(QStringLiteral("软触发"), this);
    QVBoxLayout *layout = new QVBoxLayout(box);

    m_triggerSingle = new QRadioButton(QStringLiteral("单次"), box);
    m_triggerMultiple = new QRadioButton(QStringLiteral("多次"), box);
    m_triggerUnlimited = new QRadioButton(QStringLiteral("不限次数"), box);
    m_triggerSingle->setChecked(true);

    QHBoxLayout *modeLayout = new QHBoxLayout();
    modeLayout->addWidget(m_triggerSingle);
    modeLayout->addWidget(m_triggerMultiple);
    modeLayout->addWidget(m_triggerUnlimited);
    layout->addLayout(modeLayout);

    QFormLayout *form = new QFormLayout();
    m_triggerCountSpin = spinBox(2, 254, 2);
    m_triggerIntervalSpin = spinBox(15, 10000, 15, QStringLiteral(" ms"));
    form->addRow(QStringLiteral("次数:"), m_triggerCountSpin);
    form->addRow(QStringLiteral("间隔:"), m_triggerIntervalSpin);
    layout->addLayout(form);

    QHBoxLayout *buttons = new QHBoxLayout();
    QPushButton *start = button(QStringLiteral("启动"));
    QPushButton *stop = button(QStringLiteral("停止"));
    buttons->addWidget(start);
    buttons->addWidget(stop);
    layout->addLayout(buttons);

    auto refresh = [this]() {
        m_triggerCountSpin->setEnabled(m_triggerMultiple->isChecked());
        m_triggerIntervalSpin->setEnabled(!m_triggerSingle->isChecked());
    };
    connect(m_triggerSingle, &QRadioButton::toggled, this, refresh);
    connect(m_triggerMultiple, &QRadioButton::toggled, this, refresh);
    connect(m_triggerUnlimited, &QRadioButton::toggled, this, refresh);
    refresh();

    connect(start, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        quint8 count = 1;
        quint16 interval = 0;
        if (m_triggerMultiple->isChecked()) {
            count = static_cast<quint8>(m_triggerCountSpin->value());
            interval = static_cast<quint16>(m_triggerIntervalSpin->value());
        } else if (m_triggerUnlimited->isChecked()) {
            count = 0xff;
            interval = static_cast<quint16>(m_triggerIntervalSpin->value());
        }
        showResult(QStringLiteral("启动软触发"), m_device->handle()->SF_softTrigger(count, interval));
    });
    connect(stop, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        showResult(QStringLiteral("停止软触发"), m_device->handle()->SF_softTrigger(0, 0));
    });

    return box;
}

QGroupBox *MainWindow::buildImageGroup()
{
    QGroupBox *box = new QGroupBox(QStringLiteral("图像与测试图案"), this);
    QFormLayout *form = new QFormLayout(box);

    m_mirrorVertical = new QCheckBox(QStringLiteral("长轴镜像"), box);
    m_mirrorHorizontal = new QCheckBox(QStringLiteral("短轴镜像"), box);
    QPushButton *getMirror = button(QStringLiteral("读取镜像"));
    QPushButton *setMirror = button(QStringLiteral("设置镜像"));

    QVBoxLayout *mirrorChecks = new QVBoxLayout();
    mirrorChecks->addWidget(m_mirrorVertical);
    mirrorChecks->addWidget(m_mirrorHorizontal);
    QHBoxLayout *mirrorButtons = new QHBoxLayout();
    mirrorButtons->addWidget(getMirror);
    mirrorButtons->addWidget(setMirror);
    mirrorChecks->addLayout(mirrorButtons);
    form->addRow(QStringLiteral("镜像:"), mirrorChecks);

    m_testPatternCombo = new QComboBox(box);
    m_testPatternCombo->addItem(QStringLiteral("棋盘"), TP_CHECKERBOARD);
    m_testPatternCombo->addItem(QStringLiteral("白图"), TP_WHITE_COLOR);
    m_testPatternCombo->addItem(QStringLiteral("黑图"), TP_BLACK_COLOR);
    m_testPatternCombo->addItem(QStringLiteral("网格"), TP_GRID);
    m_testPatternCombo->addItem(QStringLiteral("色阶"), TP_COLOR_RAMP);
    QPushButton *setPattern = button(QStringLiteral("设置图案"));

    QHBoxLayout *patternLayout = new QHBoxLayout();
    patternLayout->addWidget(m_testPatternCombo);
    patternLayout->addWidget(setPattern);
    form->addRow(QStringLiteral("测试图案:"), patternLayout);

    connect(getMirror, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        imageMirror_e mirror;
        const int ret = m_device->handle()->SF_getImageMirror(mirror);
        if (ret == DE_SUCCESS) {
            m_mirrorVertical->setChecked(mirror == IF_MIRROR_VERTICAL || mirror == IF_MIRROR_ROTATE);
            m_mirrorHorizontal->setChecked(mirror == IF_MIRROR_HORIZONTAL || mirror == IF_MIRROR_ROTATE);
        }
        showResult(QStringLiteral("读取镜像"), ret);
    });
    connect(setMirror, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        imageMirror_e mirror = IF_MIRROR_NONE;
        if (m_mirrorVertical->isChecked() && m_mirrorHorizontal->isChecked()) mirror = IF_MIRROR_ROTATE;
        else if (m_mirrorVertical->isChecked()) mirror = IF_MIRROR_VERTICAL;
        else if (m_mirrorHorizontal->isChecked()) mirror = IF_MIRROR_HORIZONTAL;
        showResult(QStringLiteral("设置镜像"), m_device->handle()->SF_setImageMirror(mirror));
    });
    connect(setPattern, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        showResult(QStringLiteral("设置测试图案"), m_device->handle()->SF_setTestPattern(enumAt<testPattern_e>(m_testPatternCombo)));
    });

    return box;
}

QGroupBox *MainWindow::buildRgbGroup()
{
    QGroupBox *box = new QGroupBox(QStringLiteral("RGB 灯"), this);
    QFormLayout *form = new QFormLayout(box);

    m_redEnable = new QCheckBox(QStringLiteral("红色"), box);
    m_greenEnable = new QCheckBox(QStringLiteral("绿色"), box);
    m_blueEnable = new QCheckBox(QStringLiteral("蓝色"), box);
    QHBoxLayout *enableLayout = new QHBoxLayout();
    enableLayout->addWidget(m_redEnable);
    enableLayout->addWidget(m_greenEnable);
    enableLayout->addWidget(m_blueEnable);
    QPushButton *getEnable = button(QStringLiteral("读取"));
    QPushButton *setEnable = button(QStringLiteral("设置"));
    enableLayout->addWidget(getEnable);
    enableLayout->addWidget(setEnable);
    form->addRow(QStringLiteral("使能:"), enableLayout);

    m_redCurrent = spinBox(0, 2000, 100, QStringLiteral(" mA"));
    m_greenCurrent = spinBox(0, 2000, 100, QStringLiteral(" mA"));
    m_blueCurrent = spinBox(0, 2000, 100, QStringLiteral(" mA"));
    QGridLayout *currentGrid = new QGridLayout();
    currentGrid->addWidget(new QLabel(QStringLiteral("红"), box), 0, 0);
    currentGrid->addWidget(m_redCurrent, 0, 1);
    currentGrid->addWidget(new QLabel(QStringLiteral("绿"), box), 1, 0);
    currentGrid->addWidget(m_greenCurrent, 1, 1);
    currentGrid->addWidget(new QLabel(QStringLiteral("蓝"), box), 2, 0);
    currentGrid->addWidget(m_blueCurrent, 2, 1);
    QPushButton *getCurrent = button(QStringLiteral("读取电流"));
    QPushButton *setCurrent = button(QStringLiteral("设置电流"));
    QPushButton *getMax = button(QStringLiteral("读取最大值"));
    QHBoxLayout *currentButtons = new QHBoxLayout();
    currentButtons->addWidget(getCurrent);
    currentButtons->addWidget(setCurrent);
    currentButtons->addWidget(getMax);
    currentGrid->addLayout(currentButtons, 3, 0, 1, 2);
    form->addRow(QStringLiteral("电流:"), currentGrid);

    m_rgbMaxLabel = statusLabel(QStringLiteral("-"));
    form->addRow(QStringLiteral("最大电流:"), m_rgbMaxLabel);

    connect(getEnable, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        bool red = false;
        bool green = false;
        bool blue = false;
        const int ret = m_device->handle()->SF_getRGBEnable(red, green, blue);
        if (ret == DE_SUCCESS) {
            m_redEnable->setChecked(red);
            m_greenEnable->setChecked(green);
            m_blueEnable->setChecked(blue);
        }
        showResult(QStringLiteral("读取 RGB 使能"), ret);
    });
    connect(setEnable, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        showResult(QStringLiteral("设置 RGB 使能"),
                   m_device->handle()->SF_setRGBEnable(m_redEnable->isChecked(),
                                                       m_greenEnable->isChecked(),
                                                       m_blueEnable->isChecked()));
    });
    connect(getCurrent, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        quint16 red = 0;
        quint16 green = 0;
        quint16 blue = 0;
        const int ret = m_device->handle()->SF_getRGBCurrent(red, green, blue);
        if (ret == DE_SUCCESS) {
            m_redCurrent->setValue(red);
            m_greenCurrent->setValue(green);
            m_blueCurrent->setValue(blue);
        }
        showResult(QStringLiteral("读取 RGB 电流"), ret);
    });
    connect(setCurrent, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        showResult(QStringLiteral("设置 RGB 电流"),
                   m_device->handle()->SF_setRGBCurrent(static_cast<quint16>(m_redCurrent->value()),
                                                        static_cast<quint16>(m_greenCurrent->value()),
                                                        static_cast<quint16>(m_blueCurrent->value())));
    });
    connect(getMax, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        quint16 red = 0;
        quint16 green = 0;
        quint16 blue = 0;
        const int ret = m_device->handle()->SF_getRGBMaxCurrent(red, green, blue);
        if (ret == DE_SUCCESS) {
            m_rgbMaxLabel->setText(QStringLiteral("红 %1 mA / 绿 %2 mA / 蓝 %3 mA").arg(red).arg(green).arg(blue));
            m_redCurrent->setMaximum(red);
            m_greenCurrent->setMaximum(green);
            m_blueCurrent->setMaximum(blue);
        }
        showResult(QStringLiteral("读取 RGB 最大电流"), ret);
    });

    return box;
}

QGroupBox *MainWindow::buildTriggerGroup()
{
    QGroupBox *box = new QGroupBox(QStringLiteral("触发与就绪信号"), this);
    QGridLayout *grid = new QGridLayout(box);

    m_triggerOut1Enable = new QCheckBox(QStringLiteral("输出1使能"), box);
    m_triggerOut1Invert = new QCheckBox(QStringLiteral("输出1反相"), box);
    m_triggerOut1Delay = spinBox(-999, 999, 0, QStringLiteral(" ms"));
    m_triggerOut2Enable = new QCheckBox(QStringLiteral("输出2使能"), box);
    m_triggerOut2Invert = new QCheckBox(QStringLiteral("输出2反相"), box);
    m_triggerOut2Delay = spinBox(-999, 999, 0, QStringLiteral(" ms"));
    m_triggerInEnable = new QCheckBox(QStringLiteral("输入使能"), box);
    m_triggerInPolarity = comboBox(QStringList() << QStringLiteral("低电平有效") << QStringLiteral("高电平有效"));
    m_patternReadyEnable = new QCheckBox(QStringLiteral("图案就绪使能"), box);
    m_patternReadyPolarity = comboBox(QStringList() << QStringLiteral("低电平有效") << QStringLiteral("高电平有效"));

    grid->addWidget(m_triggerOut1Enable, 0, 0);
    grid->addWidget(m_triggerOut1Invert, 0, 1);
    grid->addWidget(new QLabel(QStringLiteral("输出1延时:"), box), 0, 2);
    grid->addWidget(m_triggerOut1Delay, 0, 3);
    grid->addWidget(m_triggerOut2Enable, 1, 0);
    grid->addWidget(m_triggerOut2Invert, 1, 1);
    grid->addWidget(new QLabel(QStringLiteral("输出2延时:"), box), 1, 2);
    grid->addWidget(m_triggerOut2Delay, 1, 3);
    grid->addWidget(m_triggerInEnable, 2, 0);
    grid->addWidget(new QLabel(QStringLiteral("输入极性:"), box), 2, 1);
    grid->addWidget(m_triggerInPolarity, 2, 2, 1, 2);
    grid->addWidget(m_patternReadyEnable, 3, 0);
    grid->addWidget(new QLabel(QStringLiteral("就绪极性:"), box), 3, 1);
    grid->addWidget(m_patternReadyPolarity, 3, 2, 1, 2);

    QPushButton *read = button(QStringLiteral("读取参数"));
    QPushButton *write = button(QStringLiteral("设置参数"));
    QPushButton *readCount = button(QStringLiteral("读取触发计数"));
    QPushButton *clearCount = button(QStringLiteral("清零触发计数"));
    QPushButton *sequenceError = button(QStringLiteral("检查序列错误"));

    grid->addWidget(read, 4, 0);
    grid->addWidget(write, 4, 1);
    grid->addWidget(readCount, 4, 2);
    grid->addWidget(clearCount, 4, 3);
    grid->addWidget(sequenceError, 5, 0, 1, 2);

    connect(read, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        readySignal_t signal;
        const int ret = m_device->handle()->SF_getTriggerAndPatternSign(signal);
        if (ret == DE_SUCCESS) {
            m_triggerOut1Enable->setChecked(signal.triggerOut1Enable);
            m_triggerOut1Invert->setChecked(signal.triggerOut1Invert);
            m_triggerOut1Delay->setValue(signal.triggerOut1Delay);
            m_triggerOut2Enable->setChecked(signal.triggerOut2Enable);
            m_triggerOut2Invert->setChecked(signal.triggerOut2Invert);
            m_triggerOut2Delay->setValue(signal.triggerOut2Delay);
            m_triggerInEnable->setChecked(signal.triggerInEnable);
            m_triggerInPolarity->setCurrentIndex(signal.triggerInPolarity ? 1 : 0);
            m_patternReadyEnable->setChecked(signal.patternReadyEnable);
            m_patternReadyPolarity->setCurrentIndex(signal.patternReadyPolarity ? 1 : 0);
        }
        showResult(QStringLiteral("读取触发参数"), ret);
    });
    connect(write, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        readySignal_t signal;
        signal.triggerOut1Enable = m_triggerOut1Enable->isChecked();
        signal.triggerOut1Invert = m_triggerOut1Invert->isChecked();
        signal.triggerOut1Delay = m_triggerOut1Delay->value();
        signal.triggerOut2Enable = m_triggerOut2Enable->isChecked();
        signal.triggerOut2Invert = m_triggerOut2Invert->isChecked();
        signal.triggerOut2Delay = m_triggerOut2Delay->value();
        signal.triggerInEnable = m_triggerInEnable->isChecked();
        signal.triggerInPolarity = m_triggerInPolarity->currentIndex() == 1;
        signal.patternReadyEnable = m_patternReadyEnable->isChecked();
        signal.patternReadyPolarity = m_patternReadyPolarity->currentIndex() == 1;
        showResult(QStringLiteral("设置触发参数"), m_device->handle()->SF_setTriggerAndPatternSign(signal));
    });
    connect(readCount, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        quint32 count = 0;
        const int ret = m_device->handle()->SF_getTriggerCount(count);
        if (ret == DE_SUCCESS) QMessageBox::information(this, QStringLiteral("触发计数"), QStringLiteral("当前计数: %1").arg(count));
        showResult(QStringLiteral("读取触发计数"), ret);
    });
    connect(clearCount, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        showResult(QStringLiteral("清零触发计数"), m_device->handle()->SF_clrTriggerCount());
    });
    connect(sequenceError, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        bool error = false;
        const int ret = m_device->handle()->SF_getSequenceError(error);
        if (ret == DE_SUCCESS) QMessageBox::information(this, QStringLiteral("序列状态"), error ? QStringLiteral("存在序列错误") : QStringLiteral("序列正常"));
        showResult(QStringLiteral("检查序列错误"), ret);
    });

    return box;
}

QGroupBox *MainWindow::buildPatternControlGroup()
{
    QGroupBox *box = new QGroupBox(QStringLiteral("图案控制"), this);
    QFormLayout *form = new QFormLayout(box);

    QComboBox *control = new QComboBox(box);
    control->addItem(QStringLiteral("开始"), PC_START);
    control->addItem(QStringLiteral("停止"), PC_STOP);
    control->addItem(QStringLiteral("暂停"), PC_PAUSE);
    control->addItem(QStringLiteral("单步"), PC_STEP);
    control->addItem(QStringLiteral("继续"), PC_RESUME);
    control->addItem(QStringLiteral("重新开始"), PC_RESET);
    QSpinBox *repeat = spinBox(0, 255, 0);
    QPushButton *apply = button(QStringLiteral("执行"));

    form->addRow(QStringLiteral("动作:"), control);
    form->addRow(QStringLiteral("重复次数:"), repeat);
    form->addRow(apply);

    connect(apply, &QPushButton::clicked, this, [this, control, repeat]() {
        if (!ensureConnected()) return;
        showResult(QStringLiteral("执行图案控制"),
                   m_device->handle()->SF_setPatternControl(enumAt<patternCtl_e>(control),
                                                            static_cast<quint8>(repeat->value())));
    });

    return box;
}

QGroupBox *MainWindow::buildIicGroup()
{
    QGroupBox *box = new QGroupBox(QStringLiteral("IIC 命令"), this);
    QVBoxLayout *layout = new QVBoxLayout(box);

    QGroupBox *startup = new QGroupBox(QStringLiteral("开机自动 IIC"), box);
    QGridLayout *startupGrid = new QGridLayout(startup);
    m_startIicIndex = spinBox(0, 15, 0);
    m_startIicCmd = lineEdit(QStringLiteral("命令字节，例如 12"));
    m_startIicData = lineEdit(QStringLiteral("数据，十六进制空格分隔，例如 01 02 03"));
    m_startIicCmd->setMaxLength(2);
    m_startIicData->setMaxLength(kStartupIicMaxBytes * 3);
    QPushButton *setStart = button(QStringLiteral("写入"));
    QPushButton *getStart = button(QStringLiteral("读取"));
    QPushButton *delStart = button(QStringLiteral("删除"));
    QPushButton *clearStart = button(QStringLiteral("清空全部"));

    startupGrid->addWidget(new QLabel(QStringLiteral("索引:"), startup), 0, 0);
    startupGrid->addWidget(m_startIicIndex, 0, 1);
    startupGrid->addWidget(new QLabel(QStringLiteral("命令:"), startup), 0, 2);
    startupGrid->addWidget(m_startIicCmd, 0, 3);
    startupGrid->addWidget(new QLabel(QStringLiteral("数据:"), startup), 1, 0);
    startupGrid->addWidget(m_startIicData, 1, 1, 1, 3);
    startupGrid->addWidget(setStart, 2, 0);
    startupGrid->addWidget(getStart, 2, 1);
    startupGrid->addWidget(delStart, 2, 2);
    startupGrid->addWidget(clearStart, 2, 3);
    layout->addWidget(startup);

    QGroupBox *direct = new QGroupBox(QStringLiteral("直接读写 DLP"), box);
    QGridLayout *directGrid = new QGridLayout(direct);
    m_iicCmd = lineEdit(QStringLiteral("命令字节，例如 12"));
    m_iicReadLength = spinBox(0, kIicMaxBytes, 0);
    m_iicWriteData = lineEdit(QStringLiteral("写入数据，十六进制空格分隔"));
    m_iicReadData = lineEdit();
    m_iicReadData->setReadOnly(true);
    QPushButton *writeDlp = button(QStringLiteral("写入 DLP"));
    QPushButton *readDlp = button(QStringLiteral("读取 DLP"));

    directGrid->addWidget(new QLabel(QStringLiteral("命令:"), direct), 0, 0);
    directGrid->addWidget(m_iicCmd, 0, 1);
    directGrid->addWidget(new QLabel(QStringLiteral("读取长度:"), direct), 0, 2);
    directGrid->addWidget(m_iicReadLength, 0, 3);
    directGrid->addWidget(new QLabel(QStringLiteral("写入数据:"), direct), 1, 0);
    directGrid->addWidget(m_iicWriteData, 1, 1, 1, 3);
    directGrid->addWidget(new QLabel(QStringLiteral("读取结果:"), direct), 2, 0);
    directGrid->addWidget(m_iicReadData, 2, 1, 1, 3);
    directGrid->addWidget(writeDlp, 3, 0);
    directGrid->addWidget(readDlp, 3, 1);
    layout->addWidget(direct);

    connect(setStart, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        bool cmdOk = false;
        bool dataOk = false;
        const int cmd = m_startIicCmd->text().trimmed().toInt(&cmdOk, 16);
        const QByteArray data = hexStringToBytes(m_startIicData->text(), &dataOk);
        if (!cmdOk || cmd < 0 || cmd > 0xff || !dataOk || data.size() > kStartupIicMaxBytes) {
            QMessageBox::warning(this, QStringLiteral("参数错误"), QStringLiteral("请输入合法的十六进制命令和数据。"));
            return;
        }
        showResult(QStringLiteral("写入开机 IIC"),
                   m_device->handle()->SF_setStartIIC(m_startIicIndex->value(), static_cast<quint8>(cmd), data));
    });
    connect(getStart, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        quint8 cmd = 0;
        QByteArray data;
        const int ret = m_device->handle()->SF_getStartIIC(m_startIicIndex->value(), cmd, data);
        if (ret == DE_SUCCESS) {
            m_startIicCmd->setText(QStringLiteral("%1").arg(cmd, 2, 16, QLatin1Char('0')).toUpper());
            m_startIicData->setText(DeviceController::hex(data).toUpper());
        }
        showResult(QStringLiteral("读取开机 IIC"), ret);
    });
    connect(delStart, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        showResult(QStringLiteral("删除开机 IIC"), m_device->handle()->SF_delStartIIC(m_startIicIndex->value()));
    });
    connect(clearStart, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        if (!confirm(QStringLiteral("清空开机 IIC"), QStringLiteral("确定清空全部开机自动 IIC 命令吗？"))) return;
        showResult(QStringLiteral("清空开机 IIC"), m_device->handle()->SF_clrStartIIC());
    });
    connect(writeDlp, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        bool cmdOk = false;
        bool dataOk = false;
        const int cmd = m_iicCmd->text().trimmed().toInt(&cmdOk, 16);
        const QByteArray data = hexStringToBytes(m_iicWriteData->text(), &dataOk);
        if (!cmdOk || cmd < 0 || cmd > 0xff || !dataOk || data.size() > kIicMaxBytes) {
            QMessageBox::warning(this, QStringLiteral("参数错误"), QStringLiteral("请输入合法的十六进制命令和数据。"));
            return;
        }
        showResult(QStringLiteral("写入 DLP 数据"), m_device->handle()->SF_writeDataToDLP(static_cast<quint8>(cmd), data));
    });
    connect(readDlp, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        bool cmdOk = false;
        bool dataOk = false;
        const int cmd = m_iicCmd->text().trimmed().toInt(&cmdOk, 16);
        const QByteArray writeData = hexStringToBytes(m_iicWriteData->text(), &dataOk);
        if (!cmdOk || cmd < 0 || cmd > 0xff || !dataOk || writeData.size() > kIicMaxBytes) {
            QMessageBox::warning(this, QStringLiteral("参数错误"), QStringLiteral("请输入合法的十六进制命令和数据。"));
            return;
        }
        QByteArray readData;
        const int ret = m_device->handle()->SF_readDataFromDLP(static_cast<quint8>(cmd),
                                                               writeData,
                                                               static_cast<quint8>(m_iicReadLength->value()),
                                                               readData);
        if (ret == DE_SUCCESS) m_iicReadData->setText(DeviceController::hex(readData).toUpper());
        showResult(QStringLiteral("读取 DLP 数据"), ret);
    });

    return box;
}

QGroupBox *MainWindow::buildFlashGroup()
{
    QGroupBox *box = new QGroupBox(QStringLiteral("MCU Flash 文件"), this);
    QVBoxLayout *layout = new QVBoxLayout(box);

    m_flashTable = new QTableWidget(0, 1, box);
    m_flashTable->setHorizontalHeaderLabels(QStringList() << QStringLiteral("文件名"));
    m_flashTable->horizontalHeader()->setStretchLastSection(true);
    m_flashTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_flashTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_flashTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_flashTable, 1);

    m_flashFileName = lineEdit(QStringLiteral("手动输入 Flash 文件名"));
    layout->addWidget(m_flashFileName);

    m_flashProgress = new QProgressBar(box);
    layout->addWidget(m_flashProgress);

    QHBoxLayout *buttons = new QHBoxLayout();
    QPushButton *refresh = button(QStringLiteral("刷新列表"));
    QPushButton *deleteSelected = button(QStringLiteral("删除选中"));
    QPushButton *deleteTyped = button(QStringLiteral("删除输入文件"));
    QPushButton *clearAll = button(QStringLiteral("清空 Flash"));
    buttons->addWidget(refresh);
    buttons->addWidget(deleteSelected);
    buttons->addWidget(deleteTyped);
    buttons->addWidget(clearAll);
    layout->addLayout(buttons);

    connect(refresh, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        int used = 0;
        QStringList names;
        const int ret = m_device->handle()->SF_getFileNameFromFlash(used, names);
        if (ret == DE_SUCCESS) {
            m_flashTable->setRowCount(names.size());
            for (int row = 0; row < names.size(); ++row) {
                m_flashTable->setItem(row, 0, new QTableWidgetItem(names.at(row)));
            }
            appendLog(QStringLiteral("Flash 已使用: %1，文件数: %2").arg(used).arg(names.size()));
        }
        showResult(QStringLiteral("刷新 Flash 文件列表"), ret);
    });
    connect(deleteSelected, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        QStringList names;
        const QModelIndexList rows = m_flashTable->selectionModel()->selectedRows();
        for (const QModelIndex &row : rows) {
            QTableWidgetItem *item = m_flashTable->item(row.row(), 0);
            if (item) names << item->text();
        }
        names.removeDuplicates();
        if (names.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("未选择文件"), QStringLiteral("请先选择要删除的 Flash 文件。"));
            return;
        }
        if (!confirm(QStringLiteral("删除 Flash 文件"), QStringLiteral("确定删除选中的 %1 个文件吗？").arg(names.size()))) return;
        runWithProgress(m_flashProgress, QStringLiteral("删除 Flash 文件"),
                        [this, names]() { return m_device->handle()->SF_delFileFromFlash(names); });
    });
    connect(deleteTyped, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        const QString name = m_flashFileName->text().trimmed();
        if (name.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("文件名为空"), QStringLiteral("请输入要删除的 Flash 文件名。"));
            return;
        }
        if (!confirm(QStringLiteral("删除 Flash 文件"), QStringLiteral("确定删除 %1 吗？").arg(name))) return;
        runWithProgress(m_flashProgress, QStringLiteral("删除 Flash 文件"),
                        [this, name]() { return m_device->handle()->SF_delFileFromFlash(QStringList() << name); });
    });
    connect(clearAll, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        if (!confirm(QStringLiteral("清空 Flash"), QStringLiteral("确定清空 MCU Flash 中的所有文件吗？"))) return;
        runWithProgress(m_flashProgress, QStringLiteral("清空 Flash"),
                        [this]() { return m_device->handle()->SF_clrAllFileFromFlash(); });
    });

    return box;
}

QGroupBox *MainWindow::buildPatternFileGroup()
{
    QGroupBox *box = new QGroupBox(QStringLiteral("Pattern Time / Pattern Bin"), this);
    QHBoxLayout *outer = new QHBoxLayout(box);

    QGroupBox *ptBox = new QGroupBox(QStringLiteral("Pattern Time (PT)"), box);
    QFormLayout *ptForm = new QFormLayout(ptBox);
    m_ptActionCombo = comboBox(QStringList()
                               << QStringLiteral("上传 PT 到 DLP")
                               << QStringLiteral("上传 PT 到 MCU Flash")
                               << QStringLiteral("从 DLP 保存 PT")
                               << QStringLiteral("从 MCU Flash 保存 PT")
                               << QStringLiteral("从 MCU Flash 加载 PT 到 DLP"));
    m_ptFlashName = lineEdit(QStringLiteral("Flash 文件名，例如 0.pt"));
    m_ptSaveStartup = new QCheckBox(QStringLiteral("加载后保存为开机 PT"), ptBox);
    QPushButton *runPt = button(QStringLiteral("执行 PT 操作"));
    m_ptProgress = new QProgressBar(ptBox);
    ptForm->addRow(QStringLiteral("动作:"), m_ptActionCombo);
    ptForm->addRow(QStringLiteral("Flash 文件名:"), m_ptFlashName);
    ptForm->addRow(m_ptSaveStartup);
    ptForm->addRow(runPt, m_ptProgress);

    QGroupBox *pbBox = new QGroupBox(QStringLiteral("Pattern Bin (PB)"), box);
    QFormLayout *pbForm = new QFormLayout(pbBox);
    m_pbActionCombo = comboBox(QStringList()
                               << QStringLiteral("上传 PB 到 DLP")
                               << QStringLiteral("从 DLP 保存 PB")
                               << QStringLiteral("生成示例 PB 并上传"));
    QPushButton *runPb = button(QStringLiteral("执行 PB 操作"));
    m_pbProgress = new QProgressBar(pbBox);
    pbForm->addRow(QStringLiteral("动作:"), m_pbActionCombo);
    pbForm->addRow(runPb, m_pbProgress);

    outer->addWidget(ptBox, 1);
    outer->addWidget(pbBox, 1);

    connect(runPt, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        if (!confirm(QStringLiteral("Pattern Time 操作"), QStringLiteral("确定执行所选 PT 操作吗？"))) return;
        const int action = m_ptActionCombo->currentIndex();
        if (action == 0 || action == 1) {
            const QString fileName = selectOpenFile(QStringLiteral("打开 PT 文件"),
                                                    QStringLiteral("Pattern Time (*.pt);;所有文件 (*.*)"));
            if (fileName.isEmpty()) return;
            bool ok = false;
            const QByteArray data = readFile(fileName, &ok);
            if (!ok) return;
            if (action == 0) {
                runWithProgress(m_ptProgress, QStringLiteral("上传 PT 到 DLP"),
                                [this, data]() { return m_device->handle()->SF_updatePTtoDLPfromAPP(data); });
            } else {
                const QString flashName = m_ptFlashName->text().trimmed();
                if (flashName.isEmpty()) {
                    QMessageBox::warning(this, QStringLiteral("文件名为空"), QStringLiteral("请输入 PT 在 Flash 中保存的文件名。"));
                    return;
                }
                runWithProgress(m_ptProgress, QStringLiteral("上传 PT 到 Flash"),
                                [this, flashName, data]() { return m_device->handle()->SF_updatePTtoFlashFromAPP(flashName, data); });
            }
        } else if (action == 2 || action == 3) {
            const QString saveName = selectSaveFile(QStringLiteral("保存 PT 文件"),
                                                    QStringLiteral("Pattern Time (*.pt);;所有文件 (*.*)"));
            if (saveName.isEmpty()) return;
            QByteArray data;
            int ret = DE_FAIL;
            if (action == 3) {
                const QString flashName = m_ptFlashName->text().trimmed();
                if (flashName.isEmpty()) {
                    QMessageBox::warning(this, QStringLiteral("文件名为空"), QStringLiteral("请输入 PT Flash 文件名。"));
                    return;
                }
                runWithProgress(m_ptProgress, QStringLiteral("从 Flash 保存 PT"), [this, flashName, &data, &ret]() {
                    ret = m_device->handle()->SF_getPTtoAPPFromFlash(flashName, data);
                    return ret;
                });
            } else {
                runWithProgress(m_ptProgress, QStringLiteral("从 DLP 保存 PT"), [this, &data, &ret]() {
                    ret = m_device->handle()->SF_getPTtoAPPfromDLP(data);
                    return ret;
                });
            }
            if (ret == DE_SUCCESS) writeFile(saveName, data);
        } else {
            QString flashName = m_ptFlashName->text().trimmed();
            if (flashName.isEmpty()) {
                QMessageBox::warning(this, QStringLiteral("文件名为空"), QStringLiteral("请输入要加载的 PT Flash 文件名。"));
                return;
            }
            runWithProgress(m_ptProgress, QStringLiteral("从 Flash 加载 PT 到 DLP"), [this, &flashName]() {
                return m_device->handle()->SF_loadPTtoDLPfromFlash(m_ptSaveStartup->isChecked(), flashName);
            });
        }
    });

    connect(runPb, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        if (!confirm(QStringLiteral("Pattern Bin 操作"), QStringLiteral("确定执行所选 PB 操作吗？"))) return;
        const int action = m_pbActionCombo->currentIndex();
        if (action == 0) {
            const QString fileName = selectOpenFile(QStringLiteral("打开 PB 文件"),
                                                    QStringLiteral("Pattern Bin (*.pb *.bin *.img);;所有文件 (*.*)"));
            if (fileName.isEmpty()) return;
            bool ok = false;
            const QByteArray data = readFile(fileName, &ok);
            if (ok) {
                runWithProgress(m_pbProgress, QStringLiteral("上传 PB 到 DLP"),
                                [this, data]() { return m_device->handle()->SF_updatePBtoDLPfromAPP(data); });
            }
        } else if (action == 1) {
            const QString saveName = selectSaveFile(QStringLiteral("保存 PB 文件"),
                                                    QStringLiteral("Pattern Bin (*.pb);;所有文件 (*.*)"));
            if (saveName.isEmpty()) return;
            QByteArray data;
            int ret = DE_FAIL;
            runWithProgress(m_pbProgress, QStringLiteral("从 DLP 保存 PB"), [this, &data, &ret]() {
                ret = m_device->handle()->SF_getPBtoAPPfromDLP(data);
                return ret;
            });
            if (ret == DE_SUCCESS) writeFile(saveName, data);
        } else {
            std::vector<INT_PAT_PatternSet_t> sets;
            std::vector<INT_PAT_PatternOrderTableEntry_t> entries;
            QByteArray data;
            populateGeneratedPatternData(sets, entries);
            const int ret = m_device->handle()->SF_generatePatternData(sets, entries, true, true, data);
            releaseGeneratedPatternData(sets);
            if (ret != DE_SUCCESS) {
                showResult(QStringLiteral("生成示例 PB"), ret);
                return;
            }
            runWithProgress(m_pbProgress, QStringLiteral("上传生成的 PB"),
                            [this, data]() { return m_device->handle()->SF_updatePBtoDLPfromAPP(data); });
        }
    });

    return box;
}

QGroupBox *MainWindow::buildFirmwareGroup()
{
    QGroupBox *box = new QGroupBox(QStringLiteral("固件升级"), this);
    QFormLayout *form = new QFormLayout(box);

    QPushButton *mcuUpdate = button(QStringLiteral("升级 MCU (.bin)"));
    m_mcuProgress = new QProgressBar(box);
    form->addRow(mcuUpdate, m_mcuProgress);

    m_dlpUpdateMode = comboBox(QStringList()
                               << QStringLiteral("直接升级 DLP (.img)")
                               << QStringLiteral("保存 DLP 镜像到 MCU Flash (.img)")
                               << QStringLiteral("从 MCU Flash 升级 DLP"));
    QPushButton *dlpUpdate = button(QStringLiteral("执行 DLP 升级"));
    m_dlpProgress = new QProgressBar(box);
    form->addRow(QStringLiteral("DLP 模式:"), m_dlpUpdateMode);
    form->addRow(dlpUpdate, m_dlpProgress);

    QPushButton *getDlp = button(QStringLiteral("从设备保存 DLP 镜像"));
    m_getDlpProgress = new QProgressBar(box);
    form->addRow(getDlp, m_getDlpProgress);

    connect(mcuUpdate, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        if (!confirm(QStringLiteral("升级 MCU 固件"), QStringLiteral("将使用 .bin 文件升级 MCU。确认继续吗？"))) return;
        const QString fileName = selectOpenFile(QStringLiteral("打开 MCU 固件"), QStringLiteral("MCU 固件 (*.bin)"));
        if (fileName.isEmpty()) return;
        runWithProgress(m_mcuProgress, QStringLiteral("升级 MCU 固件"),
                        [this, fileName]() { return m_device->handle()->SF_updateMCUProgram(fileName); });
    });
    connect(dlpUpdate, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        if (!confirm(QStringLiteral("升级 DLP 固件"), QStringLiteral("确定执行所选 DLP 升级操作吗？"))) return;
        const int mode = m_dlpUpdateMode->currentIndex();
        if (mode == 2) {
            runWithProgress(m_dlpProgress, QStringLiteral("从 Flash 升级 DLP"),
                            [this]() { return m_device->handle()->SF_updateDLPFromFlash(); });
            return;
        }
        const QString fileName = selectOpenFile(QStringLiteral("打开 DLP 镜像"), QStringLiteral("DLP 镜像 (*.img)"));
        if (fileName.isEmpty()) return;
        const updateDir_e dir = (mode == 0) ? UD_TO_DLP : UD_TO_FLASH;
        runWithProgress(m_dlpProgress, QStringLiteral("升级 DLP 固件"),
                        [this, dir, fileName]() { return m_device->handle()->SF_updateDLPProgram(dir, fileName); });
    });
    connect(getDlp, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        const QString saveName = selectSaveFile(QStringLiteral("保存 DLP 镜像"), QStringLiteral("DLP 镜像 (*.img)"));
        if (saveName.isEmpty()) return;
        QByteArray data;
        int ret = DE_FAIL;
        runWithProgress(m_getDlpProgress, QStringLiteral("保存 DLP 镜像"), [this, &data, &ret]() {
            ret = m_device->handle()->SF_getDLPProgramFromDLP(data);
            return ret;
        });
        if (ret == DE_SUCCESS) writeFile(saveName, data);
    });

    return box;
}

QGroupBox *MainWindow::buildDangerGroup()
{
    QGroupBox *box = new QGroupBox(QStringLiteral("高级设备操作"), this);
    QHBoxLayout *layout = new QHBoxLayout(box);
    QPushButton *mcuReset = button(QStringLiteral("复位 MCU"));
    QPushButton *dlpReboot = button(QStringLiteral("重启 DLP"));
    QPushButton *factoryReset = button(QStringLiteral("恢复出厂"));
    QPushButton *open12v = button(QStringLiteral("打开 12V"));
    QPushButton *close12v = button(QStringLiteral("关闭 12V"));
    layout->addWidget(mcuReset);
    layout->addWidget(dlpReboot);
    layout->addWidget(factoryReset);
    layout->addWidget(open12v);
    layout->addWidget(close12v);
    layout->addStretch();

    connect(mcuReset, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        if (!confirm(QStringLiteral("复位 MCU"), QStringLiteral("现在复位 MCU 吗？"))) return;
        showResult(QStringLiteral("复位 MCU"), m_device->handle()->SF_resetMCU());
    });
    connect(dlpReboot, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        if (!confirm(QStringLiteral("重启 DLP"), QStringLiteral("现在重启 DLP 吗？"))) return;
        showResult(QStringLiteral("重启 DLP"), m_device->handle()->SF_setDlpPower(DS_REBOOT));
    });
    connect(factoryReset, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        if (!confirm(QStringLiteral("恢复出厂"), QStringLiteral("恢复出厂设置可能会清除已保存配置。确认继续吗？"))) return;
        showResult(QStringLiteral("恢复出厂"), m_device->handle()->SF_resetFactory());
    });
    connect(open12v, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        if (!confirm(QStringLiteral("打开 12V"), QStringLiteral("确定打开 DLP 12V 电源吗？"))) return;
        showResult(QStringLiteral("打开 12V"), m_device->handle()->SF_ctlHardware(CS_OPEN_12V));
    });
    connect(close12v, &QPushButton::clicked, this, [this]() {
        if (!ensureConnected()) return;
        if (!confirm(QStringLiteral("关闭 12V"), QStringLiteral("确定关闭 DLP 12V 电源吗？"))) return;
        showResult(QStringLiteral("关闭 12V"), m_device->handle()->SF_ctlHardware(CS_CLOSE_12V));
    });

    return box;
}

QPushButton *MainWindow::button(const QString &text)
{
    QPushButton *btn = new QPushButton(text, this);
    btn->setMinimumHeight(28);
    return btn;
}

QLabel *MainWindow::statusLabel(const QString &text)
{
    QLabel *label = new QLabel(text, this);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}

QLineEdit *MainWindow::lineEdit(const QString &placeholder)
{
    QLineEdit *edit = new QLineEdit(this);
    edit->setPlaceholderText(placeholder);
    return edit;
}

QSpinBox *MainWindow::spinBox(int min, int max, int value, const QString &suffix)
{
    QSpinBox *spin = new QSpinBox(this);
    spin->setRange(min, max);
    spin->setValue(value);
    spin->setSuffix(suffix);
    return spin;
}

QComboBox *MainWindow::comboBox(const QStringList &items)
{
    QComboBox *combo = new QComboBox(this);
    combo->addItems(items);
    return combo;
}

void MainWindow::refreshPorts()
{
    const QString current = m_portCombo->currentData().toString();
    m_portCombo->clear();

    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        QString display = port.portName();
        const QString description = port.description().trimmed();
        const QString manufacturer = port.manufacturer().trimmed();
        if (!description.isEmpty()) {
            display += QStringLiteral(" - %1").arg(description);
        }
        if (!manufacturer.isEmpty()) {
            display += QStringLiteral(" (%1)").arg(manufacturer);
        }
        m_portCombo->addItem(display, port.portName());
    }

    const int index = m_portCombo->findData(current);
    if (index >= 0) {
        m_portCombo->setCurrentIndex(index);
    }
}

void MainWindow::toggleConnection()
{
    if (m_device->isConnected()) {
        showResult(QStringLiteral("关闭串口"), m_device->closePort());
        return;
    }
    const QString port = m_portCombo->currentData().toString();
    if (port.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("未选择串口"), QStringLiteral("请先选择串口。"));
        return;
    }
    const int ret = m_device->openPort(port);
    showResult(QStringLiteral("打开串口并检测光机"), ret);
    if (ret != DE_SUCCESS) {
        QMessageBox::information(this,
                                 QStringLiteral("光机未响应"),
                                 QStringLiteral("串口可以打开，但光机没有返回有效响应。\n\n"
                                                "如果设备管理器里有 USB-Serial Dual Channel 的多个 COM 口，"
                                                "请换另一个 COM 口再连接。\n"
                                                "也请确认光机已上电、驱动正常、没有被官方工具或串口助手占用。"));
    }
}

void MainWindow::autoDetectDevice()
{
    if (m_device->isConnected()) {
        showResult(QStringLiteral("关闭当前串口"), m_device->closePort());
    }

    refreshPorts();
    if (m_portCombo->count() == 0) {
        QMessageBox::warning(this, QStringLiteral("没有串口"), QStringLiteral("未发现可用串口。请检查 USB 线、驱动和光机供电。"));
        return;
    }

    m_autoDetectButton->setEnabled(false);
    QApplication::setOverrideCursor(Qt::BusyCursor);

    int lastRet = DE_FAIL;
    for (int i = 0; i < m_portCombo->count(); ++i) {
        const QString port = m_portCombo->itemData(i).toString();
        if (port.isEmpty()) {
            continue;
        }

        appendLog(QStringLiteral("尝试连接 %1 ...").arg(m_portCombo->itemText(i)));
        lastRet = m_device->openPort(port);
        if (lastRet == DE_SUCCESS && m_device->isDeviceReady()) {
            m_portCombo->setCurrentIndex(i);
            QApplication::restoreOverrideCursor();
            m_autoDetectButton->setEnabled(true);
            QMessageBox::information(this, QStringLiteral("找到光机"), QStringLiteral("已连接到光机控制串口: %1").arg(port));
            return;
        }

        if (m_device->isConnected()) {
            m_device->closePort();
        }
        QApplication::processEvents();
    }

    QApplication::restoreOverrideCursor();
    m_autoDetectButton->setEnabled(true);
    QMessageBox::warning(this,
                         QStringLiteral("未找到光机"),
                         QStringLiteral("已尝试所有串口，但没有端口通过“就绪检测 + MCU 版本读取”。\n\n"
                                        "最后错误: %1\n"
                                        "请确认光机已上电、驱动正常、没有被官方 GUI/串口助手占用，"
                                        "并检查 USB-Serial 双通道是否还有其他 COM 口。")
                             .arg(DeviceController::errorText(lastRet)));
}

void MainWindow::updateConnectedUi(bool connected)
{
    if (connected && m_device->isDeviceReady()) {
        m_connectionStatus->setText(QStringLiteral("已连接，光机就绪"));
    } else if (connected) {
        m_connectionStatus->setText(QStringLiteral("串口已连接，等待光机响应"));
    } else {
        m_connectionStatus->setText(QStringLiteral("未连接"));
    }
    m_connectButton->setText(connected ? QStringLiteral("断开") : QStringLiteral("连接"));
    m_portCombo->setEnabled(!connected);
    m_refreshPortsButton->setEnabled(!connected);
    m_autoDetectButton->setEnabled(!connected);
}

bool MainWindow::confirm(const QString &title, const QString &message) const
{
    return QMessageBox::question(const_cast<MainWindow *>(this), title, message,
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
}

void MainWindow::showResult(const QString &action, int ret)
{
    const QString text = QStringLiteral("%1: %2").arg(action, DeviceController::errorText(ret));
    appendLog(text);
    if (ret != DE_SUCCESS) {
        QMessageBox::warning(this, QStringLiteral("操作失败"), text);
    }
}

void MainWindow::appendLog(const QString &line)
{
    if (m_logEdit) {
        m_logEdit->appendPlainText(line);
    }
}

bool MainWindow::ensureConnected()
{
    if (m_device->isConnected() && m_device->isDeviceReady()) {
        return true;
    }
    if (m_device->isConnected()) {
        const int ret = m_device->checkDeviceReady();
        if (ret == DE_SUCCESS) {
            return true;
        }
        QMessageBox::warning(this,
                             QStringLiteral("光机未响应"),
                             QStringLiteral("串口已打开，但光机就绪检测失败: %1\n\n"
                                            "请检查是否选错双通道 USB-Serial 的 COM 口，"
                                            "或确认光机已上电且未被其他软件占用。")
                                 .arg(DeviceController::errorText(ret)));
        return false;
    }
    QMessageBox::warning(this, QStringLiteral("未连接"), QStringLiteral("请先连接光机。"));
    return false;
}

QByteArray MainWindow::readFile(const QString &path, bool *ok)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (ok) *ok = false;
        QMessageBox::warning(this, QStringLiteral("文件错误"), QStringLiteral("无法打开 %1").arg(path));
        return QByteArray();
    }
    const QByteArray data = file.readAll();
    if (data.isEmpty()) {
        if (ok) *ok = false;
        QMessageBox::warning(this, QStringLiteral("文件错误"), QStringLiteral("文件为空: %1").arg(path));
        return QByteArray();
    }
    if (ok) *ok = true;
    return data;
}

bool MainWindow::writeFile(const QString &path, const QByteArray &data)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, QStringLiteral("文件错误"), QStringLiteral("无法写入 %1").arg(path));
        return false;
    }
    file.write(data);
    appendLog(QStringLiteral("已保存文件: %1 (%2 字节)").arg(path).arg(data.size()));
    return true;
}

QString MainWindow::selectOpenFile(const QString &title, const QString &filter)
{
    return QFileDialog::getOpenFileName(this, title, QString(), filter);
}

QString MainWindow::selectSaveFile(const QString &title, const QString &filter)
{
    return QFileDialog::getSaveFileName(this, title, QString(), filter);
}

QByteArray MainWindow::hexStringToBytes(const QString &text, bool *ok) const
{
    QByteArray out;
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        if (ok) *ok = true;
        return out;
    }
    const QStringList parts = trimmed.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        bool partOk = false;
        const int value = part.toInt(&partOk, 16);
        if (!partOk || value < 0 || value > 0xff) {
            if (ok) *ok = false;
            return QByteArray();
        }
        out.append(static_cast<char>(value));
    }
    if (ok) *ok = true;
    return out;
}

QString MainWindow::statusText(const runStatus_t &status) const
{
    return QStringLiteral(
        "DLP: %1\nMCU 温度: %2 C\nDLP 温度: %3 C\nMCU 温度告警: %4\nDLP 温度告警: %5\n12V 电源: %6\nSPI 引脚: %7\nPARKZ 引脚: %8\nDLPCM 引脚: %9")
        .arg(onOffText(status.dlpStatus))
        .arg(status.mcuTemp)
        .arg(status.dlpTemp)
        .arg(normalAlarmText(status.mcuTempAlarm))
        .arg(normalAlarmText(status.ledTempAlarm))
        .arg(onOffText(status.power12Vstate))
        .arg(status.spiPinState ? QStringLiteral("MCU") : QStringLiteral("USB"))
        .arg(onOffText(status.parkzPinState))
        .arg(onOffText(status.dlpcmPinState));
}

void MainWindow::runWithProgress(QProgressBar *progress, const QString &action, const std::function<int()> &operation)
{
    progress->setRange(0, 100);
    progress->setValue(0);
    QMetaObject::Connection connection = connect(m_device, &DeviceController::progressChanged, progress, &QProgressBar::setValue);
    QApplication::setOverrideCursor(Qt::BusyCursor);
    const int ret = operation();
    QApplication::restoreOverrideCursor();
    disconnect(connection);
    if (ret == DE_SUCCESS && progress->value() == 0) {
        progress->setValue(100);
    }
    showResult(action, ret);
}

void MainWindow::populateGeneratedPatternData(std::vector<INT_PAT_PatternSet_t> &patternSets,
                                              std::vector<INT_PAT_PatternOrderTableEntry_t> &entries)
{
    auto appendSet = [this, &patternSets](INT_PAT_BitDepth_e depth, INT_PAT_Direction_e direction,
                                          int count, int length, bool eightBit) {
        INT_PAT_PatternSet_t set;
        set.BitDepth = depth;
        set.Direction = direction;
        set.PatternCount = static_cast<uint32_t>(count);
        set.PatternArray = new INT_PAT_PatternData_t[count];
        for (int i = 0; i < count; ++i) {
            uint8_t *pixels = new uint8_t[length];
            const uint16_t bars = static_cast<uint16_t>(2 * (i + 1));
            if (eightBit) populateEightBitPatternData(static_cast<uint16_t>(length), pixels, bars);
            else populateOneBitPatternData(static_cast<uint16_t>(length), pixels, bars);
            set.PatternArray[i].PixelArray = pixels;
            set.PatternArray[i].PixelArrayCount = static_cast<uint32_t>(length);
        }
        patternSets.push_back(set);
    };

    appendSet(INT_PAT_BITDEPTH_ONE, INT_PAT_DIRECTION_HORIZONTAL, kOneBitHorizontalPatterns, kPatternHeight, false);
    appendSet(INT_PAT_BITDEPTH_ONE, INT_PAT_DIRECTION_VERTICAL, kOneBitVerticalPatterns, kPatternWidth, false);
    appendSet(INT_PAT_BITDEPTH_EIGHT, INT_PAT_DIRECTION_HORIZONTAL, kEightBitHorizontalPatterns, kPatternHeight, true);
    appendSet(INT_PAT_BITDEPTH_EIGHT, INT_PAT_DIRECTION_VERTICAL, kEightBitVerticalPatterns, kPatternWidth, true);

    const INT_PAT_IlluminationSelect_e illuminations[] = {
        INT_PAT_ILLUMINATION_RED,
        INT_PAT_ILLUMINATION_GREEN,
        INT_PAT_ILLUMINATION_BLUE,
        INT_PAT_ILLUMINATION_RGB,
    };

    for (int i = 0; i < 4; ++i) {
        INT_PAT_PatternOrderTableEntry_t entry;
        entry.PatternSetIndex = static_cast<uint8_t>(i);
        entry.NumDisplayPatterns = static_cast<uint8_t>(patternSets[i].PatternCount);
        entry.IlluminationSelect = illuminations[i];
        entry.InvertPatterns = false;
        entry.IlluminationTimeInMicroseconds = (i == 3) ? 11000 : 5000;
        entry.PreIlluminationDarkTimeInMicroseconds = 250;
        entry.PostIlluminationDarkTimeInMicroseconds = 1000;
        entries.push_back(entry);
    }
}

void MainWindow::releaseGeneratedPatternData(std::vector<INT_PAT_PatternSet_t> &patternSets)
{
    for (INT_PAT_PatternSet_t &set : patternSets) {
        if (!set.PatternArray) continue;
        for (uint32_t i = 0; i < set.PatternCount; ++i) {
            delete[] set.PatternArray[i].PixelArray;
            set.PatternArray[i].PixelArray = nullptr;
        }
        delete[] set.PatternArray;
        set.PatternArray = nullptr;
    }
    patternSets.clear();
}

void MainWindow::populateOneBitPatternData(uint16_t length, uint8_t *data, uint16_t bars)
{
    const uint16_t barWidth = qMax<uint16_t>(1, length / bars);
    uint16_t barPos = 0;
    uint8_t pixel = 0;
    for (uint16_t i = 0; i < length; ++i) {
        data[i] = pixel;
        if (++barPos >= barWidth) {
            barPos = 0;
            pixel = (pixel == 0) ? 1 : 0;
        }
    }
}

void MainWindow::populateEightBitPatternData(uint16_t length, uint8_t *data, uint16_t bars)
{
    const uint16_t barWidth = qMax<uint16_t>(1, length / (2 * bars));
    uint16_t barPos = 0;
    int pixel = 0;
    int increment = qMax(1, static_cast<int>(std::ceil(255.0 / barWidth)));
    for (uint16_t i = 0; i < length; ++i) {
        data[i] = static_cast<uint8_t>(qBound(0, pixel, 255));
        if (++barPos >= barWidth) {
            barPos = 0;
            increment = -increment;
        }
        pixel += increment;
    }
}
