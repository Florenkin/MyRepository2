#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QStringList>

#include "dlphandle.h"

class DeviceController : public QObject
{
    Q_OBJECT

public:
    explicit DeviceController(QObject *parent = nullptr);
    ~DeviceController() override;

    DlpHandle *handle() const;
    bool isConnected() const;
    bool isDeviceReady() const;
    QString connectedPort() const;

    QStringList availablePorts() const;
    int openPort(const QString &portName);
    int closePort();
    int checkDeviceReady();

    static QString errorText(int code);
    static QString commandErrorText(int code);
    static QString hex(const QByteArray &data);

signals:
    void connectionChanged(bool connected);
    void logLine(const QString &line);
    void progressChanged(int value);
    void sdkError(const QString &message);

private:
    DlpHandle *m_handle = nullptr;
    bool m_connected = false;
    bool m_deviceReady = false;
    QString m_connectedPort;
};
