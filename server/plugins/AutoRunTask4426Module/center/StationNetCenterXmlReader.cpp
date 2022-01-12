#include "StationNetCenterXmlReader.h"

#include <QDir>
#include <QFile>

#include "DevProtocol.h"
#include "LinksXmlReader.h"
#include "PlatformInterface.h"

StationNetCenterXmlReader::StationNetCenterXmlReader() {}
StationNetCenterXmlReader::~StationNetCenterXmlReader() {}

Optional<SNCCmdMap> StationNetCenterXmlReader::getLinkConfig()
{
    m_cmdDirPath = PlatformConfigTools::configBusiness("StationNetCenter");
    auto path = PlatformConfigTools::configBusiness("StationNetCenter/Cmd.xml");
    return parseLinkConfigFile(path);
}
Optional<SNCCmdMap> StationNetCenterXmlReader::parseLinkConfigFile(const QString& filePath) const
{
    using ResType = Optional<SNCCmdMap>;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        auto msg = QString("战网中心命令配置文件打开失败").arg(filePath);
        return ResType(ErrorCode::OpenFileError, msg);
    }

    SNCCmdMap sncCmdMap;
    QDomDocument domDoc;
    QString errStr;
    int errLine{ 0 };
    int errcCol{ 0 };
    if (!domDoc.setContent(&file, &errStr, &errLine, &errcCol))
    {
        errStr = QString("战网中心命令配置解析失败:%1,错误行:%2,错误列:%2").arg(errStr).arg(errLine).arg(errcCol);
        return ResType(ErrorCode::XmlParseError, errStr);
    }

    auto node = domDoc.documentElement().firstChild();
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
        if (name != "Cmd")
        {
            continue;
        }
        SNCCmdNode cmdNode;
        cmdNode.id = element.attribute("id").toInt(nullptr, 16);
        cmdNode.name = element.attribute("name").trimmed();
        /* 工作模式 */
        auto modeList = element.attribute("mode").trimmed().split("|", QString::SkipEmptyParts);
        for (auto& item : modeList)
        {
            auto workMode = SystemWorkModeHelper::systemWorkModeFromString(item);
            if (workMode != SystemWorkMode::NotDefine)
            {
                cmdNode.workModeSet << workMode;
            }
        }

        // 根据path解析该链路下的模式配置
        auto filePath = element.attribute("path");
        if (filePath.isEmpty())
        {
            continue;
        }
        auto path = Utility::dirStitchingQt(m_cmdDirPath, filePath);
        auto result = parseModeFile(cmdNode, path);
        if (!result)
        {
            return ResType(result.errorCode(), result.msg());
        }
        sncCmdMap.insert(cmdNode.id, cmdNode);
    }

    return ResType(sncCmdMap);
}

OptionalNotValue StationNetCenterXmlReader::parseModeFile(SNCCmdNode& sncCmdNode, const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        auto msg = QString("战网中心命令配置文件打开失败").arg(filePath);
        return OptionalNotValue(ErrorCode::OpenFileError, msg);
    }

    QDomDocument domDoc;
    QString errStr;
    int errLine{ 0 };
    int errcCol{ 0 };
    if (!domDoc.setContent(&file, &errStr, &errLine, &errcCol))
    {
        errStr = QString("战网中心命令配置解析失败:%1,错误行:%2,错误列:%2").arg(errStr).arg(errLine).arg(errcCol);
        return OptionalNotValue(ErrorCode::XmlParseError, errStr);
    }

    auto node = domDoc.documentElement().firstChild();
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

        if (name == "if")
        {
            QList<CommandGroup> cmdGroupList;
            LinksXmlReader::parseIfNode(element, cmdGroupList);
            sncCmdNode.cmdGroups.append(cmdGroupList);
        }
    }
    return OptionalNotValue();
}
