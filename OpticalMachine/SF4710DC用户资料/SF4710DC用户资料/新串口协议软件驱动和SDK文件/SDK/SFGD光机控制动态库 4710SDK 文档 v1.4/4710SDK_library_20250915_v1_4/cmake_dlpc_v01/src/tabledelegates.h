#ifndef TABLEDELEGATES_H_
#define TABLEDELEGATES_H_


#include <QObject>
#include <QWidget>
#include <QStyledItemDelegate>
#include <QComboBox>
#include <QCheckBox>
#include <QApplication>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>

class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ComboBoxDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    void setItems(const QStringList & items) {m_items = items;}
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(option)
        Q_UNUSED(index)

        QComboBox *comboBox = new QComboBox(parent);
        comboBox->addItems(m_items);
        return comboBox;
    }

    //设置模型数据
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        QComboBox *comboBox = static_cast<QComboBox *>(editor);
        if (comboBox) {
            model->setData(index, comboBox->currentText().trimmed(), Qt::EditRole);
        }
    }

private:
    QStringList m_items;
};

class CheckBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CheckBoxDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        bool checked = index.model()->data(index, Qt::DisplayRole).toBool();

        QStyleOptionButton checkBoxOption;
        checkBoxOption.state |= QStyle::State_Enabled;
        if (checked)
            checkBoxOption.state |= QStyle::State_On;
        else
            checkBoxOption.state |= QStyle::State_Off;

        checkBoxOption.rect = option.rect;

        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override
    {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                bool checked = index.model()->data(index, Qt::DisplayRole).toBool();
                model->setData(index, !checked, Qt::EditRole);
                return true;
            }
        }
        //忽略双击事件
        if (event->type() == QEvent::MouseButtonDblClick) {
            return true;
        }
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }
};

#endif // TABLEDELEGATES_H_
