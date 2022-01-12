#include "DTETaskHandler.h"
#include "BaseParamMacroHandler.h"
#include "BusinessMacroCommon.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "PlanRunMessageDefine.h"
#include "SatelliteManagementSerialize.h"

DTETaskHandler::DTETaskHandler(QObject* parent)
    : BaseParamMacroHandler(parent)
{
    setDTETaskTime(QDateTime(), QDateTime());
}
SystemWorkMode DTETaskHandler::getSystemWorkMode() { return SystemWorkMode::NotDefine; }

void DTETaskHandler::setDTETaskTime(const QDateTime& start, const QDateTime& end)
{
    /* 有任何一个时间无效 但是有当前的频段那就是数据出错了 */
    if (!start.isValid() || !end.isValid())
    {
        m_dteTraceStartDateTime = GlobalData::currentDateTime();
        m_dteTaskEndDateTime = GlobalData::currentDateTime().addYears(3);
    }
    else
    {
        m_dteTraceStartDateTime = start;
        m_dteTaskEndDateTime = end;
    }
}
bool DTETaskHandler::handle()
{
    // 发送DTE本控切换命令
    mSingleCommandHelper.packSingleCommand("Step_DTE_CTRLMODE", mPackCommand);
    waitExecSuccess(mPackCommand);

    /* 清空DTE任务 */
    cleanDTETask();

    // 检查是否有测控任务 或者Ka低速
    QSet<SystemWorkMode> dteWorkModeSet;
    for (auto iter = mManualMsg.linkLineMap.begin(); iter != mManualMsg.linkLineMap.end(); ++iter)
    {
        auto workMode = SystemWorkMode(iter.key());
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode) || workMode == KaDS)
        {
            dteWorkModeSet << workMode;
        }
    }
    if (dteWorkModeSet.isEmpty())
    {
        emit signalInfoMsg("当前无测控任务或者Ka低速任务跳过DTE任务下发");
        return true;
    }

    //根据任务代号获取任务标识，天线标识， 向DTE下发瞬根申请命令
    QMap<QString, QVariant> dteSunGenParamMap;
    dteSunGenParamMap["MID"] = mManualMsg.masterTaskBz;  // 任务标识
    dteSunGenParamMap["UAC"] = mManualMsg.txUACAddr;     // UAC

    // 发送DTE_A瞬根申请命令
    mSingleCommandHelper.packSingleCommand("Step_DTE_APPLY_OrbitalRoot", mPackCommand, dteSunGenParamMap);
    waitExecSuccess(mPackCommand);

    /* 20211021 wp
     * 当有STTC Skuo2 基带编号为 1 2
     * 当有Ka扩2时 基带编号为3 4
     * 当有Ka低速 时会使用5 6
     * 当测控同时有Skuo2和Ka扩2时 测控会用4台基带
     * 其他单模式测控只占用2台基带
     * 低速只占用2台基带
     */
    /*
     * 三个布尔分别代表
     * S遥测 Ka遥测 Ka低速
     */
    QMap<QString, std::tuple<bool, bool, bool>> taskCodeWorkModeMap;
    for (auto& link : mManualMsg.linkLineMap)
    {
        auto targetInfo = link.targetMap.value(link.masterTargetNo);
        if (targetInfo.taskCode.isEmpty())
        {
            continue;
        }
        if (SystemWorkModeHelper::systemWorkModeToSystemBand(link.workMode) == SBand)
        {
            auto& dteJDTuple = taskCodeWorkModeMap[targetInfo.taskCode];
            bool& dte_ycjd_s = std::get<0>(dteJDTuple);
            dte_ycjd_s = true;
        }
        else if (link.workMode == KaKuo2)
        {
            auto& dteJDTuple = taskCodeWorkModeMap[targetInfo.taskCode];
            bool& dte_ycjd_ka = std::get<1>(dteJDTuple);
            dte_ycjd_ka = true;
        }
        else if (link.workMode == KaDS)
        {
            auto& dteJDTuple = taskCodeWorkModeMap[targetInfo.taskCode];
            bool& dte_dsjd = std::get<2>(dteJDTuple);
            dte_dsjd = true;
        }
    }

    auto dteTaskPlanFunc = [&](QMap<QString, QVariant>& dteTaskLoadParamMap, QMap<int, QList<QPair<QString, QVariant>>>& dteJDNumMap,
                               const std::tuple<bool, bool, bool>& dteJDTuple, const QString& taskBZ) {
        dteTaskLoadParamMap.clear();
        dteTaskLoadParamMap["MID"] = taskBZ;                                                                   // 任务标识
        dteTaskLoadParamMap["UAC"] = mManualMsg.txUACAddr;                                                     // 天线标识
        dteTaskLoadParamMap["TrackStartDate"] = m_dteTraceStartDateTime.date().toString(DATE_DISPLAY_FORMAT);  // 跟踪开始日期
        dteTaskLoadParamMap["TrackStartTme"] = m_dteTraceStartDateTime.time().toString(TIME_DISPLAY_FORMAT);   // 跟踪开始时间
        dteTaskLoadParamMap["MissionStopDate"] = m_dteTaskEndDateTime.date().toString(DATE_DISPLAY_FORMAT);    // 任务结束日期
        dteTaskLoadParamMap["MissionSStopTime"] = m_dteTaskEndDateTime.time().toString(TIME_DISPLAY_FORMAT);   // 任务结束时间
        /* DTE的任务是动态参数 基带的数量是可变的*/
        QList<int> bbeList;
        int bbeNum = 0;

        bool dte_ycjd_s = std::get<0>(dteJDTuple);
        bool dte_ycjd_ka = std::get<1>(dteJDTuple);
        bool dte_dsjd = std::get<2>(dteJDTuple);

        if (dte_ycjd_s && dte_ycjd_ka)
        {
            bbeNum += 4;
            bbeList << 1;
            bbeList << 2;
            bbeList << 3;
            bbeList << 4;
        }
        else if (dte_ycjd_s)
        {
            bbeNum += 2;
            bbeList << 1;
            bbeList << 2;
        }
        else if (dte_ycjd_ka)
        {
            bbeNum += 2;
            bbeList << 3;
            bbeList << 4;
        }

        if (dte_dsjd)
        {
            bbeNum += 2;
            bbeList << 5;
            bbeList << 6;
        }

        dteTaskLoadParamMap["BbeNum"] = bbeNum;
        // DTE的基带参数
        int index = 0;
        QList<QPair<QString, QVariant>> tempJDNumList;
        for (auto& item : bbeList)
        {
            tempJDNumList << qMakePair(QString("Bbe1Index%1").arg(index++), QVariant(item));
        }
        dteJDNumMap[1] = tempJDNumList;
    };

    for (auto iter = taskCodeWorkModeMap.begin(); iter != taskCodeWorkModeMap.end(); ++iter)
    {
        auto taskCode = iter.key();
        auto dteJDTuple = iter.value();
        auto taskBZ = mManualMsg.masterTaskBz;
        /* 不是主跟的获取卫星的标识 */
        if (taskCode != mManualMsg.masterTaskCode)
        {
            SatelliteManagementData sateInfo;
            if (!GlobalData::getSatelliteManagementData(taskCode, sateInfo))
            {
                auto msg = QString("DET计划下发失败:获取任务代号为%1的卫星信息错误").arg(taskCode);
                emit signalErrorMsg(msg);
                continue;
            }
            taskBZ = sateInfo.m_satelliteIdentification;
        }
        QMap<QString, QVariant> dteTaskLoadParamMap;
        QMap<int, QList<QPair<QString, QVariant>>> dteJDNumMap;
        dteTaskPlanFunc(dteTaskLoadParamMap, dteJDNumMap, dteJDTuple, taskBZ);
        // 发送DTE_A任务加载命令
        mSingleCommandHelper.packSingleCommand("Step_DTE_LOADTASK", mPackCommand, dteJDNumMap, {}, dteTaskLoadParamMap);
        waitExecSuccess(mPackCommand);
    }

    /* ACU宏要在DTE之前下发但是自动化运行要在DTE宏下完之后 */
    auto onlineACUResult = BaseHandler::getOnlineACU();
    if (!onlineACUResult)
    {
        emit signalErrorMsg(onlineACUResult.msg());
        return false;
    }
    auto acuDeviceID = onlineACUResult.value();

    mSingleCommandHelper.packSingleCommand("Step_ACU_OperatMode_AutoRun", mPackCommand, acuDeviceID);
    waitExecSuccess(mPackCommand);
    return true;
}
