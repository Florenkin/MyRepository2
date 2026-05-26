#ifndef CINTVALIDATOR_H
#define CINTVALIDATOR_H

#include <QIntValidator>
#include <QWidget>
#include <QPointer>
#include <QLineEdit>

class CIntValidator : public QIntValidator
{
    Q_OBJECT
public:
    explicit CIntValidator(QWidget *parent = nullptr);
    CIntValidator(int bottom, int top, QWidget * parent = nullptr);

protected:
    void setRange(int bottom, int top) override;
    QValidator::State validate(QString &input, int &pos) const override;
    void fixup(QString &input) const override;

private:
    QPointer<QLineEdit> m_parent;

    int m_minValue = 0;
    int m_maxValue = 0;
};

#endif // CINTVALIDATOR_H
