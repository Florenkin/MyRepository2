#pragma once

#include <functional>
#include <vector>

#include <QByteArray>
#include <QObject>
#include <QMainWindow>
#include <QPair>
#include <QString>
#include <QStringList>

#include "DeviceController.h"

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QTableWidget;
class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void refreshPorts();
    void toggleConnection();
    void autoDetectDevice();
    void updateConnectedUi(bool connected);

private:
    QWidget *buildConnectionBar();
    QWidget *buildOperationPage();
    QWidget *buildAdvancedPage();
    QWidget *buildLogPage();

    QGroupBox *buildInfoGroup();
    QGroupBox *buildPowerModeGroup();
    QGroupBox *buildSoftTriggerGroup();
    QGroupBox *buildImageGroup();
    QGroupBox *buildRgbGroup();
    QGroupBox *buildTriggerGroup();
    QGroupBox *buildPatternControlGroup();
    QGroupBox *buildIicGroup();

    QGroupBox *buildFlashGroup();
    QGroupBox *buildPatternFileGroup();
    QGroupBox *buildFirmwareGroup();
    QGroupBox *buildDangerGroup();

    QPushButton *button(const QString &text);
    QLabel *statusLabel(const QString &text = QString());
    QLineEdit *lineEdit(const QString &placeholder = QString());
    QSpinBox *spinBox(int min, int max, int value = 0, const QString &suffix = QString());
    QComboBox *comboBox(const QStringList &items);

    bool confirm(const QString &title, const QString &message) const;
    void showResult(const QString &action, int ret);
    void appendLog(const QString &line);
    bool ensureConnected();
    QByteArray readFile(const QString &path, bool *ok);
    bool writeFile(const QString &path, const QByteArray &data);
    QString selectOpenFile(const QString &title, const QString &filter);
    QString selectSaveFile(const QString &title, const QString &filter);
    QByteArray hexStringToBytes(const QString &text, bool *ok) const;
    QString statusText(const runStatus_t &status) const;
    void runWithProgress(QProgressBar *progress, const QString &action, const std::function<int()> &operation);
    void populateGeneratedPatternData(std::vector<INT_PAT_PatternSet_t> &patternSets,
                                      std::vector<INT_PAT_PatternOrderTableEntry_t> &entries);
    void releaseGeneratedPatternData(std::vector<INT_PAT_PatternSet_t> &patternSets);
    void populateOneBitPatternData(uint16_t length, uint8_t *data, uint16_t bars);
    void populateEightBitPatternData(uint16_t length, uint8_t *data, uint16_t bars);

private:
    DeviceController *m_device = nullptr;

    QComboBox *m_portCombo = nullptr;
    QPushButton *m_refreshPortsButton = nullptr;
    QPushButton *m_connectButton = nullptr;
    QPushButton *m_autoDetectButton = nullptr;
    QLabel *m_connectionStatus = nullptr;

    QLabel *m_versionLabel = nullptr;
    QLabel *m_libraryLabel = nullptr;
    QLabel *m_modeLabel = nullptr;
    QTextEdit *m_statusText = nullptr;
    QSpinBox *m_colorTempSpin = nullptr;

    QComboBox *m_powerCombo = nullptr;
    QComboBox *m_operationModeCombo = nullptr;
    QComboBox *m_splashCombo = nullptr;

    QRadioButton *m_triggerSingle = nullptr;
    QRadioButton *m_triggerMultiple = nullptr;
    QRadioButton *m_triggerUnlimited = nullptr;
    QSpinBox *m_triggerCountSpin = nullptr;
    QSpinBox *m_triggerIntervalSpin = nullptr;

    QCheckBox *m_mirrorVertical = nullptr;
    QCheckBox *m_mirrorHorizontal = nullptr;
    QComboBox *m_testPatternCombo = nullptr;

    QCheckBox *m_redEnable = nullptr;
    QCheckBox *m_greenEnable = nullptr;
    QCheckBox *m_blueEnable = nullptr;
    QSpinBox *m_redCurrent = nullptr;
    QSpinBox *m_greenCurrent = nullptr;
    QSpinBox *m_blueCurrent = nullptr;
    QLabel *m_rgbMaxLabel = nullptr;

    QCheckBox *m_triggerOut1Enable = nullptr;
    QCheckBox *m_triggerOut1Invert = nullptr;
    QSpinBox *m_triggerOut1Delay = nullptr;
    QCheckBox *m_triggerOut2Enable = nullptr;
    QCheckBox *m_triggerOut2Invert = nullptr;
    QSpinBox *m_triggerOut2Delay = nullptr;
    QCheckBox *m_triggerInEnable = nullptr;
    QComboBox *m_triggerInPolarity = nullptr;
    QCheckBox *m_patternReadyEnable = nullptr;
    QComboBox *m_patternReadyPolarity = nullptr;

    QSpinBox *m_startIicIndex = nullptr;
    QLineEdit *m_startIicCmd = nullptr;
    QLineEdit *m_startIicData = nullptr;
    QLineEdit *m_iicCmd = nullptr;
    QSpinBox *m_iicReadLength = nullptr;
    QLineEdit *m_iicWriteData = nullptr;
    QLineEdit *m_iicReadData = nullptr;

    QProgressBar *m_flashProgress = nullptr;
    QTableWidget *m_flashTable = nullptr;
    QLineEdit *m_flashFileName = nullptr;

    QComboBox *m_ptActionCombo = nullptr;
    QLineEdit *m_ptFlashName = nullptr;
    QCheckBox *m_ptSaveStartup = nullptr;
    QProgressBar *m_ptProgress = nullptr;
    QComboBox *m_pbActionCombo = nullptr;
    QProgressBar *m_pbProgress = nullptr;

    QComboBox *m_dlpUpdateMode = nullptr;
    QProgressBar *m_mcuProgress = nullptr;
    QProgressBar *m_dlpProgress = nullptr;
    QProgressBar *m_getDlpProgress = nullptr;

    QPlainTextEdit *m_logEdit = nullptr;
};
