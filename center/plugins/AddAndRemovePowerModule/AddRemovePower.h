#ifndef ADDREMOVEPOWER_H
#define ADDREMOVEPOWER_H

#define AR_POWER_OBJ_ID 0xFFCC0200

#include <PowerControlType.h>

#include <QMap>
#include <QWidget>

namespace cppmicroservices
{
    class BundleContext;
}

class PowerStrip;
class QNetworkAccessManager;
class QNetworkReply;
class AddRemovePowerImpl;
class AddRemovePowerImpl;
class AddRemovePower : public QWidget
{
    Q_OBJECT

public:
    AddRemovePower(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~AddRemovePower();
    void start();

protected:
    void timerEvent(QTimerEvent* event);
    void offDev();
    void offPlug();
    void onPlug();
    void queryPlug();

private:
    void initUI(const QList<CenterBodyInfo>& centerBody);
    // signals:
    //    void sendMessage(const PackMessage&);
signals:
    /*单元参数设置命令*/
    void signalsUnitDeviceJson(QString json);

    /*发送过程控制命令*/
    void signalsCmdDeviceJson(QString json);

private:
    /* 中心体设备 */
    void allCenterOn();
    void allCenterOff();
    /* 单个插排 */
    void itemPlugConditionsOn(const QString& key);
    void itemPlugConditionsOff(const QString& key);
    /* S功放加去电 */
    void shpOnOff(int data);

    void plugStatus(QNetworkReply* reply);

private:
    AddRemovePowerImpl* m_impl;
};
#endif  // ADDREMOVEPOWER_H
