#ifndef CENTERAIRCONDITIONERITEM_H
#define CENTERAIRCONDITIONERITEM_H

#include <QWidget>
class QLabel;
namespace Ui
{
    class CenterAirConditionerItem;
}

class CenterAirConditionerItemImpl;
class CenterAirConditionerItem : public QWidget
{
    Q_OBJECT

public:
    explicit CenterAirConditionerItem(QWidget* parent = nullptr);
    ~CenterAirConditionerItem();
    void setAddr(const QString& localAddr, const QString& desAddr, quint16 recvPort, quint16 sendPort);

private:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void timerEvent(QTimerEvent* event) override;
    void onReadyRead();

private:
    bool initSocket();
    void closeConnect();
    /* 空调机组停机故障 */
    void setSFData(quint8 data1, quint8 data2);
    /* 空调机组不停机故障 */
    void setDSFData(quint8 data1, quint8 data2);
    /* 电源状态 */
    void setDYData(quint8 data1, quint8 data2);
    /* 模式状态 */
    void setMSData(quint8 data1, quint8 data2);
    /* 内风机状态 */
    void setNFJZTData(quint8 data1, quint8 data2);
    /* 内风机档位 */
    void setNFJDWData(quint8 data1, quint8 data2);
    /* 外风机状态 */
    void setWFJZTData(quint8 data1, quint8 data2);
    /* 压缩机状态 */
    void setYSJZTData(quint8 data1, quint8 data2);
    /* 辅助电加热状态 */
    void setFZDJRZTData(quint8 data1, quint8 data2);
    /* 设置数值 */
    void setNumData(QLabel*, quint8 data1, quint8 data2);
    void setNumDataDev10(QLabel*, quint8 data1, quint8 data2);
    /* 重置数据 */
    void reset();

    /* 控制 */
    void setControlCmd(quint8 addr1, quint8 addr2, quint8 data1, quint8 data2);

private:
    Ui::CenterAirConditionerItem* ui;
    CenterAirConditionerItemImpl* m_impl;
};

#endif  // CENTERAIRCONDITIONERITEM_H
