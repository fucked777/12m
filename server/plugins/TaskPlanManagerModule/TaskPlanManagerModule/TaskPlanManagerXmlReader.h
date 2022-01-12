#ifndef TASKPLANMANAGERXMLREADER_H
#define TASKPLANMANAGERXMLREADER_H

#include <QDomElement>
#include <QList>
#include <QString>

struct TaskPlanFileStorageInfo
{
    QString deviceFileRecvDir{ "PlanFileCache/RecvPlan" };    // 存放接收的设备工作计划文件路径
    QString deviceFileBackupDir{ "PlanFileCache/BakPlan" };   // 备份的设备工作计划文件路径
    QString deviceFileErrorDir{ "PlanFileCache/ErrorPlan" };  // 备份错误的设备工作计划文件路径
};

struct TaskPlanConfigCheck
{
    QList<QString> m_workUnitList;        //工作单位 26-TJ 37-BJ
    QList<QString> m_m_equipmentNoList;   //设备代号 TK-4413 TK-4424 TK-4426
    QList<QString> m_schedulingModeList;  //调度模式 SB
    QList<QString> m_workingModeList;     //工作方式 SZ LT
};
class TaskPlanManagerXmlReader
{
public:
    TaskPlanManagerXmlReader();

    bool getTaskPlanFileDirInfo(TaskPlanFileStorageInfo& taskPlanFileStorageInfo);

    bool getTaskPlanConfigCheck(TaskPlanConfigCheck& taskPlanConfigCheck);

private:
    bool parseTaskPlanStorageInfoXml(const QString& filePath, TaskPlanFileStorageInfo& taskPlanFileStorageInfo);

    bool parseTaskPlanConfigCheckXml(const QString& filePath, TaskPlanConfigCheck& taskPlanFileStorageInfo);

    // 解析Item节点
    void parseItem(const QDomElement& root, QList<QString>& link);
};

#endif  // TASKPLANMANAGERXMLREADER_H
