#include "CenterAirConditionerMonitor.h"
#include "CppMicroServicesUtility.h"
#include "Endian.h"
#include "PlatformInterface.h"
#include "QssCommonHelper.h"
#include "ui_CenterAirConditionerMonitor.h"
#include <QSettings>
#include <QUdpSocket>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
#include <QNetworkDatagram>
#endif

class CenterAirConditionerMonitorImpl
{
public:
    cppmicroservices::BundleContext context;
};
CenterAirConditionerMonitor::CenterAirConditionerMonitor(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , m_impl(new CenterAirConditionerMonitorImpl)
    , ui(new Ui::CenterAirConditionerMonitor)
{
    m_impl->context = context;
    ui->setupUi(this);
    //初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    QString dbConnnectPath = PlatformConfigTools::configBusiness("CenterAirConditionerConfig/Air.ini");
    QSettings setting(dbConnnectPath, QSettings::IniFormat);

    /*
     *  注：现在空调的NPort IP已被更改为192.20.1.11  方便进入网站
     */
    auto localIP = setting.value("Local/localIP", "192.20.1.76").toString();
    /* 空调1 */
    auto desIP_1 = setting.value("AirConfig_1/desIP", "192.20.1.49").toString();
    auto sendPort_1 = setting.value("AirConfig_1/sendPort", 65534).toUInt();
    auto recvPort_1 = setting.value("AirConfig_1/recvPort", 65533).toUInt();

    /* 空调1 */
    auto desIP_2 = setting.value("AirConfig_2/desIP", "192.20.1.49").toString();
    auto sendPort_2 = setting.value("AirConfig_2/sendPort", 65534).toUInt();
    auto recvPort_2 = setting.value("AirConfig_2/recvPort", 65533).toUInt();

    ui->air1->setAddr(localIP, desIP_1, recvPort_1, sendPort_1);
    ui->air2->setAddr(localIP, desIP_2, recvPort_2, sendPort_2);
}

CenterAirConditionerMonitor::~CenterAirConditionerMonitor()
{
    delete ui;
    delete m_impl;
}
