#ifndef PARAMMACROXMLREADER_H
#define PARAMMACROXMLREADER_H

#include "ProtocolXmlTypeDefine.h"
#include <QDomDocument>
#include <QList>
#include <QString>

//用于保存当前模式下卫星的公共参数
struct ParamMacroModeInfoItem
{
    QString m_devName;                      // 设备名称
    QString m_deviceID;                     // 当前设备的ID
    QString m_groupName;                    // 当前参数分组
    QList<ParameterAttribute> m_paramList;  // 点频的参数
    QString enumGroupId;                    // 当前对应的组id，用于去查找枚举

    bool isDynamicParam = false;  // 是否动态参数
    QString dynamicGroupId;  // 动态参数组id，必须和卫星管理中选择的组id一样，下发参数宏的时候通过这个组id去获取卫星管理中该组选择需要下发的第几组参数
};

struct DeviceParamMacroInfoItem
{
    QString m_devName;      //设备名称
    QString m_stationName;  //站名称
    QString m_deviceID;     //当前设备的ID
    QString m_groupName;    //当前参数分组
    QMap<int, QString> m_unitNameMap;
    QMap<int, QList<ParameterAttribute>> m_unitConfigMap;
    QMap<int, QMap<int, QList<ParameterAttribute>>> m_unitRecycleConfigMap;
    QMap<QString, DevEnum> m_unitEnumMap;  // 点频参数对应的枚举Map key为字段名
};

struct ParamMacroModeInfo
{
    SystemWorkMode m_modeID{ SystemWorkMode::NotDefine };  //当前卫星的模式
    QString m_modeName;                                    //模式名称
    QList<ParamMacroModeInfoItem> m_frequencyList;         // 点频参数的配置
    QMap<QString, QMap<QString, DevEnum>> m_frequencyEnumMap;  // 点频参数对应的枚举Map key为字段名  QMap<组id, QMap<枚举名, 枚举信息>>
    QList<DeviceParamMacroInfoItem> m_deviceList;              //设备参数序列
};

struct ParamMacroXMLReader
{
public:
    ParamMacroXMLReader();
    /* 用于加载对应系统配置 主要用于界面显示*/
    void loadParamMacroWorksystemXML(QList<ParamMacroModeInfo>& map);

public:
private:
    void loadParamMacroXmlPath(ParamMacroModeInfo& data, const QString& xmlPath);

    void parseDevice(QList<DeviceParamMacroInfoItem>& list, const QDomNode& docElem);

    // 加载模式下的设备参数
    void loadDeviceItemParamMacroXmlPath(ParamMacroModeInfo& data, const QDomNode& docElem);

    //加载每一个分机的设置参数
    void loadDeviceItemParamMacroItem(DeviceParamMacroInfoItem& data, const QString& xmlPath);

    void loadDeviceUnitParamMacro(DeviceParamMacroInfoItem& data, const QDomNode& docElem);

    void loadDeviceUnitEnum(DeviceParamMacroInfoItem& data, QString xmlPath);

    // 加载点频下的参数配置
    void loadFrequencyNode(QList<ParamMacroModeInfoItem>& list, const QDomNode& docElem);

    // 加载点频下的枚举配置
    void loadFrequencyEnumXmlPath(ParamMacroModeInfo& data, QString xmlPath);

    void parseEntry(QList<DevEnumEntry>& list, const QDomNode& element);

    // 解析ParameterAttribute
    void parseParameterAttribute(ParameterAttribute& base, const QDomNode& element);
    // 解析ParameterAttributeList
    void parseParameterAttributeList(QList<ParameterAttribute>& list, const QDomNode& element);

    QDomElement GetFileDomElement(QString& filePath);
};

#endif  // PARAMMACROXMLREADER_H
