#include "XGSParamMacroHandler.h"

#include "BusinessMacroCommon.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "SatelliteManagementDefine.h"
#include "SingleCommandHelper.h"

XGSParamMacroHandler::XGSParamMacroHandler(QObject* parent)
    : BaseParamMacroHandler(parent)
{
}

bool XGSParamMacroHandler::handle()
{
    SingleCommandHelper singleCommandHelper;
    PackCommand packCommand;

    if (mLinkLine.targetMap.isEmpty())
    {
        emit signalErrorMsg("需要选择一个任务代号");
        return false;
    }

    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(mLinkLine.workMode);

    // 使用主用目标的数据
    auto masterTargetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);
    auto masterTargetTaskCode = masterTargetInfo.taskCode;
    auto masterTargetPointFreqNo = masterTargetInfo.pointFreqNo;
    auto masterTargetWorkMode = masterTargetInfo.workMode;

    DeviceID gsjdDeviceID;
    if (!getGSJDDeviceID(gsjdDeviceID, 1))
    {
        return false;
    }

    TargetInfo firstTargetInfo;
    TargetInfo secondTargetInfo;

    ParamMacroModeData firstParamMacroModeData;
    ParamMacroModeData secondParamMacroModeData;

    SatelliteManagementData firstSatelliteData;
    SatelliteManagementData secondSatelliteData;

    SystemOrientation firstOrientation = SystemOrientation::Unkonwn;
    SystemOrientation secondOrientation = SystemOrientation::Unkonwn;

    QMap<int, QMap<QString, QVariant>> firstPointFreqUnitParamMap;
    QMap<int, QMap<QString, QVariant>> secondPointFreqUnitParamMap;

    // 单点频
    if (mLinkLine.targetMap.size() == 1)
    {
        firstTargetInfo = mLinkLine.targetMap.first();
        // 获取卫星数据
        if (!GlobalData::getSatelliteManagementData(firstTargetInfo.taskCode, firstSatelliteData))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的卫星数据失败").arg(firstTargetInfo.taskCode).arg(workModeDesc));
            return false;
        }

        // 获取极化
        if (!firstSatelliteData.getSReceivPolar(mLinkLine.workMode, firstTargetInfo.pointFreqNo, firstOrientation))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的卫星数据，模式为%2的点频%3的旋向失败")
                                    .arg(firstTargetInfo.taskCode)
                                    .arg(workModeDesc)
                                    .arg(firstTargetInfo.targetNo));
            return false;
        }

        // 获取指定任务代号指定模式的参数宏数据
        if (!getParamMacroModeData(firstTargetInfo.taskCode, mLinkLine.workMode, firstParamMacroModeData))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式参数失败").arg(firstTargetInfo.taskCode).arg(workModeDesc));
            return false;
        }

        // 获取点频参数
        if (!getPointFreqParamMap(firstTargetInfo.taskCode, mLinkLine.workMode, firstTargetInfo.pointFreqNo, gsjdDeviceID,
                                  firstPointFreqUnitParamMap))
        {
            return false;
        }
    }
    // 双点频
    else if (mLinkLine.targetMap.size() == 2)
    {
        firstTargetInfo = mLinkLine.targetMap.first();
        // 获取卫星数据
        if (!GlobalData::getSatelliteManagementData(firstTargetInfo.taskCode, firstSatelliteData))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的卫星数据失败").arg(firstTargetInfo.taskCode).arg(workModeDesc));
            return false;
        }
        // 获取极化
        if (!firstSatelliteData.getSReceivPolar(mLinkLine.workMode, firstTargetInfo.pointFreqNo, firstOrientation))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的卫星数据，模式为%2的点频%3的旋向失败")
                                    .arg(firstTargetInfo.taskCode)
                                    .arg(workModeDesc)
                                    .arg(firstTargetInfo.targetNo));
            return false;
        }
        // 获取指定任务代号指定模式的参数宏数据
        if (!getParamMacroModeData(firstTargetInfo.taskCode, mLinkLine.workMode, firstParamMacroModeData))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式参数失败").arg(firstTargetInfo.taskCode).arg(workModeDesc));
            return false;
        }
        // 获取点频参数
        if (!getPointFreqParamMap(firstTargetInfo.taskCode, mLinkLine.workMode, firstTargetInfo.pointFreqNo, gsjdDeviceID,
                                  firstPointFreqUnitParamMap))
        {
            return false;
        }

        secondTargetInfo = mLinkLine.targetMap.last();
        // 获取卫星数据
        if (!GlobalData::getSatelliteManagementData(secondTargetInfo.taskCode, secondSatelliteData))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的卫星数据失败").arg(secondTargetInfo.taskCode).arg(workModeDesc));
            return false;
        }
        // 获取极化
        if (!secondSatelliteData.getSReceivPolar(mLinkLine.workMode, secondTargetInfo.pointFreqNo, secondOrientation))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的卫星数据，模式为%2的点频%3的旋向失败")
                                    .arg(firstTargetInfo.taskCode)
                                    .arg(workModeDesc)
                                    .arg(firstTargetInfo.targetNo));
            return false;
        }
        // 获取指定任务代号指定模式的参数宏数据
        if (!getParamMacroModeData(secondTargetInfo.taskCode, mLinkLine.workMode, secondParamMacroModeData))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式参数失败").arg(secondTargetInfo.taskCode).arg(workModeDesc));
            return false;
        }
        // 获取点频参数
        if (!getPointFreqParamMap(secondTargetInfo.taskCode, mLinkLine.workMode, secondTargetInfo.pointFreqNo, gsjdDeviceID,
                                  secondPointFreqUnitParamMap))
        {
            return false;
        }
    }
    else
    {
        emit signalErrorMsg(QString("%1模式最多2个点频").arg(workModeDesc));
        return false;
    }

    QMap<int, QMap<int, QMap<QString, QVariant>>> unitTargetParamMap1;  // 点频1的参数宏数据
    QMap<int, QMap<int, QMap<QString, QVariant>>> unitTargetParamMap2;  // 点频2的参数宏数据
    // 单点频
    if (mLinkLine.targetMap.size() == 1)
    {
        if (!getGSJDUnitTargetParamMap(firstTargetInfo, unitTargetParamMap1))
        {
            return false;
        }

        unitTargetParamMap2 = unitTargetParamMap1;  // 两台高速基带下同样参数
    }
    // 双点频
    else if (mLinkLine.targetMap.size() == 2)
    {
        // 两个点频都是单极化，两个点频数据往同一个基带下
        if (firstOrientation != SystemOrientation::LRCircumflex && secondOrientation != SystemOrientation::LRCircumflex)
        {
            unitTargetParamMap1[1][0]["XTaskIdent"] = firstSatelliteData.m_satelliteIdentification;     // 任务标识1
            unitTargetParamMap1[1][0]["XTaskCode"] = firstSatelliteData.m_satelliteCode;                // 任务代号1
            unitTargetParamMap1[1][0]["KaTaskIdent2"] = secondSatelliteData.m_satelliteIdentification;  // 任务标识2
            unitTargetParamMap1[1][0]["KaTaskCode2"] = secondSatelliteData.m_satelliteCode;             // 任务代号2
            // 通道一的单元 使用点频1
            unitTargetParamMap1[2][0] = firstPointFreqUnitParamMap[2];
            unitTargetParamMap1[3][0] = firstPointFreqUnitParamMap[3];
            unitTargetParamMap1[4][0] = firstPointFreqUnitParamMap[4];
            // 通道二的单元 使用点频2
            unitTargetParamMap1[6][0] = secondPointFreqUnitParamMap[2];
            unitTargetParamMap1[7][0] = secondPointFreqUnitParamMap[3];
            unitTargetParamMap1[8][0] = secondPointFreqUnitParamMap[4];

            unitTargetParamMap2 = unitTargetParamMap1;  // 两台高速基带下同样参数
        }
        else  // 有一个点频是双极化或者两个都是双极化，分别下不同基带
        {
            if (!getGSJDUnitTargetParamMap(firstTargetInfo, unitTargetParamMap1))
            {
                return false;
            }

            if (!getGSJDUnitTargetParamMap(secondTargetInfo, unitTargetParamMap2))
            {
                return false;
            }
        }
    }

    // 设置模拟源单元参数，默认为通道一的参数
    for (auto paramId : unitTargetParamMap1[2][0].keys())  // 解调单元
    {
        unitTargetParamMap1[10][0][paramId] = unitTargetParamMap1[2][0][paramId];
    }
    for (auto paramId : unitTargetParamMap1[3][0].keys())  // I路
    {
        QString mnyIParamId = QString("I_%1").arg(paramId);
        unitTargetParamMap1[10][0][mnyIParamId] = unitTargetParamMap1[3][0][paramId];
    }
    for (auto paramId : unitTargetParamMap1[4][0].keys())  // Q路
    {
        QString mnyQParamId = QString("Q_%1").arg(paramId);
        unitTargetParamMap1[10][0][mnyQParamId] = unitTargetParamMap1[4][0][paramId];
    }

    for (auto paramId : unitTargetParamMap2[2][0].keys())  // 解调单元
    {
        unitTargetParamMap2[10][0][paramId] = unitTargetParamMap2[2][0][paramId];
    }
    for (auto paramId : unitTargetParamMap2[3][0].keys())  // I路
    {
        QString mnyIParamId = QString("I_%1").arg(paramId);
        unitTargetParamMap2[10][0][mnyIParamId] = unitTargetParamMap2[3][0][paramId];
    }
    for (auto paramId : unitTargetParamMap2[4][0].keys())  // Q路
    {
        QString mnyQParamId = QString("Q_%1").arg(paramId);
        unitTargetParamMap2[10][0][mnyQParamId] = unitTargetParamMap2[4][0][paramId];
    }

    // 获取主用高速基带数量
    int masterGSJDNumber = 0;
    if (!getGSJDMasterNumber(masterGSJDNumber))
    {
        return false;
    }

    auto curProject = ExtendedConfig::curProjectID();
    if (curProject == "4424")
    {
        // 一主两备，三台高速基带下相同参数
        if (masterGSJDNumber == 1)
        {
            if (getGSJDDeviceID(gsjdDeviceID, 1))
            {
                packGroupParamSetData(gsjdDeviceID, 0xFFFF, unitTargetParamMap1, packCommand);
                waitExecSuccess(packCommand);
            }
            if (getGSJDDeviceID(gsjdDeviceID, 2))
            {
                packGroupParamSetData(gsjdDeviceID, 0xFFFF, unitTargetParamMap1, packCommand);
                waitExecSuccess(packCommand);
            }
            if (getGSJDDeviceID(gsjdDeviceID, 3))
            {
                packGroupParamSetData(gsjdDeviceID, 0xFFFF, unitTargetParamMap1, packCommand);
                waitExecSuccess(packCommand);
            }
        }
        // 两主一备，第一台主下点频1参数，第二台主下点频2参数，备用下点频1参数
        else if (masterGSJDNumber == 2)
        {
            if (getGSJDDeviceID(gsjdDeviceID, 1))
            {
                packGroupParamSetData(gsjdDeviceID, 0xFFFF, unitTargetParamMap1, packCommand);
                waitExecSuccess(packCommand);
            }
            if (getGSJDDeviceID(gsjdDeviceID, 2))
            {
                packGroupParamSetData(gsjdDeviceID, 0xFFFF, unitTargetParamMap2, packCommand);
                waitExecSuccess(packCommand);
            }
            if (getGSJDDeviceID(gsjdDeviceID, 3))
            {
                packGroupParamSetData(gsjdDeviceID, 0xFFFF, unitTargetParamMap1, packCommand);
                waitExecSuccess(packCommand);
            }
        }
    }
    else if (curProject == "4426")
    {
        // 一主一备，两台高速基带下相同参数
        if (masterGSJDNumber == 1)
        {
            if (getGSJDDeviceID(gsjdDeviceID, 1))
            {
                packGroupParamSetData(gsjdDeviceID, 0xFFFF, unitTargetParamMap1, packCommand);
                waitExecSuccess(packCommand);
            }
            if (getGSJDDeviceID(gsjdDeviceID, 2))
            {
                packGroupParamSetData(gsjdDeviceID, 0xFFFF, unitTargetParamMap1, packCommand);
                waitExecSuccess(packCommand);
            }
        }
        // 两主，第一台主下点频1参数，第二台主下点频2参数
        else if (masterGSJDNumber == 2)
        {
            if (getGSJDDeviceID(gsjdDeviceID, 1))
            {
                packGroupParamSetData(gsjdDeviceID, 0xFFFF, unitTargetParamMap1, packCommand);
                waitExecSuccess(packCommand);
            }
            if (getGSJDDeviceID(gsjdDeviceID, 2))
            {
                packGroupParamSetData(gsjdDeviceID, 0xFFFF, unitTargetParamMap2, packCommand);
                waitExecSuccess(packCommand);
            }
        }
    }

    // 获取指定任务代号指定模式的参数宏数据
    ParamMacroModeData paramMacroModeData;
    if (!getParamMacroModeData(masterTargetTaskCode, masterTargetWorkMode, paramMacroModeData))
    {
        emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式参数失败").arg(masterTargetTaskCode).arg(workModeDesc));
        return false;
    }

    // 获取下行的频率
    QVariant downFrequency;
    MacroCommon::getDownFrequency(masterTargetTaskCode, masterTargetWorkMode, masterTargetPointFreqNo, downFrequency);

    // 设置X频段射频开关网络参数
    DeviceID xpdspkgwlDeviceID(3, 2, 1);
    QMap<int, QMap<QString, QVariant>> xkgwlUnitParamMap;
    paramMacroModeData.getDeviceUnitParamMap(xpdspkgwlDeviceID, xkgwlUnitParamMap);
    auto xkgwlParamMap = xkgwlUnitParamMap[1];
    singleCommandHelper.packSingleCommand(QString("StepXPDSPKGWL_XHSJ"), packCommand, xpdspkgwlDeviceID, xkgwlParamMap);
    waitExecSuccess(packCommand);

    // 设置X跟踪下变频器A、B参数
    DeviceID xgzxbpqADeviceID(3, 3, 1);
    QMap<int, QMap<QString, QVariant>> xgzxbpqUnitParamMap;
    paramMacroModeData.getDeviceUnitParamMap(xgzxbpqADeviceID, xgzxbpqUnitParamMap);
    auto xgzxbpqParamMap = xkgwlUnitParamMap[1];
    xgzxbpqParamMap["InputFreq"] = downFrequency;  // 替换为点频的下行频率

    singleCommandHelper.packSingleCommand(QString("StepXGZXBPQ_SJL"), packCommand, xgzxbpqADeviceID, xgzxbpqParamMap);
    waitExecSuccess(packCommand);

    DeviceID xgzxbpqBDeviceID(3, 3, 2);
    singleCommandHelper.packSingleCommand(QString("StepXGZXBPQ_SJL"), packCommand, xgzxbpqBDeviceID, xgzxbpqParamMap);
    waitExecSuccess(packCommand);

    // 设置X1.2GHz下变频器参数
    DeviceID xxbpqDeviceID;
    if (!getXXBPQDeviceID(xxbpqDeviceID))
    {
        return false;
    }
    QMap<int, QMap<QString, QVariant>> xxbpqUnitParamMap;
    paramMacroModeData.getDeviceUnitParamMap(xxbpqDeviceID, xxbpqUnitParamMap);
    auto xxbpqParamMap = xkgwlUnitParamMap[1];
    xxbpqParamMap["InputFreq1"] = downFrequency;  // 替换为点频的下行频率
    xxbpqParamMap["InputFreq2"] = downFrequency;
    singleCommandHelper.packSingleCommand(QString("StepXXBPQ_SJL"), packCommand, xxbpqDeviceID, xxbpqParamMap);
    waitExecSuccess(packCommand);

    return true;
}

SystemWorkMode XGSParamMacroHandler::getSystemWorkMode() { return SystemWorkMode::XGS; }

bool XGSParamMacroHandler::getGSJDUnitTargetParamMap(const TargetInfo& targetInfo, QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap)
{
    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(mLinkLine.workMode);

    // 获取卫星数据
    SatelliteManagementData satelliteData;
    if (!GlobalData::getSatelliteManagementData(targetInfo.taskCode, satelliteData))
    {
        emit signalErrorMsg(QString("获取任务代号为：%1的卫星数据失败").arg(targetInfo.taskCode).arg(workModeDesc));
        return false;
    }

    // 获取极化
    SystemOrientation orientation = SystemOrientation::Unkonwn;
    if (!satelliteData.getSReceivPolar(mLinkLine.workMode, targetInfo.pointFreqNo, orientation))
    {
        emit signalErrorMsg(
            QString("获取任务代号为：%1的卫星数据，模式为%2的点频%3的旋向失败").arg(targetInfo.taskCode).arg(workModeDesc).arg(targetInfo.targetNo));
        return false;
    }

    DeviceID deviceID(0x4101);
    // 获取点频参数
    QMap<int, QMap<QString, QVariant>> pointFreqUnitParamMap;
    if (!getPointFreqParamMap(targetInfo.taskCode, mLinkLine.workMode, targetInfo.pointFreqNo, deviceID, pointFreqUnitParamMap))
    {
        return false;
    }

    if (orientation == SystemOrientation::LCircumflex || orientation == SystemOrientation::RCircumflex)
    {
        unitTargetParamMap[1][0]["XTaskIdent"] = satelliteData.m_satelliteIdentification;  //任务标识1
        unitTargetParamMap[1][0]["XTaskCode"] = satelliteData.m_satelliteCode;             //任务代号1
        // 通道一的单元
        unitTargetParamMap[2][0] = pointFreqUnitParamMap[2];
        unitTargetParamMap[3][0] = pointFreqUnitParamMap[3];
        unitTargetParamMap[4][0] = pointFreqUnitParamMap[4];
    }
    else if (orientation == SystemOrientation::LRCircumflex)
    {
        unitTargetParamMap[1][0]["XTaskIdent"] = satelliteData.m_satelliteIdentification;    //任务标识1
        unitTargetParamMap[1][0]["XTaskCode"] = satelliteData.m_satelliteCode;               //任务代号1
        unitTargetParamMap[1][0]["KaTaskIdent2"] = satelliteData.m_satelliteIdentification;  //任务标识2
        unitTargetParamMap[1][0]["KaTaskCode2"] = satelliteData.m_satelliteCode;             //任务代号2

        // 通道一的单元
        unitTargetParamMap[2][0] = pointFreqUnitParamMap[2];
        unitTargetParamMap[3][0] = pointFreqUnitParamMap[3];
        unitTargetParamMap[4][0] = pointFreqUnitParamMap[4];
        // 通道二的单元
        unitTargetParamMap[6][0] = pointFreqUnitParamMap[6];
        unitTargetParamMap[7][0] = pointFreqUnitParamMap[7];
        unitTargetParamMap[8][0] = pointFreqUnitParamMap[8];
    }

    return true;
}
