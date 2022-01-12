#include "TaskPlanManagerXmlReader.h"

#include <QDomDocument>
#include <QFile>
#include <QMessageBox>

#include "PlatformInterface.h"

TaskPlanManagerXmlReader::TaskPlanManagerXmlReader() {}

bool TaskPlanManagerXmlReader::getTaskPlanFileDirInfo(TaskPlanFileStorageInfo& taskPlanFileStorageInfo)
{
    QString path = PlatformConfigTools::configBusiness("TaskPlanManager.xml");
    return parseTaskPlanStorageInfoXml(path, taskPlanFileStorageInfo);
}

bool TaskPlanManagerXmlReader::getTaskPlanConfigCheck(TaskPlanConfigCheck& taskPlanConfigCheck)
{
    QString path = PlatformConfigTools::configBusiness("TaskPlanConfigCheck.xml");
    return parseTaskPlanConfigCheckXml(path, taskPlanConfigCheck);
}

bool TaskPlanManagerXmlReader::parseTaskPlanStorageInfoXml(const QString& filePath, TaskPlanFileStorageInfo& taskPlanFileStorageInfo)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(nullptr, QString("警告"), QString("任务计划存放信息文件打开失败:%1,%2").arg(filePath).arg(file.errorString()),
                                 QString("确定"));
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

    QDomNode node = dom.documentElement().firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (element.nodeName() == "DevPlanFileRecvDir")
        {
            taskPlanFileStorageInfo.deviceFileRecvDir = element.text();
        }
        if (element.nodeName() == "DevPlanFileBakDir")
        {
            taskPlanFileStorageInfo.deviceFileBackupDir = element.text();
        }
        if (element.nodeName() == "DevPlanFileErrorDir")
        {
            taskPlanFileStorageInfo.deviceFileErrorDir = element.text();
        }

        node = node.nextSibling();
    }

    return true;
}

bool TaskPlanManagerXmlReader::parseTaskPlanConfigCheckXml(const QString& filePath, TaskPlanConfigCheck& taskPlanConfigCheck)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(nullptr, QString("警告"), QString("任务计划存放信息文件打开失败:%1,%2").arg(filePath).arg(file.errorString()),
                                 QString("确定"));
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

    QDomNode node = dom.documentElement().firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();

        if (element.nodeName() == "工作单位")
        {
            parseItem(element, taskPlanConfigCheck.m_workUnitList);
        }
        if (element.nodeName() == "设备代号")
        {
            parseItem(element, taskPlanConfigCheck.m_m_equipmentNoList);
        }
        if (element.nodeName() == "调度模式")
        {
            parseItem(element, taskPlanConfigCheck.m_schedulingModeList);
        }
        if (element.nodeName() == "工作方式")
        {
            parseItem(element, taskPlanConfigCheck.m_workingModeList);
        }
        node = node.nextSibling();
    }

    return true;
}

void TaskPlanManagerXmlReader::parseItem(const QDomElement& root, QList<QString>& link)
{
    QDomNode node = root.firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            auto tagName = element.tagName();
            auto text = element.text();
            if (QString("Item") == tagName)
            {
                link.append(text.trimmed());
            }
        }
        node = node.nextSibling();
    }
}
