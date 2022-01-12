#ifndef CONFIGMACROREADER_H
#define CONFIGMACROREADER_H

#include <QDomElement>
#include <QList>
#include <QVariant>

#include "DevProtocol.h"

struct ControlMode
{
    int id;        // id
    QString desc;  // 参数描述
    int maxdp;     // 参数值
};

Q_DECLARE_METATYPE(ControlMode)

struct WorkSystem
{
    QList<ControlMode> ckCommands;
    QList<ControlMode> dtCommands;
};

class WorkModeXMLReader
{
public:
    WorkModeXMLReader();

    WorkSystem getWorkSystem();

private:
    void parseManualProcessFile(const QString& filePath, WorkSystem& workSystems);
    void parseWorkSystemNode(const QDomNode& workSysNode, QList<ControlMode>& commands);
};

#endif  // ConfigMacroCONFIG_H
