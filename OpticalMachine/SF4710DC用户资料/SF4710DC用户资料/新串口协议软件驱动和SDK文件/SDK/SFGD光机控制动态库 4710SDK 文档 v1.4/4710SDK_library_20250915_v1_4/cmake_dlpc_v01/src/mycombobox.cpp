#include "mycombobox.h"
#include <QMouseEvent>

MyComboBox::MyComboBox(QWidget *parent) : QComboBox(parent)
{

}

void MyComboBox::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        emit clicked();  //触发clicked信号
    }
    QComboBox::mousePressEvent(event);
}
