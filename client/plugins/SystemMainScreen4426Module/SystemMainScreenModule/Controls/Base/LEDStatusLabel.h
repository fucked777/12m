#ifndef LEDSTATUSLABEL_H
#define LEDSTATUSLABEL_H

#include "BaseWidget.h"
#include "StatusController.h"

#include <QDomElement>
#include <QLabel>
#include <QVariant>

// 状态灯控件
class LEDStatusLabel : public BaseWidget, public StatusController
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)

    struct LEDItem
    {
        QVariant value;
        Status status{ Unknown };
        QString toolTip{ "未知" };
    };

public:
    explicit LEDStatusLabel(const QDomElement& domEle, QWidget* parent = nullptr);

    void setValue(const QVariant& value);
    QVariant getValue() const;

protected:
    virtual void statusChanged(Status status) override;

private:
    void initLayout();
    void parseNode(const QDomElement& domEle);

private:
    QLabel* mLabel = nullptr;

    QMap<QVariant, LEDItem> mLedItemMap;
};

#endif  // LEDSTATUSLABEL_H
