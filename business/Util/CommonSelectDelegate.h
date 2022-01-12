#ifndef COMMONSELECTDELEGATE_H
#define COMMONSELECTDELEGATE_H
#include <QStyledItemDelegate>

/* TableView行选中的颜色优先级高于文字的颜色会导致选中后文字颜色被覆盖
 * 此类是为了解决此问题的
 */
class CommonSelectDelegate:public QStyledItemDelegate
{
public:
    explicit CommonSelectDelegate(QObject *parent = nullptr);
    ~CommonSelectDelegate();

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif  // COMMONSELECTDELEGATE_H
