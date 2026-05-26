#pragma once

#include <QMainWindow>
#include <QString>

#include "PointCloudLoader.h"

class QLabel;
class QListWidget;
class QDoubleSpinBox;
class QPushButton;
class PointCloudView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void choosePcdDirectory();
    void choosePcdFile();
    void reloadDirectory();
    void loadSelectedFile();
    void resetView();
    void applyFilter();
    void restoreOriginalCloud();

private:
    void buildUi();
    void connectSignals();
    void loadDirectory(const QString& directoryPath);
    void loadCloudFile(const QString& filePath);
    void updateInfo(const PointCloudLoadResult& result);
    void showCloudResult(const PointCloudLoadResult& result);
    QString defaultPcdDirectory() const;

    QString currentDirectory_;
    QString currentFilePath_;
    PointCloudLoadResult originalCloud_;
    PointCloudLoadResult currentCloud_;
    QListWidget* fileList_ = nullptr;
    QLabel* directoryLabel_ = nullptr;
    QLabel* fileNameLabel_ = nullptr;
    QLabel* pointCountLabel_ = nullptr;
    QLabel* boundsLabel_ = nullptr;
    QLabel* originalPointCountLabel_ = nullptr;
    QLabel* filteredPointCountLabel_ = nullptr;
    QDoubleSpinBox* leafSizeSpinBox_ = nullptr;
    QPushButton* applyFilterButton_ = nullptr;
    QPushButton* restoreButton_ = nullptr;
    QPushButton* reloadButton_ = nullptr;
    QPushButton* openDirectoryButton_ = nullptr;
    QPushButton* openFileButton_ = nullptr;
    QPushButton* resetViewButton_ = nullptr;
    PointCloudView* cloudView_ = nullptr;
};
