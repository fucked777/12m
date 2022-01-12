#ifndef CCZFPARAMMACROHANDLER_H
#define CCZFPARAMMACROHANDLER_H

#include "BaseParamMacroHandler.h"
#include "SatelliteManagementDefine.h"
#include <QScopedPointer>

/* 自动化运行的数据 */
struct AutomaticRunningData
{
    int circleNo{ 0 };                                           /* 圈号 外部赋值 */
    int dpNo{ 0 };                                               /* 点频号 */
    SystemOrientation orientation{ SystemOrientation::Unkonwn }; /* 旋向 */
    QString taskCode;                                            /* 任务代号 */
    QString taskBZ;                                              /* 任务标识 */

    DataTransMode dtWorkingMode{ DataTransMode::Unknown };     /* 数传工作模式 外部赋值 */
    QDateTime taskStartTime;                                   /* 任务开始时间 外部赋值 */
    QDateTime dtStartTime;                                     /* 数传开始时间 外部赋值 */
    QDateTime dtEndTime;                                       /* 数传结束时间 外部赋值 */
    QDateTime taskEndTime;                                     /* 任务结束时间 外部赋值 */
    DataTransPriority priority{ DataTransPriority::Priority }; /* 优先级 */

    QString centerName;                                  /* 中心名称 用+分隔 */
    QStringList centerNameList;                          /* 中心名称 */
    CenterProtocol dataProtocal{ CenterProtocol::PDXP }; /* 数传中心协议 */
};

class CCZFParamMacroHandlerImpl;
class CCZFParamMacroHandler : public BaseParamMacroHandler
{
    Q_OBJECT
public:
    explicit CCZFParamMacroHandler(QObject* parent = nullptr);
    ~CCZFParamMacroHandler();

    bool handle() override;
    SystemWorkMode getSystemWorkMode() override;
    void setCCZFPlan(const TaskPlanData& taskPlanData);

private:
    /* Ka高速 */
    void configCCZFKaHS();
    /* 低速 */
    void configCCZFLS();
    /* 参数转换 */
    bool paramConvert();

private:
    QScopedPointer<CCZFParamMacroHandlerImpl> m_impl;
};
#endif  // CCZFPARAMMACROHANDLER_H
