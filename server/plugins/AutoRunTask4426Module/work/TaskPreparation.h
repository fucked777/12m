#ifndef TASKPREPARATIONEVENT_H
#define TASKPREPARATIONEVENT_H

#include "BaseEvent.h"
#include "TaskRunCommonHelper.h"
#include <QMultiMap>
#include <QObject>

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
    //下发配置宏和参数宏
    ManualMessage createManualMessage();
    void setParamMacro();
    void STTC_ConfigAndParamSetting(ManualMessage& msg);    // S标准TTC
    void Skuo2_ConfigAndParamSetting(ManualMessage& msg);   // S扩二
    void KaKuo2_ConfigAndParamSetting(ManualMessage& msg);  // Ka扩二
    void SKT_ConfigAndParamSetting(ManualMessage& msg);     // 扩跳
    void XDS_ConfigAndParamSetting(ManualMessage& msg);     // X低速
    void KaDS_ConfigAndParamSetting(ManualMessage& msg);    // Ka低速
    void KaGS_ConfigAndParamSetting(ManualMessage& msg);    // Ka高速

    // 任务资源获取和检查
    bool getTaskResourcesAndCheck();
    bool checkDeviceStatus();
    bool getTaskResources();
    // 下发任务给ACU
    void configACU();
    //设备初始化，下发设备组合号  存盘送数停止  站址处理
    // void deviceInit();

private:
    TaskTimeList m_taskTimeList;
};
// Q_DECLARE_METATYPE(TaskPreparation)

#endif  // TASKPREPARATIONEVENT_H
