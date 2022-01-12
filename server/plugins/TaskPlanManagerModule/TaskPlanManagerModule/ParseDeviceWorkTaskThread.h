#ifndef PARSEDEVICETASKXML_H
#define PARSEDEVICETASKXML_H

#include "TaskPlanMessageDefine.h"

#include "TaskPlanManagerXmlReader.h"
#include <QDomElement>
#include <QThread>

class QDir;
class QFileInfo;

// 定时解析设备工作计划的xml文件
class ParseDeviceWorkTaskThread : public QThread
{
    Q_OBJECT
public:
    ParseDeviceWorkTaskThread(QObject* parent = nullptr);

    void stop();

protected:
    void run() override;

private:
    // 初始化存储文件夹
    void initStorageDir();

    // 解析文件名称
    bool parseFileName(const QFileInfo& fileInfo, DeviceWorkTaskFileInfo& deviceWorkTaskFileInfo, QString& error);

    // 解析任务计划文件
    void parseTaskPlanXml(const QString& filePath, DeviceWorkTaskList& deviceWorkTaskList);
    // 解析任务节点
    void parseTaskNode(const QDomElement& root, DeviceWorkTask& deviceWorkTask);
    // 解析目标节点
    void parseTargetNode(const QDomElement& root, DeviceWorkTaskTarget& target);
    // 解析目标时间节点
    void parseTargetTimeNode(const QDomElement& root, DeviceWorkTaskTarget& target);
    // 解析目标中心节点
    void parseTargetCenterNode(const QDomElement& root, DeviceWorkTaskTarget& target);
    // 解析链路节点
    // 这个比较特殊 当只要有一个节点的工作体质为空 就认为是全模式
    // 当点频为空取卫星管理默认的
    void parseLinkNode(const QDomElement& root, DeviceWorkTaskTargetLink& link, bool& allMode);
    // 全模式
    void allModeNode(DeviceWorkTaskTarget& target, const SatelliteManagementData& sateData, DeviceWorkTaskTargetLink link);

    // 校验计划文件是否正确
    bool checkTaskPlan(DeviceWorkTaskList& deviceWorkTaskList);
    // 检查设备工作任务信息
    bool checkDeviceWorkTask(const DeviceWorkTask& deviceWorkTask, const SatelliteManagementDataMap& satelliteDataMap, QString& errorMsg);
    // 检查目标信息
    bool checkTarget(const DeviceWorkTaskTarget& target, const SatelliteManagementDataMap& satelliteDataMap, QString& errorMsg);
    // 检查链路信息
    bool checkLink(const DeviceWorkTaskTargetLink& link, QString& errorMsg);

    // 复制文件到指定目录
    bool copyFileToDir(const QString& filePath, const QString& dirPath, bool isDeleteSrcFile, const QString& newFileName = "");

    // 解析文件名称
    bool parseDtFileName(const QFileInfo& fileInfo, DeviceWorkTaskFileInfo& deviceWorkTaskFileInfo, QString& error);

    // 解析任务计划文件
    void parseDtTaskPlanXml(const QString& filePath, DataTranWorkTaskList& dataTranWorkTaskList);
    // 解析任务节点
    void parseDtTaskNode(const QDomElement& root, DataTranWorkTask& dataTranWorkTask);

    // 插入新的数传工作计划
    void newDataTransWorkTask(const DataTranWorkTaskList& dataTranWorkTaskList);
    // 插入新的设备工作计划
    void newDeviceWorkTask(const QList<DeviceWorkTask>& deviceWorkTaskList);
signals:
    // 解析设备工作计划文件成功后触发
    //void signalNewDeviceWorkTask(const QList<DeviceWorkTask>& deviceWorkTaskList);

    // 解析数据传输计划文件成功后触发
    //void signalNewDataTransWorkTask(const DataTranWorkTaskList& dataTranWorkTaskList);

private:
    bool mIsRunning = true;

    //    QList<QString> mWorkUnitList = { "26-TJ", "27-JD", "26-XA" };
    //    QList<QString> mEquipmentNumList = { "TK-4413" };
    //    QList<QString> mWorkWayList = { "SZ", "LT", "HL", "BJ", "ZY" };  // SZ:实战任务 LT:联试任务 HL:全区合练 BJ:标校 ZY:占用

    TaskPlanConfigCheck taskPlanConfigCheck;
};
#endif  // PARSEDEVICETASKXML_H
