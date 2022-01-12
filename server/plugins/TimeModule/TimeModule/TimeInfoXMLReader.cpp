#include "TimeInfoXMLReader.h"

#include "PlatformInterface.h"

#include <QDebug>
#include <QDir>
#include <QDomElement>
#include <QMessageBox>

TimeInfoXMLReader::TimeInfoXMLReader() { mFilePath = PlatformConfigTools::configBusiness("TimeConfig/TimeConfig.xml"); }

bool TimeInfoXMLReader::getTimeConfigInfo(TimeConfigInfo& timeConfigInfo, QString* errorMsg) const
{
    return parseTimeConfigInfo(mFilePath, timeConfigInfo, errorMsg);
}

bool TimeInfoXMLReader::saveTimeConfigInfo(const TimeConfigInfo& timeConfigInfo, QString* errorMsg) const
{
    return saveTimeConfigInfo(timeConfigInfo, mFilePath, errorMsg);
}

bool TimeInfoXMLReader::parseTimeConfigInfo(const QString& filePath, TimeConfigInfo& timeConfigInfo, QString* errorMsg) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(nullptr, QString("警告"),
                                 QString("时间配置信息文件打开失败:%1,%2").arg(filePath).arg(file.errorString(), QString("确定")));
        return false;
    }

    QString tempErrorMsg;
    int errorRow = -1;
    int errorCol = -1;
    QDomDocument dom;
    if (!dom.setContent(&file, &tempErrorMsg, &errorRow, &errorCol))
    {
        file.close();
        *errorMsg = QString("解析XML文件错误:%1,第%2行第%3列，%4").arg(filePath).arg(errorRow).arg(errorCol).arg(tempErrorMsg);
        return false;
    }

    file.close();

    auto node = dom.documentElement().firstChild();
    while (!node.isNull())
    {
        auto ele = node.toElement();

        if (ele.nodeName() == "UsedTimeType")
        {
            timeConfigInfo.usedTimeType = (TimeType)ele.attribute("type", "1").toInt();
        }
        else if (ele.nodeName() == "NTPMasterServerAddr")
        {
            timeConfigInfo.ntpMasterServerAddr = QHostAddress(ele.attribute("ip"));
        }
        else if (ele.nodeName() == "NTPSlaveServerAddr")
        {
            timeConfigInfo.ntpSlaveServerAddr = QHostAddress(ele.attribute("ip"));
        }
        else if (ele.nodeName() == "MaxJumpInterval")
        {
            timeConfigInfo.maxJumpInterval = ele.attribute("value", "5").toInt();
        }
        else if (ele.nodeName() == "LongTimeNotUpdated")
        {
            timeConfigInfo.longTimeNotUpdatedInterval = ele.attribute("value", "5").toInt();
        }

        node = node.nextSibling();
    }

    return true;
}

bool TimeInfoXMLReader::saveTimeConfigInfo(const TimeConfigInfo& timeConfigInfo, const QString& filePath, QString* errorMsg) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        *errorMsg = QString("时间配置信息文件打开失败:%1,%2").arg(filePath).arg(file.errorString());
        return false;
    }

    QString tmpeErrorMsg;
    int errorRow = -1;
    int errorCol = -1;
    QDomDocument dom;
    if (!dom.setContent(&file, &tmpeErrorMsg, &errorRow, &errorCol))
    {
        file.close();

        *errorMsg = QString("解析XML文件错误:%1,第%2行第%3列，%4").arg(filePath).arg(errorRow).arg(errorCol).arg(tmpeErrorMsg);
        return false;
    }
    file.close();

    auto parent = dom.documentElement();
    auto node = parent.firstChild();
    while (!node.isNull())
    {
        auto ele = node.toElement();

        if (ele.nodeName() == "UsedTimeType")
        {
            auto newNode = ele;
            newNode.setAttribute("type", static_cast<int>(timeConfigInfo.usedTimeType));

            parent.replaceChild(newNode, ele);
        }
        else if (ele.nodeName() == "NTPMasterServerAddr")
        {
            auto newNode = ele;
            newNode.setAttribute("ip", timeConfigInfo.ntpMasterServerAddr.toString());

            parent.replaceChild(newNode, ele);
        }
        else if (ele.nodeName() == "NTPSlaveServerAddr")
        {
            auto newNode = ele;
            newNode.setAttribute("ip", timeConfigInfo.ntpSlaveServerAddr.toString());

            parent.replaceChild(newNode, ele);
        }
        else if (ele.nodeName() == "MaxJumpInterval")
        {
            auto newNode = ele;
            newNode.setAttribute("value", timeConfigInfo.maxJumpInterval);

            parent.replaceChild(newNode, ele);
        }
        else if (ele.nodeName() == "LongTimeNotUpdated")
        {
            auto newNode = ele;
            newNode.setAttribute("value", timeConfigInfo.longTimeNotUpdatedInterval);

            parent.replaceChild(newNode, ele);
        }

        node = node.nextSibling();
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        *errorMsg = QString("时间配置信息文件打开失败:%1,%2").arg(filePath).arg(file.errorString());
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF_8");
    dom.save(stream, 4, QDomNode::EncodingFromTextStream);
    file.close();

    return true;
}
