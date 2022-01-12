#include "STTCParamMacroHandler.h"

#include "BusinessMacroCommon.h"
#include "EquipmentCombinationHelper.h"
#include "GlobalData.h"
#include "SatelliteManagementDefine.h"

STTCParamMacroHandler::STTCParamMacroHandler(QObject* parent)
    : BaseParamMacroHandler(parent)
{
}

bool STTCParamMacroHandler::handle()
{
    ParamMacroModeData paramMacroModeItem;
    {
        /* 如果是单模式则前端频率和当前频率相同
         * 否则前端是S的频率 当前是当前的
         */
        TargetInfo targetInfoFreq;
        TargetInfo targetInfoParamMacro;
        if (isDoubleMode())
        {
            targetInfoFreq = mManualMsg.linkLineMap.value(Skuo2).targetMap.value(mLinkLine.masterTargetNo);
            targetInfoParamMacro = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);
        }
        else
        {
            targetInfoFreq = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);
            targetInfoParamMacro = targetInfoFreq;
        }

        // TODO 记录当前链路的使用情况
        auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(targetInfoParamMacro.workMode);

        // 获取指定任务代号的参数宏数据
        if (!GlobalData::getParamMacroModeData(targetInfoParamMacro.taskCode, targetInfoParamMacro.workMode, paramMacroModeItem))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式参数失败").arg(targetInfoParamMacro.taskCode, workModeDesc));
            return false;
        }

        // 获取上行和下行的频率
        if (!MacroCommon::getUpFrequency(paramMacroModeItem, targetInfoParamMacro.pointFreqNo, m_upFrequency))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式点频为%3的上行频率失败")
                                    .arg(targetInfoParamMacro.taskCode, workModeDesc)
                                    .arg(targetInfoParamMacro.pointFreqNo));
            return false;
        }
        if (!MacroCommon::getDownFrequency(paramMacroModeItem, targetInfoParamMacro.pointFreqNo, m_downFrequency))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式点频为%3的下行频率失败")
                                    .arg(targetInfoParamMacro.taskCode, workModeDesc)
                                    .arg(targetInfoParamMacro.pointFreqNo));
            return false;
        }

        if (targetInfoFreq.workMode == targetInfoParamMacro.workMode)
        {
            m_qdUpFrequency = m_upFrequency;
            m_qdDownFrequency = m_downFrequency;
        }
        else
        {
            ParamMacroModeData paramMacroModeDataFreq;
            workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(targetInfoFreq.workMode);
            if (!GlobalData::getParamMacroModeData(targetInfoFreq.taskCode, targetInfoFreq.workMode, paramMacroModeDataFreq))
            {
                emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式参数失败").arg(targetInfoFreq.taskCode, workModeDesc));
                return false;
            }

            // 获取上行和下行的频率
            if (!MacroCommon::getUpFrequency(paramMacroModeDataFreq, targetInfoFreq.pointFreqNo, m_qdUpFrequency))
            {
                emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式点频为%3的上行频率失败")
                                        .arg(targetInfoFreq.taskCode, workModeDesc)
                                        .arg(targetInfoFreq.pointFreqNo));
                return false;
            }
            if (!MacroCommon::getDownFrequency(paramMacroModeDataFreq, targetInfoFreq.pointFreqNo, m_qdDownFrequency))
            {
                emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式点频为%3的下行频率失败")
                                        .arg(targetInfoFreq.taskCode, workModeDesc)
                                        .arg(targetInfoFreq.pointFreqNo));
                return false;
            }
        }
    }

    // 判断是否是双测控
    QSet<SystemWorkMode> workModeSet;
    for (auto& link : mManualMsg.linkLineMap)
    {
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(link.workMode))
        {
            workModeSet << link.workMode;
        }
    }

    /* 提前判断一下 双模式是S扩2+Ka扩2对于测控基带只下一次宏就行
     * 双主机也只下一次宏
     * 跳过第二次下宏
     */
    if ((workModeSet.size() >= 2 && (mLinkLine.workMode == *(workModeSet.begin()))) || workModeSet.size() == 1)
    {
        // 执行测控基带组参数设置
        ckjdParam(workModeSet);
        ExitCheck(false);

        // 执行测控数字化前端组参数设置
        ckqgParam(workModeSet, paramMacroModeItem);
        ExitCheck(false);
    }

    // 执行跟踪数字化前端组参数设置
    gzqdParam(paramMacroModeItem, m_downFrequency);
    ExitCheck(false);

    // 其他设备
    otherDeviceParam(paramMacroModeItem);

    return true;
}

SystemWorkMode STTCParamMacroHandler::getSystemWorkMode() { return SystemWorkMode::STTC; }
bool STTCParamMacroHandler::ckjdParam(const QSet<SystemWorkMode>& workModeSet)
{
    // 获取测控基带ID
    auto result = getCKJDControlInfo(mLinkLine.workMode);
    auto msg = result.msg();
    if (!msg.isEmpty())
    {
        emit signalErrorMsg(msg);
    }
    if (!result)
    {
        return false;
    }
    m_ckjdInfo = result.value();

    if (workModeSet.size() == 1)
    {
        singleMode(workModeSet);
    }
    else if (m_ckjdInfo.afterDoubleMode)
    {
        doubleMode(workModeSet);
    }
    else
    {
        doubleMasterMode(workModeSet);
    }

    clearQueue();

    ckTaskSS(workModeSet);
    zzzz(workModeSet);

    execQueue();
    return true;
}

bool STTCParamMacroHandler::doubleMasterMode(const QSet<SystemWorkMode>& workModeSet)
{
    auto doubleMasterParamFunc = [&](const CKJDControlInfo& deviceInfo, SystemWorkMode workMode, bool master) {
        auto beforeDeviceID = master ? deviceInfo.beforeMaster : deviceInfo.beforeSlave;
        auto afterDeviceID = master ? deviceInfo.afterMaster : deviceInfo.afterSlave;
        if (!beforeDeviceID.isValid() || !afterDeviceID.isValid())
        {
            return;
        }
        // 判断当前测控基带是否为该模式，如果不是进行模式切换
        if (!switchJDWorkMode(deviceInfo, workMode, master))
        {
            return;
        }
        auto modeID = SystemWorkModeHelper::systemWorkModeConverToModeId(workMode);
        auto linkLine = mManualMsg.linkLineMap.value(workMode);
        auto targetInfo = linkLine.targetMap.value(linkLine.masterTargetNo);
        auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(workMode);
        ParamMacroModeData paramMacroModeItem;
        if (!GlobalData::getParamMacroModeData(targetInfo.taskCode, targetInfo.workMode, paramMacroModeItem))
        {
            emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式参数失败").arg(targetInfo.taskCode, workModeDesc));
            return;
        }
        // 获取点频参数转换的下发结构
        UnitTargetParamMap unitTargetParamMap;
        if (!getUnitTargetParamMap(workMode, linkLine, afterDeviceID, unitTargetParamMap))
        {
            return;
        }

        // 需要获取测控基带设备参数，添加到下发的参数宏中
        QMap<int, QVariantMap> ckjdUnitParamMap;
        paramMacroModeItem.getDeviceUnitParamMap(afterDeviceID, ckjdUnitParamMap);
        appendDeviceParam(ckjdUnitParamMap, unitTargetParamMap);

        packGroupParamSetData(afterDeviceID, modeID, unitTargetParamMap, mPackCommand);
        waitExecSuccess(mPackCommand);

        if (workMode == STTC)
        {
            // 捕获方式设置为自动
            mSingleCommandHelper.packSingleCommand("StepSTTC_BH_AUTO", mPackCommand, afterDeviceID);
            waitExecSuccess(mPackCommand);
            QThread::sleep(2);

            // 双捕开始
            mSingleCommandHelper.packSingleCommand("StepSTTC_SB_START", mPackCommand, afterDeviceID);
            waitExecSuccess(mPackCommand);
        }
    };

    auto workMode1 = *workModeSet.begin();
    auto workMode2 = *(++workModeSet.begin());
    doubleMasterParamFunc(m_ckjdInfo, workMode1, true);
    doubleMasterParamFunc(m_ckjdInfo, workMode2, false);

    return true;
}

bool STTCParamMacroHandler::doubleMode(const QSet<SystemWorkMode>& /*workModeSet*/)
{
    // 判断当前测控基带是否为该模式，如果不是进行模式切换
    auto ckjdChangeResult = switchJDWorkMode(m_ckjdInfo, mLinkLine.workMode);
    if (CKJDSwitchStatus::Failed == ckjdChangeResult)
    {
        emit signalErrorMsg(QString("%1切换模式失败，预设工作模式与设备当前模式不符合").arg(GlobalData::getExtensionName(m_ckjdInfo.beforeMaster)));
        return false;
    }
    // 排序S扩2在Ka扩2上面 所以workMode1是S扩2
    // auto  workMode1 = *workModeSet.begin();
    // auto  workMode2 = *(++workModeSet.begin());
    // 20211022 直接写死
    // m_ckjdInfo这个处理过双模式切换后一定是4001/4003
    auto workMode1 = Skuo2;
    auto workMode2 = KaKuo2;

    auto doubleModeParamFunc = [&](const DeviceID& deviceIDMaster, const DeviceID& deviceIDSlave, SystemWorkMode workMode, bool master, bool slave) {
        UnitTargetParamMap unitTargetParamMap;
        auto modeID = SystemWorkModeHelper::systemWorkModeConverToModeId(workMode);
        clearQueue();
        if (master)
        {
            // 这里得用对应模式的LinkLine
            auto linkLine = mManualMsg.linkLineMap.value(workMode);
            auto targetInfo = linkLine.targetMap.value(linkLine.masterTargetNo);
            auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(workMode);
            ParamMacroModeData paramMacroModeItem;
            if (!GlobalData::getParamMacroModeData(targetInfo.taskCode, targetInfo.workMode, paramMacroModeItem))
            {
                emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式参数失败").arg(targetInfo.taskCode, workModeDesc));
                return;
            }
            // 获取点频参数转换的下发结构
            if (!getUnitTargetParamMap(workMode, linkLine, deviceIDMaster, unitTargetParamMap))
            {
                return;
            }

            // 需要获取测控基带设备参数，添加到下发的参数宏中
            QMap<int, QVariantMap> ckjdUnitParamMap;
            paramMacroModeItem.getDeviceUnitParamMap(deviceIDMaster, ckjdUnitParamMap);
            appendDeviceParam(ckjdUnitParamMap, unitTargetParamMap);

            /* 设备组合号 */
            quint64 devNum = 0;
            EquipmentCombinationHelper::getCKDevNumber(workMode, mManualMsg.configMacroData, devNum);
            unitTargetParamMap[1][0]["EquipCombNumb"] = QString::number(devNum, 16);
            packGroupParamSetData(deviceIDMaster, modeID, unitTargetParamMap, mPackCommand);
            appendExecQueue(mPackCommand);
        }
        if (slave)
        {
            if (!unitTargetParamMap.isEmpty())
            {
                packGroupParamSetData(deviceIDSlave, modeID, unitTargetParamMap, mPackCommand);
                appendExecQueue(mPackCommand);
            }
        }

        execQueue();
    };

    auto master = (ckjdChangeResult == CKJDSwitchStatus::All || ckjdChangeResult == CKJDSwitchStatus::Master) && m_ckjdInfo.afterMaster.isValid();
    auto slave = (ckjdChangeResult == CKJDSwitchStatus::All || ckjdChangeResult == CKJDSwitchStatus::Slave) && m_ckjdInfo.afterSlave.isValid();
    doubleModeParamFunc(m_ckjdInfo.afterMaster, m_ckjdInfo.afterSlave, workMode1, master, slave);

    auto afterMaster2 = getEqModeCKDeviceID(m_ckjdInfo.afterMaster);
    auto afterSlave2 = getEqModeCKDeviceID(m_ckjdInfo.afterSlave);
    doubleModeParamFunc(afterMaster2, afterSlave2, workMode2, master, slave);

    return true;
}

bool STTCParamMacroHandler::singleMode(const QSet<SystemWorkMode>& workModeSet)
{
    auto workMode = (*workModeSet.begin());

    // 获取模式ID
    auto modeId = SystemWorkModeHelper::systemWorkModeConverToModeId(workMode);
    // 判断当前测控基带是否为该模式，如果不是进行模式切换
    auto ckjdChangeResult = switchJDWorkMode(m_ckjdInfo, workMode);
    if (CKJDSwitchStatus::Failed == ckjdChangeResult)
    {
        emit signalErrorMsg(QString("%1切换模式失败，预设工作模式与设备当前模式不符合").arg(GlobalData::getExtensionName(m_ckjdInfo.beforeMaster)));
        return false;
    }

    // 主用目标的数据
    auto masterTargetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);
    auto masterTargetTaskCode = masterTargetInfo.taskCode;

    // TODO 记录当前链路的使用情况
    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(workMode);

    // 获取指定任务代号的参数宏数据
    ParamMacroModeData paramMacroModeItem;
    if (!GlobalData::getParamMacroModeData(masterTargetTaskCode, workMode, paramMacroModeItem))
    {
        emit signalErrorMsg(QString("获取任务代号为：%1的参数宏的%2模式参数失败").arg(masterTargetTaskCode, workModeDesc));
        return false;
    }

    // 获取点频参数转换的下发结构
    UnitTargetParamMap unitTargetParamMap;
    if (!getUnitTargetParamMap(workMode, mLinkLine, m_ckjdInfo.afterMaster, unitTargetParamMap))
    {
        return false;
    }

    // 需要获取测控基带设备参数，添加到下发的参数宏中
    QMap<int, QVariantMap> ckjdUnitParamMap;
    paramMacroModeItem.getDeviceUnitParamMap(m_ckjdInfo.afterMaster, ckjdUnitParamMap);
    appendDeviceParam(ckjdUnitParamMap, unitTargetParamMap);

    /* 设备组合号 */
    quint64 devNum = 0;
    EquipmentCombinationHelper::getCKDevNumber(workMode, mManualMsg.configMacroData, devNum);
    unitTargetParamMap[1][0]["EquipCombNumb"] = QString::number(devNum, 16);
    if ((ckjdChangeResult == CKJDSwitchStatus::All || ckjdChangeResult == CKJDSwitchStatus::Master) && m_ckjdInfo.afterMaster.isValid())
    {
        packGroupParamSetData(m_ckjdInfo.afterMaster, modeId, unitTargetParamMap, mPackCommand);
        waitExecSuccess(mPackCommand);
    }

    if ((ckjdChangeResult == CKJDSwitchStatus::All || ckjdChangeResult == CKJDSwitchStatus::Slave) && m_ckjdInfo.afterSlave.isValid())
    {
        // 下发备用测控基带参数宏
        packGroupParamSetData(m_ckjdInfo.afterSlave, modeId, unitTargetParamMap, mPackCommand);
        waitExecSuccess(mPackCommand);
    }

    // 如果是STTC 则切换为自动
    if (workMode == STTC)
    {
        // 捕获方式设置为自动
        mSingleCommandHelper.packSingleCommand("StepSTTC_BH_AUTO", mPackCommand, m_ckjdInfo.afterMaster);
        waitExecSuccess(mPackCommand);
        QThread::sleep(2);

        // 双捕开始
        mSingleCommandHelper.packSingleCommand("StepSTTC_SB_START", mPackCommand, m_ckjdInfo.afterMaster);
        waitExecSuccess(mPackCommand);

        if ((ckjdChangeResult == CKJDSwitchStatus::All || ckjdChangeResult == CKJDSwitchStatus::Slave) && m_ckjdInfo.afterSlave.isValid())
        {
            // 下发备机
            mSingleCommandHelper.packSingleCommand("StepSTTC_BH_AUTO", mPackCommand, m_ckjdInfo.afterSlave);
            waitExecSuccess(mPackCommand);
        }
    }
    return true;
}

void STTCParamMacroHandler::ckTaskSS(const QSet<SystemWorkMode>& workModeSet)
{
    // 无测控模式
    if (workModeSet.isEmpty())
    {
        return;
    }
    auto skuo2Mode = workModeSet.contains(Skuo2);
    auto kakuo2Mode = workModeSet.contains(KaKuo2);
    auto sttcMode = workModeSet.contains(STTC);
    auto sktMode = workModeSet.contains(SKT);

    if (skuo2Mode && kakuo2Mode)
    {
        // S存盘送数
        auto size = mManualMsg.linkLineMap.value(Skuo2).targetMap.size();
        for (auto i = 1; i < size; ++i)
        {
            //有多少通道就打开多少通道的送数
            mSingleCommandHelper.packSingleCommand(QString("StepCKJDSS_Start_Skuo2_%1").arg(i), mPackCommand, m_ckjdInfo.afterMaster);
            appendExecQueue(mPackCommand);

            //有多少通道就打开多少通道的存盘
            mSingleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Skuo2_%1").arg(i), mPackCommand, m_ckjdInfo.afterMaster);
            appendExecQueue(mPackCommand);
            mSingleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Skuo2_%1").arg(i), mPackCommand, m_ckjdInfo.afterSlave);
            appendExecQueue(mPackCommand);
        }

        // Ka存盘送数
        auto afterMaster2 = BaseHandler::getEqModeCKDeviceID(m_ckjdInfo.afterMaster);
        auto afterSlave2 = BaseHandler::getEqModeCKDeviceID(m_ckjdInfo.afterSlave);
        size = mManualMsg.linkLineMap.value(KaKuo2).targetMap.size();
        //默认打开存盘
        for (auto i = 1; i < size; i++)
        {
            //有多少通道就打开多少通道的送数
            mSingleCommandHelper.packSingleCommand(QString("StepCKJDSS_Start_KaKuo2_%1").arg(i), mPackCommand, afterMaster2);
            appendExecQueue(mPackCommand);

            //有多少通道就打开多少通道的存盘
            mSingleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Kakuo2_%1").arg(i), mPackCommand, afterMaster2);
            appendExecQueue(mPackCommand);
            mSingleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Kakuo2_%1").arg(i), mPackCommand, afterSlave2);
            appendExecQueue(mPackCommand);
        }
    }
    // S扩二
    else if (skuo2Mode)
    {
        // S存盘送数
        auto size = mManualMsg.linkLineMap.value(Skuo2).targetMap.size();
        for (auto i = 1; i < size; ++i)
        {
            //有多少通道就打开多少通道的送数
            mSingleCommandHelper.packSingleCommand(QString("StepCKJDSS_Start_Skuo2_%1").arg(i), mPackCommand, m_ckjdInfo.afterMaster);
            appendExecQueue(mPackCommand);

            //有多少通道就打开多少通道的存盘
            mSingleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Skuo2_%1").arg(i), mPackCommand, m_ckjdInfo.afterMaster);
            appendExecQueue(mPackCommand);
            mSingleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Skuo2_%1").arg(i), mPackCommand, m_ckjdInfo.afterSlave);
            appendExecQueue(mPackCommand);
        }
    }
    // Ka扩二
    else if (kakuo2Mode)
    {
        auto size = mManualMsg.linkLineMap.value(KaKuo2).targetMap.size();
        for (auto i = 1; i < size; ++i)
        {
            //有多少通道就打开多少通道的送数
            mSingleCommandHelper.packSingleCommand(QString("StepCKJDSS_Start_KaKuo2_%1").arg(i), mPackCommand, m_ckjdInfo.afterMaster);
            appendExecQueue(mPackCommand);

            //有多少通道就打开多少通道的存盘
            mSingleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Kakuo2_%1").arg(i), mPackCommand, m_ckjdInfo.afterMaster);
            appendExecQueue(mPackCommand);
            mSingleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_Kakuo2_%1").arg(i), mPackCommand, m_ckjdInfo.afterSlave);
            appendExecQueue(mPackCommand);
        }
    }
    else if (sttcMode)
    {
        //打开基带送数
        mSingleCommandHelper.packSingleCommand("StepCKJDSS_Start_TTC", mPackCommand, m_ckjdInfo.afterMaster);
        appendExecQueue(mPackCommand);

        //打开基带存盘
        mSingleCommandHelper.packSingleCommand("StepCKJDCP_Start_TTC", mPackCommand, m_ckjdInfo.afterMaster);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("StepCKJDCP_Start_TTC", mPackCommand, m_ckjdInfo.afterSlave);
        appendExecQueue(mPackCommand);
    }
    // 扩跳
    else if (sktMode)
    {
        //送数控制
        mSingleCommandHelper.packSingleCommand(QString("StepCKJDSS_Start_SKT"), mPackCommand, m_ckjdInfo.afterMaster);
        appendExecQueue(mPackCommand);

        //默认打开存盘
        mSingleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_SKT"), mPackCommand, m_ckjdInfo.afterMaster);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand(QString("StepCKJDCP_Start_SKT"), mPackCommand, m_ckjdInfo.afterSlave);
        appendExecQueue(mPackCommand);
    }
}

void STTCParamMacroHandler::zzzz(const QSet<SystemWorkMode>& workModeSet)
{
    QVariantMap replaceMap;
    replaceMap["UACAddr"] = mManualMsg.txUACAddr;

    if (workModeSet.contains(Skuo2) && workModeSet.contains(KaKuo2))
    {
        mSingleCommandHelper.packSingleCommand("Step_CK_SK2_ZZZZ", mPackCommand, m_ckjdInfo.afterMaster, replaceMap);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("Step_CK_SK2_ZZZZ", mPackCommand, m_ckjdInfo.afterSlave, replaceMap);
        appendExecQueue(mPackCommand);

        auto afterMaster2 = getEqModeCKDeviceID(m_ckjdInfo.afterMaster);
        auto afterSlave2 = getEqModeCKDeviceID(m_ckjdInfo.afterSlave);

        mSingleCommandHelper.packSingleCommand("Step_CK_KAK2_ZZZZ", mPackCommand, afterMaster2, replaceMap);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("Step_CK_KAK2_ZZZZ", mPackCommand, afterSlave2, replaceMap);
        appendExecQueue(mPackCommand);
    }
    else if (workModeSet.contains(Skuo2))
    {
        mSingleCommandHelper.packSingleCommand("Step_CK_SK2_ZZZZ", mPackCommand, m_ckjdInfo.afterMaster, replaceMap);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("Step_CK_SK2_ZZZZ", mPackCommand, m_ckjdInfo.afterSlave, replaceMap);
        appendExecQueue(mPackCommand);
    }
    else if (workModeSet.contains(KaKuo2))
    {
        mSingleCommandHelper.packSingleCommand("Step_CK_KAK2_ZZZZ", mPackCommand, m_ckjdInfo.afterMaster, replaceMap);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("Step_CK_KAK2_ZZZZ", mPackCommand, m_ckjdInfo.afterSlave, replaceMap);
        appendExecQueue(mPackCommand);
    }
    else if (workModeSet.contains(STTC))
    {
        mSingleCommandHelper.packSingleCommand("Step_CK_STTC_ZZZZ", mPackCommand, m_ckjdInfo.afterMaster, replaceMap);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("Step_CK_STTC_ZZZZ", mPackCommand, m_ckjdInfo.afterSlave, replaceMap);
        appendExecQueue(mPackCommand);
    }
    else if (workModeSet.contains(SKT))
    {
        mSingleCommandHelper.packSingleCommand("Step_CK_SKT_ZZZZ", mPackCommand, m_ckjdInfo.afterMaster, replaceMap);
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand("Step_CK_SKT_ZZZZ", mPackCommand, m_ckjdInfo.afterSlave, replaceMap);
        appendExecQueue(mPackCommand);
    }
}

bool STTCParamMacroHandler::ckqgParam(const QSet<SystemWorkMode>& workModeSet, const ParamMacroModeData& paramMacroModeItem)
{
    auto result = getCKQDDeviceID();
    if (!result)
    {
        emit signalErrorMsg(result.msg());
        return false;
    }
    auto info = result.value();

    UnitTargetParamMap unitTargetParamMap;
    QMap<int, QMap<QString, QVariant>> ckqdUnitParamMap;
    paramMacroModeItem.getDeviceUnitParamMap(info.master, ckqdUnitParamMap);
    for (auto iter = ckqdUnitParamMap.begin(); iter != ckqdUnitParamMap.end(); ++iter)
    {
        auto unitId = iter.key();
        unitTargetParamMap[unitId][0] = iter.value();
    }

    // TODO 818 workModeSet
    auto existSTTC = workModeSet.contains(STTC);
    auto existSkuo2 = workModeSet.contains(Skuo2);
    auto existKaKuo2 = workModeSet.contains(KaKuo2);
    auto existSKT = workModeSet.contains(SKT);
    // 1.只有STTC
    // 2.只有Skuo2
    // 3.只有SKT
    if ((existSTTC && !existSkuo2 && !existKaKuo2 && !existSKT) ||  //
        (!existSTTC && existSkuo2 && !existKaKuo2 && !existSKT) ||  //
        (!existSTTC && !existSkuo2 && !existKaKuo2 && existSKT))
    {
        unitTargetParamMap[2][0]["SendFreq_1"] = m_qdUpFrequency;    // 任务的发送频率对应上行
        unitTargetParamMap[2][0]["SendFreq_2"] = m_qdDownFrequency;  // 模拟源的频率对应下行

        unitTargetParamMap[3][0]["ReceiveFreq_1"] = m_qdDownFrequency;  // 对应下行
        unitTargetParamMap[3][0]["ReceiveFreq_2"] = m_qdDownFrequency;  // 对应下行
        unitTargetParamMap[3][0]["ReceiveFreq_3"] = m_qdUpFrequency;    // 小环频率对应上行，去功放发送
    }

    // 4.只有Kakuo2
    if (!existSTTC && !existSkuo2 && existKaKuo2 && !existSKT)
    {
        unitTargetParamMap[2][0]["SendFreq_1"] = 2075;  // 任务的发送频率对应上行
        unitTargetParamMap[2][0]["SendFreq_2"] = 2250;  // 模拟源的频率对应下行

        unitTargetParamMap[3][0]["ReceiveFreq_1"] = 2250;  // 对应下行
        unitTargetParamMap[3][0]["ReceiveFreq_2"] = 2250;  // 对应下行
        unitTargetParamMap[3][0]["ReceiveFreq_3"] = 2075;  // 小环频率对应上行，去功放发送 已经隐藏了
    }

    // 5.Skuo2+Kakuo2
    if (!existSTTC && existSkuo2 && existKaKuo2 && !existSKT)
    {
        /* 组合模式默认使用Skuo2为上行  在计划里面可以切换
         * 下行直接两个都发
         */
        // 获取主跟的参数宏
        unitTargetParamMap.clear();
        if (mLinkLine.workMode == Skuo2)
        {
            unitTargetParamMap[2][0]["SendFreq_1"] = m_qdUpFrequency;    // 任务的发送频率对应上行
            unitTargetParamMap[2][0]["SendFreq_2"] = m_qdDownFrequency;  // 模拟源的频率对应下行
        }
        else
        {
            unitTargetParamMap[2][0]["SendFreq_1"] = 2075;  // 任务的发送频率对应上行
            unitTargetParamMap[2][0]["SendFreq_2"] = 2250;  // 模拟源的频率对应下行
        }

        unitTargetParamMap[3][0]["ReceiveFreq_1"] = m_qdDownFrequency;  // 对应下行
        unitTargetParamMap[3][0]["ReceiveFreq_2"] = 2250;               // 对应Ka下行
        unitTargetParamMap[3][0]["ReceiveFreq_3"] = m_qdUpFrequency;    // 小环频率对应上行，去功放发送
    }

    // 公共单元这里如果是测控混合模式需要改一下公共单元
    if (existSkuo2 && existKaKuo2)
    {
        unitTargetParamMap[1][0]["YBRate"] = 4;  // 组合模式
    }
    else if (existSKT)
    {
        unitTargetParamMap[1][0]["YBRate"] = 3;  // KT
    }
    else
    {
        unitTargetParamMap[1][0]["YBRate"] = 1;  // KT
    }

    // 参数宏里面下了
    //    if(mLinkLine.workMode == Skuo2)
    //    {
    //        common["MainSubSwitch"] = 2;
    //    }
    //    else
    //    {
    //        common["MainSubSwitch"] = 1;
    //    }
    clearQueue();
    packGroupParamSetData(info.master, 0xFFFF, unitTargetParamMap, mPackCommand);
    appendExecQueue(mPackCommand);

    if (info.slave.isValid())
    {
        packGroupParamSetData(info.slave, 0xFFFF, unitTargetParamMap, mPackCommand);
        appendExecQueue(mPackCommand);
    }
    execQueue();
    return true;
}
bool STTCParamMacroHandler::otherDeviceParam(const ParamMacroModeData& paramMacroModeItem)
{
    clearQueue();
    // 功放 开关网络等设备
    if (mLinkLine.workMode == STTC || mLinkLine.workMode == Skuo2 || mLinkLine.workMode == SKT)
    {
        // s高功放
        DeviceID sggfDeviceID;
        if (getSGGFDeviceID(sggfDeviceID))
        {
            QMap<int, QVariantMap> sggfUnitParamMap;
            paramMacroModeItem.getDeviceUnitParamMap(sggfDeviceID, sggfUnitParamMap);
            /* 这里没做判断 是MAP就算为空直接增加一个空数据 */
            auto sggfParamMap = sggfUnitParamMap[1];
            mSingleCommandHelper.packSingleCommand(QString("Step_26SHPA_TransmPower"), mPackCommand, sggfDeviceID, sggfParamMap);
            appendExecQueue(mPackCommand);
        }

        // s频段射频开关网络
        QMap<int, QVariantMap> spdspkgwlUnitParamMap;
        auto indexCmdKey = QString("Step_SPDSPKGWL_XHSJ");
        auto spdspkgwlDeviceID = mSingleCommandHelper.getSingleCmdDeviceID(indexCmdKey, "0x3201");
        paramMacroModeItem.getDeviceUnitParamMap(spdspkgwlDeviceID, spdspkgwlUnitParamMap);
        auto skgwlParamMap = spdspkgwlUnitParamMap[1];
        mSingleCommandHelper.packSingleCommand(indexCmdKey, mPackCommand, skgwlParamMap);
        appendExecQueue(mPackCommand);
    }
    else if (mLinkLine.workMode == KaKuo2)
    {
        //设置ka测控及跟踪高频箱下行频率和增益
        int miniUnitID = 5;
        auto indexCmdKey = QString("Step_kackgzgpx_kaFreqAndgain_%1");
        QMap<int, QVariantMap> kackgzgpxUnitParamMap;
        auto kackgzgpxDeviceID = mSingleCommandHelper.getSingleCmdDeviceID(indexCmdKey.arg(miniUnitID), "0x3101");
        paramMacroModeItem.getDeviceUnitParamMap(kackgzgpxDeviceID, kackgzgpxUnitParamMap);
        for (auto iter = kackgzgpxUnitParamMap.begin(); iter != kackgzgpxUnitParamMap.end(); ++iter)
        {
            auto unitID = iter.key();
            if (unitID < miniUnitID)
                continue;

            auto kackgzgpxParamMap = iter.value();
            kackgzgpxParamMap["SBandDownFreq"] = m_downFrequency;  // 替换为点频的数传下行频率
            mSingleCommandHelper.packSingleCommand(indexCmdKey.arg(unitID), mPackCommand, kackgzgpxDeviceID, kackgzgpxParamMap);
            appendExecQueue(mPackCommand);
        }

        //设置ka测控发射高频箱上行频率和增益
        miniUnitID = 2;
        indexCmdKey = QString("Step_kackfsgpx_kaFreqAndgain_%1");
        QMap<int, QVariantMap> kackFSgpxUnitParamMap;
        auto kackFSgpxDeviceID = mSingleCommandHelper.getSingleCmdDeviceID(indexCmdKey.arg(miniUnitID), "0x2402");
        paramMacroModeItem.getDeviceUnitParamMap(kackFSgpxDeviceID, kackFSgpxUnitParamMap);
        for (auto iter = kackFSgpxUnitParamMap.begin(); iter != kackFSgpxUnitParamMap.end(); ++iter)
        {
            auto unitID = iter.key();
            if (unitID < miniUnitID)
                continue;
            auto kackfsgpxParamMap = iter.value();
            kackfsgpxParamMap["WorkRate"] = m_upFrequency;
            mSingleCommandHelper.packSingleCommand(indexCmdKey.arg(unitID), mPackCommand, kackFSgpxDeviceID, kackfsgpxParamMap);
            appendExecQueue(mPackCommand);
        }

        //测控前端接收单元下发接受功率
        //设置ka测控高功放300w
        indexCmdKey = QString("Step_kaHPA300W_TransmPower");
        QMap<int, QVariantMap> kackggfUnitParamMap;
        auto kackggfDeviceID = mSingleCommandHelper.getSingleCmdDeviceID(indexCmdKey, "0x2003");
        paramMacroModeItem.getDeviceUnitParamMap(kackggfDeviceID, kackggfUnitParamMap);
        auto kackggfParamMap = kackggfUnitParamMap[1];
        kackggfParamMap["WorkRate"] = 30000;
        mSingleCommandHelper.packSingleCommand(indexCmdKey, mPackCommand, kackggfDeviceID, kackggfParamMap);
        appendExecQueue(mPackCommand);

        //设置ka测控高功放30W
        QMap<int, QVariantMap> kackggf30wUnitParamMap;
        indexCmdKey = QString("Step_kaHPA30W_TransmPower");
        auto kaCKggf30wDeviceID = mSingleCommandHelper.getSingleCmdDeviceID(indexCmdKey, "0x2004");
        paramMacroModeItem.getDeviceUnitParamMap(kaCKggf30wDeviceID, kackggf30wUnitParamMap);
        auto kackggf30wParamMap = kackggf30wUnitParamMap[1];
        kackggf30wParamMap["WorkRate"] = 30000;
        mSingleCommandHelper.packSingleCommand(indexCmdKey, mPackCommand, kaCKggf30wDeviceID, kackggf30wParamMap);
        appendExecQueue(mPackCommand);
    }

    execQueue();
    return true;
}
