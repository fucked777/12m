#ifndef SALLITEXMLREADER_H
#define SALLITEXMLREADER_H
#include <QString>

#include <QDomElement>

#include "ProtocolXmlTypeDefine.h"
#include <QList>
#include <QString>

//用于保存当前模式下卫星的公共参数
class SalliteModeInfoItem
{
public:
    SalliteModeInfoItem() {}

    QString m_devName;    //设备名称
    QString m_deviceID;   //当前设备的ID
    int m_unitID{ 0 };    //当前对应的单元信息
    QString m_groupName;  //当前参数分组
    QList<ParameterAttribute> m_paramList;
};

class SalliteModeInfo
{
public:
    int m_modeID{ 0 };                           //当前卫星的模式
    QString m_modeName;                          //模式名称
    QList<SalliteModeInfoItem> m_modeParamList;  // 当前模式的参数
    QMap<QString, DevEnum> m_enumMap;            // 参数对应的枚举Map key为字段名
};

class SalliteModeXMLReader
{
public:
    SalliteModeXMLReader();

    /* 用于加载对应系统配置 主要用于界面显示*/
    void loadConfigMacroWorksystemXML(QList<SalliteModeInfo>& map, int& maxWorkMode, int& stationBz);

private:
    void loadSalliteXmlPath(SalliteModeInfo& data, const QString& xmlPath);

    void loadSalliteEnumXmlPath(SalliteModeInfo& data, const QString& xmlPath);

    void parseEntry(QList<DevEnumEntry>& list, const QDomNode& element);

    /* 用于加载配置宏的json序列 */
    void loadJsonComand(const QString& filePath, QMap<QString, QString>);
    QDomElement GetFileDomElement(QString& filePath);

    void parseParameterAttribute(QList<ParameterAttribute>& list, const QDomNode& element);
};

#endif  // SALLITEXMLREADER_H
