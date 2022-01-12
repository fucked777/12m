#include "DataExchangeWidgetDelegate.h"

#include <QFontMetrics>
#include <QPainter>

DataExchangeWidgetDelegate::DataExchangeWidgetDelegate(QObject* parent)
    : QItemDelegate(parent)
{
}

void DataExchangeWidgetDelegate::setEnumEntryList(const QList<DevEnumEntry>& enumEntryList) { mEnumEntryList = enumEntryList; }

void DataExchangeWidgetDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    for (auto devEnumEntry : mEnumEntryList)
    {
        if (devEnumEntry.uValue == index.data().toString())
        {
            int i = index.data(Qt::DisplayRole).toInt();
            if (i < 0 || i >= mEnumEntryList.size())
                continue;
            auto text = mEnumEntryList.at(i).desc;
            auto fm = painter->fontMetrics();
            auto textWidth = fm.width(text);
            auto textHeight = fm.height();

            int x = (option.rect.width() - textWidth) / 2 + option.rect.x();
            int y = (option.rect.height() - textHeight) / 2 + option.rect.y();
            painter->drawText(QRect(x, y, textWidth, textHeight), text);
        }
    }

    //     int size = qMin(option.rect.width(), option.rect.height());
    //     int brightness = index.model()->data(index, Qt::DisplayRole).toInt();
    //     double radius = (size / 2.0) - (brightness / 255.0 * size / 2.0);
    //     if (radius == 0.0)
    //         return;

    //     painter->save();
    //     painter->setRenderHint(QPainter::Antialiasing, true);
    //     painter->setPen(Qt::NoPen);
    //     if (option.state & QStyle::State_Selected)
    //         painter->setBrush(option.palette.highlightedText());
}
