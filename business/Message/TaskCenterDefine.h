#ifndef TASKCENTERDEFINE_H
#define TASKCENTERDEFINE_H

#include <QDataStream>
#include <QList>
#include <QMap>
#include <QObject>

/* 任务中心 方向类型 */
enum class TCDirectionType
{
    Unknown = 0x0,
    MACC = 0x01, /* 测控中心 */
};
/* 任务中心 帧格式 */
enum class TCFrameFormat
{
    Unknown = 0x0,
    PDXP = 0x01, /* 测控中心 */
};

struct TaskCenterData
{
public:
    QString centerName;                                    // 中心名称
    QString centerCode;                                    // 中心代号
    QString uacAddr;                                       // UAC地址
    TCFrameFormat frameFormat{ TCFrameFormat::Unknown };   // 帧格式
    TCDirectionType direType{ TCDirectionType::Unknown };  // 方向类型
    bool isUsedLinkMonitor{ false };                       // 是否链监
    bool isUsed{ true };                                   // 是否使用
};
Q_DECLARE_METATYPE(TaskCenterData)

using TaskCenterMap = QMap<QString, TaskCenterData>;
using TaskCenterList = QList<TaskCenterData>;

#endif  // TASKCENTERDEFINE_H
