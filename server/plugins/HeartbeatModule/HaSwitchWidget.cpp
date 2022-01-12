#include "HaSwitchWidget.h"
#include "CppMicroServicesUtility.h"
#include "GlobalData.h"
#include "HeartBeatXMLReader.h"
#include "HeartbeatThread.h"
#include "ui_HaSwitchWidget.h"
#include <QHostAddress>
#include <QMessageBox>
#include <QNetworkAddressEntry>
#include <QNetworkInterface>

#ifdef TEST_MASTER_SLAVE
#include "HeartbeatMessageSerialize.h"
#include "RedisHelper.h"
#endif

HaSwitchWidget::HaSwitchWidget(cppmicroservices::BundleContext /*context*/, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::HaSwitchWidget)
{
    ui->setupUi(this);

    connect(ui->changeMaster, &QPushButton::clicked, this, &HaSwitchWidget::changeMaster);
    connect(ui->changeSlave, &QPushButton::clicked, this, &HaSwitchWidget::changeSlave);
    m_timerID = startTimer(1000);

    HeartBeatInfo heartbeatInfo;
    HeartBeatXMLReader heartbeatXmlReader;
    heartbeatXmlReader.getHeartbeatConfig(heartbeatInfo);
    m_masterToSlave = heartbeatInfo.masterToSlave;
    m_slaveToMaster = heartbeatInfo.slaveToMaster;
}

HaSwitchWidget::~HaSwitchWidget()
{
    if (m_timerID != -1)
    {
        killTimer(m_timerID);
        m_timerID = -1;
    }
    delete ui;
}

void HaSwitchWidget::timerEvent(QTimerEvent* /*event*/)
{
    m_isMaster = GlobalData::masterSlave();
    ui->ha_info->setText(m_isMaster ? "主机" : "备机");
}

void HaSwitchWidget::changeMaster()
{
#ifdef TEST_MASTER_SLAVE
    HeartbeatMessage testInfo;
    testInfo.currentMasterIP = "localhost";
    testInfo.isUsed = true;
    testInfo.serverID = "Server";
    testInfo.status = MasterSlaveStatus::Master;
    GlobalData::setMasterSlaveInfo(testInfo);
    /* Redis推送 */
    QString jsonData;
    jsonData << testInfo;
    RedisHelper::getInstance().setData("HA_STATUS", jsonData);
    return;
#endif

    if (m_slaveToMaster.isEmpty())
    {
        QMessageBox::warning(this, "错误", "配置文件错误,主机切换备机脚本错误", "确认");
        return;
    }

    // 当前是备机 -> 主机
    if (m_isMaster)
    {
        QMessageBox::information(this, "提示", "当前已是主机", "确认");
        return;
    }

    auto tempDate = GlobalData::getMasterSlaveChangeTime();
    if(GlobalData::currentDateTime().toMSecsSinceEpoch() - tempDate.toMSecsSinceEpoch() < 60 * 1000)
    {
        QMessageBox::information(this, "提示", "主备主动切换时间间隔需要间隔1分钟", "确认");
        return;
    }

    QProcess process;
    process.start(m_slaveToMaster);

    process.waitForStarted();
    process.waitForFinished();

    QMessageBox::information(this, "提示", "主机切换指令已下发", "确认");
    GlobalData::setMasterSlaveChangeTime();
    qWarning() << process.readAllStandardOutput();

}

void HaSwitchWidget::changeSlave()
{
#ifdef TEST_MASTER_SLAVE
    HeartbeatMessage testInfo;
    testInfo.currentMasterIP = "localhost";
    testInfo.isUsed = true;
    testInfo.serverID = "Server";
    testInfo.status = MasterSlaveStatus::Slave;
    GlobalData::setMasterSlaveInfo(testInfo);
    /* Redis推送 */
    QString jsonData;
    jsonData << testInfo;
    RedisHelper::getInstance().setData("HA_STATUS", jsonData);
    return;
#endif
    if (m_masterToSlave.isEmpty())
    {
        QMessageBox::warning(this, "错误", "配置文件错误,主机切换备机脚本错误", "确认");
        return;
    }

    if(!m_isMaster)
    {
        QMessageBox::information(this, "提示", "当前已是备机", "确认");
    }
    auto tempDate = GlobalData::getMasterSlaveChangeTime();
    if(GlobalData::currentDateTime().toMSecsSinceEpoch() - tempDate.toMSecsSinceEpoch() < 60 * 1000)
    {
        QMessageBox::information(this, "提示", "主备主动切换时间间隔需要间隔1分钟", "确认");
        return;
    }

    QProcess process;
    process.start(m_masterToSlave);

    process.waitForStarted();
    process.waitForFinished();

    QMessageBox::information(this, "提示", "备机切换指令已下发", "确认");
    GlobalData::setMasterSlaveChangeTime();
    qWarning() << process.readAllStandardOutput();
}
