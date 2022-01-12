#include "LinksXmlReader.h"

#include <QDir>
#include <QFile>

#include "DevProtocol.h"
#include "PlatformInterface.h"

LinksXmlReader::LinksXmlReader() {}

QMap<LinkType, LinkConfig> LinksXmlReader::getLinkConfig(QString& errorMsg)
{
    mLinkDirPath = PlatformConfigTools::configBusiness("Links");

    QString path = mLinkDirPath + QDir::separator() + "LinkConfig.xml";
    QMap<LinkType, LinkConfig> links;
    parseLinkConfigFile(path, links, errorMsg);

    return links;
}

void LinksXmlReader::parseLinkConfigFile(const QString& filePath, QMap<LinkType, LinkConfig>& links, QString& errorMsg) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorMsg = QString("链路配置文件打开失败").arg(filePath);
        return;
    }

    int errorRow = -1;
    int errorCol = -1;
    QDomDocument dom;
    if (!dom.setContent(&file, &errorMsg))
    {
        file.close();
        errorMsg = QString("解析XML文件错误:%1,第%3第%4列，%2").arg(filePath).arg(errorRow).arg(errorCol).arg(errorMsg);
        return;
    }
    file.close();

    auto linkNode = dom.documentElement().firstChild();
    while (!linkNode.isNull())
    {
        auto linkElement = linkNode.toElement();
        if (linkElement.nodeName() == "Link")
        {
            LinkConfig link;
            link.linkType = (LinkType)linkElement.attribute("id").toInt();
            link.name = linkElement.attribute("name").trimmed();

            auto modeNode = linkElement.firstChild();
            while (!modeNode.isNull())
            {
                auto modeElement = modeNode.toElement();
                if (modeElement.nodeName() == "Mode")
                {
                    LinkMode linkMode;
                    linkMode.id = modeElement.attribute("id").toInt();
                    linkMode.name = modeElement.attribute("name");

                    // 根据path解析该链路下的模式配置
                    auto filePath = modeElement.attribute("path");
                    if (filePath.isEmpty())
                    {
                        errorMsg = QString("链路配置文件错误，%1%2链路需要配置path属性").arg(link.name).arg(linkMode.name);
                        modeNode = modeNode.nextSibling();
                        continue;
                    }
                    auto path = mLinkDirPath + QDir::separator() + filePath;
                    parseModeFile(path, linkMode, errorMsg);

                    link.modeMap[linkMode.id] = linkMode;
                }
                modeNode = modeNode.nextSibling();
            }

            links[link.linkType] = link;
        }
        linkNode = linkNode.nextSibling();
    }
}

void LinksXmlReader::parseModeFile(const QString& filePath, LinkMode& linkMode, QString& errorMsg)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorMsg = QString("链路配置文件打开失败:%1").arg(filePath);
        return;
    }

    int errorRow = -1;
    int errorCol = -1;
    QDomDocument dom;
    if (!dom.setContent(&file, &errorMsg))
    {
        file.close();
        errorMsg = QString("解析XML文件错误:%1,第%3第%4列，%2").arg(filePath).arg(errorRow).arg(errorCol).arg(errorMsg);
        return;
    }
    file.close();

    auto rootEle = dom.documentElement();
    parseModeNode(rootEle, linkMode);
}

void LinksXmlReader::parseModeNode(const QDomElement& modeEle, LinkMode& mode)
{
    if (modeEle.tagName() != "Mode")
    {
        return;
    }

    mode.name = modeEle.attribute("name");

    auto node = modeEle.firstChild();
    while (!node.isNull())
    {
        auto element = node.toElement();
        if (!element.isNull())
        {
            auto tagName = element.tagName();
            if (tagName == "if")
            {
                QList<CommandGroup> cmdGroupList;
                parseIfNode(element, cmdGroupList);

                mode.cmdGroups.append(cmdGroupList);
            }
        }

        node = node.nextSibling();
    }
}

void LinksXmlReader::parseIfNode(const QDomElement& ifEle, QList<CommandGroup>& cmdGroupList)
{
    if (ifEle.tagName() != "if")
    {
        return;
    }

    // OR表示多个命令组 用相同的命令
    QString condition = ifEle.attribute("condition").trimmed();
    QStringList itemList = condition.replace("(", "").replace(")", "").split("OR");

    QSet<QSet<QString>> curConditionSet;
    for (const auto& item : itemList)
    {
        auto conditionItemSet = item.trimmed().split("|").toSet();
        curConditionSet << conditionItemSet;
    }

    CommandGroup cmdGroup;
    cmdGroup.conditionSet = curConditionSet;

    auto node = ifEle.firstChild();
    while (!node.isNull())
    {
        auto element = node.toElement();
        if (!element.isNull())
        {
            auto tagName = element.tagName();
            if (tagName == "if")
            {
                QList<CommandGroup> subCmdGroupList;
                parseIfNode(element, subCmdGroupList);

                for (auto& cmdGroup : subCmdGroupList)  // 子节点if添加当前if节点的条件
                {
                    QSet<QSet<QString>> newConditionSet;
                    for (auto set : cmdGroup.conditionSet)
                    {
                        for (auto newSet : curConditionSet)
                        {
                            QSet<QString> tempSet = set;
                            tempSet.unite(newSet);
                            newConditionSet << tempSet;
                        }
                    }
                    cmdGroup.conditionSet = newConditionSet;
                }

                cmdGroupList.append(subCmdGroupList);  // 添加子节点命令组
            }
            else if (tagName == "Command")
            {
                Command command;
                parseCommandNode(element, command);

                cmdGroup.commands << command;
            }
        }

        node = node.nextSibling();
    }

    // if节点下有命令才保存
    if (!cmdGroup.commands.isEmpty())
    {
        cmdGroupList << cmdGroup;
    }
}

void LinksXmlReader::parseCommandNode(const QDomElement& commandEle, Command& command)
{
    if (commandEle.tagName() != "Command")
    {
        return;
    }

    command.deviceId = commandEle.attribute("deviceId").toInt(nullptr, 16);
    command.deviceName = commandEle.attribute("deviceName");
    command.modeId = commandEle.attribute("modeId").toInt(nullptr, 16);
    command.operatorInfo = commandEle.attribute("operatorInfo");

    // 过程控制命令
    if (commandEle.hasAttribute("cmdId"))
    {
        command.cmdId = commandEle.attribute("cmdId").toInt();
        command.cmdType = DevMsgType::ProcessControlCmd;
    }
    // 单元命令
    else if (commandEle.hasAttribute("unitId"))
    {
        command.cmdId = commandEle.attribute("unitId").toInt();
        command.cmdType = DevMsgType::UnitParameterSetCmd;
    }

    // 解析命令需要发送的参数
    QMap<QString, QVariant> parms;
    parseParamNode(commandEle, parms);
    command.paramMap = parms;
}

void LinksXmlReader::parseParamNode(const QDomElement& domEle, QMap<QString, QVariant>& parms)
{
    auto paramNode = domEle.firstChild();
    while (!paramNode.isNull())
    {
        auto paramEle = paramNode.toElement();
        if (paramEle.nodeName() == "Param")
        {
            auto paramId = paramEle.attribute("id");
            auto value = paramEle.attribute("value");

            parms[paramId] = value;
        }
        paramNode = paramNode.nextSibling();
    }
}
