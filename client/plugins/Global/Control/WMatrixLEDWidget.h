#ifndef WMATRIXLEDWIDGET_H
#define WMATRIXLEDWIDGET_H

#include "WWidget.h"

class WMatrixLED;
class WMatrixLEDWidget: public WWidget
{
public:
    WMatrixLEDWidget(QWidget *parent = nullptr);

    void init(const QList<QPair<QString, QString>> &idDescList, const QMap<int, QString> &uValueDescMap);
    void resetDefaultStatus();
    void setValue(const QVariant &value);
    QVariant value();

private:
    QGridLayout *ledGridLayout;
    QMap<QString, QList<WMatrixLED*>> mIdRowLedMap; // 以id为键，储存每一行的LED
};

#endif // WMATRIXLEDWIDGET_H
