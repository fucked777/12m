#ifndef DATAEXCHANGEWIDGETDELEGATE_H
#define DATAEXCHANGEWIDGETDELEGATE_H

#include "ProtocolXmlTypeDefine.h"
#include <QItemDelegate>

class DataExchangeWidgetDelegate : public QItemDelegate
{
public:
    DataExchangeWidgetDelegate(QObject* parent = nullptr);

    void setEnumEntryList(const QList<DevEnumEntry>& enumEntryList);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
    QList<DevEnumEntry> mEnumEntryList;
};

#endif  // DATAEXCHANGEWIDGETDELEGATE_H
