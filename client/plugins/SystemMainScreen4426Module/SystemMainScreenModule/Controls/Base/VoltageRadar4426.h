#ifndef VoltageRadar4426_H
#define VoltageRadar4426_H

#include "qcustomplot.h"
#include <QFrame>
#include <QTimer>
#include <QWidget>

class QDomElement;
class VoltageRadar4426Impl;
/* S Ka的当前方位俯仰电压构成的点 */
class VoltageRadar4426 : public QWidget
{
    Q_OBJECT
public:
    struct PointInfo
    {
        int unitId;
        QString x_point;
        QString y_point;
        bool isVisible{ true };
    };

    // 0x1011是ACU的设备ID，因为4426的ACU设备ID写死了
    explicit VoltageRadar4426(const QDomElement& domEle, QWidget* parent = nullptr, const int deviceId = 0x1011, const int modeId = 0xFFFF);
    ~VoltageRadar4426();
    void setSPoint(double x, double y);
    void setKaRPoint(double x, double y);
    void setKaDPoint(double x, double y);
    void setSVisible(bool visible);
    void setKaRVisible(bool visible);
    void setKaDVisible(bool visible);
    void resetValue(bool);

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    void initUI();
    void updateStarMap();
    void parseNode(const QDomElement& domEle);
    void parseItemNode(const QDomElement& domEle, PointInfo& info);

private:
    VoltageRadar4426Impl* m_impl;
    int d_deviceId;
    int d_modeId;
    PointInfo d_sPointInfo;
    PointInfo d_kaRPointInfo;
    PointInfo d_kaDPointInfo;
};

#endif  // VoltageRadar4426_H
