#ifndef VoltageRadar4424_H
#define VoltageRadar4424_H

#include "qcustomplot.h"
#include <QFrame>
#include <QTimer>
#include <QWidget>

class QDomElement;
class VoltageRadar4424Impl;
/* S Ka的当前方位俯仰电压构成的点 */
class VoltageRadar4424 : public QWidget
{
    Q_OBJECT
public:
    explicit VoltageRadar4424(const QDomElement& domEle, QWidget* parent = nullptr);
    ~VoltageRadar4424();
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

private:
    VoltageRadar4424Impl* m_impl;
};

#endif  // VoltageRadar4424_H
