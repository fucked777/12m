#ifndef EnumStatusLabel_H
#define EnumStatusLabel_H

#include "StatusLabel.h"

#include <QVariant>

class EnumStatusLabel : public StatusLabel
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)
public:
    explicit EnumStatusLabel(const QDomElement& domEle, QWidget* parent = nullptr, const int deviceId = 0, const int modeId = 0);

    void setValue(const QVariant& value);
    QVariant getValue() const;

    int getDeviceId() const;
    int getModeId() const;

private:
    void initMap();

private:
    void parseNode(const QDomElement& domEle);
    int d_deviceId;
    int d_modeId;
    int d_unitId;
    int d_targetId;
    QString d_paramId;
    QString d_enumId;
    QString d_defaultDesc;

    QMap<QString, QString> d_enums;
};

#endif  // EnumStatusLabel_H
