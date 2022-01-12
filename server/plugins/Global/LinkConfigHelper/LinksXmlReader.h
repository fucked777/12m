#ifndef LINKSXMLREADER_H
#define LINKSXMLREADER_H

#include <QDomElement>
#include <QVariant>

#include "LinkConfigMessageDefine.h"

// 链路配置文件读取
class LinksXmlReader
{
public:
    LinksXmlReader();

    QMap<LinkType, LinkConfig> getLinkConfig(QString& errorMsg);

private:
    void parseLinkConfigFile(const QString& filePath, QMap<LinkType, LinkConfig>& links, QString& errorMsg) const;

public:
    static void parseModeFile(const QString& filePath, LinkMode& linkMode, QString& errorMsg);
    static void parseModeNode(const QDomElement& domEle, LinkMode& mode);
    static void parseIfNode(const QDomElement& ifEle, QList<CommandGroup>& cmdGroupList);
    static void parseCommandNode(const QDomElement& commandEle, Command& command);
    static void parseParamNode(const QDomElement& domEle, QMap<QString, QVariant>& parms);

private:
    QString mLinkDirPath;
};

#endif  // LINKSXMLREADER_H
