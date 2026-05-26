#include "MainWindow.h"

#include "PointCloudView.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    buildUi();
    connectSignals();
    loadDirectory(defaultPcdDirectory());
}

void MainWindow::buildUi()
{
    setWindowTitle(QStringLiteral("PointCloud 点云可视化"));
    resize(1280, 760);

    auto* central = new QWidget(this);
    auto* rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(10, 10, 10, 10);

    auto* splitter = new QSplitter(Qt::Horizontal, central);
    rootLayout->addWidget(splitter);

    auto* sidePanel = new QWidget(splitter);
    sidePanel->setMinimumWidth(300);
    sidePanel->setMaximumWidth(420);

    auto* sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(0, 0, 8, 0);

    auto* actionLayout = new QHBoxLayout();
    openDirectoryButton_ = new QPushButton(QStringLiteral("选择目录"), sidePanel);
    openFileButton_ = new QPushButton(QStringLiteral("打开文件"), sidePanel);
    reloadButton_ = new QPushButton(QStringLiteral("刷新"), sidePanel);
    actionLayout->addWidget(openDirectoryButton_);
    actionLayout->addWidget(openFileButton_);
    actionLayout->addWidget(reloadButton_);
    sideLayout->addLayout(actionLayout);

    directoryLabel_ = new QLabel(sidePanel);
    directoryLabel_->setWordWrap(true);
    sideLayout->addWidget(directoryLabel_);

    fileList_ = new QListWidget(sidePanel);
    sideLayout->addWidget(fileList_, 1);

    auto* infoBox = new QGroupBox(QStringLiteral("点云信息"), sidePanel);
    auto* infoLayout = new QVBoxLayout(infoBox);
    fileNameLabel_ = new QLabel(QStringLiteral("文件：未加载"), infoBox);
    pointCountLabel_ = new QLabel(QStringLiteral("点数：-"), infoBox);
    originalPointCountLabel_ = new QLabel(QStringLiteral("原始点数：-"), infoBox);
    filteredPointCountLabel_ = new QLabel(QStringLiteral("滤波点数：-"), infoBox);
    boundsLabel_ = new QLabel(QStringLiteral("范围：-"), infoBox);
    boundsLabel_->setWordWrap(true);
    resetViewButton_ = new QPushButton(QStringLiteral("重置视角"), infoBox);
    leafSizeSpinBox_ = new QDoubleSpinBox(infoBox);
    leafSizeSpinBox_->setRange(0.001, 100.0);
    leafSizeSpinBox_->setDecimals(3);
    leafSizeSpinBox_->setSingleStep(0.01);
    leafSizeSpinBox_->setValue(0.05);
    applyFilterButton_ = new QPushButton(QStringLiteral("应用体素滤波"), infoBox);
    restoreButton_ = new QPushButton(QStringLiteral("恢复原始点云"), infoBox);
    infoLayout->addWidget(fileNameLabel_);
    infoLayout->addWidget(pointCountLabel_);
    infoLayout->addWidget(originalPointCountLabel_);
    infoLayout->addWidget(filteredPointCountLabel_);
    infoLayout->addWidget(boundsLabel_);
    infoLayout->addWidget(new QLabel(QStringLiteral("体素大小"), infoBox));
    infoLayout->addWidget(leafSizeSpinBox_);
    infoLayout->addWidget(applyFilterButton_);
    infoLayout->addWidget(restoreButton_);
    infoLayout->addWidget(resetViewButton_);
    sideLayout->addWidget(infoBox);

    cloudView_ = new PointCloudView(splitter);
    splitter->addWidget(sidePanel);
    splitter->addWidget(cloudView_);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(central);
    statusBar()->showMessage(QStringLiteral("请选择或双击 PCD 文件进行加载"));
}

void MainWindow::connectSignals()
{
    connect(openDirectoryButton_, &QPushButton::clicked,
            this, &MainWindow::choosePcdDirectory);
    connect(openFileButton_, &QPushButton::clicked,
            this, &MainWindow::choosePcdFile);
    connect(reloadButton_, &QPushButton::clicked,
            this, &MainWindow::reloadDirectory);
    connect(resetViewButton_, &QPushButton::clicked,
            this, &MainWindow::resetView);
    connect(applyFilterButton_, &QPushButton::clicked,
            this, &MainWindow::applyFilter);
    connect(restoreButton_, &QPushButton::clicked,
            this, &MainWindow::restoreOriginalCloud);
    connect(fileList_, &QListWidget::itemDoubleClicked,
            this, &MainWindow::loadSelectedFile);
}

void MainWindow::choosePcdDirectory()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this,
        QStringLiteral("选择 PCD 文件目录"),
        currentDirectory_.isEmpty() ? defaultPcdDirectory() : currentDirectory_);
    if (!dir.isEmpty()) {
        loadDirectory(dir);
    }
}

void MainWindow::choosePcdFile()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("打开 PCD 文件"),
        currentDirectory_.isEmpty() ? defaultPcdDirectory() : currentDirectory_,
        QStringLiteral("PCD 文件 (*.pcd);;所有文件 (*.*)"));
    if (!filePath.isEmpty()) {
        loadCloudFile(filePath);
    }
}

void MainWindow::reloadDirectory()
{
    loadDirectory(currentDirectory_.isEmpty() ? defaultPcdDirectory() : currentDirectory_);
}

void MainWindow::loadSelectedFile()
{
    const auto* item = fileList_->currentItem();
    if (!item) {
        return;
    }

    const QString filePath = item->data(Qt::UserRole).toString();
    if (!filePath.isEmpty()) {
        loadCloudFile(filePath);
    }
}

void MainWindow::resetView()
{
    cloudView_->resetCamera();
}

void MainWindow::applyFilter()
{
    if (!originalCloud_.success || !originalCloud_.cloud) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先加载一个点云文件。"));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const PointCloudLoadResult filtered = PointCloudLoader::applyVoxelGridFilter(
        originalCloud_.cloud,
        static_cast<float>(leafSizeSpinBox_->value()),
        currentFilePath_);
    QApplication::restoreOverrideCursor();

    if (!filtered.success) {
        QMessageBox::warning(this, QStringLiteral("滤波失败"), filtered.errorMessage);
        return;
    }

    currentCloud_ = filtered;
    showCloudResult(currentCloud_);
    statusBar()->showMessage(QStringLiteral("滤波完成：%1 个点").arg(currentCloud_.pointCount));
}

void MainWindow::restoreOriginalCloud()
{
    if (!originalCloud_.success) {
        return;
    }

    currentCloud_ = originalCloud_;
    showCloudResult(currentCloud_);
    statusBar()->showMessage(QStringLiteral("已恢复原始点云"));
}

void MainWindow::loadDirectory(const QString& directoryPath)
{
    currentDirectory_ = QDir(directoryPath).absolutePath();
    directoryLabel_->setText(QStringLiteral("目录：%1").arg(currentDirectory_));
    fileList_->clear();

    const QDir dir(currentDirectory_);
    const QFileInfoList files = dir.entryInfoList(
        QStringList() << QStringLiteral("*.pcd"),
        QDir::Files | QDir::Readable,
        QDir::Name | QDir::IgnoreCase);

    for (const QFileInfo& fileInfo : files) {
        auto* item = new QListWidgetItem(fileInfo.fileName(), fileList_);
        item->setData(Qt::UserRole, fileInfo.absoluteFilePath());
    }

    statusBar()->showMessage(
        QStringLiteral("已扫描 %1 个 PCD 文件").arg(files.size()));

    if (files.isEmpty()) {
        QMessageBox::information(
            this,
            QStringLiteral("未找到 PCD 文件"),
            QStringLiteral("目录中没有可读取的 .pcd 文件：\n%1").arg(currentDirectory_));
    }
}

void MainWindow::loadCloudFile(const QString& filePath)
{
    statusBar()->showMessage(QStringLiteral("正在读取：%1").arg(QFileInfo(filePath).fileName()));
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const PointCloudLoadResult result = PointCloudLoader::loadPcdFile(filePath);
    QApplication::restoreOverrideCursor();

    if (!result.success) {
        cloudView_->clearCloud();
        originalCloud_ = PointCloudLoadResult();
        currentCloud_ = PointCloudLoadResult();
        currentFilePath_.clear();
        QMessageBox::warning(this, QStringLiteral("读取失败"), result.errorMessage);
        statusBar()->showMessage(QStringLiteral("读取失败"));
        return;
    }

    currentFilePath_ = filePath;
    originalCloud_ = result;
    currentCloud_ = result;
    showCloudResult(result);
    statusBar()->showMessage(QStringLiteral("读取完成：%1 个点").arg(result.pointCount));
}

void MainWindow::updateInfo(const PointCloudLoadResult& result)
{
    const QFileInfo fileInfo(result.filePath);
    fileNameLabel_->setText(QStringLiteral("文件：%1").arg(fileInfo.fileName()));
    pointCountLabel_->setText(QStringLiteral("点数：%1").arg(result.pointCount));
    originalPointCountLabel_->setText(QStringLiteral("原始点数：%1").arg(originalCloud_.pointCount));
    filteredPointCountLabel_->setText(QStringLiteral("滤波点数：%1").arg(currentCloud_.pointCount));
    boundsLabel_->setText(QStringLiteral("范围：\nX [%1, %2]\nY [%3, %4]\nZ [%5, %6]")
                              .arg(result.minPoint.x(), 0, 'f', 3)
                              .arg(result.maxPoint.x(), 0, 'f', 3)
                              .arg(result.minPoint.y(), 0, 'f', 3)
                              .arg(result.maxPoint.y(), 0, 'f', 3)
                              .arg(result.minPoint.z(), 0, 'f', 3)
                              .arg(result.maxPoint.z(), 0, 'f', 3));
}

void MainWindow::showCloudResult(const PointCloudLoadResult& result)
{
    cloudView_->showCloud(result.cloud);
    updateInfo(result);
}

QString MainWindow::defaultPcdDirectory() const
{
    const QStringList candidates = {
        QDir::current().absoluteFilePath(QStringLiteral("Resource/PCD")),
        QCoreApplication::applicationDirPath() + QStringLiteral("/Resource/PCD"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../Resource/PCD"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../../Resource/PCD"),
        QStringLiteral("D:/code/PointCloud/Resource/PCD")
    };

    for (const QString& candidate : candidates) {
        const QDir dir(candidate);
        if (dir.exists()) {
            return dir.absolutePath();
        }
    }

    return QDir::current().absoluteFilePath(QStringLiteral("Resource/PCD"));
}
