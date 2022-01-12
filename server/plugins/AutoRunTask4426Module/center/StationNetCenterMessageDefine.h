#ifndef STATIONNETCENTERMESSAGEDEFINE_H
#define STATIONNETCENTERMESSAGEDEFINE_H

#include "LinkConfigMessageDefine.h"
#include "SystemWorkMode.h"
#include <QSet>
#include <QString>

/* 中心命令的节点信息 */
struct SNCCmdNode
{
    int id{ 0 };                      /* 命令ID */
    QString name;                     /* 命令名称 */
    QSet<SystemWorkMode> workModeSet; /* 命令在什么工作模式下发 */
    QList<CommandGroup> cmdGroups;    /* 命令分组 */
};

using SNCCmdMap = QMap<int, SNCCmdNode>;

#endif  // STATIONNETCENTERMESSAGEDEFINE_H
