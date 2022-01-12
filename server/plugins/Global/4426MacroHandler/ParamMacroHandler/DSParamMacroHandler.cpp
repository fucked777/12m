#include "DSParamMacroHandler.h"

#include "BusinessMacroCommon.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "SatelliteManagementDefine.h"

class DSParamMacroHandlerPrivate
{
public:
    /* 特殊处理 */
    static void dealWithSpcialParam(QMap<int, QMap<int, QVariantMap>>& unitTargetParamMap, int zp, int i, int q);

    /* 模拟源 */
    static void dsMNY(QMap<int, QMap<int, QVariantMap>>& unitTargetParamMap, int zp, int i, int q, int mny);
};

/* 模拟源 */
void DSParamMacroHandlerPrivate::dsMNY(QMap<int, QMap<int, QVariantMap>>& unitTargetParamMap, int zp, int i, int q, int mny)
{
    /* 模拟源 */
    unitTargetParamMap[mny][0] = unitTargetParamMap[zp][0];
    auto varMap = unitTargetParamMap[i][0];
    for (auto iter = varMap.begin(); iter != varMap.end(); ++iter)
    {
        unitTargetParamMap[mny][0].insert(QString("I%1").arg(iter.key()), iter.value());
    }
    varMap = unitTargetParamMap[q][0];
    for (auto iter = varMap.begin(); iter != varMap.end(); ++iter)
    {
        unitTargetParamMap[mny][0].insert(QString("Q%1").arg(iter.key()), iter.value());
    }
    /*
     * CarrierOutSwitch 载波输出开关
     * ModulationSwitch 调制输出开关
     * NoiseOutSwicth 噪声输出开关
     */
    unitTargetParamMap[mny][0]["CarrierOutSwitch"] = 1; /* 载波输出开 */
    unitTargetParamMap[mny][0]["ModulationSwitch"] = 1; /* 加调 */
    unitTargetParamMap[mny][0]["NoiseOutSwicth"] = 2;   /* 噪声关 */
    /* I/Q合路分路要+1 */
    unitTargetParamMap[mny][0]["DataOutWay"] = unitTargetParamMap[mny][0]["DataOutWay"].toInt() + 1;
    /* I+Q/I-Q-->自适应随便给一个值 */
    auto& phaseMapping = unitTargetParamMap[mny][0]["PhaseMapping"];
    phaseMapping = phaseMapping.toInt() == 3 ? QVariant(1) : phaseMapping;
    /* 编码类型处理 56对应的LDPC TURBO是相反的 */
    auto& iCodeWay = unitTargetParamMap[mny][0]["ICodeWay"];
    iCodeWay = iCodeWay.toInt() == 5 ? 6 : 5;
    auto& qCodeWay = unitTargetParamMap[mny][0]["QCodeWay"];
    qCodeWay = qCodeWay.toInt() == 5 ? 6 : 5;
}

/* I/Q码率处理 */
void DSParamMacroHandlerPrivate::dealWithSpcialParam(QMap<int, QMap<int, QVariantMap>>& unitTargetParamMap, int zp, int i, int q)
{
    //通道1解调单元码速率处理
    auto& modulatSystem = unitTargetParamMap[zp][0]["CarrierModulation"];  //调制体制
    auto& dataOutWay = unitTargetParamMap[zp][0]["DataOutWay"];            // 数据输出方式
    auto& iCodeSpeed = unitTargetParamMap[i][0]["CodeSpeed"];              // I路码速率
    auto& qCodeSpeed = unitTargetParamMap[q][0]["CodeSpeed"];              // Q路码速率

    /*
     * BPSK 肯定是只有单I路 1 不用管
     * QPSK 可分可合 2 合路IQ相加=I  分路IQ相等
     * UQPSK 必须分路 4 各玩个的 不用管
     * OQPSK 可分可合 5 同 QPSK
     *
     * I/Q合 0
     * I/Q分 1
     * 2 独立
     */
    auto numModulatSystem = modulatSystem.toInt();
    if (numModulatSystem == 2 || numModulatSystem == 5)
    {
        if (dataOutWay.toInt() == 0)
        {
            iCodeSpeed = iCodeSpeed.toInt() + qCodeSpeed.toInt();
            qCodeSpeed = iCodeSpeed;
        }
        else
        {
            qCodeSpeed = iCodeSpeed;
        }
    }
}

/*******************************************************************************************************************/
/*******************************************************************************************************************/
/*******************************************************************************************************************/

DSParamMacroHandler::DSParamMacroHandler(QObject* parent)
    : BaseParamMacroHandler(parent)
{
}

bool DSParamMacroHandler::handle()
{
    m_dsjdInfo = MasterSlaveDeviceIDInfo();
    m_ckqdInfo = MasterSlaveDeviceIDInfo();
    m_paramMacroModeDataMaster = ParamMacroModeData();
    m_xParamMacroModeDataMaster = ParamMacroModeData();

    /* 同时存在两种低速 只处理Ka的 X就不进了 */
    if (mManualMsg.linkLineMap.contains(XDS) && mManualMsg.linkLineMap.contains(KaDS) && mLinkLine.workMode == KaDS)
    {
        kaxjdParam();
        ExitCheck(false);

        /* m_paramMacroModeDataMaster 代码里面处理过是Ka低速的数据 */
        if (m_paramMacroModeDataMaster.pointFreqParamMap.isEmpty())
        {
            return false;
        }

        // 执行跟踪数字化前端组参数设置 KaDS
        gzqdParam(m_paramMacroModeDataMaster);
        ExitCheck(false);

        clearQueue();

        /* 默认打开送数 */
        mSingleCommandHelper.packSingleCommand("StepDSJDSS_Start_All", mPackCommand, m_dsjdInfo.master);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepDSJDSS_Start_All", mPackCommand, m_dsjdInfo.slave);
        appendExecQueue(mPackCommand);
        /* 默认停止存盘 */
        mSingleCommandHelper.packSingleCommand("StepDSJDCP_Stop_Ka", mPackCommand, m_dsjdInfo.master);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepDSJDCP_Stop_Ka", mPackCommand, m_dsjdInfo.slave);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepDSJDCP_Stop_X", mPackCommand, m_dsjdInfo.master);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepDSJDCP_Stop_X", mPackCommand, m_dsjdInfo.slave);
        appendExecQueue(mPackCommand);

        // 测控前端 KaDS X
        ckqdParam(m_paramMacroModeDataMaster);
        ExitCheck(false);

        if (!m_xParamMacroModeDataMaster.pointFreqParamMap.isEmpty())
        {
            // X频段高频箱
            xgpxParam(m_xParamMacroModeDataMaster);
            ExitCheck(false);
        }

        // ka低速数传及跟踪高频箱 KaDS X
        kadsgpxParam(m_paramMacroModeDataMaster);
        ExitCheck(false);

        // ka数传发射高频箱
        kafsgpxParam(m_paramMacroModeDataMaster);
        ExitCheck(false);

        // Ka数传功放
        kagfParam(m_paramMacroModeDataMaster);

        execQueue();
    }
    else if (mManualMsg.linkLineMap.contains(XDS))
    {
        xjdParam();
        ExitCheck(false);

        /* m_paramMacroModeDataMaster 代码里面处理过是Ka低速的数据 */
        if (m_paramMacroModeDataMaster.pointFreqParamMap.isEmpty())
        {
            return false;
        }

        clearQueue();

        /* 默认打开送数 */
        mSingleCommandHelper.packSingleCommand("StepDSJDSS_Start_X", mPackCommand, m_dsjdInfo.master);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepDSJDSS_Start_X", mPackCommand, m_dsjdInfo.slave);
        appendExecQueue(mPackCommand);
        /* 默认停止存盘 */
        mSingleCommandHelper.packSingleCommand("StepDSJDCP_Stop_X", mPackCommand, m_dsjdInfo.master);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepDSJDCP_Stop_X", mPackCommand, m_dsjdInfo.slave);
        appendExecQueue(mPackCommand);

        // 测控前端 KaDS X
        ckqdParam(m_paramMacroModeDataMaster);
        ExitCheck(false);

        xgpxParam(m_paramMacroModeDataMaster);
        ExitCheck(false);

        execQueue();
    }
    else if (mManualMsg.linkLineMap.contains(KaDS))
    {
        kajdParam();
        ExitCheck(false);

        /* m_paramMacroModeDataMaster 代码里面处理过是Ka低速的数据 */
        if (m_paramMacroModeDataMaster.pointFreqParamMap.isEmpty())
        {
            return false;
        }

        // 执行跟踪数字化前端组参数设置 KaDS
        gzqdParam(m_paramMacroModeDataMaster);
        ExitCheck(false);

        clearQueue();

        /* 默认打开送数 */
        mSingleCommandHelper.packSingleCommand("StepDSJDSS_Start_Ka", mPackCommand, m_dsjdInfo.master);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepDSJDSS_Start_Ka", mPackCommand, m_dsjdInfo.slave);
        appendExecQueue(mPackCommand);
        /* 默认停止存盘 */
        mSingleCommandHelper.packSingleCommand("StepDSJDCP_Stop_Ka", mPackCommand, m_dsjdInfo.master);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepDSJDCP_Stop_Ka", mPackCommand, m_dsjdInfo.slave);
        appendExecQueue(mPackCommand);

        // 测控前端 KaDS X
        ckqdParam(m_paramMacroModeDataMaster);
        ExitCheck(false);

        // ka低速数传及跟踪高频箱 KaDS
        kadsgpxParam(m_paramMacroModeDataMaster);
        ExitCheck(false);

        // ka数传发射高频箱
        kafsgpxParam(m_paramMacroModeDataMaster);
        ExitCheck(false);

        // Ka数传功放
        kagfParam(m_paramMacroModeDataMaster);

        execQueue();
    }

    return true;
}

SystemWorkMode DSParamMacroHandler::getSystemWorkMode() { return SystemWorkMode::KaDS; }

bool DSParamMacroHandler::getPointFreqParam(ParamMacroModeData& paramMacroModeData, QMap<int, QMap<int, QVariantMap>>& unitTargetParamMap,
                                            const TargetInfo& targetInfo, const DeviceID& deviceID, int zp, int i, int q, int mny, bool isKa)
{
    /*
     * ka低速 的单元号 1 2 4 5 8 10 11 --> 8是模拟源
     * X低速  的单元号 1 3 6 7 9       --> 9是模拟源
     */
    // 点频1的目标信息获取
    SatelliteManagementData satelliteData;
    SystemOrientation orientation = SystemOrientation::Unkonwn;
    QMap<int, QVariantMap> pointFreqUnitParamMap;
    QVariant downFreq;
    if (!targetInfoMapParamDown(satelliteData, orientation, downFreq, pointFreqUnitParamMap, paramMacroModeData, targetInfo, deviceID))
    {
        return false;
    }

    if (isKa)
    {
        unitTargetParamMap[1][0]["KaTaskIdent"] = satelliteData.m_satelliteIdentification;
        unitTargetParamMap[1][0]["KaTaskCode"] = satelliteData.m_satelliteCode;
        unitTargetParamMap[1][0]["RWTaskIdent"] = satelliteData.m_satelliteIdentification;
        unitTargetParamMap[1][0]["RWTaskCode"] = satelliteData.m_satelliteCode;
        unitTargetParamMap[1][0]["KaSBandDownFreq"] = downFreq;

        unitTargetParamMap[10][0] = pointFreqUnitParamMap[10];
        unitTargetParamMap[11][0] = pointFreqUnitParamMap[11];
    }
    else
    {
        unitTargetParamMap[1][0]["XTaskIdent"] = satelliteData.m_satelliteIdentification;
        unitTargetParamMap[1][0]["XTaskCode"] = satelliteData.m_satelliteCode;
        unitTargetParamMap[1][0]["XSBandDownFreq"] = downFreq;
    }

    //下发到主用基带的点频
    unitTargetParamMap[zp][0] = pointFreqUnitParamMap[zp];
    unitTargetParamMap[i][0] = pointFreqUnitParamMap[i];
    unitTargetParamMap[q][0] = pointFreqUnitParamMap[q];

    /* 处理一下I/Q码速率 */
    DSParamMacroHandlerPrivate::dealWithSpcialParam(unitTargetParamMap, zp, i, q);
    /* 模拟源 */
    DSParamMacroHandlerPrivate::dsMNY(unitTargetParamMap, zp, i, q, mny);

    return true;
}

// bool DSParamMacroHandler::getKaPointFreqParam(ParamMacroModeData& paramMacroModeData, QMap<int, QMap<int, QVariantMap>>& unitTargetParamMap,
//                                              const TargetInfo& targetInfo, const DeviceID& deviceID)
//{
//    return getPointFreqParam(paramMacroModeData, unitTargetParamMap, targetInfo, deviceID, 2, 4, 5, 8, true);
//}
// bool DSParamMacroHandler::getXPointFreqParam(ParamMacroModeData& paramMacroModeData, QMap<int, QMap<int, QVariantMap>>& unitTargetParamMap,
//                                             const TargetInfo& targetInfo, const DeviceID& deviceID)
//{
//    return getPointFreqParam(paramMacroModeData, unitTargetParamMap, targetInfo, deviceID, 3, 6, 7, 9, false);
//}

bool DSParamMacroHandler::singlePointFreq(const LinkLine& linkLine, QMap<int, QMap<int, QVariantMap>>& masterParam, int zp, int i, int q, int mny,
                                          bool isKa)
{
    // 使用主用目标的数据
    // 单点频
    auto targetInfo = linkLine.targetMap.first();
    auto validDeviceID = m_dsjdInfo.master.isValid() ? m_dsjdInfo.master : m_dsjdInfo.slave;
    ParamMacroModeData paramMacroModeData;
    if (!getPointFreqParam(paramMacroModeData, masterParam, targetInfo, validDeviceID, zp, i, q, mny, isKa))
    {
        return false;
    }

    m_paramMacroModeDataMaster = paramMacroModeData;
    return true;
}
bool DSParamMacroHandler::doublePointFreq(const LinkLine& linkLine, QMap<int, QMap<int, QVariantMap>>& masterParam,
                                          QMap<int, QMap<int, QVariantMap>>& slaveParam, int zp, int i, int q, int mny, bool isKa)
{
    auto validDeviceID = m_dsjdInfo.master.isValid() ? m_dsjdInfo.master : m_dsjdInfo.slave;
    // 点频1参数
    auto target1Info = linkLine.targetMap.first();
    QMap<int, QMap<int, QVariantMap>> unitTargetParamMap1;
    ParamMacroModeData paramMacroModeData1;
    if (!getPointFreqParam(paramMacroModeData1, unitTargetParamMap1, target1Info, validDeviceID, zp, i, q, mny, isKa))
    {
        return false;
    }

    // 点频2参数
    auto target2Info = linkLine.targetMap.last();
    QMap<int, QMap<int, QVariantMap>> unitTargetParamMap2;
    ParamMacroModeData paramMacroModeData2;
    if (!getPointFreqParam(paramMacroModeData2, unitTargetParamMap2, target2Info, validDeviceID, zp, i, q, mny, isKa))
    {
        return false;
    }

    /* 主备分配 */
    auto masterTargetInfo = linkLine.targetMap.value(linkLine.masterTargetNo);
    if (target1Info.pointFreqNo == masterTargetInfo.pointFreqNo)
    {
        masterParam = unitTargetParamMap1;
        slaveParam = unitTargetParamMap2;
        m_paramMacroModeDataMaster = paramMacroModeData1;
    }
    else
    {
        masterParam = unitTargetParamMap2;
        slaveParam = unitTargetParamMap1;
        m_paramMacroModeDataMaster = paramMacroModeData2;
    }

    return true;
}

bool DSParamMacroHandler::jdParam(const LinkLine& linkLine, QMap<int, QMap<int, QVariantMap>>& masterParam,
                                  QMap<int, QMap<int, QVariantMap>>& slaveParam, int zp, int i, int q, int mny, bool isKa)
{
    if (linkLine.targetMap.size() == 1)
    {
        /* 单点频 */
        if (!singlePointFreq(linkLine, masterParam, zp, i, q, mny, isKa))
        {
            return false;
        }
        slaveParam = masterParam;
        return true;
    }
    else if (linkLine.targetMap.size() == 2)
    {
        if (!doublePointFreq(linkLine, masterParam, slaveParam, zp, i, q, mny, isKa))
        {
            return false;
        }
        return true;
    }

    emit signalErrorMsg(QString("当前%1低速点频数量大于2,当前系统不支持").arg(isKa ? "Ka" : "X"));
    return false;
}
bool DSParamMacroHandler::jdParam(int zp, int i, int q, int mny, bool isKa)
{
    // 获取主用低速基带ID
    auto result = getDSJDDeviceID();
    auto msg = result.msg();
    if (!msg.isEmpty())
    {
        emit signalErrorMsg(msg);
    }
    if (!result)
    {
        return false;
    }
    m_dsjdInfo = result.value();

    QMap<int, QMap<int, QVariantMap>> masterParam;
    QMap<int, QMap<int, QVariantMap>> slaveParam;
    if (!jdParam(mLinkLine, masterParam, slaveParam, zp, i, q, mny, isKa))
    {
        return false;
    }

    clearQueue();

    if (m_dsjdInfo.master.isValid() && !masterParam.isEmpty())
    {
        packGroupParamSetData(m_dsjdInfo.master, 0xFFFF, masterParam, mPackCommand);
        appendExecQueue(mPackCommand);
    }

    if (m_dsjdInfo.slave.isValid() && !slaveParam.isEmpty())
    {
        packGroupParamSetData(m_dsjdInfo.slave, 0xFFFF, slaveParam, mPackCommand);
        appendExecQueue(mPackCommand);
    }

    execQueue();
    return true;
}
bool DSParamMacroHandler::kajdParam()
{
    int zp = 2;
    int i = 4;
    int q = 5;
    int mny = 8;
    bool isKa = true;

    return jdParam(zp, i, q, mny, isKa);
}
bool DSParamMacroHandler::xjdParam()
{
    int zp = 3;
    int i = 6;
    int q = 7;
    int mny = 9;
    bool isKa = false;

    return jdParam(zp, i, q, mny, isKa);
}
bool DSParamMacroHandler::kaxjdParam()
{
    // 获取主用低速基带ID
    auto result = getDSJDDeviceID();
    auto msg = result.msg();
    if (!msg.isEmpty())
    {
        emit signalErrorMsg(msg);
    }
    if (!result)
    {
        return false;
    }
    m_dsjdInfo = result.value();

    auto kaLinkLine = mManualMsg.linkLineMap.value(KaDS);
    auto xLinkLine = mManualMsg.linkLineMap.value(XDS);

    QMap<int, QMap<int, QVariantMap>> masterParam;
    QMap<int, QMap<int, QVariantMap>> slaveParam;

    {
        int zp = 3;
        int i = 6;
        int q = 7;
        int mny = 9;
        bool isKa = false;
        QMap<int, QMap<int, QVariantMap>> tempMasterParam;
        QMap<int, QMap<int, QVariantMap>> tempSlaveParam;

        if (jdParam(xLinkLine, tempMasterParam, tempSlaveParam, zp, i, q, mny, isKa))
        {
            masterParam = tempMasterParam;
            slaveParam = tempSlaveParam;
            m_xParamMacroModeDataMaster = m_paramMacroModeDataMaster;
        }
    }

    {
        int zp = 2;
        int i = 4;
        int q = 5;
        int mny = 8;
        bool isKa = true;
        QMap<int, QMap<int, QVariantMap>> tempMasterParam;
        QMap<int, QMap<int, QVariantMap>> tempSlaveParam;

        if (jdParam(kaLinkLine, tempMasterParam, tempSlaveParam, zp, i, q, mny, isKa))
        {
            /* 公共单元数据有冲突先备份一下 */
            auto bakMasterUnit1 = masterParam.value(1).value(0);
            auto bakSlaveUnit1 = slaveParam.value(1).value(0);
            for (auto iter = tempMasterParam.begin(); iter != tempMasterParam.end(); ++iter)
            {
                auto unitID = iter.key();
                masterParam[unitID] = iter.value();
            }
            for (auto iter = tempSlaveParam.begin(); iter != tempSlaveParam.end(); ++iter)
            {
                auto unitID = iter.key();
                masterParam[unitID] = iter.value();
            }
            /* 恢复公共单元的 */
            for (auto iter = bakMasterUnit1.begin(); iter != bakMasterUnit1.end(); ++iter)
            {
                masterParam[1][0][iter.key()] = iter.value();
            }
            for (auto iter = bakSlaveUnit1.begin(); iter != bakSlaveUnit1.end(); ++iter)
            {
                masterParam[1][0][iter.key()] = iter.value();
            }
        }
    }

    clearQueue();

    if (m_dsjdInfo.master.isValid() && !masterParam.isEmpty())
    {
        packGroupParamSetData(m_dsjdInfo.master, 0xFFFF, masterParam, mPackCommand);
        appendExecQueue(mPackCommand);
    }

    if (m_dsjdInfo.slave.isValid() && !slaveParam.isEmpty())
    {
        packGroupParamSetData(m_dsjdInfo.slave, 0xFFFF, slaveParam, mPackCommand);
        appendExecQueue(mPackCommand);
    }

    execQueue();
    return true;
}

bool DSParamMacroHandler::ckqdParam(const ParamMacroModeData& masterTargetParamMacro)
{
    auto result = getCKQDDeviceID();
    if (!result)
    {
        emit signalErrorMsg(result.msg());
        return false;
    }
    m_ckqdInfo = result.value();

    QMap<int, QMap<int, QVariantMap>> unitTargetParamMap;
    QMap<int, QVariantMap> ckqdUnitParamMap;
    masterTargetParamMacro.getDeviceUnitParamMap(m_ckqdInfo.master, ckqdUnitParamMap);
    for (auto iter = ckqdUnitParamMap.begin(); iter != ckqdUnitParamMap.end(); ++iter)
    {
        auto unitId = iter.key();
        unitTargetParamMap[unitId][0] = iter.value();
    }

    /* 无论是哪个低速，前端的频率都是固定的 */
    unitTargetParamMap[1][0]["YBRate"] = 1;
    unitTargetParamMap[1][0]["MainSubSwitch"] = 1;

    unitTargetParamMap[2][0]["SendFreq_1"] = 2075;
    unitTargetParamMap[2][0]["SendFreq_2"] = 2250;

    unitTargetParamMap[3][0]["ReceiveFreq_1"] = 2250;
    unitTargetParamMap[3][0]["ReceiveFreq_2"] = 2250;
    unitTargetParamMap[3][0]["ReceiveFreq_3"] = 2075;

    packGroupParamSetData(m_ckqdInfo.master, 0xFFFF, unitTargetParamMap, mPackCommand);
    appendExecQueue(mPackCommand);

    if (m_ckqdInfo.slave.isValid())
    {
        packGroupParamSetData(m_ckqdInfo.slave, 0xFFFF, unitTargetParamMap, mPackCommand);
        appendExecQueue(mPackCommand);
    }

    return true;
}
bool DSParamMacroHandler::kadsgpxParam(const ParamMacroModeData& masterTargetParamMacro)
{
    auto linkLine = mManualMsg.linkLineMap.value(KaDS);
    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(linkLine.workMode);

    auto masterTargetInfo = linkLine.targetMap.value(linkLine.masterTargetNo);
    auto masterTargetTaskCode = masterTargetInfo.taskCode;
    auto masterTargetPointFreqNo = masterTargetInfo.pointFreqNo;

    QVariant downFreq;
    if (!MacroCommon::getDownFrequency(masterTargetParamMacro, masterTargetPointFreqNo, downFreq))
    {
        emit signalErrorMsg(QString("获取任务代号为：%1的参数宏%2模式的下行频率失败").arg(masterTargetTaskCode, workModeDesc));
        return false;
    }

    /* 获取设备ID拿到整个设备的数据 */
    auto fmtKey = QString("Step_KaDSTrack_FreqGain_Add_%1");
    int miniUnitID = 5;
    QMap<int, QVariantMap> kaLowSpeedTrackUnitParamMap;
    auto kaDStrackDeviceID = mSingleCommandHelper.getSingleCmdDeviceID(fmtKey.arg(miniUnitID), "0x3102");
    masterTargetParamMacro.getDeviceUnitParamMap(kaDStrackDeviceID, kaLowSpeedTrackUnitParamMap);

    auto waitCmdSendFunc = [&](const QString& fmtKey, const QVariant& downFreq, int unitID) {
        auto& replaceMap = kaLowSpeedTrackUnitParamMap[unitID];
        replaceMap["SBandDownFreq"] = downFreq;

        mSingleCommandHelper.packSingleCommand(fmtKey.arg(unitID), mPackCommand, kaDStrackDeviceID, replaceMap);
        appendExecQueue(mPackCommand);
    };
    /*
     * 低速用5、6、7、8、9、10
     * 高速用7、8、9、10
     *
     * 高速不用管56他走基带是独立的线路
     * 低速走基带走的是56所以低速要先发56然后再判断是否使用跟踪
     */

    /* 和路 */
    waitCmdSendFunc(fmtKey.arg(miniUnitID), downFreq, miniUnitID);

    ++miniUnitID;
    waitCmdSendFunc(fmtKey.arg(miniUnitID), downFreq, miniUnitID);

    /* 差路 */
    int enable = 0;
    mManualMsg.configMacroData.getDTGSOrDS(linkLine.workMode, enable);
    if (enable == 2)
    {
        miniUnitID = 7;
        fmtKey = QString("Step_KaDSTrack_FreqGain_Diff_%1");
        for (auto iter = kaLowSpeedTrackUnitParamMap.begin(); iter != kaLowSpeedTrackUnitParamMap.end(); ++iter)
        {
            ExitCheck(false);
            auto unitID = iter.key();
            if (unitID < miniUnitID)
                continue;
            waitCmdSendFunc(fmtKey, downFreq, unitID);
        }
    }

    return true;
}
bool DSParamMacroHandler::xgpxParam(const ParamMacroModeData& masterTargetParamMacro)
{
    auto linkLine = mManualMsg.linkLineMap.value(XDS);
    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(linkLine.workMode);

    auto masterTargetInfo = linkLine.targetMap.value(linkLine.masterTargetNo);
    auto masterTargetTaskCode = masterTargetInfo.taskCode;
    auto masterTargetPointFreqNo = masterTargetInfo.pointFreqNo;

    QVariant downFreq;
    if (!MacroCommon::getDownFrequency(masterTargetParamMacro, masterTargetPointFreqNo, downFreq))
    {
        emit signalErrorMsg(QString("获取任务代号为：%1的参数宏%2模式的下行频率失败").arg(masterTargetTaskCode, workModeDesc));
        return false;
    }

    // x频段高频箱数传下行频率和增益
    int miniUnitID = 6;
    auto fmtKey = QString("Step_xgpx_sFreq_%1");
    auto xpdgpxDeviceID = mSingleCommandHelper.getSingleCmdDeviceID(fmtKey.arg(miniUnitID), "0x3104");

    QMap<int, QVariantMap> xpdgpxUnitParamMap;
    masterTargetParamMacro.getDeviceUnitParamMap(xpdgpxDeviceID, xpdgpxUnitParamMap);

    for (auto iter = xpdgpxUnitParamMap.begin(); iter != xpdgpxUnitParamMap.end(); ++iter)
    {
        auto unitID = iter.key();
        if (unitID < miniUnitID)
            continue;

        auto xgzxParamMap = iter.value();
        xgzxParamMap["SBandDownFreq"] = downFreq;
        mSingleCommandHelper.packSingleCommand(QString("Step_xgpx_sFreq_%1").arg(unitID), mPackCommand, xpdgpxDeviceID, xgzxParamMap);
        appendExecQueue(mPackCommand);
    }

    return true;
}

bool DSParamMacroHandler::kafsgpxParam(const ParamMacroModeData& masterTargetParamMacro)
{
    auto linkLine = mManualMsg.linkLineMap.value(KaDS);
    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(linkLine.workMode);

    auto masterTargetInfo = linkLine.targetMap.value(linkLine.masterTargetNo);
    auto masterTargetTaskCode = masterTargetInfo.taskCode;
    auto masterTargetPointFreqNo = masterTargetInfo.pointFreqNo;

    QVariant upFreq;
    if (!MacroCommon::getUpFrequency(masterTargetParamMacro, masterTargetPointFreqNo, upFreq))
    {
        emit signalErrorMsg(QString("获取任务代号为：%1的参数宏%2模式的上行频率失败").arg(masterTargetTaskCode, workModeDesc));
        return false;
    }

    // ka低速数传发射高频箱
    int miniUnitID = 2;
    auto fmtKey = QString("Step_KaDSsend_gain_%1");
    auto kaDSsendDeviceID = mSingleCommandHelper.getSingleCmdDeviceID(fmtKey.arg(miniUnitID), "0x2401");
    QMap<int, QVariantMap> kaLowSpeedsendUnitParamMap;
    masterTargetParamMacro.getDeviceUnitParamMap(kaDSsendDeviceID, kaLowSpeedsendUnitParamMap);

    for (auto iter = kaLowSpeedsendUnitParamMap.begin(); iter != kaLowSpeedsendUnitParamMap.end(); ++iter)
    {
        auto unitID = iter.key();
        if (unitID < miniUnitID)
            continue;

        auto& kaDSsendMap = kaLowSpeedsendUnitParamMap[unitID];
        kaDSsendMap["WorkRate"] = upFreq;

        mSingleCommandHelper.packSingleCommand(fmtKey.arg(unitID), mPackCommand, kaDSsendDeviceID, kaDSsendMap);
        appendExecQueue(mPackCommand);
    }

    return true;
}
bool DSParamMacroHandler::kagfParam(const ParamMacroModeData& masterTargetParamMacro)
{
    auto linkLine = mManualMsg.linkLineMap.value(KaDS);
    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(linkLine.workMode);

    auto masterTargetInfo = linkLine.targetMap.value(linkLine.masterTargetNo);
    auto masterTargetTaskCode = masterTargetInfo.taskCode;
    auto masterTargetPointFreqNo = masterTargetInfo.pointFreqNo;

    QVariant upFreq;
    if (!MacroCommon::getUpFrequency(masterTargetParamMacro, masterTargetPointFreqNo, upFreq))
    {
        emit signalErrorMsg(QString("获取任务代号为：%1的参数宏%2模式的上行频率失败").arg(masterTargetTaskCode, workModeDesc));
        return false;
    }

    //设置ka数传高功放300w
    auto stepKey = QString("Step_kaSCG300W_TransmPower");
    auto kackggfDeviceID = mSingleCommandHelper.getSingleCmdDeviceID(stepKey, "0x2005");
    QMap<int, QVariantMap> kackggfUnitParamMap;
    masterTargetParamMacro.getDeviceUnitParamMap(kackggfDeviceID, kackggfUnitParamMap);
    auto kackggfParamMap = kackggfUnitParamMap[1];
    kackggfParamMap["WorkRate"] = 22900;
    mSingleCommandHelper.packSingleCommand(stepKey, mPackCommand, kackggfDeviceID, kackggfParamMap);
    appendExecQueue(mPackCommand);

    // ka数传高功放30w
    stepKey = QString("Step_kaSCG30W_TransmPower");
    auto kaCKggf30wDeviceID = mSingleCommandHelper.getSingleCmdDeviceID(stepKey, "0x2006");
    QMap<int, QVariantMap> kackggf30wUnitParamMap;
    masterTargetParamMacro.getDeviceUnitParamMap(kaCKggf30wDeviceID, kackggf30wUnitParamMap);
    auto kackggf30wParamMap = kackggf30wUnitParamMap[1];
    kackggf30wParamMap["WorkRate"] = 22900;
    mSingleCommandHelper.packSingleCommand(stepKey, mPackCommand, kaCKggf30wDeviceID, kackggf30wParamMap);
    appendExecQueue(mPackCommand);

    return true;
}
