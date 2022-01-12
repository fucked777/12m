#ifndef TASKPREPARATIONEVENT_H
#define TASKPREPARATIONEVENT_H

#include "BaseEvent.h"
#include "TaskRunCommonHelper.h"
#include <QMultiMap>
#include <QObject>

class GlobalConfigMacroData;
class GlobalStationResourcesData;
class GlobalSatelliteData;
struct TaskTimeList;
struct ManualMessage;
struct LinkLine;
class TaskPreparation : public BaseEvent
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit TaskPreparation(QObject* parent = nullptr);
    static int typeId;  //实现反射使用的

    void doSomething();

private:
    // 通过当前任务信息生成配置宏和参数宏
    ManualMessage createManualMessage();

    //下发配置宏和参数宏
    void STTC_ConfigAndParamSetting(LinkLine& linkLine);           // S标准TTC
    void Skuo2_ConfigAndParamSetting(LinkLine& linkLine);          // S扩二
    void KaKuo2_ConfigAndParamSetting(LinkLine& linkLine);         // Ka扩二
    void SYTHSMJ_ConfigAndParamSetting(LinkLine& linkLine);        // 一体化上面级
    void SYTHWX_ConfigAndParamSetting(LinkLine& linkLine);         // 一体化卫星
    void SYTHGML_ConfigAndParamSetting(LinkLine& linkLine);        // 一体化高码率
    void SYTHSMJK2GZB_ConfigAndParamSetting(LinkLine& linkLine);   // 一体化上面级+扩二共载波
    void SYTHSMJK2BGZB_ConfigAndParamSetting(LinkLine& linkLine);  // 一体化上面级+扩二不共载波
    void SYTHWXSK2_ConfigAndParamSetting(LinkLine& linkLine);      // 一体化卫星+扩二
    void SYTHGMLSK2_ConfigAndParamSetting(LinkLine& linkLine);     // 一体化高码率+扩二
    void SKT_ConfigAndParamSetting(LinkLine& linkLine);            // 扩跳
    void XDS_ConfigAndParamSetting(LinkLine& linkLine);            // X低速
    void KaDS_ConfigAndParamSetting(LinkLine& linkLine);           // Ka低速
    void XGS_ConfigAndParamSetting(LinkLine& linkLine);            // X高速
    void KaGS_ConfigAndParamSetting(LinkLine& linkLine);           // Ka高速
    void setParamMacro();

    // 任务资源获取和检查
    bool getTaskResourcesAndCheck();
    bool checkDeviceStatus();
    bool getTaskResources();

    // 下发任务给DTE
    void configDTE();
    // 下发任务给ACU
    void configACU();
    //下发任务给存储转发 Ka高速
    void configCCZFKaHS();

    //存储转发单元参数下发map封装
    void setCCZFSaveUnitMap(SystemWorkMode workMode, const QString& deviceName, int baseChannel, TaskTimeEntity& taskItem,
                            QMap<QString, QVariant>& m_saveDataMap);
    //存储转发过程控制命令参数下发map封装
    void setCCZFSaveCmdMap(int channel, int dataTranWorkType, DataTranWorkTask& dataTranWorkTask, QList<DataTranWorkTask> connectDtTaskList,
                           TaskTimeEntity& taskItem, QMap<QString, QVariant>& satPlanParamMap);
    //记录存储转发命令
    void setTaskTimeListCCZFCmdMap(DeviceID deviceID, QMap<QString, QVariant>& cmdMap);

    // 处理基带，ACU，存储转发，
    //    void checkDeviceStatus(TaskTimeList taskTimeList);

    //设备初始化，下发设备组合号  存盘送数停止  站址处理
    void deviceInit();

    // 跟踪基带下发角误差
    void configGZDeviceJWC();
    //配置基带任务开始状态
    void configDeviceStartStatus();

    //替换存储转发任务计划里的基带名称，如果是主机名称，则切换为备机名称，如果是备机名称，则切换为主机名称
    QString replaceBakSatName(QString deviceName);

private:
    GlobalConfigMacroData* m_globalConfigData = nullptr;
    GlobalStationResourcesData* mGlobalStationResourceData = nullptr;
    QMap<QString, bool> m_simpleMap = { { "左旋", true }, { "LHCP", true }, { "右旋", true }, { "RHCP", true } };
    QMap<QString, bool> m_doubleMap = { { "HLP", true }, { "左右旋同时", true }, { "VLP", true } };
    TaskTimeList m_taskTimeList;
};
// Q_DECLARE_METATYPE(TaskPreparation)

#endif  // TASKPREPARATIONEVENT_H
