#ifndef PROTOCOLXMLCMDREADER_H
#define PROTOCOLXMLCMDREADER_H

#include "ProtocolXmlTypeDefine.h"

#include <QDir>
#include <QDomElement>
#include <QFileInfoList>

class ProtocolXMLCmdReader
{
public:
    explicit ProtocolXMLCmdReader();
    //加载命令xml
    QMap<QString, CmdXMLData> loadCmdProtocolXml(QString filePath);
    //遍历xml文件名
    void loadProtocolXml(const QString& filePath, QFileInfoList& infoList);

private:
    //遍历枚举xml
    void openAndParseCmdEnumXml(QString path, CmdEnumMap& enumMap);
    void parseEnumXML(QDomElement* docElem, QMap<QString, DevEnum>& ret_Map, QString& parentName);
    //遍历参数xml
    bool openAndParseCmdProtocolXML(QString path, CmdAttrMap& ret_Map);

    void parseCmdProtocolXML(QDomElement* docElem, CmdAttribute& cmdAttribute, QString& parentName, int& recycleId,
                             QMap<int, UnitMultiTarget>& targetMap, QMap<QString, QList<ParameterAttribute>>& statusGroupMap,  //普通状态分组
                             QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiStatusGroupMap,                         //多目标状态分组
                             QMap<QString, QList<ParameterAttribute>>& settingGroupMap,                  //普通设置参数分组
                             QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiSettingGroupMap,  //多目标设置参数分组
                             QMap<QString, UnitLayout>& unitLayoutMap);
    void parseParameterAttribute(ParameterAttribute& base, const QDomElement& element, QMap<int, UnitMultiTarget>& targetMap,
                                 QMap<QString, QList<ParameterAttribute>>& statusGroupMap,                  //普通状态分组
                                 QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiStatusGroupMap,  //多目标状态分组
                                 QMap<QString, QList<ParameterAttribute>>& settingGroupMap,                 //普通设置参数分组
                                 QMap<int, QMap<QString, QList<ParameterAttribute>>>& multiSettingGroupMap  //多目标设置参数分组
    );
};

#endif  // PROTOCOLXMLCMDREADER_H
