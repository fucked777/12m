#include "HeartBeatXMLReader.h"

#include <QDebug>
#include <QDir>
#include <QMessageBox>

#include "PlatformInterface.h"

HeartBeatXMLReader::HeartBeatXMLReader() {}

bool HeartBeatXMLReader::getHeartbeatConfig(HeartBeatInfo& heartbeatInfo)
{
    QString filePath = PlatformConfigTools::configBusiness("Heartbeat/HeartbeatConfig.xml");

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(nullptr, QString("警告"), QString("加载主备心跳配置:%1,%2").arg(filePath, file.errorString(), QString("确定")));
        return false;
    }

    QString errorMsg;
    int errorRow = -1;
    int errorCol = -1;
    QDomDocument dom;
    if (!dom.setContent(&file, &errorMsg))
    {
        file.close();
        QMessageBox::warning(nullptr, QString("警告"),
                             QString("解析XML文件错误:%1,第%3第%4列，%2").arg(filePath).arg(errorRow).arg(errorCol).arg(errorMsg), QString("确定"));
        return false;
    }

    file.close();

    QDomNode heatbeatConfig = dom.documentElement().firstChild();
    while (!heatbeatConfig.isNull())
    {
        QDomElement heatbeatConfigElement = heatbeatConfig.toElement();

        if (heatbeatConfigElement.nodeName() == "AIP")
        {
            heartbeatInfo.aIP = heatbeatConfigElement.text().trimmed();
        }
        else if (heatbeatConfigElement.nodeName() == "BIP")
        {
            heartbeatInfo.bIP = heatbeatConfigElement.text().trimmed();
        }
        else if (heatbeatConfigElement.nodeName() == "VIP")
        {
            heartbeatInfo.vIP = heatbeatConfigElement.text().trimmed();
        }
        else if (heatbeatConfigElement.nodeName() == "IsUsed")
        {
            heartbeatInfo.isUsed = QVariant(heatbeatConfigElement.text().trimmed()).toBool();
        }
        else if (heatbeatConfigElement.nodeName() == "MasterToSlave")
        {
            heartbeatInfo.masterToSlave = PlatformConfigTools::configBusiness(heatbeatConfigElement.text().trimmed());
        }
        else if (heatbeatConfigElement.nodeName() == "SlaveToMaster")
        {
            heartbeatInfo.slaveToMaster = PlatformConfigTools::configBusiness(heatbeatConfigElement.text().trimmed());
        }
        heatbeatConfig = heatbeatConfig.nextSibling();
    }
    return true;
}
