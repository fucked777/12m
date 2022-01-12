#ifndef DYNAMICSTATUSLABEL_H
#define DYNAMICSTATUSLABEL_H

#include "StatusLabel.h"

#include <QVariant>

class DynamicStatusLabel : public StatusLabel
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)
    Q_PROPERTY(QString dealType READ getDealType WRITE setDealType)
public:
    explicit DynamicStatusLabel(const QDomElement& domEle, QWidget* parent = nullptr);

    void setValue(const QVariant& value);
    QVariant getValue() const;

    void setDealType(const QString& dealType);
    QString getDealType() const;

private:
    void parseNode(const QDomElement& domEle);
    QString d_dealType;
};

#endif  // DYNAMICSTATUSLABEL_H
