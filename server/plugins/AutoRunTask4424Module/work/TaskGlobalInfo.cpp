#include "TaskGlobalInfo.h"

void TaskGlobalInfoSingleton::instanceData() {}

CalibResultInfo TaskGlobalInfoSingleton::getCalibResultInfo(int workMode, const QString& taskCode, double downFrequency)
{
    CalibResultInfo info;
    info.azpr = 0;
    info.elpr = 0;
    info.azge = 0.1;
    info.elge = 0.1;
    //    //获取校相数据
    //    auto ipcMgr = getPCInterface();
    //    if (nullptr == ipcMgr)
    //    {
    //        return info;
    //    }
    //    else
    //    {
    //        QList<QVariantMap> dataList;
    //        QString error;
    //        bool flag = ipcMgr->selectPCItem(taskCode, downFrequency, workMode, dataList, error);

    //        if (!flag || dataList.size() == 0)
    //        {
    //            return info;
    //        }
    //        else
    //        {
    //            if (dataList[0].contains("AZPR"))
    //                info.AZPR = dataList[0]["AZPR"].toDouble();
    //            if (dataList[0].contains("ELPR"))
    //                info.ELPR = dataList[0]["ELPR"].toDouble();
    //            if (dataList[0].contains("AZGE"))
    //                info.AZGE = dataList[0]["AZGE"].toDouble();
    //            if (dataList[0].contains("ELGE"))
    //                info.ELGE = dataList[0]["ELGE"].toDouble();
    //            return info;
    //        }
    //    }
    return info;
}

void TaskGlobalInfoSingleton::updateCalibResultInfoList(CalibResultInfo info)
{
    // 校相接口更新
    //    auto ipcMgr = getPCInterface();
    //    if (nullptr == ipcMgr) {}
    //    else
    //    {
    //        QVariantMap dataList;
    //        dataList["AZPR"] = info.AZPR;
    //        dataList["ELPR"] = info.ELPR;
    //        dataList["AZGE"] = info.AZGE;
    //        dataList["ELGE"] = info.ELGE;

    //        QString errorMsg;
    //        ipcMgr->insertPCItem(info.taskCode, info.workFreg, info.workMode, dataList, errorMsg);
    //    }
}

QList<CalibResultInfo> TaskGlobalInfoSingleton::getCalibResulitList() const { return m_calibResulitList; }
