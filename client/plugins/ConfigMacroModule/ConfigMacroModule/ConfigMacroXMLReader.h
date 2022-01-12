#ifndef CONFIGMACROREADER_H
#define CONFIGMACROREADER_H

#include <QDomElement>

#include "ConfigMacroXmlWorkModeDefine.h"

// 读取生成配置宏界面和主界面手动控制的配置文件数据(配置宏界面和主界面手动控制界面共用同一个配置文件)
class ConfigMacroXMLReader
{
public:
    ConfigMacroXMLReader();

    bool getConfigMacroWorkMode(QList<ConfigMacroWorkMode>& configMacroWorkModes, QString& stationName);

private:
    bool parseConfigMacroFile(const QString& filePath, QList<ConfigMacroWorkMode>& configMacroWorkModes, QString& stationName);
    void parseItemNode(const QDomNode& workModeNode, QList<Item>& items);
    void parseEnumNode(const QDomNode& itemNode, QList<EnumEntry>& enums);
};

#endif  // ConfigMacroCONFIG_H
