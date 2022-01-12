#ifndef COMBOBOXSTATUSLABEL_H
#define COMBOBOXSTATUSLABEL_H

#include <QDomElement>
#include <QVariant>

#include "StatusLabel.h"

// 枚举类型状态标签控件
class ComboBoxStatusLabel : public StatusLabel
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)

public:
    explicit ComboBoxStatusLabel(const QDomElement& domEle, QWidget* parent = nullptr);

    void setValue(const QVariant& value);
    QVariant getValue() const;

private:
    void parseNode(const QDomElement& domEle);

private:
    QMap<QVariant, QString> mEnumMap;
};

#endif  // COMBOBOXSTATUSLABEL_H
