#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include <QObject>
#include <QWidget>
#include <QComboBox>

class MyComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit MyComboBox(QWidget *parent = nullptr);

signals:
    void clicked(void);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
};

#endif // MYCOMBOBOX_H
