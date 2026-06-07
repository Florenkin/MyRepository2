#include "MainWindow.h"
#include <iostream>
#include <imaging.h>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClass())
{
    ui->setupUi(this);
    //DlpHandle* m_dlpHandle = new DlpHandle(this);
    //m_dlpHandle->SF_initLibrary();
    //m_dlpHandle->SF_openUart("COM3");
    //int ret = m_dlpHandle->SF_deviceCheckReady();
    //std::cout << "ret:" << ret << std::endl;
    //quint16 blueBright;
    //ret = m_dlpHandle->SF_getBlueBright(blueBright);
    //std::cout << "ret:" << ret << std::endl;
    //std::cout << "blueBright:" << blueBright << std::endl;
    IMGING::Imaging* m_imagingControl = new IMGING::Imaging;
    bool ret = m_imagingControl->OpenCamAndProj();
    //if (ret)
    //{
    //    std::cout << "OpenCamAndProj success" << std::endl;
    //}
    //else
    //{
    //    std::cout << "OpenCamAndProj failed" << std::endl;
    //}
}

MainWindow::~MainWindow()
{
    delete ui;
}

