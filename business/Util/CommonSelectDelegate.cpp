#include "CommonSelectDelegate.h"

CommonSelectDelegate::CommonSelectDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}
CommonSelectDelegate::~CommonSelectDelegate()
{

}

void CommonSelectDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem tempOption(option);
    tempOption.palette.setColor(QPalette::HighlightedText, index.data(Qt::ForegroundRole).value<QColor>());
    QStyledItemDelegate::paint(painter,tempOption,index);
}
