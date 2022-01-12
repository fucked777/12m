#include "PlatformConfigLoad.h"
#include "PlatformConfigTools.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QMap>
#include <QRegularExpression>
#include <QString>

namespace platform_config_impl
{
    /* 配置文件路径 */
    static constexpr auto gConfigPath = "./System.xml";
    static void loadMainConfig(QDomElement& element, PlatformMainConfig& info)
    {
        auto node = element.firstChild();
        while (!node.isNull())
        {
            auto isComment = node.isComment();
            auto configElement = node.toElement();
            auto name = node.nodeName();
            node = node.nextSibling();

            /* 跳过注释 */
            if (isComment)
            {
                continue;
            }

            if (name == "InstanceID")
            {
                info.instanceID = configElement.text().trimmed();
            }
            else if (name == "PlatformConfigDir")
            {
                info.platformConfigDir = configElement.text().trimmed();
            }
            else if (name == "BusinessConfigDir")
            {
                info.businessConfigDir = configElement.text().trimmed();
            }
            else if (name == "ShareConfigDir")
            {
                info.shareConfigDir = configElement.text().trimmed();
            }
            else if (name == "Name")
            {
                info.name = configElement.text().trimmed();
            }
            else if (name == "ErrorIsExit")
            {
                info.serviceInstallErrorIsExit = QVariant(configElement.text().trimmed()).toBool();
            }
        }
    }
    static void loadExtendedConfig(QDomElement& element, PlatformExtendedConfig& info)
    {
        auto node = element.firstChild();
        while (!node.isNull())
        {
            auto isComment = node.isComment();
            auto configElement = node.toElement();
            auto name = node.nodeName();
            node = node.nextSibling();

            /* 跳过注释 */
            if (isComment)
            {
                continue;
            }

            info.insert(name, configElement.text().trimmed());
        }
    }
    static void loadUIConfig(QDomElement& element, PlatformUIConfig& info)
    {
        auto node = element.firstChild();
        while (!node.isNull())
        {
            auto isComment = node.isComment();
            auto configElement = node.toElement();
            auto name = node.nodeName();
            node = node.nextSibling();

            /* 跳过注释 */
            if (isComment)
            {
                continue;
            }

            if (name == "Wizards")
            {
                info.wizards = configElement.text().trimmed();
            }
            else if (name == "Start")
            {
                info.start = configElement.text().trimmed();
            }
            else if (name == "End")
            {
                info.end = configElement.text().trimmed();
            }
            else if (name == "MainWindow")
            {
                info.mainWindow = configElement.text().trimmed();
            }
        }
    }
    static void loadConfigXml(QDomElement& root, PlatformConfigInfo& info)
    {
        auto node = root.firstChild();
        while (!node.isNull())
        {
            auto isComment = node.isComment();
            auto element = node.toElement();
            auto name = node.nodeName();
            node = node.nextSibling();

            /* 跳过注释 */
            if (isComment)
            {
                continue;
            }
            /* 主配置 */
            if (name == "Main")
            {
                loadMainConfig(element, info.mainInfo);
                continue;
            }
            /* 扩展配置 */
            else if (name == "Extended")
            {
                loadExtendedConfig(element, info.extendedInfo);
                continue;
            }
            /* UI */
            else if (name == "UI")
            {
                loadUIConfig(element, info.uiInfo);
                continue;
            }
        }
    }

}  // namespace platform_config_impl

PlatformConfigLoad::PlatformConfigLoad() {}
PlatformConfigLoad ::~PlatformConfigLoad() {}

bool PlatformConfigLoad::load()
{
    QFile configFile(platform_config_impl::gConfigPath);
    if (!configFile.open(QFile::ReadOnly))
    {
        m_errMsg = QString("打开文件错误:%1").arg(configFile.errorString());
        return false;
    }
    QDomDocument doc;
    QString errorMsg;
    int errorLine{ 0 };
    int errorColumn{ 0 };
    if (!doc.setContent(&configFile, &errorMsg, &errorLine, &errorColumn))
    {
        m_errMsg = QString("读取配置文件错误:%1:第%2行,第%3列").arg(errorMsg).arg(errorLine).arg(errorColumn);
        return false;
    }
    auto root = doc.documentElement();

    platform_config_impl::loadConfigXml(root, m_info);

    /* 这里需要检查一下
     * 20210529
     * 主配置 里面的参数绝对不能为空
     * 这里说明一下
     * 平台的默认加载配置文件路径就是gConfigPath
     * 其他的配置,全部是通过gConfigPath读取的
     * 所以其他的配置就没有默认配置的说法了
     */
    auto& mainInfo = m_info.mainInfo;
    if (mainInfo.instanceID.isEmpty())
    {
        m_errMsg = "配置文件内容错误:当前实例ID不能为空";
        return false;
    }
    if (mainInfo.platformConfigDir.isEmpty())
    {
        m_errMsg = "配置文件内容错误:平台配置文件路径不能为空";
        return false;
    }
    /*
     * businessConfigDir
     * shareConfigDir
     * 这两个属于业务配置的路径
     * 有或者没有都不影响平台启动
     */
    if (mainInfo.name.isEmpty())
    {
        mainInfo.name = "ZKHXPlatform";
    }
    /*
     * serviceInstallErrorIsExit
     * 安装错误是否退出 默认是不退出
     */
    return true;
}
