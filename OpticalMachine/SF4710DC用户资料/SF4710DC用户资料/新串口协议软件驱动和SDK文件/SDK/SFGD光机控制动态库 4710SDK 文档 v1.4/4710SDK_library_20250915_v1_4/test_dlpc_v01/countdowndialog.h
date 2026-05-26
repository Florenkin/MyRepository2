#ifndef COUNTDOWNDIALOG_H
#define COUNTDOWNDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QTimer>

namespace Ui {
class CountDownDialog;
}

class QCloseEvent;

class CountDownDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CountDownDialog(QWidget *parent = nullptr);
    ~CountDownDialog() override;

    void setStartTime(int sec);
    int getCurrentTime(void);

    void setInfor(const QString & info);

    void startCount(void);
    void stopCount(void);

    void setResetState(void);

protected:
    void closeEvent(QCloseEvent * event) override;

private:
    void showCount(void);

private:
    Ui::CountDownDialog *ui;

    QTimer m_timer;
    int m_currentCount = 0;
};

#endif // COUNTDOWNDIALOG_H
