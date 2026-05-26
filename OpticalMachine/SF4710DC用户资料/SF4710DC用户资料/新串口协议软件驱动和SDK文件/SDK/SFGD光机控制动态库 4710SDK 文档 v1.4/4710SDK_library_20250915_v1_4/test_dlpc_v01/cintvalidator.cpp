#include "cintvalidator.h"
#include <QDebug>

CIntValidator::CIntValidator(QWidget *parent) : QIntValidator(parent)
{

}

CIntValidator::CIntValidator(int bottom, int top, QWidget *parent) :
    QIntValidator(bottom, top, parent)
{
    m_minValue = bottom;
    m_maxValue = top;
    m_parent = qobject_cast<QLineEdit *>(parent);
    connect(m_parent, &QLineEdit::editingFinished, this, [this](){
        bool ok = false;
        int value = m_parent->text().toInt(&ok);
        if (ok == false){
            return ;
        }
        if (value < m_minValue){
            value = m_minValue;
        }else if (value > m_maxValue){
            value = m_maxValue;
        }
        m_parent->setText(QString::number(value));
    });
}

void CIntValidator::setRange(int bottom, int top)
{
    QIntValidator::setRange(bottom, top);
}

QValidator::State CIntValidator::validate(QString &input, int &pos) const
{
    if (input.isEmpty()) {
        return QIntValidator::Intermediate;
    }
    //qDebug()<<"input="<<input<<"pos="<<pos;

    if (input == "-"){
        return QIntValidator::validate(input, pos);
    }

    bool ok = false;
    int intValue = input.toInt(&ok);
    if (ok == false) {
        return QIntValidator::Invalid;
    }

    if (intValue > top()) {
        intValue = top();
    }
    input = QString::number(intValue, 10);

    return QIntValidator::validate(input, pos);
}

void CIntValidator::fixup(QString &input) const
{
    if (input.isEmpty()){
        return ;
    }

    qDebug()<<"fixup input="<<input;

    bool ok = false;
    int intValue = input.toInt(&ok);
    if (ok == false){
        return ;
    }

    if (intValue < bottom()){
        input = QString("%1").arg(bottom());
    }
}
