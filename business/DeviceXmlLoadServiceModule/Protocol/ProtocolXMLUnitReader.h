#ifndef PROTOCOLXMLREADER_H
#define PROTOCOLXMLREADER_H

#include "ProtocolXmlTypeDefine.h"

#include <QDir>
#include <QDomElement>
#include <QFileInfoList>

class QDomElement;
class ProtocolXMLUnitReader
{
public:
    explicit ProtocolXMLUnitReader();

    //加载单元xml
    QMap<QString, UnitXmlDataMap> loadUnitProtocolXml(QString filePath);
    //遍历xml文件名
    void loadProtocolXml(const QString& filePath, QFileInfoList& infoList);

private:
    //遍历枚举xml
    void openAndParseUnitEnumXml(QString path, UnitXmlDataMap& unitXmlDataMap);
    void parseEnumXML(QDomElement* docElem, QMap<QString, DevEnum>& ret_Map, QString& parentName);
    //遍历参数xml
    bool openAndParseUnitProtocolXML(QString path, UnitXmlDataMap& unitXmlDataMap);

    //新的单元解析
    void parseNewUnitProtocolXML(QDomElement* docElem, QMap<int, ParameterAttribute>& commonSettingMap,
                                 QMap<QString, QMap<int, ParameterAttribute>>& commonMultiSettingMap, QMap<int, ParameterAttribute>& commonStatusMap,
                                 QMap<QString, QMap<int, ParameterAttribute>>& commonMultiStatusMap);

    //新的单元分组解析
    void parseNewUnitGroupProtocolXML(QDomElement* docElem, QMap<QString, UnitLayout>& unitLayoutMap, QMap<int, UnitMultiTarget>& targetMap,
                                      QMap<int, ParameterAttribute>& newCommonSettingMap,
                                      QMap<int, QMap<int, ParameterAttribute>>& newCommonMultiSettingMap,
                                      QMap<int, ParameterAttribute>& newCommonStatusMap,
                                      QMap<int, QMap<int, ParameterAttribute>>& newCommonMultiStatusMap,
                                      QMap<QString, QList<ParameterAttribute>>& statusGroupMap,                   //普通状态分组
                                      QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiStatusGroupMap,   //多目标状态分组
                                      QMap<QString, QList<ParameterAttribute>>& settingGroupMap,                  //普通设置参数分组
                                      QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiSettingGroupMap,  //多目标设置参数分组
                                      QStringList& list);

    //解析Xml 只解析result   内部区分过程还是控制 解析分组
    void parseNewQueryResultGroupBaseAttribute(const QDomElement& element, QMap<int, UnitMultiTarget>& targetMap,  //多目标的数据类型
                                               QMap<int, ParameterAttribute>& newCommonSettingMap,
                                               QMap<int, QMap<int, ParameterAttribute>>& newCommonMultiSettingMap,
                                               QMap<int, ParameterAttribute>& newCommonStatusMap,
                                               QMap<int, QMap<int, ParameterAttribute>>& newCommonMultiStatusMap,
                                               QMap<QString, QList<ParameterAttribute>>& statusGroupMap,                  //普通状态分组
                                               QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiStatusGroupMap,  //多目标状态分组
                                               QMap<QString, QList<ParameterAttribute>>& settingGroupMap,                 //普通设置参数分组
                                               QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiSettingGroupMap  //多目标设置参数分组
    );
};

#endif  // PROTOCOLXMLREADER_H
