#include "countdowndialog.h"
#include "ui_countdowndialog.h"

#include <QTime>
#include <QCloseEvent>

CountDownDialog::CountDownDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CountDownDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Tips"));
    this->setFixedSize(400, 100);
    this->setAttribute(Qt::WA_DeleteOnClose);

    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, [this](){
        m_currentCount--;
        if (m_currentCount <= 0){
            stopCount();
            return ;
        }
        showCount();
    });
}

CountDownDialog::~CountDownDialog()
{
    delete ui;
}

void CountDownDialog::setStartTime(int sec)
{
    m_currentCount = sec;
}

int CountDownDialog::getCurrentTime()
{
    return m_currentCount;
}

void CountDownDialog::setInfor(const QString &info)
{
    ui->label_tips->setText(info);
}

void CountDownDialog::startCount()
{
    showCount();
    m_timer.start();
    this->setModal(true);
    this->show();
}

void CountDownDialog::stopCount()
{
    m_currentCount = 0;
    m_timer.stop();
    this->hide();
}

void CountDownDialog::setResetState()
{
    m_timer.stop();
    ui->label_tips->hide();
    ui->label_time->setText(tr("Please wait for the system to restart"));
    QTimer::singleShot(10000, this, [this](){ stopCount(); });
}

void CountDownDialog::closeEvent(QCloseEvent *event)
{
    event->ignore();
}

void CountDownDialog::showCount()
{
    QString timeStr = QTime(0, 0, 0, 0).addSecs(m_currentCount).toString("hh:mm:ss");
    ui->label_time->setText(timeStr);
}
