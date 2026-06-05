#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <reconstruction3d_sdk.hpp>  // Included for DPL Structured Light SDK

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    std::vector<std::vector<cv::Mat *>> src_image_vec;
    std::vector<cv::Mat>                original_signal_vec;
    std::vector<cv::Mat>                original_gray_code_signal_vec;
    std::vector<cv::Mat>                original_phase_signal_vec;
    std::vector<cv::Mat>                compared_signal_vec;
};

#endif // MAINWINDOW_H
