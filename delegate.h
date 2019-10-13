#ifndef DELEGATE_H
#define DELEGATE_H

#include <QComboBox>
#include <QStyledItemDelegate>
#include <QAbstractItemView>
#include <QDebug>

class PopupItemDelegate: public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize s = QStyledItemDelegate::sizeHint(option, index);
        s.setHeight(s.height()*5/4);
        return s;
    }
};

#endif // DELEGATE_H
