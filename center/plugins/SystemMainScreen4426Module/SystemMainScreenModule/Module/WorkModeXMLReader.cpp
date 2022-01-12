#include "WorkModeXMLReader.h"

#include <QDebug>
#include <QDir>
#include <QMessageBox>

#include "PlatformInterface.h"

WorkModeXMLReader::WorkModeXMLReader() {}

WorkSystem WorkModeXMLReader::getWorkSystem()
{
    QString path = PlatformConfigTools::configBusiness("TaskProcess/ManualProcess.xml");

    WorkSystem workSystems;
    parseManualProcessFile(path, workSystems);

    return workSystems;
}

void WorkModeXMLReader::parseManualProcessFile(const QString& filePath, WorkSystem& workSystems)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, QString("警告"), QString("工作模式文件打开失败:%1,%2").arg(filePath).arg(file.errorString()), QString("确定"));
        return;
    }

    QString errorMsg;
    int errorRow = -1;
    int errorCol = -1;
    QDomDocument dom;
    if (!dom.setContent(&file, &errorMsg))
    {
        file.close();
        QMessageBox::critical(nullptr, QString("警告"),
                              QString("解析XML文件错误:%1,第%3第%4列，%2").arg(filePath).arg(errorRow).arg(errorCol).arg(errorMsg), QString("确定"));
        return;
    }

    file.close();

    QDomNode workSysNode = dom.documentElement().firstChild();
    while (!workSysNode.isNull())
    {
        QDomElement workSysElement = workSysNode.toElement();
        if (workSysElement.nodeName().toUpper() == "CK")
        {
            parseWorkSystemNode(workSysNode, workSystems.ckCommands);
        }
        else if (workSysElement.nodeName().toUpper() == "DT")
        {
            parseWorkSystemNode(workSysNode, workSystems.dtCommands);
        }
        workSysNode = workSysNode.nextSibling();
    }
}

void WorkModeXMLReader::parseWorkSystemNode(const QDomNode& workSysNode, QList<ControlMode>& commands)
{
    QDomNode child = workSysNode.firstChild();
    while (!child.isNull())
    {
        QDomElement childElement = child.toElement();

        if (childElement.nodeName() == "WorkSystem")
        {
            ControlMode item;
            item.id = childElement.attribute("id").toUInt();
            item.desc = childElement.attribute("desc").trimmed();
            item.maxdp = childElement.attribute("maxdp").toUInt();

            commands.append(item);
        }
        child = child.nextSibling();
    }
}
