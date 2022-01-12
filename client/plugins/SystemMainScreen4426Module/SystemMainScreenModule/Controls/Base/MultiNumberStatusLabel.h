#ifndef MULTINUMBERSTATUSLABEL_H
#define MULTINUMBERSTATUSLABEL_H

#include "StatusLabel.h"

#include <QVariant>

struct ParamsInfo
{
    int deviceId;
    int modeId;
    int targetId;
    int unitId;
    QString paramId;
};

class MultiNumberStatusLabel : public StatusLabel
{
    Q_OBJECT
    Q_PROPERTY(QVariant unitValue READ getUnitValue WRITE setUnitValue)
public:
    explicit MultiNumberStatusLabel(const QDomElement& domEle, QWidget* parent = nullptr, const int deviceId = 0, const int modeId = 0);

    //不缓存单元参数
    void setUnitValue(const QVariant& value);
    QVariant getUnitValue() const;

    // protected:
    //    void timerEvent(QTimerEvent* event) override;

private:
    void parseNode(const QDomElement& domEle);

private:
    QList<ParamsInfo> d_paramsInfo;
    QStringList d_values;
    QString d_curValue;
    int d_deviceId;
    int d_modeId;
};

#endif  // MULTINUMBERSTATUSLABEL_H
