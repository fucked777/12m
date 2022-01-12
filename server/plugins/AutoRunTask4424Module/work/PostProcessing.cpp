#include "PostProcessing.h"

#include "TaskGlobalInfo.h"
/*
#include "TaskReportTableData.h"
#include "TaskReportTableImpl.h"*/

// int PostProcessing::typeId = qRegisterMetaType<PostProcessing>();
PostProcessing::PostProcessing(QObject* parent)
    : BaseEvent(parent)
{
}

void PostProcessing::doSomething()
{
    notifyInfoLog("事后处理流程开始执行");

    /***
     * T8 事后处理时间
     * 1.监控释放任务参数（包括测控基带及前端的IP地址置为空闲），接触与任务链路设备资源的绑定，显示工作计划执行完毕
     * 2.系统监控按流程计划中的第二任务弧段进行工作体制切换，调用并下发任务宏参数控制DTE停止送数，上行信号禁止。
     ***/
    const auto& deviceWorkTask = taskMachine->deviceWorkTask();

    //    TaskReportTable reportTable;
    //    reportTable.mUuid = deviceWorkTask.m_uuid;
    //    reportTable.mStartTime = deviceWorkTask.m_startTime;
    //    reportTable.mCreateTime = deviceWorkTask.m_createTime;
    //    reportTable.mPreStartTime = deviceWorkTask.m_preStartTime;
    //    reportTable.mPlanSerialNumber = deviceWorkTask.m_plan_serial_number;
    //    reportTable.mEndTime = deviceWorkTask.m_endTime;
    //    reportTable.mTaskCode = deviceWorkTask.m_lord_with_target;

    //    TaskReportTableImpl impl;
    //    if (impl.insertData(reportTable))
    //    {
    //        notifyInfoLog("保存任务报表成功");
    //    }
    //    else
    //    {
    //        notifyErrorLog("保存任务报表失败");
    //    }
    notifyInfoLog("事后处理流程完成");
    QThread::sleep(1);
    notify("finish");
}
