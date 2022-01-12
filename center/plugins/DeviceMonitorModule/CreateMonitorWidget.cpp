#include "CreateMonitorWidget.h"
#include "CppMicroServicesUtility.h"
#include "DeviceProcessService.h"
#include "GlobalData.h"
#include "PlatformConfigTools.h"
#include "SystemMonitor4424.h"
#include "SystemMonitor4426.h"
#include <QMap>
#include <QString>

QMap<QString, QMap<QString, int>> CreateMonitorWidget::getSystemNameMap()
{
    QFile file(PlatformConfigTools::configBusiness("DeviceMonitor/SystemMapSystemNum.xml"));
    if (!file.open(QFile::ReadOnly))
    {
        auto msg = QString("加载系统号映射表错误：%1:%2").arg(file.fileName()).arg(file.errorString());
        qWarning() << msg;
        return {};
    }

    QDomDocument doc;
    QString error;
    int errorRow = -1;
    int errorCol = -1;
    if (!doc.setContent(&file, &error, &errorRow, &errorCol))
    {
        auto msg = QString("加载系统号映射表错误：%1,在%2第%3行第%4列").arg(error).arg(file.fileName()).arg(errorRow).arg(errorCol);
        qWarning() << msg;
        return {};
    }
    QMap<QString, QMap<QString, int>> systemMap;
    QMap<QString, int> systemNameMap;
    QString currentProjectName;
    auto root = doc.documentElement();
    parseXMl(&root, currentProjectName, systemNameMap);

    systemMap[currentProjectName] = systemNameMap;

    return systemMap;
}

void CreateMonitorWidget::parseXMl(QDomElement* docElem, QString& currentProjectName, QMap<QString, int>& systemMap)
{
    QDomNode node = docElem->firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            QString tagName = element.tagName();
            if (tagName == "currentProjectID")
            {
                currentProjectName = element.attribute("ID", "4424");
            }
            else if (tagName == "map")
            {
                auto name = element.attribute("name");
                auto sysID = element.attribute("sysID").toInt();
                systemMap[name] = sysID;
            }
            parseXMl(&element, currentProjectName, systemMap);
        }
        node = node.nextSibling();
    }
}

QWidget* CreateMonitorWidget::createWidget(const QString& subWidgetName, std::shared_ptr<DeviceProcessService> service)
{
    /* 20210402 待定 这里可以考虑每次都加载一下 wp?? */
    static auto g_systemNameMap = getSystemNameMap();

    if (g_systemNameMap.isEmpty())
    {
        return nullptr;
    }

    auto projectName = g_systemNameMap.firstKey();
    auto systemMap = g_systemNameMap[projectName];
    auto find = systemMap.find(subWidgetName);
    /* 未找到当前分系统 */
    if (find == systemMap.end())
    {
        return nullptr;
    }

    auto m_systemMap = GlobalData::getSystemMap();
    auto systemMapFind = m_systemMap.find(find.value());
    /* 当前分系统没找到对应的数据 */
    if (systemMapFind == m_systemMap.end())
    {
        return nullptr;
    }

    if (projectName == "4426")
    {
        auto systemWidget = new SystemMonitor4426;
        systemWidget->setSystem(systemMapFind.value());  //设置该分系统的数据
        systemWidget->initSystemWidget();                //初始化该分系统界面

        if (service != nullptr)
        {
            //发送单元数据
            QObject::connect(systemWidget, &SystemMonitor4426::signalsUnitDeviceJson, service.get(), &DeviceProcessService::slotsUnitDeviceJson);
            //发送命令数据
            QObject::connect(systemWidget, &SystemMonitor4426::signalsCmdDeviceJson, service.get(), &DeviceProcessService::slotsCmdDeviceJson);
            //发送参数宏
            QObject::connect(systemWidget, &SystemMonitor4426::signalsParaMacroDeviceJson, service.get(),
                             &DeviceProcessService::slotsParaMacroDeviceJson);
            //命令响应
            QObject::connect(service.get(), &DeviceProcessService::signalsCMDResponceJson, systemWidget,
                             &SystemMonitor4426::slotsControlCmdResponseJson);
            //过程控制命令结果上报
            QObject::connect(service.get(), &DeviceProcessService::signalsCMDResultJson, systemWidget, &SystemMonitor4426::slotsCmdResultJson);
            //状态上报
            //                                QObject::connect(service.get(), &DeviceProcessService::signalsStatusReportJson,
            //                                systemWidget,
            //                                                 &SystemMonitor::slotsStatusReportJson);
            return systemWidget;
        }
    }
    else
    {
        auto systemWidget = new SystemMonitor4424;
        systemWidget->setSystem(systemMapFind.value());  //设置该分系统的数据
        systemWidget->initSystemWidget();                //初始化该分系统界面

        if (service != nullptr)
        {
            //发送单元数据
            QObject::connect(systemWidget, &SystemMonitor4424::signalsUnitDeviceJson, service.get(), &DeviceProcessService::slotsUnitDeviceJson);
            //发送命令数据
            QObject::connect(systemWidget, &SystemMonitor4424::signalsCmdDeviceJson, service.get(), &DeviceProcessService::slotsCmdDeviceJson);
            //发送参数宏
            QObject::connect(systemWidget, &SystemMonitor4424::signalsParaMacroDeviceJson, service.get(),
                             &DeviceProcessService::slotsParaMacroDeviceJson);
            //命令响应
            QObject::connect(service.get(), &DeviceProcessService::signalsCMDResponceJson, systemWidget,
                             &SystemMonitor4424::slotsControlCmdResponseJson);
            //过程控制命令结果上报
            QObject::connect(service.get(), &DeviceProcessService::signalsCMDResultJson, systemWidget, &SystemMonitor4424::slotsCmdResultJson);
            //状态上报
            //                                QObject::connect(service.get(), &DeviceProcessService::signalsStatusReportJson,
            //                                systemWidget,
            //                                                 &SystemMonitor::slotsStatusReportJson);
            return systemWidget;
        }
    }
    return nullptr;
}
