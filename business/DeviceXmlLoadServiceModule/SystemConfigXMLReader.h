#ifndef SYSTEMCONFIGXMLREADER_H
#define SYSTEMCONFIGXMLREADER_H

#include "ProtocolXmlTypeDefine.h"

#include <QDomElement>
#include <QObject>

class SystemConfigXMLReader : public QObject
{
    Q_OBJECT
public:
    explicit SystemConfigXMLReader(QObject* parent = nullptr);

    void loadSystemConfigXML(QString filePath, QMap<int, System>& sysCfgMap, const QMap<QString, UnitXmlDataMap>& unitXmlMap,
                             const QMap<QString, CmdXMLData>& cmdXMlMap);
    QMap<int, System> getSystemConfigXML(QString filePath, const QMap<QString, UnitXmlDataMap>& unitXmlMap,
                                         const QMap<QString, CmdXMLData>& cmdXMlMap);

private:
    void parseSystemConfigXML(QDomElement* docElem, QMap<int, System>& systemMap, int& sysId, int& devId, int& extenId, int& modeType,
                              const QMap<QString, UnitXmlDataMap>& unitXmlMap, const QMap<QString, CmdXMLData>& cmdXMlMap);
signals:

public slots:
};

#endif  // SYSTEMCONFIGXMLREADER_H
