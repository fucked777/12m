#include "ButtonDelegate.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

ButtonDelegate::ButtonDelegate(QObject *parent) : QItemDelegate(parent) {}

void ButtonDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    QPair<QStyleOptionButton *, QStyleOptionButton *> *buttons = m_btns.value(index);
    if (!buttons) {
        QStyleOptionButton *button1 = new QStyleOptionButton();
        // button1->rect = option.rect.adjusted(4, 4, -(option.rect.width() / 2 + 4) , -4); //
        button1->text = "install";
        button1->state |= QStyle::State_Enabled;

        QStyleOptionButton *button2 = new QStyleOptionButton();
        // button2->rect = option.rect.adjusted(button1->rect.width() + 4, 4, -4, -4);
        button2->text = "uninstall";
        button2->state |= QStyle::State_Enabled;
        buttons = new QPair<QStyleOptionButton *, QStyleOptionButton *>(button1, button2);
        (const_cast<ButtonDelegate *>(this))->m_btns.insert(index, buttons);
    }
    buttons->first->rect  = option.rect.adjusted(4, 4, -(option.rect.width() / 2 + 4), -4);  //
    buttons->second->rect = option.rect.adjusted(buttons->first->rect.width() + 4, 4, -4, -4);

    painter->save();

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }
    painter->restore();
    QApplication::style()->drawControl(QStyle::CE_PushButton, buttons->first, painter);
    QApplication::style()->drawControl(QStyle::CE_PushButton, buttons->second, painter);
}

bool ButtonDelegate::editorEvent(QEvent *event,
                                 QAbstractItemModel *model,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index)
{

    Q_UNUSED(model);
    Q_UNUSED(option);

    if (event->type() == QEvent::MouseButtonPress) {

        QMouseEvent *e = (QMouseEvent *)event;

        if (m_btns.contains(index)) {
            QPair<QStyleOptionButton *, QStyleOptionButton *> *btns = m_btns.value(index);
            if (btns->first->rect.contains(e->x(), e->y())) {
                btns->first->state |= QStyle::State_Sunken;
            }
            else if (btns->second->rect.contains(e->x(), e->y())) {
                btns->second->state |= QStyle::State_Sunken;
            }
        }
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *e = (QMouseEvent *)event;

        if (m_btns.contains(index)) {
            QPair<QStyleOptionButton *, QStyleOptionButton *> *btns = m_btns.value(index);
            if (btns->first->rect.contains(e->x(), e->y())) {
                btns->first->state &= (~QStyle::State_Sunken);
                emit signal_install(index.row());
            }
            else if (btns->second->rect.contains(e->x(), e->y())) {
                btns->second->state &= (~QStyle::State_Sunken);
                emit signal_uninstall(index.row());
            }
        }
    }
    return true;
}
