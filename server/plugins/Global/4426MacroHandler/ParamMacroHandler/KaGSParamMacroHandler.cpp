#include "KaGSParamMacroHandler.h"

#include "BusinessMacroCommon.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"

KaGSParamMacroHandler::KaGSParamMacroHandler(QObject* parent)
    : BaseParamMacroHandler(parent)
{
}
/*
 * 20211108 wp?
 * 更新
 * 高速其实是可以没有主跟目标的
 * 他可以不跟踪的
 */
bool KaGSParamMacroHandler::handle()
{
    // 高速基带
    kaGSJDParam();
    ExitCheck(false);

    /* 跟踪 */
    if (m_paramMacroModeData.pointFreqParamMap.isEmpty())
    {
        m_paramMacroModeData = m_paramMacroModeData1;
    }

    // Ka高速下变频器
    kaGSXBPQParam();
    ExitCheck(false);

    // Ka低速数传及跟踪高频箱
    kaDSSCGZGPXParam(m_paramMacroModeData);
    ExitCheck(false);

    // 执行跟踪数字化前端组参数设置
    gzqdParam(m_paramMacroModeData);
    return true;
}

SystemWorkMode KaGSParamMacroHandler::getSystemWorkMode() { return SystemWorkMode::KaGS; }

bool KaGSParamMacroHandler::kaGSJDParam()
{
    // 获取主用高速基带ID
    auto result = getGSJDDeviceID();
    auto msg = result.msg();
    if (!msg.isEmpty())
    {
        emit signalErrorMsg(msg);
    }
    if (!result)
    {
        return false;
    }
    auto gsjdInfo = result.value();

    bool jdParamExecResult = false;
    if (mLinkLine.targetMap.size() == 1)
    {
        /* 单点频 */
        jdParamExecResult = singlePointFreq(gsjdInfo);
    }
    else if (mLinkLine.targetMap.size() == 2)
    {
        jdParamExecResult = doublePointFreq(gsjdInfo);
    }
    else
    {
        emit signalErrorMsg("当前Ka高速点频数量大于2,当前系统不支持");
        return false;
    }

    clearQueue();

    /* 默认打开送数 */
    mSingleCommandHelper.packSingleCommand("StepGSJDSS_Start_1", mPackCommand, gsjdInfo.master);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepGSJDSS_Start_2", mPackCommand, gsjdInfo.master);
    appendExecQueue(mPackCommand);

    mSingleCommandHelper.packSingleCommand("StepGSJDSS_Start_1", mPackCommand, gsjdInfo.slave);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepGSJDSS_Start_2", mPackCommand, gsjdInfo.slave);
    appendExecQueue(mPackCommand);

    /* 默认打开关闭存盘 */
    mSingleCommandHelper.packSingleCommand("StepGSJDCP_Stop_I1", mPackCommand, gsjdInfo.master);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepGSJDCP_Stop_Q1", mPackCommand, gsjdInfo.master);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepGSJDCP_Stop_I2", mPackCommand, gsjdInfo.master);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepGSJDCP_Stop_Q2", mPackCommand, gsjdInfo.master);
    appendExecQueue(mPackCommand);

    mSingleCommandHelper.packSingleCommand("StepGSJDCP_Stop_I1", mPackCommand, gsjdInfo.slave);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepGSJDCP_Stop_Q1", mPackCommand, gsjdInfo.slave);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepGSJDCP_Stop_I2", mPackCommand, gsjdInfo.slave);
    appendExecQueue(mPackCommand);
    mSingleCommandHelper.packSingleCommand("StepGSJDCP_Stop_Q2", mPackCommand, gsjdInfo.slave);
    appendExecQueue(mPackCommand);

    execQueue();

    return jdParamExecResult;
}
void KaGSParamMacroHandler::gsMNY(QMap<int, QMap<int, QVariantMap>>& unitTargetParamMap)
{
    unitTargetParamMap[10][0] = unitTargetParamMap[2][0];

    auto varMap = unitTargetParamMap[3][0];
    for (auto iter = varMap.begin(); iter != varMap.end(); ++iter)  // I路
    {
        unitTargetParamMap[10][0].insert(QString("I_%1").arg(iter.key()), iter.value());
    }
    varMap = unitTargetParamMap[4][0];
    for (auto iter = varMap.begin(); iter != varMap.end(); ++iter)  // Q路
    {
        unitTargetParamMap[10][0].insert(QString("Q_%1").arg(iter.key()), iter.value());
    }
}
bool KaGSParamMacroHandler::singlePointFreq(const MasterSlaveDeviceIDInfo& gsjdInfo)
{
    //下发的参数Map
    QMap<int, QMap<int, QVariantMap>> unitTargetParamMap;  // 点频1的参数宏数据
    // 点频1的目标信息获取
    auto targetInfo1 = mLinkLine.targetMap.first();
    SatelliteManagementData satelliteData1;
    SystemOrientation orientation1 = SystemOrientation::Unkonwn;
    QMap<int, QVariantMap> pointFreqUnitParamMap1;
    ParamMacroModeData paramMacroModeData;
    auto validDeviceID = gsjdInfo.master.isValid() ? gsjdInfo.master : gsjdInfo.slave;
    if (!targetInfoMapParamDown(satelliteData1, orientation1, m_downFreq1, pointFreqUnitParamMap1, paramMacroModeData, targetInfo1, validDeviceID))
    {
        return false;
    }

    /* 单点频所有频率都一样 */
    m_downFreq2 = m_downFreq1;
    /* 跟踪 */
    m_paramMacroModeData = paramMacroModeData;
    m_paramMacroModeData1 = paramMacroModeData;
    m_paramMacroModeData2 = paramMacroModeData;

    if (orientation1 == SystemOrientation::LCircumflex || orientation1 == SystemOrientation::RCircumflex)
    {
        unitTargetParamMap[1][0]["XTaskIdent"] = satelliteData1.m_satelliteIdentification;  //任务标识1
        unitTargetParamMap[1][0]["XTaskCode"] = satelliteData1.m_satelliteCode;             //任务代号1
        // 通道一的单元
        unitTargetParamMap[2][0] = pointFreqUnitParamMap1[2];
        unitTargetParamMap[3][0] = pointFreqUnitParamMap1[3];
        unitTargetParamMap[4][0] = pointFreqUnitParamMap1[4];
    }
    else if (orientation1 == SystemOrientation::LRCircumflex)
    {
        unitTargetParamMap[1][0]["XTaskIdent"] = satelliteData1.m_satelliteIdentification;    //任务标识1
        unitTargetParamMap[1][0]["XTaskCode"] = satelliteData1.m_satelliteCode;               //任务代号1
        unitTargetParamMap[1][0]["KaTaskIdent2"] = satelliteData1.m_satelliteIdentification;  //任务标识2
        unitTargetParamMap[1][0]["KaTaskCode2"] = satelliteData1.m_satelliteCode;             //任务代号2

        // 通道一的单元
        unitTargetParamMap[2][0] = pointFreqUnitParamMap1[2];
        unitTargetParamMap[3][0] = pointFreqUnitParamMap1[3];
        unitTargetParamMap[4][0] = pointFreqUnitParamMap1[4];
        // 通道二的单元
        unitTargetParamMap[6][0] = pointFreqUnitParamMap1[6];
        unitTargetParamMap[7][0] = pointFreqUnitParamMap1[7];
        unitTargetParamMap[8][0] = pointFreqUnitParamMap1[8];
    }

    // 设置模拟源单元参数，默认为通道一的参数
    gsMNY(unitTargetParamMap);

    //处理特殊参数
    dealWithSpcialParam(unitTargetParamMap);

    clearQueue();

    packGroupParamSetData(gsjdInfo.master, 0xFFFF, unitTargetParamMap, mPackCommand);
    appendExecQueue(mPackCommand);

    packGroupParamSetData(gsjdInfo.slave, 0xFFFF, unitTargetParamMap, mPackCommand);
    appendExecQueue(mPackCommand);

    execQueue();

    return true;
}

bool KaGSParamMacroHandler::doublePointFreq(const MasterSlaveDeviceIDInfo& gsjdInfo)
{
    /* 点频1 数据 */
    // 点频1的目标信息获取
    auto targetInfo1 = *(mLinkLine.targetMap.begin());
    SatelliteManagementData satelliteData1;
    SystemOrientation orientation1 = SystemOrientation::Unkonwn;
    QMap<int, QVariantMap> pointFreqUnitParamMap1;
    auto validDeviceID = gsjdInfo.master.isValid() ? gsjdInfo.master : gsjdInfo.slave;
    if (!targetInfoMapParamDown(satelliteData1, orientation1, m_downFreq1, pointFreqUnitParamMap1, m_paramMacroModeData1, targetInfo1, validDeviceID))
    {
        return false;
    }
    auto targetInfoTemp = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);
    if (targetInfo1.pointFreqNo == targetInfoTemp.pointFreqNo)
    {
        m_paramMacroModeData = m_paramMacroModeData1;
    }

    // 点频2的目标信息获取
    auto targetInfo2 = *(++mLinkLine.targetMap.begin());
    SatelliteManagementData satelliteData2;
    SystemOrientation orientation2 = SystemOrientation::Unkonwn;
    QMap<int, QVariantMap> pointFreqUnitParamMap2;
    if (!targetInfoMapParamDown(satelliteData2, orientation2, m_downFreq2, pointFreqUnitParamMap2, m_paramMacroModeData2, targetInfo2, validDeviceID))
    {
        return false;
    }
    if (targetInfo2.pointFreqNo == targetInfoTemp.pointFreqNo)
    {
        m_paramMacroModeData = m_paramMacroModeData2;
    }
    //下发的参数Map
    QMap<int, QMap<int, QVariantMap>> unitTargetParamMap1;  // 点频1的参数宏数据
    QMap<int, QMap<int, QVariantMap>> unitTargetParamMap2;  // 点频2的参数宏数据

    // 两个点频都是单极化，两个点频数据往同一个基带下
    if (orientation1 != SystemOrientation::LRCircumflex && orientation2 != SystemOrientation::LRCircumflex)
    {
        unitTargetParamMap1[1][0]["XTaskIdent"] = satelliteData1.m_satelliteIdentification;    // 任务标识1
        unitTargetParamMap1[1][0]["XTaskCode"] = satelliteData1.m_satelliteCode;               // 任务代号1
        unitTargetParamMap1[1][0]["KaTaskIdent2"] = satelliteData2.m_satelliteIdentification;  // 任务标识2
        unitTargetParamMap1[1][0]["KaTaskCode2"] = satelliteData2.m_satelliteCode;             // 任务代号2
        // 通道一的单元 使用点频1
        unitTargetParamMap1[2][0] = pointFreqUnitParamMap1[2];
        unitTargetParamMap1[3][0] = pointFreqUnitParamMap1[3];
        unitTargetParamMap1[4][0] = pointFreqUnitParamMap1[4];
        // 通道二的单元 使用点频2
        unitTargetParamMap1[6][0] = pointFreqUnitParamMap2[2];
        unitTargetParamMap1[7][0] = pointFreqUnitParamMap2[3];
        unitTargetParamMap1[8][0] = pointFreqUnitParamMap2[4];

        unitTargetParamMap2 = unitTargetParamMap1;  // 两台高速基带下同样参数
    }
    else  //要么两个点频一个单极化,一个双极化  要么都是双极化,所以里面还得判断
    {
        if (orientation1 == SystemOrientation::LCircumflex || orientation1 == SystemOrientation::RCircumflex)
        {
            unitTargetParamMap1[1][0]["XTaskIdent"] = satelliteData1.m_satelliteIdentification;  //任务标识1
            unitTargetParamMap1[1][0]["XTaskCode"] = satelliteData1.m_satelliteCode;             //任务代号1
            // 通道一的单元
            unitTargetParamMap1[2][0] = pointFreqUnitParamMap1[2];
            unitTargetParamMap1[3][0] = pointFreqUnitParamMap1[3];
            unitTargetParamMap1[4][0] = pointFreqUnitParamMap1[4];
        }
        else if (orientation1 == SystemOrientation::LRCircumflex)
        {
            unitTargetParamMap1[1][0]["XTaskIdent"] = satelliteData1.m_satelliteIdentification;    //任务标识1
            unitTargetParamMap1[1][0]["XTaskCode"] = satelliteData1.m_satelliteCode;               //任务代号1
            unitTargetParamMap1[1][0]["KaTaskIdent2"] = satelliteData1.m_satelliteIdentification;  //任务标识2
            unitTargetParamMap1[1][0]["KaTaskCode2"] = satelliteData1.m_satelliteCode;             //任务代号2

            // 通道一的单元
            unitTargetParamMap1[2][0] = pointFreqUnitParamMap1[2];
            unitTargetParamMap1[3][0] = pointFreqUnitParamMap1[3];
            unitTargetParamMap1[4][0] = pointFreqUnitParamMap1[4];
            // 通道二的单元         1
            unitTargetParamMap1[6][0] = pointFreqUnitParamMap1[6];
            unitTargetParamMap1[7][0] = pointFreqUnitParamMap1[7];
            unitTargetParamMap1[8][0] = pointFreqUnitParamMap1[8];
        }

        if (orientation2 == SystemOrientation::LCircumflex || orientation2 == SystemOrientation::RCircumflex)
        {
            unitTargetParamMap2[1][0]["XTaskIdent"] = satelliteData2.m_satelliteIdentification;  //任务标识1
            unitTargetParamMap2[1][0]["XTaskCode"] = satelliteData2.m_satelliteCode;             //任务代号1
            // 通道一的单元
            unitTargetParamMap2[2][0] = pointFreqUnitParamMap2[2];
            unitTargetParamMap2[3][0] = pointFreqUnitParamMap2[3];
            unitTargetParamMap2[4][0] = pointFreqUnitParamMap2[4];
        }
        else if (orientation2 == SystemOrientation::LRCircumflex)
        {
            unitTargetParamMap2[1][0]["XTaskIdent"] = satelliteData2.m_satelliteIdentification;    //任务标识1
            unitTargetParamMap2[1][0]["XTaskCode"] = satelliteData2.m_satelliteCode;               //任务代号1
            unitTargetParamMap2[1][0]["KaTaskIdent2"] = satelliteData2.m_satelliteIdentification;  //任务标识2
            unitTargetParamMap2[1][0]["KaTaskCode2"] = satelliteData2.m_satelliteCode;             //任务代号2

            // 通道一的单元
            unitTargetParamMap2[2][0] = pointFreqUnitParamMap2[2];
            unitTargetParamMap2[3][0] = pointFreqUnitParamMap2[3];
            unitTargetParamMap2[4][0] = pointFreqUnitParamMap2[4];
            // 通道二的单元         1
            unitTargetParamMap2[6][0] = pointFreqUnitParamMap2[6];
            unitTargetParamMap2[7][0] = pointFreqUnitParamMap2[7];
            unitTargetParamMap2[8][0] = pointFreqUnitParamMap2[8];
        }
    }

    // 设置模拟源单元参数，默认为通道一的参数
    gsMNY(unitTargetParamMap1);
    gsMNY(unitTargetParamMap2);

    //处理特殊参数
    dealWithSpcialParam(unitTargetParamMap1);
    dealWithSpcialParam(unitTargetParamMap2);

    // 获取主用高速基带数量
    int masterGSJDNumber = 0;
    if (!getGSJDMasterNumber(masterGSJDNumber))
    {
        return false;
    }

    clearQueue();
    // 一主一备，两台高速基带下相同参数  这里需要这个判断的原因是双点频单极化是往一个基带下发的
    if (masterGSJDNumber == 1)
    {
        if (gsjdInfo.master.isValid())
        {
            packGroupParamSetData(gsjdInfo.master, 0xFFFF, unitTargetParamMap1, mPackCommand);
            appendExecQueue(mPackCommand);
        }

        if (gsjdInfo.slave.isValid())
        {
            packGroupParamSetData(gsjdInfo.slave, 0xFFFF, unitTargetParamMap1, mPackCommand);
            appendExecQueue(mPackCommand);
        }
    }
    // 两主，第一台主下点频1参数，第二台主下点频2参数
    else if (masterGSJDNumber == 2)
    {
        if (gsjdInfo.master.isValid())
        {
            packGroupParamSetData(gsjdInfo.master, 0xFFFF, unitTargetParamMap1, mPackCommand);
            appendExecQueue(mPackCommand);
        }

        if (gsjdInfo.slave.isValid())
        {
            packGroupParamSetData(gsjdInfo.slave, 0xFFFF, unitTargetParamMap2, mPackCommand);
            appendExecQueue(mPackCommand);
        }
    }
    execQueue();

    return true;
}

bool KaGSParamMacroHandler::kaGSXBPQParam()
{
    DeviceID deviceID;
    int unitId1 = 0;
    int unitId2 = 0;
    int unitId3 = 0;
    int unitId4 = 0;

    mManualMsg.configMacroData.getKaGSXBBQInfo(mLinkLine.workMode, deviceID, unitId1, 1);
    mManualMsg.configMacroData.getKaGSXBBQInfo(mLinkLine.workMode, deviceID, unitId2, 2);
    mManualMsg.configMacroData.getKaGSXBBQInfo(mLinkLine.workMode, deviceID, unitId3, 3);
    mManualMsg.configMacroData.getKaGSXBBQInfo(mLinkLine.workMode, deviceID, unitId4, 4);

    // 下变频编号和单元编号相差2
    unitId1 += 2;
    unitId2 += 2;
    unitId3 += 2;
    unitId4 += 2;

    auto waitCmdSendFunc = [&](const ParamMacroModeData& paramMacroModeData, const QVariant& downFreq, int unitID) {
        QVariantMap replaceMap;
        paramMacroModeData.getDeviceUnitParamMap(deviceID, unitID, replaceMap);
        replaceMap["SBandDownFreq"] = downFreq;

        mSingleCommandHelper.packSingleCommand(QString("Step_KaGSSCGPX_freq_gain_%1").arg(unitID), mPackCommand, deviceID, replaceMap);
        appendExecQueue(mPackCommand);
    };
    clearQueue();
    // 单点频
    if (mLinkLine.targetMap.size() == 1)
    {
        waitCmdSendFunc(m_paramMacroModeData1, m_downFreq1, unitId1);
        waitCmdSendFunc(m_paramMacroModeData1, m_downFreq1, unitId3);

        waitCmdSendFunc(m_paramMacroModeData1, m_downFreq1, unitId2);
        waitCmdSendFunc(m_paramMacroModeData1, m_downFreq1, unitId4);
    }
    else if (mLinkLine.targetMap.size() == 2)  // 双点频
    {
        waitCmdSendFunc(m_paramMacroModeData1, m_downFreq1, unitId1);
        waitCmdSendFunc(m_paramMacroModeData1, m_downFreq1, unitId3);

        waitCmdSendFunc(m_paramMacroModeData2, m_downFreq2, unitId2);
        waitCmdSendFunc(m_paramMacroModeData2, m_downFreq2, unitId4);
    }
    execQueue();

    return true;
}

bool KaGSParamMacroHandler::kaDSSCGZGPXParam(const ParamMacroModeData& masterTargetParamMacro)
{
    /*
     * 低速用5、6、7、8、9、10
     * 高速用7、8、9、10
     *
     * 高速不用管56他走基带是独立的线路
     * 低速走基带走的是56所以低速要先发56然后再判断是否使用跟踪
     */
    /* 这个是检查高速和低速冲突的 */
    int enable = 0;
    mManualMsg.configMacroData.getDTGSOrDS(mLinkLine.workMode, enable);
    if (enable != 1)
    {
        return true;
    }

    // Ka低速数传及跟踪高频箱后4路增益以及下发频率，下行频率默认使用第一个点频下行频率
    auto fmtKey = QString("Step_KaDSTrack_FreqGain_Diff_%1");
    int miniUnitID = 7;
    QMap<int, QVariantMap> kaLowSpeedTrackUnitParamMap;
    auto kaDStrackDeviceID = mSingleCommandHelper.getSingleCmdDeviceID(fmtKey.arg(miniUnitID), "0x3102");
    masterTargetParamMacro.getDeviceUnitParamMap(kaDStrackDeviceID, kaLowSpeedTrackUnitParamMap);

    clearQueue();

    for (auto iter = kaLowSpeedTrackUnitParamMap.begin(); iter != kaLowSpeedTrackUnitParamMap.end(); ++iter)
    {
        ExitCheck(false);
        auto unitID = iter.key();
        auto& kaDStrackMap = iter.value();
        if (unitID < miniUnitID)
        {
            continue;
        }

        kaDStrackMap["SBandDownFreq"] = m_downFreq1;
        mSingleCommandHelper.packSingleCommand(fmtKey.arg(unitID), mPackCommand, kaDStrackDeviceID, kaDStrackMap);
        appendExecQueue(mPackCommand);
    }

    execQueue();

    return true;
}
void KaGSParamMacroHandler::dealWithSpcialParam(QMap<int, QMap<int, QVariantMap>>& unitTargetParamMap)
{
    {
        //通道1解调单元码速率处理
        auto& ModulatSystem = unitTargetParamMap[2][0]["ModulatSystem"];  //调制体制
        auto& allCodeSpeed = unitTargetParamMap[2][0]["CodeRate"];        //总码率
        auto& ICodeSpeed = unitTargetParamMap[2][0]["ICodeRate"];         // I路码速率
        auto& QCodeSpeed = unitTargetParamMap[2][0]["QCodeRate"];         // Q路码速率

        if (ModulatSystem != 3)  //不为UQPSK时I和Q的码速率值就为总码率
        {
            ICodeSpeed = allCodeSpeed;
            QCodeSpeed = allCodeSpeed;
        }

        //通道2解调单元码速率处理
        auto& ModulatSystem2 = unitTargetParamMap[6][0]["ModulatSystem"];  //调制体制
        auto& allCodeSpeed2 = unitTargetParamMap[6][0]["CodeRate"];        //总码率
        auto& ICodeSpeed2 = unitTargetParamMap[6][0]["ICodeRate"];         // I路码速率
        auto& QCodeSpeed2 = unitTargetParamMap[6][0]["QCodeRate"];         // Q路码速率

        if (ModulatSystem2 != 3)  //不为UQPSK时I和Q的码速率值就为总码率
        {
            ICodeSpeed2 = allCodeSpeed2;
            QCodeSpeed2 = allCodeSpeed2;
        }

        //上面处理完后让模拟源也处理下
        unitTargetParamMap[10][0]["ICodeRate"] = unitTargetParamMap[2][0]["ICodeRate"];
        unitTargetParamMap[10][0]["QCodeRate"] = unitTargetParamMap[2][0]["QCodeRate"];
    }

    /*
     * 高速基带LDPC锁定需要解调单元前解扰对应模拟源单元后加扰,后解扰对应前加扰 ---所里童芳给吴天煜说的,2021/11/5
     */
    {
        // I路前加扰处理
        unitTargetParamMap[10][0]["I_PreDecodDesSwitch"] = unitTargetParamMap[3][0]["DecodPostSwitch"];
        unitTargetParamMap[10][0]["I_PreDecodScraPoly"] = unitTargetParamMap[3][0]["DecodPostPolyn"];
        unitTargetParamMap[10][0]["I_PreScrambPolyLength"] = unitTargetParamMap[3][0]["DecodScrambPolyLength"];
        unitTargetParamMap[10][0]["I_PreDecodInitTerm"] = unitTargetParamMap[3][0]["DecodInitTerm"];
        unitTargetParamMap[10][0]["I_DecodOutPosit"] = unitTargetParamMap[3][0]["DecodOutputPosit"];
        // I路后加扰处理
        unitTargetParamMap[10][0]["I_DecodPostSwitch"] = unitTargetParamMap[3][0]["PreDecodDesSwitch"];
        unitTargetParamMap[10][0]["I_DecodPostPolyn"] = unitTargetParamMap[3][0]["PreDecodScraPoly"];
        unitTargetParamMap[10][0]["I_DecodScrambPolyLength"] = unitTargetParamMap[3][0]["PreScrambPolyLength"];
        unitTargetParamMap[10][0]["I_DecodInitTerm"] = unitTargetParamMap[3][0]["PreDecodInitTerm"];
        unitTargetParamMap[10][0]["I_DecodOutputPosit"] = unitTargetParamMap[3][0]["DecodOutPosit"];
        // Q路前加扰处理
        unitTargetParamMap[10][0]["Q_PreDecodDesSwitch"] = unitTargetParamMap[4][0]["DecodPostSwitch"];
        unitTargetParamMap[10][0]["Q_PreDecodScraPoly"] = unitTargetParamMap[4][0]["DecodPostPolyn"];
        unitTargetParamMap[10][0]["Q_PreScrambPolyLength"] = unitTargetParamMap[4][0]["DecodScrambPolyLength"];
        unitTargetParamMap[10][0]["Q_PreDecodInitTerm"] = unitTargetParamMap[4][0]["DecodInitTerm"];
        unitTargetParamMap[10][0]["Q_DecodOutPosit"] = unitTargetParamMap[4][0]["DecodOutputPosit"];
        // Q路后加扰处理
        unitTargetParamMap[10][0]["Q_DecodPostSwitch"] = unitTargetParamMap[4][0]["PreDecodDesSwitch"];
        unitTargetParamMap[10][0]["Q_DecodPostPolyn"] = unitTargetParamMap[4][0]["PreDecodScraPoly"];
        unitTargetParamMap[10][0]["Q_DecodScrambPolyLength"] = unitTargetParamMap[4][0]["PreScrambPolyLength"];
        unitTargetParamMap[10][0]["Q_DecodInitTerm"] = unitTargetParamMap[4][0]["PreDecodInitTerm"];
        unitTargetParamMap[10][0]["Q_DecodOutputPosit"] = unitTargetParamMap[4][0]["DecodOutPosit"];
        //默认噪声开关关掉
        unitTargetParamMap[10][0]["NoiseSwitch"] = 2;
    }
}
