#include "MainWindow.h"
#include <iostream>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClass())
{
    ui->setupUi(this);
    DlpHandle* m_dlpHandle = new DlpHandle(this);
    m_dlpHandle->SF_initLibrary();
    m_dlpHandle->SF_openUart("COM3");
    int ret = m_dlpHandle->SF_deviceCheckReady();
    std::cout << "ret:" << ret << std::endl;
    quint16 blueBright;
    ret = m_dlpHandle->SF_getBlueBright(blueBright);
    std::cout << "ret:" << ret << std::endl;
    std::cout << "blueBright:" << blueBright << std::endl;
}

MainWindow::~MainWindow()
{
    delete ui;
}

