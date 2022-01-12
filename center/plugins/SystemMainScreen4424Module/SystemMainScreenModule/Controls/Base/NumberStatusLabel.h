#ifndef NUMBERSTATUSLABEL_H
#define NUMBERSTATUSLABEL_H

#include "StatusLabel.h"

#include <QVariant>

class NumberStatusLabel : public StatusLabel
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)
public:
    explicit NumberStatusLabel(const QDomElement& domEle, QWidget* parent = nullptr);

    void setValue(const QVariant& value);
    QVariant getValue() const;

private:
    void parseNode(const QDomElement& domEle);
};

#endif  // NUMBERSTATUSLABEL_H
