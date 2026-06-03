#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include <dlpdefine.h>
#include <dlphandle.h>
#include <dlphandle_global.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowClass; };
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindowClass *ui;
};

