#include "SystemCaptureProcessXmlReader.h"

#include "PlatformConfigTools.h"

#include <QFileInfo>

SystemCaptureProcessXmlReader::SystemCaptureProcessXmlReader()
{
    auto fileInfo = QFileInfo(PlatformConfigTools::configBusiness("SystemMainScreenModule/SystemCaptureProcess.xml"));
    parseXML(fileInfo.absoluteFilePath());
}

bool SystemCaptureProcessXmlReader::getCmdList(QList<CommandTemplate>& cmdList, QString* errorMsg)
{
    if (!mErrorMsg.isEmpty())
    {
        *errorMsg = mErrorMsg;
        return false;
    }

    cmdList = mCmdList;

    return true;
}

void SystemCaptureProcessXmlReader::parseXML(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        mErrorMsg = QString("系统捕获流程配置文件打开失败：%1,%2").arg(path).arg(file.errorString());
        return;
    }

    QString errorMsg;
    int errorRow = -1;
    int errorCol = -1;

    QDomDocument dom;
    if (!dom.setContent(&file, &errorMsg, &errorRow, &errorCol))
    {
        file.close();
        mErrorMsg = QString("解析XML文件错误：1,第%3第%4列，%2").arg(path).arg(errorRow).arg(errorCol).arg(errorMsg);
        return;
    }

    file.close();

    QDomElement root = dom.documentElement();

    QDomElement commandElem = root.firstChildElement();
    QString defaultDeviceId = commandElem.attribute("defaultDeviceId");
    QString defaultModeId = commandElem.attribute("defaultModeId");
    QString defaultTargetId = commandElem.attribute("defaultTargetId");

    QDomElement concreteCommandElem = commandElem.firstChildElement();

    while (!concreteCommandElem.isNull())
    {
        CommandTemplate temp;

        temp.defaultDeviceId = defaultDeviceId;
        temp.defaultModeId = defaultModeId;
        temp.defaultTargetId = defaultTargetId;
        if (concreteCommandElem.hasAttribute("targetId"))
            temp.defaultTargetId = concreteCommandElem.attribute("targetId").toInt();

        temp.name = concreteCommandElem.attribute("name");
        QString tagName = concreteCommandElem.tagName();

        QString strStaticParams = concreteCommandElem.attribute("staticParams");
        QStringList listStaticParams = strStaticParams.split(" ");
        for (auto& s : listStaticParams)
        {
            if (s.split("|").size() == 2)
            {
                QString key = s.split("|").at(0);
                QString val = s.split("|").at(1);
                temp.staticParams.insert(key, val);
            }
        }

        QString strDynamicParams = concreteCommandElem.attribute("dynamicParams");
        QStringList listDynamicParams = strDynamicParams.split(" ");
        for (auto& s : listDynamicParams)
        {
            if (s.split("|").size() == 2)
            {
                QString key = s.split("|").at(0);
                QString val = s.split("|").at(1);
                temp.dynamicParams.insert(key, val);
            }
        }

        if (tagName == "ProcessCtrl")
        {
            temp.cmdType = DevMsgType::ProcessControlCmd;
            temp.cmdIdOrUnitId = concreteCommandElem.attribute("cmdId");
        }
        else if (tagName == "UnitParam")
        {
            temp.cmdType = DevMsgType::UnitParameterSetCmd;
            temp.cmdIdOrUnitId = concreteCommandElem.attribute("unitId");
        }

        if (concreteCommandElem.hasAttribute("validIdent"))
        {
            temp.validIdent = concreteCommandElem.attribute("validIdent");
        }
        else
        {
            temp.validIdent = "-1";
        }

        QList<QDomElement> elements;
        QDomNodeList nodeList = concreteCommandElem.childNodes();
        for (int i = 0; i < nodeList.size(); i++)
        {
            elements.append(nodeList.at(i).toElement());
        }
        temp.domELems = elements;
        concreteCommandElem = concreteCommandElem.nextSiblingElement();
        mCmdList.append(temp);
    }
}
