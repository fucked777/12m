#ifndef LINKCONFIGMESSAGEDEFINE_H
#define LINKCONFIGMESSAGEDEFINE_H

#include <QString>

#include "DevProtocol.h"
#include "PlanRunMessageDefine.h"

#include <QSet>


struct Command
{
    DevMsgType cmdType{ DevMsgType::Unknown };  // 命令类型，单元参数设置命令或者过程控制命令

    int deviceId{ 0 };                 // 设备ID
    QString deviceName;                // 设备名称
    int modeId{ 0 };                   // 模式id
    int cmdId{ 0 };                    // 命令或者单元号
    QString operatorInfo;              // 该命令执行的操作
    QMap<QString, QVariant> paramMap;  // QMap<参数Id, 参数值>
};

// 链路配置文件命令组节点信息
struct CommandGroup
{
    QSet<QSet<QString>> conditionSet;  // 条件
    QList<Command> commands;           // 满足条件时执行的命令
};

// 链路配置文件模式节点信息
struct LinkMode
{
    int id{ 0 };
    QString name;
    QList<CommandGroup> cmdGroups;
};

// 链路
struct LinkConfig
{
    LinkType linkType{ LinkType::Unknown };  // 链路类型
    QString name;                            // 链路名称

    QMap<int, LinkMode> modeMap;  // 该链路含有的模式 QMap<模式id, 模式>
};

using LinkConfigMap = QMap<LinkType, LinkConfig>;

#endif  // LINKCONFIGMESSAGEDEFINE_H
