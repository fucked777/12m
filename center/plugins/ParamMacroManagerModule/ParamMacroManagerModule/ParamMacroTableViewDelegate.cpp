#include "ParamMacroTableViewDelegate.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDialog>
#include <QItemDelegate>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QStyleOption>
ParamMacroTableViewDelegate::ParamMacroTableViewDelegate(QObject* parent)
    : QItemDelegate(parent)
{
    m_pbtn = new QPushButton();
}

QWidget* ParamMacroTableViewDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(parent);
    Q_UNUSED(option);
    Q_UNUSED(index);
    return nullptr;
}

void ParamMacroTableViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QVector<QStyleOptionButton*> vtStyleButton;
    if (!m_mapStyleBtn.contains(index) || m_mapStyleBtn[index].size() == 0)
    {
        {
            QStyleOptionButton* pbtnStyle = new QStyleOptionButton();

            pbtnStyle->text = "查看";
            vtStyleButton.push_back(pbtnStyle);
        }
        {
            QStyleOptionButton* pbtnStyle = new QStyleOptionButton();
            pbtnStyle->text = "编辑";
            vtStyleButton.push_back(pbtnStyle);
        }
        {
            QStyleOptionButton* pbtnStyle = new QStyleOptionButton();
            pbtnStyle->text = "删除";

            vtStyleButton.push_back(pbtnStyle);
        }
        (const_cast<ParamMacroTableViewDelegate*>(this))->m_mapStyleBtn[index] = vtStyleButton;
    }
    else
    {
        vtStyleButton = m_mapStyleBtn[index];
    }

    int iTopSpaceHeight = static_cast<float>(option.rect.height() - m_iBtnHeight) / 2;
    int iX = m_iIntervalWidth + option.rect.x();
    int iY = option.rect.y() + iTopSpaceHeight;
    for (auto pBtn : vtStyleButton)
    {
        pBtn->rect = QRect(iX, iY, m_iBtnWidth, m_iBtnHeight);
        pBtn->state |= QStyle::State_Enabled;
        iX = iX + m_iBtnWidth + m_iIntervalWidth;
    }
    //选择颜色
    QStyleOptionViewItem itemOption(option);
    itemOption.palette.setColor(QPalette::HighlightedText, index.data(Qt::ForegroundRole).value<QColor>());
    QItemDelegate::paint(painter, itemOption, index);

    for (auto pBtn : vtStyleButton)
    {
        m_pbtn->style()->drawControl(QStyle::CE_PushButtonLabel, pBtn, painter, m_pbtn);
    }
}

bool ParamMacroTableViewDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    Q_UNUSED(model);
    Q_UNUSED(option);
    if (event->type() == QEvent::MouseButtonPress)  // 鼠标按下
    {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (m_mapStyleBtn.contains(index))
        {
            const QVector<QStyleOptionButton*> vtStyleBtns = m_mapStyleBtn[index];
            for (auto& pBtn : vtStyleBtns)
            {
                if (pBtn->rect.contains(pMouseEvent->x(), pMouseEvent->y()))
                {
                    pBtn->state |= QStyle::State_Sunken;
                    m_pairClickedButton = { true, pBtn };
                }
            }
        }
        return true;
    }
    else if (event->type() == QEvent::MouseButtonRelease)  // 鼠标松开或者移出
    {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(event);

        if (m_mapStyleBtn.contains(index))
        {
            const QVector<QStyleOptionButton*> vtStyleBtns = m_mapStyleBtn[index];
            for (auto& pBtn : vtStyleBtns)
            {
                if (pBtn->rect.contains(pMouseEvent->x(), pMouseEvent->y()))
                {
                    pBtn->state &= (~QStyle::State_Sunken);
                    if (pBtn->text == "查看")
                    {
                        emit signal_show(index);
                    }
                    if (pBtn->text == "编辑")
                    {
                        emit signal_edit(index);
                    }
                    if (pBtn->text == "删除")
                    {
                        emit signal_del(index);
                    }

                    m_pairClickedButton = { false, nullptr };
                }
            }
        }
        return true;
    }
    else if (event->type() == QEvent::MouseMove)  // 鼠标移出, 使被点击按钮恢复状态
    {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(event);

        if (m_pairClickedButton.first && m_pairClickedButton.second)
        {
            if (!m_pairClickedButton.second->rect.contains(pMouseEvent->x(), pMouseEvent->y()))
            {
                m_pairClickedButton.second->state &= (~QStyle::State_Sunken);
                m_pairClickedButton = { false, nullptr };
            }
        }
        return true;
    }

    return false;
}
