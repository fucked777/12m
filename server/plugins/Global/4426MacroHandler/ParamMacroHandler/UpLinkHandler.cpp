#include "UpLinkHandler.h"
#include "BaseParamMacroHandler.h"
#include "BusinessMacroCommon.h"
#include "GlobalData.h"

UpLinkHandler::UpLinkHandler(QObject* parent)
    : BaseParamMacroHandler(parent)
{
}
SystemWorkMode UpLinkHandler::getSystemWorkMode() { return SystemWorkMode::NotDefine; }
void UpLinkHandler::setEnableHpa(EnableMode enable) { m_enableHpa = enable; }
void UpLinkHandler::setEnableJD(EnableMode enable) { m_enableJD = enable; }
void UpLinkHandler::setEnableQD(EnableMode enable) { m_enableQD = enable; }
void UpLinkHandler::setIgnoreXLDevice(bool ignore) { m_enableXL = (ignore ? EnableMode::Ignore : EnableMode::Disable); }
void UpLinkHandler::setIgnoreOtherDevice(bool ignore) { m_enableOther = (ignore ? EnableMode::Ignore : EnableMode::Disable); }

void UpLinkHandler::enableControl(UpLinkDeviceControl control) { m_control = control; }
void UpLinkHandler::setQDSendSource(CKQDSendSource source) { m_ckqdSource = source; }
void UpLinkHandler::setRFOutputMode(RFOutputMode mode) { m_outputMode = mode; }
void UpLinkHandler::setAdd(JDOutPut mode) { m_jdOutPut = mode; }

void UpLinkHandler::zbsc(const DeviceID& deviceID)
{
    if (STTC == mLinkLine.workMode)
    {
        // 载波输出
        mSingleCommandHelper.packSingleCommand("StepSTTC_UplinkCarrierOutput", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
    }
    else if (Skuo2 == mLinkLine.workMode)
    {
        // 获取输出电平
        QVariantMap replaceMap;
        auto level = GlobalData::getReportParamData(deviceID, 3, "CarrierLevel");
        if (level.isValid())
        {
            replaceMap["OutputLev"] = level;
        }
        // 载波输出
        mSingleCommandHelper.packSingleCommand("StepSkuo2_UplinkCarrierOutput", mPackCommand, deviceID, replaceMap);
        appendExecQueue(mPackCommand);
    }
    else if (KaKuo2 == mLinkLine.workMode)
    {
        // 获取输出电平
        QVariantMap replaceMap;
        auto level = GlobalData::getReportParamData(deviceID, 3, "CarrierLevel");
        if (level.isValid())
        {
            replaceMap["OutputLev"] = level;
        }
        // 载波输出
        mSingleCommandHelper.packSingleCommand("StepKakuo2_UplinkCarrierOutput", mPackCommand, deviceID, replaceMap);
        appendExecQueue(mPackCommand);
    }
    else if (SKT == mLinkLine.workMode)
    {
        // 获取输出电平
        QVariantMap replaceMap;
        auto level = GlobalData::getReportParamData(deviceID, 3, "CarrierLevel");
        if (level.isValid())
        {
            replaceMap["OutputLev"] = level;
        }
        // 载波输出
        mSingleCommandHelper.packSingleCommand("StepSKT_UplinkCarrierOutput", mPackCommand, deviceID, replaceMap);
        appendExecQueue(mPackCommand);
    }

    // 测控基带功率上天线
    mSingleCommandHelper.packSingleCommand("Step_GLSTX", mPackCommand, deviceID);
    appendExecQueue(mPackCommand);
}

void UpLinkHandler::ykjt(const DeviceID& deviceID)
{
    if (STTC == mLinkLine.workMode)
    {
        mSingleCommandHelper.packSingleCommand("StepMACBDC_YK_UP", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
    }
    else if (Skuo2 == mLinkLine.workMode)
    {
        // 加调
        mSingleCommandHelper.packSingleCommand("StepMACBDC_SK2_YK_UP", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
    }
    else if (KaKuo2 == mLinkLine.workMode)
    {
        // 加调
        mSingleCommandHelper.packSingleCommand("StepMACBDC_KaK2_YK_UP", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
    }
    else if (SKT == mLinkLine.workMode)
    {
        // 加调
        mSingleCommandHelper.packSingleCommand("StepMACBDC_SKT_YK_UP", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
    }
}
void UpLinkHandler::cljt(const DeviceID& deviceID)
{
    if (STTC == mLinkLine.workMode)
    {
        // 加调
        mSingleCommandHelper.packSingleCommand("StepMACBDC_CJ_UP", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
    }
    else if (Skuo2 == mLinkLine.workMode)
    {
        // 加调
        mSingleCommandHelper.packSingleCommand("StepMACBDC_SK2_CJ_UP", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
    }
    else if (KaKuo2 == mLinkLine.workMode)
    {
        // 加调
        mSingleCommandHelper.packSingleCommand("StepMACBDC_KaK2_CJ_UP", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
    }
    else if (SKT == mLinkLine.workMode)
    {
        // 加调
        mSingleCommandHelper.packSingleCommand("StepMACBDC_SKT_YK_UP", mPackCommand, deviceID);
        appendExecQueue(mPackCommand);
    }
}

void UpLinkHandler::ckjdEnable()
{
    // 获取所有测控模式
    if (!SystemWorkModeHelper::isMeasureContrlWorkMode(mLinkLine.workMode))
    {
        return;
    }

    DeviceID masterDeviceID;
    mManualMsg.configMacroData.getCKJDDeviceID(mLinkLine.workMode, masterDeviceID, 1);
    if (!masterDeviceID.isValid())
    {
        return;
    }
    /* 双模式需要处理下设备ID */
    if (isDoubleMode())
    {
        masterDeviceID = getChangeModeCKDeviceID(masterDeviceID, mLinkLine.workMode);
    }

    clearQueue();
    if (m_jdOutPut == JDOutPut::Carrier)
    {
        zbsc(masterDeviceID);
    }
    else if (m_jdOutPut == JDOutPut::RangAdd)
    {
        cljt(masterDeviceID);
    }
    else if (m_jdOutPut == JDOutPut::RemoteAdd)
    {
        ykjt(masterDeviceID);
    }
    else if (m_jdOutPut == JDOutPut::CarrierRangAdd)
    {
        zbsc(masterDeviceID);
        cljt(masterDeviceID);
    }
    else if (m_jdOutPut == JDOutPut::CarrierRemoteAdd)
    {
        zbsc(masterDeviceID);
        ykjt(masterDeviceID);
    }
    else
    {
        zbsc(masterDeviceID);
        cljt(masterDeviceID);
        ykjt(masterDeviceID);
    }
    execQueue();
}
void UpLinkHandler::ckjdDisable(const QList<DeviceID>& deviceIDList)
{
    for (auto& item : deviceIDList)
    {
        if (!GlobalData::getDeviceOnline(item))
        {
            continue;
        }

        clearQueue();

        // 当前模式ID
        auto curModeID = GlobalData::getOnlineDeviceModeId(item);
        if (SystemWorkModeHelper::systemWorkModeConverToModeId(STTC) == curModeID)
        {
            // 去调
            mSingleCommandHelper.packSingleCommand("StepMACBDC_CJ_DOWN", mPackCommand, item);
            appendExecQueue(mPackCommand);
            mSingleCommandHelper.packSingleCommand("StepMACBDC_YK_DOWN", mPackCommand, item);
            appendExecQueue(mPackCommand);

            // 关闭所有载波输出
            mSingleCommandHelper.packSingleCommand("StepSTTC_UplinkCarrierOutput_STOP", mPackCommand, item);
            appendExecQueue(mPackCommand);

            // 测控基带功率上天线禁止
            mSingleCommandHelper.packSingleCommand("Step_GLSTX_STOP", mPackCommand, item);
            appendExecQueue(mPackCommand);
        }
        else if (SystemWorkModeHelper::systemWorkModeConverToModeId(Skuo2) == curModeID)
        {
            // 去调
            mSingleCommandHelper.packSingleCommand("StepMACBDC_SK2_YK_DOWN", mPackCommand, item);
            appendExecQueue(mPackCommand);

            // 获取输出电平
            QVariantMap replaceMap;
            auto level = GlobalData::getReportParamData(item, 3, "CarrierLevel");
            if (level.isValid())
            {
                replaceMap["OutputLev"] = level;
            }
            // 关闭所有载波输出
            mSingleCommandHelper.packSingleCommand("StepSkuo2_UplinkCarrierOutput_STOP", mPackCommand, item, replaceMap);
            appendExecQueue(mPackCommand);

            // 关闭噪声源
            // mSingleCommandHelper.packSingleCommand("StepKP_NoiseOutSwicth_OFF", mPackCommand, item, curModeID);
            // appendExecQueue(mPackCommand);

            // 测控基带功率上天线禁止
            mSingleCommandHelper.packSingleCommand("Step_GLSTX_STOP", mPackCommand, item);
            appendExecQueue(mPackCommand);
        }
        else if (SystemWorkModeHelper::systemWorkModeConverToModeId(KaKuo2) == curModeID)
        {
            // 去调
            mSingleCommandHelper.packSingleCommand("StepMACBDC_KaK2_YK_DOWN", mPackCommand, item);
            appendExecQueue(mPackCommand);

            // 获取输出电平
            QVariantMap replaceMap;
            auto level = GlobalData::getReportParamData(item, 3, "CarrierLevel");
            if (level.isValid())
            {
                replaceMap["OutputLev"] = level;
            }

            // 关闭所有载波输出
            mSingleCommandHelper.packSingleCommand("StepKakuo2_UplinkCarrierOutput_STOP", mPackCommand, item, replaceMap);
            appendExecQueue(mPackCommand);

            // 关闭噪声源
            // mSingleCommandHelper.packSingleCommand("StepKP_NoiseOutSwicth_OFF", mPackCommand, item, curModeID);
            // appendExecQueue(mPackCommand);

            // 测控基带功率上天线禁止
            mSingleCommandHelper.packSingleCommand("Step_GLSTX_STOP", mPackCommand, item);
            appendExecQueue(mPackCommand);
        }
        else if (SystemWorkModeHelper::systemWorkModeConverToModeId(SKT) == curModeID)
        {
            // 去调
            mSingleCommandHelper.packSingleCommand("StepMACBDC_SKT_YK_DOWN", mPackCommand, item);
            appendExecQueue(mPackCommand);

            // 获取输出电平
            QVariantMap replaceMap;
            auto level = GlobalData::getReportParamData(item, 3, "CarrierLevel");
            if (level.isValid())
            {
                replaceMap["OutputLev"] = level;
            }
            // 关闭所有载波输出
            mSingleCommandHelper.packSingleCommand("StepSKT_UplinkCarrierOutput_STOP", mPackCommand, item, replaceMap);
            appendExecQueue(mPackCommand);

            // 测控基带功率上天线禁止
            mSingleCommandHelper.packSingleCommand("Step_GLSTX_STOP", mPackCommand, item);
            appendExecQueue(mPackCommand);
        }

        execQueue();
    }
}

void UpLinkHandler::dsjdEnable()
{
    auto workMode = KaDS;
    DeviceID masterDeviceID;
    mManualMsg.configMacroData.getCKJDDeviceID(workMode, masterDeviceID, 1);
    if (masterDeviceID.isValid())
    {
        mSingleCommandHelper.packSingleCommand("Step_kaDSJD_UplinkCarrierOutput", mPackCommand, masterDeviceID);
        waitExecSuccess(mPackCommand);
    }
}
void UpLinkHandler::dsjdDisable(const QList<DeviceID>& deviceIDList)
{
    for (auto& item : deviceIDList)
    {
        // 关闭所有载波输出
        mSingleCommandHelper.packSingleCommand("Step_kaDSJD_UplinkCarrierOutput_Stop", mPackCommand, item);
        waitExecSuccess(mPackCommand);
    }
}

void UpLinkHandler::jdEnable()
{
    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::CK)
    {
        ckjdEnable();
    }

    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::SC)
    {
        dsjdEnable();
    }
}
void UpLinkHandler::jdDisable()
{
    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::CK)
    {
        /* 直接关闭所有的 */
        QList<DeviceID> deviceIDList = { DeviceID(4, 0, 1), DeviceID(4, 0, 2), DeviceID(4, 0, 3), DeviceID(4, 0, 4) };
        ckjdDisable(deviceIDList);
    }

    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::SC)
    {
        /* 直接关闭所有的 */
        QList<DeviceID> deviceIDList = { DeviceID(4, 4, 1), DeviceID(4, 4, 2) };
        dsjdDisable(deviceIDList);
    }
}
void UpLinkHandler::jdDisableSpecific()
{
    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::CK)
    {
        QList<DeviceID> deviceIDList;
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(mLinkLine.workMode))
        {
            DeviceID masterDeviceID;
            mManualMsg.configMacroData.getCKJDDeviceID(mLinkLine.workMode, masterDeviceID, 1);
            if (masterDeviceID.isValid())
            {
                /* 双模式需要处理下设备ID */
                if (isDoubleMode())
                {
                    masterDeviceID = getChangeModeCKDeviceID(masterDeviceID, mLinkLine.workMode);
                }
                deviceIDList << masterDeviceID;
                ckjdDisable(deviceIDList);
            }
        }
    }
    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::SC)
    {
        auto workMode = KaDS;
        DeviceID masterDeviceID;
        mManualMsg.configMacroData.getCKJDDeviceID(workMode, masterDeviceID, 1);
        QList<DeviceID> deviceIDList;
        if (masterDeviceID.isValid())
        {
            deviceIDList << masterDeviceID;
            dsjdDisable(deviceIDList);
        }
    }
}

void UpLinkHandler::qdEnableImpl(const DeviceID& masterDeviceID, SystemWorkMode workMode)
{
    /*
     * 这里要考虑校零的时候可以在两个模式之间切换
     * 所以频率需要更改 这里要获取上下行频率
     * 校零的模式只有 STTC Skuo2 SKT Kakuo2
     */
    auto targetInfo = mLinkLine.targetMap.value(mLinkLine.masterTargetNo);
    ParamMacroModeData paramMacroModeData;
    GlobalData::getParamMacroModeData(targetInfo.taskCode, mLinkLine.workMode, paramMacroModeData);

    QVariant downFreq;
    QVariant upFreq;

    /* 只要发射单元其他都可以固定 */
    auto sendUnit = paramMacroModeData.deviceParamMap.value(DeviceID(0xAAAA)).unitParamMap.value(2);
    if(MacroCommon::getFrequency(paramMacroModeData, targetInfo.pointFreqNo, upFreq, downFreq))
    {
        if (!SystemWorkModeHelper::systemWorkModeToSystemBand(mLinkLine.workMode) == SBand)
        {
            upFreq = 2075;
            downFreq = 2250;
        }
    }

    BaseParamMacroHandler::UnitTargetParamMap unitTargetParamMap;

    // 公共
    // 低速
    if (SystemWorkModeHelper::isDataTransmissionWorkMode(workMode))
    {
        auto& common = unitTargetParamMap[1][0];
        common["MainSubSwitch"] = 1;
        common["YBRate"] = 1;
    }
    else if (isDoubleMode())
    {
        auto& common = unitTargetParamMap[1][0];
        common["MainSubSwitch"] = mLinkLine.workMode == KaKuo2 ? 2 : 1;
        common["YBRate"] = 4;
    }
    // S标准TTC S扩二 XDS KADS
    else if (workMode == STTC || workMode == Skuo2)
    {
        auto& common = unitTargetParamMap[1][0];
        common["MainSubSwitch"] = 1;
        common["YBRate"] = 1;
    }
    // Ka扩二
    else if (workMode == KaKuo2)
    {
        auto& common = unitTargetParamMap[1][0];
        common["MainSubSwitch"] = 2;
        common["YBRate"] = 1;
    }
    // 扩跳
    else if (workMode == SKT)
    {
        auto& common = unitTargetParamMap[1][0];
        common["MainSubSwitch"] = 1;
        common["YBRate"] = 3;
    }

    // 发射
    auto& send = unitTargetParamMap[2][0];
    /*
     * 发射开关
     * 1关2开
     *
     * 输出来源
     * 1基带 2前端
     */
    send["SendSwitch_1"] = 2;
    send["SendSwitch_2"] = 2;
    send["SendSource_1"] = m_ckqdSource == CKQDSendSource::JD ? 1 : 2;
    send["SendSource_2"] = m_ckqdSource == CKQDSendSource::JD ? 1 : 2;
    if (upFreq.isValid())
    {
        send["SendFreq_1"] = upFreq;
    }
    if (downFreq.isValid())
    {
        send["SendFreq_2"] = downFreq;
    }

    if(SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SBand)
    {
        send["SendPower_1"] = sendUnit.value("SendPower_1", -30);
        send["SendPower_2"] = sendUnit.value("SendPower_2", -30);
    }
    else
    {
        send["SendPower_1"] = sendUnit.value("SendPower_1", -10);
        send["SendPower_2"] = sendUnit.value("SendPower_2", -10);
    }

    // 接收 不用改 这里只管上行
    //    if (downFreq.isValid())
    //    {
    //        auto& recv = unitTargetParamMap[3][0];
    //        recv["ReceiveFreq_1"] = downFreq;  // 对应下行
    //        recv["ReceiveFreq_2"] = 2250;      // 对应Ka下行
    //    }
    if (upFreq.isValid())
    {
        auto& recv = unitTargetParamMap[3][0];
        recv["ReceiveFreq_3"] = upFreq;  // 小环频率对应上行，去功放发送
    }

    packGroupParamSetData(masterDeviceID, 0xFFFF, unitTargetParamMap, mPackCommand);
    waitExecSuccess(mPackCommand);
}

void UpLinkHandler::ckqdEnable()
{
    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::CK)
    {
        DeviceID masterDeviceID;
        mManualMsg.configMacroData.getCKQDDeviceID(mLinkLine.workMode, masterDeviceID);
        if (masterDeviceID.isValid() && SystemWorkModeHelper::isMeasureContrlWorkMode(mLinkLine.workMode))
        {
            qdEnableImpl(masterDeviceID, mLinkLine.workMode);
        }
    }

    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::SC)
    {
        DeviceID masterDeviceID;
        mManualMsg.configMacroData.getCKQDDeviceID(KaDS, masterDeviceID);
        if (masterDeviceID.isValid())
        {
            qdEnableImpl(masterDeviceID, KaDS);
        }
    }
}
void UpLinkHandler::ckqdDisable()
{
    /* 直接关闭所有的 */
    QVariantMap unitDataMap;
    unitDataMap["SendSwitch_1"] = 1;
    unitDataMap["SendSwitch_2"] = 1;
    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::CK)
    {
        QList<DeviceID> deviceIDList = { DeviceID(7, 0, 1), DeviceID(7, 0, 2) };
        for (auto& item : deviceIDList)
        {
            mSingleCommandHelper.packSingleCommand("StepCKQD_Unit_2", mPackCommand, item, unitDataMap);
            waitExecSuccess(mPackCommand);
        }
    }
    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::SC)
    {
        QList<DeviceID> deviceIDList = { DeviceID(7, 0, 3), DeviceID(7, 0, 4) };
        for (auto& item : deviceIDList)
        {
            mSingleCommandHelper.packSingleCommand("StepCKQD_Unit_2", mPackCommand, item, unitDataMap);
            waitExecSuccess(mPackCommand);
        }
    }
}

void UpLinkHandler::hpaEnable()
{
    clearQueue();

    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::CK)
    {
        if (SystemWorkModeHelper::systemWorkModeToSystemBand(mLinkLine.workMode) == SystemBandMode::SBand)
        {
            DeviceID masterDeviceID;
            mManualMsg.configMacroData.getSGGFDeviceID(mLinkLine.workMode, masterDeviceID);
            if (masterDeviceID.isValid())
            {
                QVariantMap replaceMap;
                if (m_outputMode == RFOutputMode::TX)
                {
                    replaceMap["SwitcherSet"] = masterDeviceID.extenID == 1 ? 1 : 2;
                }
                else
                {
                    replaceMap["SwitcherSet"] = masterDeviceID.extenID == 3;
                }

                replaceMap["SwitchMode"] = 2;      // 手动
                replaceMap["AmplifMode"] = 2;      // 手动
                replaceMap["AmplifSwitcher"] = 1;  // 打开告警
                replaceMap["RFSwitch"] = 1;        // RF开
                mSingleCommandHelper.packSingleCommand("StepGFQHKG_Unit", mPackCommand, replaceMap);
                appendExecQueue(mPackCommand);

                mSingleCommandHelper.packSingleCommand("Step_SHPA_POWER_ON", mPackCommand, masterDeviceID);
                appendExecQueue(mPackCommand);
            }
        }
        else if (mLinkLine.workMode == KaKuo2)
        {
            DeviceID masterDeviceID;
            mManualMsg.configMacroData.getKaGGFDeviceID(mLinkLine.workMode, masterDeviceID);
            if (masterDeviceID.isValid())
            {
                mSingleCommandHelper.packSingleCommand("StepKaHPA_Power_Allow", mPackCommand, masterDeviceID);
                appendExecQueue(mPackCommand);

                auto cmdID = m_outputMode == RFOutputMode::TX ? "StepKaHPA_GO_ACU" : "StepKaHPA_UNGO_ACU";
                mSingleCommandHelper.packSingleCommand(cmdID, mPackCommand, masterDeviceID);
                appendExecQueue(mPackCommand);

                // 主备切换 2003 2004
                cmdID = masterDeviceID.extenID == 3 ? "StepKaHPA_AB" : "StepKaHPA_BA";
                mSingleCommandHelper.packSingleCommand(cmdID, mPackCommand, masterDeviceID);
                appendExecQueue(mPackCommand);
            }
        }
    }
    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::SC)
    {
        DeviceID masterDeviceID;
        mManualMsg.configMacroData.getKaDtGGFDeviceID(KaDS, masterDeviceID);
        if (masterDeviceID.isValid())
        {
            mSingleCommandHelper.packSingleCommand("StepKaSCHPA_Power_Allow", mPackCommand, masterDeviceID);
            appendExecQueue(mPackCommand);

            auto cmdID = m_outputMode == RFOutputMode::TX ? "StepKaSCHPA_GO_ACU" : "StepKaSCHPA_UNGO_ACU";
            mSingleCommandHelper.packSingleCommand(cmdID, mPackCommand, masterDeviceID);
            appendExecQueue(mPackCommand);

            // 主备切换 2005 2006
            cmdID = masterDeviceID.extenID == 5 ? "StepKaSCHPA_AB" : "StepKaSCHPA_BA";
            mSingleCommandHelper.packSingleCommand(cmdID, mPackCommand, masterDeviceID);
            appendExecQueue(mPackCommand);
        }
    }

    execQueue();
}
void UpLinkHandler::sHpaDisable()
{
    QList<DeviceID> deviceIDList = { DeviceID(2, 0, 1), DeviceID(2, 0, 2) };
    for (auto& item : deviceIDList)
    {
        disableDevice("Step_SHPA_POWER_OFF", "ElectricControlor", item);
    }


    // S
    // S校零RF关
    QVariantMap replaceMap;
    replaceMap["SwitcherSet"] = 3;     // 去负载
    replaceMap["SwitchMode"] = 2;      // 手动
    replaceMap["AmplifMode"] = 2;      // 手动
    replaceMap["AmplifSwitcher"] = 1;  // 打开告警
    replaceMap["RFSwitch"] = 2;        // RF关
    disableDevice("StepGFQHKG_Unit", replaceMap);
    // disableDevice("StepGFQHKG_JL_OFF", replaceMap);
}

void UpLinkHandler::kackHpaDisable()
{
    // Ka遥测
    QList<DeviceID> deviceIDList = QList<DeviceID>{ DeviceID(2, 0, 3), DeviceID(2, 0, 4) };
    for (auto& item : deviceIDList)
    {
        disableDevice("StepKaHPA_Power_Forbid", "RFOutput", item, 2);
    }
}
void UpLinkHandler::kascHpaDisable()
{
    // Ka数传
    QList<DeviceID> deviceIDList = QList<DeviceID>{ DeviceID(2, 0, 5), DeviceID(2, 0, 6) };
    for (auto& item : deviceIDList)
    {
        disableDevice("StepKaSCHPA_Power_Forbid", "RFOutput", item, 2);
    }
}

void UpLinkHandler::hpaDisableSpecific()
{
    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::CK)
    {
        if (SystemWorkModeHelper::systemWorkModeToSystemBand(mLinkLine.workMode) == SystemBandMode::SBand)
        {
            sHpaDisable();
        }
        else if (mLinkLine.workMode == KaKuo2)
        {
            kackHpaDisable();
        }
    }
    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::SC)
    {
        kascHpaDisable();
    }
}
void UpLinkHandler::hpaDisable()
{
    /* 直接关闭所有的 */
    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::CK)
    {
        sHpaDisable();
        kackHpaDisable();
    }

    if (m_control == UpLinkDeviceControl::All || m_control == UpLinkDeviceControl::SC)
    {
        kascHpaDisable();
    }
}

void UpLinkHandler::xlDisable()
{
    // S校零变频器去电
    disableDevice("StepSXL_STOP", "AddRemElecCon");
    // Ka校零变频器去电
    disableDevice("StepKaXL_STOP", "AddRemElecCon");
}

void UpLinkHandler::otherDisable()
{
    // 前端测试开关网络 关机去电
    disableDevice("StepQDMNY_OFF_POWER", "PowerStatus");

    // 测试高频箱 关机去电 前端测试开关关闭这个就没必要了关了 省时间
    // disableDevice("StepCSGPX_OFF_POWER", "EquipPlusCtrl");
}

void UpLinkHandler::disableDevice(const QString& cmdKey, int unitID, const QString& powerKey, const DeviceID& deviceID, int powerOffValue,
                                  const QVariantMap& replaceMap)
{
    if (GlobalData::getReportParamData(deviceID, unitID, powerKey).toInt() != powerOffValue)
    {
        mSingleCommandHelper.packSingleCommand(cmdKey, mPackCommand, deviceID, replaceMap);
        waitExecSuccess(mPackCommand);
    }
}
void UpLinkHandler::disableDevice(const QString& cmdKey, const QString& powerKey, int powerOffValue, const QVariantMap& replaceMap)
{
    auto deviceID = mSingleCommandHelper.getSingleCmdDeviceID(cmdKey);
    disableDevice(cmdKey, 1, powerKey, deviceID, powerOffValue, replaceMap);
}

void UpLinkHandler::disableDevice(const QString& cmdKey, const QVariantMap& replaceMap)
{
    mSingleCommandHelper.packSingleCommand(cmdKey, mPackCommand, replaceMap);
    waitExecSuccess(mPackCommand);
}
void UpLinkHandler::disableDevice(const QString& cmdKey, const QString& powerKey, const DeviceID& deviceID, int powerOffValue)
{
    disableDevice(cmdKey, 1, powerKey, deviceID, powerOffValue);
}

bool UpLinkHandler::handle()
{
    if (m_enableJD == EnableMode::Enable)
    {
        jdEnable();
    }
    else if (m_enableJD == EnableMode::Disable)
    {
        jdDisable();
    }
    ExitCheck(false);

    if (m_enableQD == EnableMode::Enable)
    {
        ckqdEnable();
    }
    else if (m_enableQD == EnableMode::Disable)
    {
        ckqdDisable();
    }

    ExitCheck(false);
    if (m_enableHpa == EnableMode::Enable)
    {
        hpaEnable();
    }
    else if (m_enableHpa == EnableMode::Disable)
    {
        hpaDisable();
    }

    ExitCheck(false);
    if (EnableMode::Disable == m_enableOther)
    {
        otherDisable();
    }

    ExitCheck(false);
    if (EnableMode::Disable == m_enableXL)
    {
        xlDisable();
    }

    return true;
}
bool UpLinkHandler::handleSpecific()
{
    if (m_enableJD == EnableMode::Enable)
    {
        jdEnable();
    }
    else if (m_enableJD == EnableMode::Disable)
    {
        jdDisableSpecific();
    }
    ExitCheck(false);

    if (m_enableQD == EnableMode::Enable)
    {
        ckqdEnable();
    }
    else if (m_enableQD == EnableMode::Disable)
    {
        ckqdDisable();
    }

    ExitCheck(false);
    if (m_enableHpa == EnableMode::Enable)
    {
        hpaEnable();
    }
    else if (m_enableHpa == EnableMode::Disable)
    {
        hpaDisableSpecific();
    }

    ExitCheck(false);
    if (EnableMode::Disable == m_enableOther)
    {
        otherDisable();
    }

    ExitCheck(false);
    if (EnableMode::Disable == m_enableXL)
    {
        xlDisable();
    }

    return true;
}
