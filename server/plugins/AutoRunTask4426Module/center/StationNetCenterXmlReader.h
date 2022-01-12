#ifndef STATIONNETCENTERXMLREADER_H
#define STATIONNETCENTERXMLREADER_H

#include "StationNetCenterMessageDefine.h"
#include "Utility.h"
#include <QDomElement>
#include <QVariant>

// 链路配置文件读取
class StationNetCenterXmlReader
{
public:
    StationNetCenterXmlReader();
    ~StationNetCenterXmlReader();
    Optional<SNCCmdMap> getLinkConfig();

private:
    Optional<SNCCmdMap> parseLinkConfigFile(const QString& filePath) const;
    OptionalNotValue parseModeFile(SNCCmdNode& sncCmdNode, const QString& filePath) const;

private:
    QString m_cmdDirPath;
};

#endif  // STATIONNETCENTERXMLREADER_H
