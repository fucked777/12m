#include "DeviceAlloter.h"

#include "BusinessMacroCommon.h"
#include "ExtendedConfig.h"
#include "ResourceRestructuringMessageDefine.h"
#include "SatelliteManagementDefine.h"

DeviceAlloter::DeviceAlloter() {}

bool DeviceAlloter::allot(ManualMessage& msg, QString& errorMsg)
{
    mManualMsg = msg;

    // 如果是手动控制，不存在设备冲突的情况，不自动分配，不是手动控制时才分配
    if (msg.isManualContrl)
    {
        // 设置链路信息
        setLinkInfo(msg.linkLineMap, msg.configMacroData);
    }
    else
    {
        // 获取每个模式的基带配置数据保存起来
        if (!initResourceData(errorMsg))
        {
            return false;
        }

        // 获取配置宏数据
        ConfigMacroData configMacroData;
        if (!getConfigMacro(configMacroData, errorMsg))
        {
            return false;
        }

        // 设置链路信息
        setLinkInfo(msg.linkLineMap, configMacroData);

        // 分配测控基带
        if (!allotCKJD(configMacroData, errorMsg))
        {
            return false;
        }
        // 分配高速基带
        if (!allotGSJD(configMacroData, errorMsg))
        {
            return false;
        }
        // 分配测控前端
        if (!allotCKQD(configMacroData, errorMsg))
        {
            return false;
        }

        // 把分配完成的替换原来的
        msg.configMacroData = configMacroData;
    }

    return true;
}

void DeviceAlloter::setLinkInfo(const QMap<SystemWorkMode, LinkLine>& linkLineMap, ConfigMacroData& configMacroData)
{
    for (const auto& link : linkLineMap)
    {
        auto linkWorkMode = link.workMode;

        // 设置当前模式的链路点频数
        configMacroData.setPointFreqNum(linkWorkMode, link.targetMap.size());
        // 设置当前链路模式
        configMacroData.setLinkWorkMode(linkWorkMode);

        // 如果是一体化+XXX组合模式，获取非主用目标信息
        TargetInfo noMasterTarget;
        if (SystemWorkModeHelper::isMixWorkMode(linkWorkMode))
        {
            for (auto targetNo : link.targetMap.keys())
            {
                if (targetNo != link.masterTargetNo)
                {
                    noMasterTarget = link.targetMap.value(targetNo);
                }
            }
        }

        // 设置主用数传还是扩频 24中一体化XXX+扩频使用
        configMacroData.setMasterType(linkWorkMode, link.masterType);

        // 设置每个目标的极化
        for (auto& targetInfo : link.targetMap)
        {
            auto targetWorkMode = targetInfo.workMode;
            auto taskCode = targetInfo.taskCode;
            auto pointFreqNo = targetInfo.pointFreqNo;
            auto targetNo = targetInfo.targetNo;

            SatelliteManagementData satelliteData;
            if (!GlobalData::getSatelliteManagementData(taskCode, satelliteData))
            {
                qWarning() << QString("获取任务代号为：%1的卫星数据失败").arg(taskCode);
                continue;
            }
            if (!satelliteData.m_workModeParamMap.contains(targetWorkMode))
            {
                qWarning()
                    << QString("任务代号为：%1的卫星不包含%2模式").arg(taskCode).arg(SystemWorkModeHelper::systemWorkModeToDesc(targetWorkMode));
                continue;
            }

            // 设置发射极化
            SystemOrientation launchPolar;
            if (satelliteData.getLaunchPolar(targetWorkMode, pointFreqNo, launchPolar))
            {
                configMacroData.setLaunchPolar(linkWorkMode, launchPolar, targetNo);
            }

            // 设置接收极化
            SystemOrientation receivPolar;
            if (satelliteData.getSReceivPolar(targetWorkMode, pointFreqNo, receivPolar))
            {
                configMacroData.setReceivePolar(linkWorkMode, receivPolar, targetNo);
            }

            // 设置跟踪极化
            SystemOrientation trackPolar;
            if (satelliteData.getSTrackPolar(targetWorkMode, pointFreqNo, trackPolar))
            {
                configMacroData.setTrackPolar(linkWorkMode, trackPolar, targetNo);
            }

            // 如果是主跟目标，设置主跟目标极化
            if (targetInfo.targetNo == link.masterTargetNo)
            {
                configMacroData.setMasterLaunchPolar(linkWorkMode, launchPolar);
                configMacroData.setMasterReceivePolar(linkWorkMode, receivPolar);
                configMacroData.setMasterTrackPolar(linkWorkMode, trackPolar);
            }

            // 如果是非主跟目标，设置非主跟目标极化（24一体化+XXX混合模式下）
            if (targetInfo.targetNo == noMasterTarget.targetNo)
            {
                configMacroData.setSlaveLaunchPolar(linkWorkMode, launchPolar);
                configMacroData.setSlaveReceivePolar(linkWorkMode, receivPolar);
                configMacroData.setSlaveTrackPolar(linkWorkMode, trackPolar);
            }
        }
    }
}

bool DeviceAlloter::initResourceData(QString& errorMsg)
{
    for (const auto& link : mManualMsg.linkLineMap)
    {
        // 获取基带配置
        ResourceRestructuringData resourceRestructureData;
        if (!GlobalData::getResourceRestructuringData(link.workMode, resourceRestructureData, mManualMsg.linkMasterSlave))
        {
            errorMsg = (QString("获取%1用的%2模式基带配置失败")
                            .arg(SystemWorkModeHelper::masterSlaveToDesc(mManualMsg.linkMasterSlave))
                            .arg(SystemWorkModeHelper::systemWorkModeToDesc(link.workMode)));

            return false;
        }

        mResourceDataMap[link.workMode] = resourceRestructureData;
    }

    return true;
}

bool DeviceAlloter::getConfigMacro(ConfigMacroData& configMacroData, QString& errorMsg)
{
    auto workModeSet = mManualMsg.linkLineMap.keys().toSet();

    // 优先去匹配到最全的配置宏数据
    if (GlobalData::getConfigMacroData(workModeSet, configMacroData, mManualMsg.linkMasterSlave))
    {
        return true;
    }

    // 没有匹配到，就获取每个单模式的配置宏，然后组成全部需要的模式
    for (auto workMode : workModeSet)
    {
        ConfigMacroData tempConfigMacroData;
        if (!GlobalData::getConfigMacroData(workMode, tempConfigMacroData, mManualMsg.linkMasterSlave))
        {
            errorMsg = QString("获取%1用的%2模式配置宏失败")
                           .arg(SystemWorkModeHelper::masterSlaveToDesc(mManualMsg.linkMasterSlave))
                           .arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
            return false;
        }

        configMacroData.configMacroCmdModeMap[workMode] = tempConfigMacroData.configMacroCmdModeMap[workMode];
    }

    return true;
}

bool DeviceAlloter::allotCKJD(ConfigMacroData& configMacroData, QString& errorMsg)
{
    // 获取所有测控模式
    QSet<SystemWorkMode> ckWorkModeSet;
    for (auto workMode : mManualMsg.linkLineMap.keys())
    {
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
        {
            ckWorkModeSet << workMode;
        }
    }
    // 不包含测控模式，不进行分配
    if (ckWorkModeSet.isEmpty())
    {
        return true;
    }

    // 只有两台测控基带 测控模式不能大于2
    if (ckWorkModeSet.size() > 2)
    {
        QStringList list;
        for (auto workMode : ckWorkModeSet)
        {
            list << QString("%1").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        }
        errorMsg = QString("当前流程测控模式有：%1，测控基带数量不足支撑功能").arg(list.join("、"));
        return false;
    }

    // 如果只有标准TTC模式，最多两个点频，一个点频一个基带
    if (ckWorkModeSet.size() == 1 && ckWorkModeSet.contains(STTC) && mManualMsg.linkLineMap.value(STTC).targetMap.size() > 2)
    {
        errorMsg = QString("当前流程中%1模式的点频数为%2，点频数量超过2个，测控基带数量不足支撑功能")
                       .arg(SystemWorkModeHelper::systemWorkModeToDesc(STTC))
                       .arg(mManualMsg.linkLineMap.value(STTC).targetMap.size());
        return false;
    }

    // 如果有两个测控模式，并且其中一个是标准TTC，那么标准TTC的只能有一个点频
    if (ckWorkModeSet.size() == 2 && ckWorkModeSet.contains(STTC) && mManualMsg.linkLineMap.value(STTC).targetMap.size() > 1)
    {
        errorMsg = QString("当前流程中%1模式的点频数为%2，点频数量超过2个，测控基带数量不足支撑功能")
                       .arg(SystemWorkModeHelper::systemWorkModeToDesc(STTC))
                       .arg(mManualMsg.linkLineMap.value(STTC).targetMap.size());
        return false;
    }

    // S扩二测控基带最多四个目标
    if (ckWorkModeSet.contains(Skuo2) && mManualMsg.linkLineMap.value(Skuo2).targetMap.size() > 4)
    {
        auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(Skuo2);
        auto targetNumber = mManualMsg.linkLineMap.value(Skuo2).targetMap.size();
        errorMsg =
            QString("当前流程中%1模式的点频数为%2，点频数量超过4个，%3模式最多支持4个目标").arg(workModeDesc).arg(targetNumber).arg(workModeDesc);
        return false;
    }

    // Ka扩二测控基带最多四个目标
    if (ckWorkModeSet.contains(KaKuo2) && mManualMsg.linkLineMap.value(KaKuo2).targetMap.size() > 4)
    {
        auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(KaKuo2);
        auto targetNumber = mManualMsg.linkLineMap.value(KaKuo2).targetMap.size();
        errorMsg =
            QString("当前流程中%1模式的点频数为%2，点频数量超过4个，%3模式最多支持4个目标").arg(workModeDesc).arg(targetNumber).arg(workModeDesc);
        return false;
    }

    // 扩跳模式只有单点频
    if (ckWorkModeSet.contains(SKT) && mManualMsg.linkLineMap.value(SKT).targetMap.size() > 1)
    {
        auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(SKT);
        auto targetNumber = mManualMsg.linkLineMap.value(SKT).targetMap.size();
        errorMsg = QString("当前流程中%1模式的点频数为%2，点频数量超过1个").arg(workModeDesc).arg(targetNumber).arg(workModeDesc);
        return false;
    }

    auto ckjdType = BBEType::CK_BBE;
    // 只有一个测控基带模式
    if (ckWorkModeSet.size() == 1)
    {
        // 如果是标准TTC模式，并且两个点频，一个点频下一个测控基带，两主用
        if (ckWorkModeSet.contains(STTC) && mManualMsg.linkLineMap.value(STTC).targetMap.size() == 2)
        {
            auto masterDeviceID1 = mResourceDataMap[STTC].getDeviceId(ckjdType | Main | First);
            auto masterDeviceID2 = mResourceDataMap[STTC].getDeviceId(ckjdType | Bak | First);

            // 设置主用1、主用2
            configMacroData.setCKJDDeviceID(STTC, masterDeviceID1, 1);
            configMacroData.setCKJDDeviceID(STTC, masterDeviceID2, 2);

            configMacroData.setCKJDMasterNumber(STTC, 2);  // 设置主用基带数量
        }
        else
        {
            // 直接使用当前模式的基带配置
            auto workMode = mManualMsg.linkLineMap.firstKey();
            auto masterDeviceID = mResourceDataMap[workMode].getDeviceId(ckjdType | Main | First);
            auto slaveDeviceID = mResourceDataMap[workMode].getDeviceId(ckjdType | Bak | First);

            // 设置主用和备用
            configMacroData.setCKJDDeviceID(workMode, masterDeviceID, 1);
            configMacroData.setCKJDDeviceID(workMode, slaveDeviceID, 2);

            configMacroData.setCKJDMasterNumber(workMode, 1);  // 设置主用基带数量
        }
    }
    // 使用的测控模式数量等于2，两主，一种模式使用一台测控基带
    else if (ckWorkModeSet.size() == 2)
    {
        QMap<SystemWorkMode, DeviceID> masterDeviceIDMap;  // QMap<工作模式, 主用设备ID>
        for (auto workMode : mManualMsg.linkLineMap.keys())
        {
            auto deviceID = mResourceDataMap[workMode].getDeviceId(ckjdType | Main | First);
            masterDeviceIDMap[workMode] = deviceID;
        }

        auto firstWorkMode = masterDeviceIDMap.firstKey();
        auto firstDeviceID = masterDeviceIDMap.first();

        auto secondWorkMode = masterDeviceIDMap.lastKey();
        auto secondDeviceID = masterDeviceIDMap.last();

        // 两个模式使用的是同一台测控基带进行分配，一种模式设置一台主用基带
        if (firstDeviceID == secondDeviceID)
        {
            // 按照SystemWorkMode的枚举值从小到大优先分配（QMap键自动排序）
            secondDeviceID.extenID = firstDeviceID.extenID == 1 ? 2 : 1;
        }
        configMacroData.setCKJDDeviceID(firstWorkMode, firstDeviceID, 1);
        configMacroData.setCKJDDeviceID(secondWorkMode, secondDeviceID, 1);

        configMacroData.setCKJDMasterNumber(firstWorkMode, 1);   // 设置主用基带数量
        configMacroData.setCKJDMasterNumber(secondWorkMode, 1);  // 设置主用基带数量
    }

    return true;
}

bool DeviceAlloter::allotGSJD(ConfigMacroData& configMacroData, QString& errorMsg)
{
    // 4424有Ka高速和X高速，如果两个都不包含就不进行分配
    if (!mManualMsg.linkLineMap.contains(KaGS) && !mManualMsg.linkLineMap.contains(XGS))
    {
        return true;
    }

    // Ka高速和X高速不能同时存在
    if (mManualMsg.linkLineMap.contains(KaGS) && mManualMsg.linkLineMap.contains(XGS))
    {
        errorMsg = QString("Ka高速和X高速不能同时执行");
        return false;
    }

    LinkLine linkLine;
    if (mManualMsg.linkLineMap.contains(KaGS))
    {
        linkLine = mManualMsg.linkLineMap.value(KaGS);
    }
    else if (mManualMsg.linkLineMap.contains(XGS))
    {
        linkLine = mManualMsg.linkLineMap.value(XGS);
    }

    // 根据点频和极化进行判断是否是需要两台都是主机，双点频双极化时需要两台主机
    bool isDoubleMaster = false;

    // 双点频
    if (linkLine.targetMap.size() == 2)
    {
        // 获取第一个点频的接收极化
        SystemOrientation firstOrientation = SystemOrientation::Unkonwn;
        configMacroData.getReceivePolar(linkLine.workMode, firstOrientation, 1);

        // 获取第二个点频的接收极化
        SystemOrientation secondOrientation = SystemOrientation::Unkonwn;
        configMacroData.getReceivePolar(linkLine.workMode, secondOrientation, 2);

        // 两个点频只要有一个是左右旋同时，就是双极化
        if (firstOrientation == SystemOrientation::LRCircumflex || secondOrientation == SystemOrientation::LRCircumflex)
        {
            isDoubleMaster = true;
        }
    }

    // 设置主用基带数量
    if (!isDoubleMaster)
    {
        configMacroData.setGSJDMasterNumber(linkLine.workMode, 1);
    }
    else
    {
        configMacroData.setGSJDMasterNumber(linkLine.workMode, 2);
    }

    // 从资源重组获取高速基带设备ID
    auto resourceData = mResourceDataMap[linkLine.workMode];

    DeviceID gsjd1DeviceID;
    DeviceID gsjd2DeviceID;
    DeviceID gsjd3DeviceID;
    if (linkLine.targetMap.size() == 1)
    {
        BBEType gsckjdType = BBEType::GSDDP_BBE;
        gsjd1DeviceID = resourceData.getDeviceId(gsckjdType | Main | First);  // 获取主用
        gsjd2DeviceID = resourceData.getDeviceId(gsckjdType | Bak | First);   // 获取备用
        gsjd3DeviceID = resourceData.getDeviceId(gsckjdType | Bak | Second);  // 获取备用
    }
    else if (linkLine.targetMap.size() == 2)
    {
        BBEType gsckjdType = BBEType::GSSDP_BBE;
        gsjd1DeviceID = resourceData.getDeviceId(gsckjdType | Main | First);   // 获取主用
        gsjd2DeviceID = resourceData.getDeviceId(gsckjdType | Main | Second);  // 获取主用
        gsjd3DeviceID = resourceData.getDeviceId(gsckjdType | Bak | First);    // 获取备用
    }

    configMacroData.setGSJDDeviceID(linkLine.workMode, gsjd1DeviceID, 1);
    configMacroData.setGSJDDeviceID(linkLine.workMode, gsjd2DeviceID, 2);
    configMacroData.setGSJDDeviceID(linkLine.workMode, gsjd3DeviceID, 3);

    return true;
}

bool DeviceAlloter::allotCKQD(ConfigMacroData& configMacroData, QString& /*errorMsg*/)
{
    QMap<SystemWorkMode, DeviceID> ckDeviceIDMap;  // 测控使用的测控前端，测控使用测控前端1、2
    QMap<SystemWorkMode, DeviceID> scDeviceIDMap;  // 低速数传使用的测控前端 数传使用前端3、4

    for (auto workMode : mManualMsg.linkLineMap.keys())
    {
        DeviceID ckqdDeviceID;
        configMacroData.getCKQDDeviceID(workMode, ckqdDeviceID);
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))  // 测控
        {
            ckDeviceIDMap[workMode] = ckqdDeviceID;
        }
        else if (SystemWorkModeHelper::isDsDataTransmissionWorkMode(workMode))  // 低速
        {
            scDeviceIDMap[workMode] = ckqdDeviceID;
        }
    }

    // 分配测控基带使用的测控前端
    if (ckDeviceIDMap.size() == 2)
    {
        auto firstWorkMode = ckDeviceIDMap.firstKey();
        auto firstDeviceID = ckDeviceIDMap.first();

        auto secondWorkMode = ckDeviceIDMap.lastKey();
        auto secondDeviceID = ckDeviceIDMap.last();

        // 如果两个模式使用的是同一台测控前端进行分配，一种模式设置一台测控前端
        if (firstDeviceID == secondDeviceID)
        {
            // 按照SystemWorkMode的枚举值从小到大优先分配（QMap键自动排序）
            configMacroData.setCKQDDeviceID(firstWorkMode, firstDeviceID);

            secondDeviceID.extenID = firstDeviceID.extenID == 1 ? 2 : 1;  // 使用另一台
            configMacroData.setCKQDDeviceID(secondWorkMode, secondDeviceID);
        }
    }

    return true;
}
