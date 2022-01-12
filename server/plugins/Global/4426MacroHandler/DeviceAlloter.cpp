#include "DeviceAlloter.h"

#include "BaseHandler.h"
#include "BusinessMacroCommon.h"
#include "ExtendedConfig.h"
#include "ResourceRestructuringMessageDefine.h"
#include "SatelliteManagementDefine.h"

DeviceAlloter::DeviceAlloter() {}
bool DeviceAlloter::allot(ManualMessage& msg, QString& errorMsg)
{
    mManualMsg = msg;

    // 如果是手动控制，不存在设备冲突的情况，不自动分配，不是手动控制时才分配
    // 20211114 这个在界面上已经去掉  26弃用
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
        if (!getConfigMacro(configMacroData))
        {
            errorMsg = QString("配置宏获取失败当前模式:%1").arg(configMacroData.configMacroName);
            return false;
        }
        // 设置链路信息
        setLinkInfo(msg.linkLineMap, configMacroData);

        // 分配测控基带
        // TODO 818 固定第一个主用 第二个备用
        if (!allotCKJD(configMacroData, errorMsg))
        {
            return false;
        }
        // 分配高速
        if (!allotGSJD(configMacroData, errorMsg))
        {
            return false;
        }
        // 分配低速基带
        if (!allotDSJD(configMacroData, errorMsg))
        {
            return false;
        }
        // 分配测控前端
        // TODO 818 固定第一个主用 第二个备用
        if (!allotCKQD(configMacroData, errorMsg))
        {
            return false;
        }
        //分配高速下变频
        if (!allotGSDC(configMacroData, errorMsg))
        {
            return false;
        }

        // 存储转发
        if (!allotCCZF(configMacroData, errorMsg))
        {
            return false;
        }

        //分配跟踪基带 跟踪必须最后
        if (!allotGZ(configMacroData, errorMsg))
        {
            return false;
        }

        // 把分配完成的替换原来的
        msg.configMacroData = configMacroData;
    }

    mManualMsg = msg;

    // 查看结果的  测试用
    // QMap<DeviceID, QSet<SystemWorkMode>> gzjdDeviceIDMap;
    // QMap<DeviceID, QSet<SystemWorkMode>> gzqdDeviceIDMap;
    // DeviceID gzjdDeviceID;
    // for (auto workMode : mManualMsg.linkLineMap.keys())
    // {
    //     if (mManualMsg.configMacroData.getGZJDDeviceID(workMode, gzjdDeviceID))
    //     {
    //         if (gzjdDeviceID.isValid())
    //         {
    //             gzjdDeviceIDMap[gzjdDeviceID] << workMode;
    //         }
    //     }
    //     if (mManualMsg.configMacroData.getGZQDDeviceID(workMode, gzjdDeviceID))
    //     {
    //         if (gzjdDeviceID.isValid())
    //         {
    //             gzqdDeviceIDMap[gzjdDeviceID] << workMode;
    //         }
    //     }
    // }
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
                qWarning() << QString("任务代号为：%1的卫星不包含%2模式").arg(taskCode, SystemWorkModeHelper::systemWorkModeToDesc(targetWorkMode));
                continue;
            }

            // 设置发射极化
            SystemOrientation launchPolar = SystemOrientation::Unkonwn;
            if (satelliteData.getLaunchPolar(targetWorkMode, pointFreqNo, launchPolar))
            {
                configMacroData.setLaunchPolar(linkWorkMode, launchPolar, targetNo);
            }

            // 设置接收极化
            SystemOrientation receivPolar = SystemOrientation::Unkonwn;
            if (satelliteData.getSReceivPolar(targetWorkMode, pointFreqNo, receivPolar))
            {
                configMacroData.setReceivePolar(linkWorkMode, receivPolar, targetNo);
            }

            // 设置跟踪极化
            SystemOrientation trackPolar = SystemOrientation::Unkonwn;
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
                            .arg(SystemWorkModeHelper::masterSlaveToDesc(mManualMsg.linkMasterSlave),
                                 SystemWorkModeHelper::systemWorkModeToDesc(link.workMode)));

            return false;
        }

        mResourceDataMap[link.workMode] = resourceRestructureData;
    }

    return true;
}

bool DeviceAlloter::getConfigMacro(ConfigMacroData& configMacroData)
{
    auto workModeSet = mManualMsg.linkLineMap.keys().toSet();

    // 优先去匹配到最全的配置宏数据
    return GlobalData::getConfigMacroData(workModeSet, configMacroData, mManualMsg.linkMasterSlave);

    //    // 没有匹配到，就获取每个单模式的配置宏，然后组成全部需要的模式
    //    for (auto workMode : workModeSet)
    //    {
    //        ConfigMacroData tempConfigMacroData;
    //        if (!GlobalData::getConfigMacroData(workMode, tempConfigMacroData, mManualMsg.linkMasterSlave))
    //        {
    //            errorMsg = QString("获取%1用的%2模式配置宏失败")
    //                           .arg(SystemWorkModeHelper::masterSlaveToDesc(mManualMsg.linkMasterSlave))
    //                           .arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
    //            return false;
    //        }

    //        configMacroData.configMacroCmdListMap[workMode] = tempConfigMacroData.configMacroCmdListMap[workMode];
    //    }

    //    return true;
}

bool DeviceAlloter::allotCKJD(ConfigMacroData& configMacroData, QString& errorMsg)
{
    // 获取所有测控模式
    QSet<SystemWorkMode> ckWorkModeSet;
    for (auto iter = mManualMsg.linkLineMap.begin(); iter != mManualMsg.linkLineMap.end(); ++iter)
    {
        auto workMode = iter.key();
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

    auto curProject = ExtendedConfig::curProjectID();
    if (curProject == "4426")
    {
        return allotCKJD4426(ckWorkModeSet, configMacroData, errorMsg);
    }

    return allotCKJD4424(ckWorkModeSet, configMacroData, errorMsg);
}
bool DeviceAlloter::allotCKJD4424(const QSet<SystemWorkMode>& ckWorkModeSet, ConfigMacroData& configMacroData, QString& errorMsg)
{
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
            if (!mResourceDataMap.contains(STTC))
            {
                errorMsg = QString("当前基带配置中无%1的基带配置信息").arg(SystemWorkModeHelper::systemWorkModeToDesc(STTC));
                return false;
            }
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
            auto workMode = *ckWorkModeSet.begin();
            if (!mResourceDataMap.contains(workMode))
            {
                errorMsg = QString("当前基带配置中无%1的基带配置信息").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
                return false;
            }
            auto masterDeviceID = mResourceDataMap[workMode].getDeviceId(ckjdType | Main | First);
            auto slaveDeviceID = mResourceDataMap[workMode].getDeviceId(ckjdType | Bak | First);

            // 设置主用和备用
            configMacroData.setCKJDDeviceID(workMode, masterDeviceID, 1);
            configMacroData.setCKJDDeviceID(workMode, slaveDeviceID, 2);
            configMacroData.setCKJDMasterNumber(workMode, 1);  // 设置主用基带数量
        }
        return true;
    }
    // 使用的测控模式数量等于2，两主，一种模式使用一台测控基带
    else if (ckWorkModeSet.size() == 2)
    {
        QMap<SystemWorkMode, DeviceID> masterDeviceIDMap;  // QMap<工作模式, 主用设备ID>
        for (auto iter = ckWorkModeSet.begin(); iter != ckWorkModeSet.end(); ++iter)
        {
            auto workMode = *iter;
            if (!mResourceDataMap.contains(workMode))
            {
                errorMsg = QString("当前基带配置中无%1的基带配置信息").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
                return false;
            }
            auto deviceID = mResourceDataMap[workMode].getDeviceId(ckjdType | Main | First);
            masterDeviceIDMap[workMode] = deviceID;
        }

        auto firstWorkMode = masterDeviceIDMap.firstKey();
        auto firstDeviceID = masterDeviceIDMap.first();

        auto secondWorkMode = masterDeviceIDMap.lastKey();
        auto secondDeviceID = masterDeviceIDMap.last();
        // 两个模式使用的是同一台测控基带进行分配，4424一种模式设置一台主用基带,4426特殊处理
        if (firstDeviceID == secondDeviceID)
        {
            // 按照SystemWorkMode的枚举值从小到大优先分配（QMap键自动排序）
            configMacroData.setCKJDDeviceID(firstWorkMode, firstDeviceID, 1);
            secondDeviceID.extenID = firstDeviceID.extenID == 1 ? 2 : 1;
            configMacroData.setCKJDDeviceID(secondWorkMode, secondDeviceID, 1);
        }

        configMacroData.setCKJDMasterNumber(firstWorkMode, 1);   // 设置主用基带数量
        configMacroData.setCKJDMasterNumber(secondWorkMode, 1);  // 设置主用基带数量

        return true;
    }

    return false;
}
bool DeviceAlloter::allotCKJD4426(const QSet<SystemWorkMode>& ckWorkModeSet, ConfigMacroData& configMacroData, QString& errorMsg)
{
    /* 4426 截止20211025 所支持的模式只有 5种
     * STTC
     * SKUO2
     * KAKUO2
     * SKT
     * SKUO2+KAKUO2
     *
     * 26理论上不存在两台主机的情况
     */
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

    // 多模式组合只支持Skuo2+Kakuo2
    if (ckWorkModeSet.size() == 2 && !(ckWorkModeSet.contains(Skuo2) && ckWorkModeSet.contains(KaKuo2)))
    {
        QStringList list;
        for (auto workMode : ckWorkModeSet)
        {
            list << QString("%1").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
        }
        errorMsg = QString("当前测控基带只支持S扩2+Ka扩2组合当前组合是: %1").arg(list.join("、"));
        return false;
    }

    // 只能是单点频
    if (ckWorkModeSet.contains(STTC) && mManualMsg.linkLineMap.value(STTC).targetMap.size() >= 2)
    {
        errorMsg = QString("当前流程不支持STTC多点频");
        return false;
    }

    auto ckjdType = BBEType::CK_BBE;
    // 只有一个测控基带模式
    if (ckWorkModeSet.size() == 1)
    {
        // 直接使用当前模式的基带配置
        auto workMode = *ckWorkModeSet.begin();
        if (!mResourceDataMap.contains(workMode))
        {
            errorMsg = QString("当前基带配置中无%1的基带配置信息").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
            return false;
        }
        auto masterDeviceID = mResourceDataMap[workMode].getDeviceId(ckjdType | Main | First);
        auto slaveDeviceID = mResourceDataMap[workMode].getDeviceId(ckjdType | Bak | First);

        if (!masterDeviceID.isValid())
        {
            masterDeviceID = (*ckWorkModeSet.begin() == KaKuo2) ? DeviceID(4, 0, 2) : DeviceID(4, 0, 1);
        }
        if (!slaveDeviceID.isValid())
        {
            slaveDeviceID = masterDeviceID;
        }

        if (masterDeviceID == slaveDeviceID)
        {
            slaveDeviceID = BaseHandler::getNextCKDeviceID(masterDeviceID);
        }
        // 设置主用和备用
        configMacroData.setCKJDDeviceID(workMode, masterDeviceID, 1);
        configMacroData.setCKJDDeviceID(workMode, slaveDeviceID, 2);
        configMacroData.setCKJDMasterNumber(workMode, 1);  // 设置主用基带数量
        configMacroData.setCKTargetNumber(workMode, mManualMsg.linkLineMap.value(workMode).targetMap.size());
        return true;
    }

    // 只使用一种模式的就行 否则会错乱 因为不同模式的ID不一样
    // 双模式Skuo2+Ka扩2
    auto firstWorkMode = *ckWorkModeSet.begin();
    auto secondWorkMode = *(++ckWorkModeSet.begin());
    if (!mResourceDataMap.contains(firstWorkMode))
    {
        errorMsg = QString("当前基带配置中无%1的基带配置信息").arg(SystemWorkModeHelper::systemWorkModeToDesc(firstWorkMode));
        return false;
    }
    auto firstDeviceID = mResourceDataMap[firstWorkMode].getDeviceId(ckjdType | Main | First);
    // 这里没写错就是一样的模式获取
    auto secondDeviceID = mResourceDataMap[firstWorkMode].getDeviceId(ckjdType | Bak | First);

    if (!firstDeviceID.isValid())
    {
        firstDeviceID = DeviceID(4, 0, 1);
    }
    if (!secondDeviceID.isValid())
    {
        secondDeviceID = firstDeviceID;
    }

    if (firstDeviceID == secondDeviceID)
    {
        secondDeviceID = BaseHandler::getNextCKDeviceID(firstDeviceID);
    }

    firstDeviceID = BaseHandler::getChangeModeCKDeviceID(firstDeviceID, firstWorkMode);
    secondDeviceID = BaseHandler::getChangeModeCKDeviceID(secondDeviceID, firstWorkMode);
    configMacroData.setCKJDDeviceID(firstWorkMode, firstDeviceID, 1);
    configMacroData.setCKJDDeviceID(firstWorkMode, secondDeviceID, 2);

    firstDeviceID = BaseHandler::getChangeModeCKDeviceID(firstDeviceID, secondWorkMode);
    secondDeviceID = BaseHandler::getChangeModeCKDeviceID(secondDeviceID, secondWorkMode);
    configMacroData.setCKJDDeviceID(secondWorkMode, firstDeviceID, 1);
    configMacroData.setCKJDDeviceID(secondWorkMode, secondDeviceID, 2);

    configMacroData.setCKJDMasterNumber(firstWorkMode, 1);
    configMacroData.setCKJDMasterNumber(secondWorkMode, 1);

    configMacroData.setCKTargetNumber(firstWorkMode, mManualMsg.linkLineMap.value(firstWorkMode).targetMap.size());
    configMacroData.setCKTargetNumber(secondWorkMode, mManualMsg.linkLineMap.value(secondWorkMode).targetMap.size());
    return true;
}

bool DeviceAlloter::allotGSJD(ConfigMacroData& configMacroData, QString& errorMsg)
{
    auto workMode = KaGS;
    // 4426只有Ka高速，不包含Ka高速模式，不进行分配
    if (!mManualMsg.linkLineMap.contains(workMode))
    {
        return true;
    }

    // 根据点频和极化进行判断是否是需要两台都是主机，双点频双极化时需要两台都是主机
    bool isDoubleMaster = false;
    auto& linkLine = mManualMsg.linkLineMap[workMode];
    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(linkLine.workMode);
    if (linkLine.targetMap.size() == 2)  // 双点频
    {
        // 获取第一个点频的极化
        auto firstTargetInfo = linkLine.targetMap.first();
        QVariant value;
        if (!MacroCommon::getRecvPolarization(firstTargetInfo.taskCode, firstTargetInfo.workMode, firstTargetInfo.pointFreqNo, value))
        {
            errorMsg = QString("获取任务代号为：%1的卫星数据，模式为%2的点频%3的旋向失败")
                           .arg(firstTargetInfo.taskCode, workModeDesc)
                           .arg(firstTargetInfo.targetNo);
            return false;
        }
        auto firstOrientation = SystemOrientation(value.toInt());

        // 获取第二个点频的极化
        auto secondTargetInfo = linkLine.targetMap.last();
        if (!MacroCommon::getRecvPolarization(secondTargetInfo.taskCode, secondTargetInfo.workMode, secondTargetInfo.pointFreqNo, value))
        {
            errorMsg = QString("获取任务代号为：%1的卫星数据，模式为%2的点频%3的旋向失败")
                           .arg(secondTargetInfo.taskCode, workModeDesc)
                           .arg(secondTargetInfo.targetNo);
            return false;
        }
        auto secondOrientation = SystemOrientation(value.toInt());

        // 两个点频只要有一个是左右旋同时，就是双极化
        if (firstOrientation == SystemOrientation::LRCircumflex || secondOrientation == SystemOrientation::LRCircumflex)
        {
            isDoubleMaster = true;
        }
    }

    /* 处理一下主跟目标 不存在才去管 这里的主跟目标和计划的有些许不同 */
    if (!linkLine.targetMap.contains(linkLine.masterTargetNo))
    {
        linkLine.masterTargetNo = linkLine.targetMap.firstKey();
    }

    auto resourceData = mResourceDataMap[workMode];
    BBEType gsckjdType = BBEType::GS_BBE;
    DeviceID gsjd1DeviceID = resourceData.getDeviceId(gsckjdType | Main | First);  // 获取主用
    DeviceID gsjd2DeviceID = resourceData.getDeviceId(gsckjdType | Bak | First);   // 获取备用

    configMacroData.setGSJDDeviceID(linkLine.workMode, gsjd1DeviceID, 1);
    configMacroData.setGSJDDeviceID(linkLine.workMode, gsjd2DeviceID, 2);

    // 设置主用基带数量
    if (!isDoubleMaster)
    {
        configMacroData.setGSJDMasterNumber(linkLine.workMode, 1);
    }
    else
    {
        configMacroData.setGSJDMasterNumber(linkLine.workMode, 2);
    }

    return true;
}

bool DeviceAlloter::allotDSJD(ConfigMacroData& configMacroData, QString& errorMsg)
{
    // 没有低速不用管低速基带
    if (!(mManualMsg.linkLineMap.contains(XDS) || mManualMsg.linkLineMap.contains(KaDS)))
    {
        return true;
    }
    auto modeList = SystemWorkModeHelper::getDSContrlWorkMode();
    QSet<SystemWorkMode> dsjdWorkModeSet;
    bool hasDoublePointFreq = false;
    for (auto& workMode : modeList)
    {
        if (!mManualMsg.linkLineMap.contains(workMode))
        {
            continue;
        }
        auto& link = mManualMsg.linkLineMap[workMode];

        // Ka低速或者X低速，低速基带可以看成两台Ka低速、两台X低速
        dsjdWorkModeSet << link.workMode;
        // 获取模式对应的低速基带
        if (!mResourceDataMap.contains(workMode))
        {
            errorMsg = QString("当前基带配置中无%1的基带配置信息").arg(SystemWorkModeHelper::systemWorkModeToDesc(workMode));
            return false;
        }

        auto resourceData = mResourceDataMap[workMode];
        auto deviceType = BBEType::DS_BBE;
        auto dsjd1DeviceID = resourceData.getDeviceId(deviceType | Main | First);
        auto dsjd2DeviceID = resourceData.getDeviceId(deviceType | Bak | First);

        configMacroData.setDSJDDeviceID(link.workMode, dsjd1DeviceID, 1);
        configMacroData.setDSJDDeviceID(link.workMode, dsjd2DeviceID, 2);
        // 根据点频进行判断是否是需要两台都是主机，双点频时需要两台都是主机
        if (link.targetMap.size() == 2)
        {
            hasDoublePointFreq = true;
        }
        if (!link.targetMap.contains(link.masterTargetNo))
        {
            link.masterTargetNo = link.targetMap.firstKey();
        }
    }

    // 如果Ka低速或者X低速同时存在，只要有一个是双点频，那么低速基带就是两主
    for (auto& workMode : dsjdWorkModeSet)
    {
        if (hasDoublePointFreq)
        {
            configMacroData.setDSJDMasterNumber(workMode, 2);
        }
        else
        {
            configMacroData.setDSJDMasterNumber(workMode, 1);
        }
    }
    return true;
}

bool DeviceAlloter::allotCKQD(ConfigMacroData& configMacroData, QString& errorMsg)
{
    // 单高速不用管测控前端
    if (mManualMsg.linkLineMap.size() == 1 && mManualMsg.linkLineMap.contains(KaGS))
    {
        return true;
    }
    /* 截止20211029
     * 测控 只考虑 所有的单模式 + (S扩2+Ka扩2组合)
     * 低速 只考虑 XDS KaDS 和 （XDS+KaDS）
     */
    QMap<SystemWorkMode, DeviceID> ckDeviceIDMap;  // 测控使用的测控前端，测控使用测控前端1、2
    QMap<SystemWorkMode, DeviceID> scDeviceIDMap;  // 低速数传使用的测控前端 数传使用前端3、4

    for (auto iter = mManualMsg.linkLineMap.begin(); iter != mManualMsg.linkLineMap.end(); ++iter)
    {
        auto workMode = iter.key();
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

    /* 20211028
     * 当前测控前端下发链路时信号没有给备机
     * 这里更改此问题
     */
    /* 1.模式处理确保
     * 测控使用1 2
     * 低速使用3 4
     */
    for (auto& item : ckDeviceIDMap)
    {
        if (!item.isValid())
        {
            item = DeviceID(7, 0, 1);
            continue;
        }
        if (!(item.extenID == 1 || item.extenID == 2))
        {
            item.extenID = 1;
        }
    }
    for (auto& item : scDeviceIDMap)
    {
        if (!item.isValid())
        {
            item = DeviceID(7, 0, 3);
            continue;
        }
        if (!(item.extenID == 3 || item.extenID == 4))
        {
            item.extenID = 3;
        }
    }

    /*
     * 2.前端主备
     * 这里的主备是要保证同时能收到链路信号
     * 要和基带一样改成 CKQD1 CKQD2的形式
     * 都是主备机器
     * 所以这里出现两种模式的情况一定是Skuo2+Kakuo2 需要改成一样
     */
    // 单模式
    if (ckDeviceIDMap.size() == 1)
    {
        auto workMode = ckDeviceIDMap.firstKey();
        auto masterDeviceID = ckDeviceIDMap.first();
        DeviceID slaveDeviceID;
        if (masterDeviceID.isValid())
        {
            slaveDeviceID = masterDeviceID;
            slaveDeviceID.extenID = slaveDeviceID.extenID == 1 ? 2 : 1;
        }
        else
        {
            masterDeviceID = DeviceID(7, 0, 1);
            slaveDeviceID = DeviceID(7, 0, 2);
        }

        configMacroData.setCKQDDeviceID(workMode, masterDeviceID, 1);
        configMacroData.setCKQDDeviceID(workMode, slaveDeviceID, 2);
        configMacroData.setDoubleMode(workMode, false);
    }
    else if (ckDeviceIDMap.size() == 2)
    {
        auto firstWorkMode = ckDeviceIDMap.firstKey();
        auto firstDeviceID = ckDeviceIDMap.first();

        auto secondWorkMode = ckDeviceIDMap.lastKey();
        auto secondDeviceID = ckDeviceIDMap.last();

        if (!firstDeviceID.isValid())
        {
            firstDeviceID = DeviceID(7, 0, 1);
        }
        secondDeviceID = firstDeviceID;
        secondDeviceID.extenID = secondDeviceID.extenID == 1 ? 2 : 1;

        // 双模式 使用同一个前端
        if (ckDeviceIDMap.contains(Skuo2) && ckDeviceIDMap.contains(KaKuo2))
        {
            // 双模式 使用同一个前端
            configMacroData.setCKQDDeviceID(firstWorkMode, firstDeviceID, 1);
            configMacroData.setCKQDDeviceID(secondWorkMode, firstDeviceID, 1);

            configMacroData.setCKQDDeviceID(firstWorkMode, secondDeviceID, 2);
            configMacroData.setCKQDDeviceID(secondWorkMode, secondDeviceID, 2);

            // 加入一个特殊参数
            configMacroData.setDoubleMode(firstWorkMode, true);
            configMacroData.setDoubleMode(secondWorkMode, true);
        }
        /* 两主 */
        else
        {
            configMacroData.setCKQDDeviceID(firstWorkMode, firstDeviceID, 1);
            configMacroData.setCKQDDeviceID(secondWorkMode, secondDeviceID, 1);
            configMacroData.setCKQDDeviceID(firstWorkMode, firstDeviceID, 2);
            configMacroData.setCKQDDeviceID(secondWorkMode, secondDeviceID, 2);

            configMacroData.setDoubleMode(firstWorkMode, false);
            configMacroData.setDoubleMode(secondWorkMode, false);
        }
    }
    else if (ckDeviceIDMap.size() >= 3)
    {
        errorMsg = "当前测控前端数量超限";
        return false;
    }

    // 低速这里的双模式 指的是XDS和KaDS
    // 单模式
    if (scDeviceIDMap.size() == 1)
    {
        auto workMode = scDeviceIDMap.firstKey();
        auto masterDeviceID = scDeviceIDMap.first();
        DeviceID slaveDeviceID;
        if (masterDeviceID.isValid())
        {
            slaveDeviceID = masterDeviceID;
            slaveDeviceID.extenID = slaveDeviceID.extenID == 3 ? 4 : 3;
        }
        else
        {
            masterDeviceID = DeviceID(7, 0, 3);
            slaveDeviceID = DeviceID(7, 0, 4);
        }

        configMacroData.setCKQDDeviceID(workMode, masterDeviceID, 1);
        configMacroData.setCKQDDeviceID(workMode, slaveDeviceID, 2);
    }
    /* X低速 和 Ka低速 */
    else if (scDeviceIDMap.size() == 2)
    {
        auto firstWorkMode = scDeviceIDMap.firstKey();
        auto firstDeviceID = scDeviceIDMap.first();

        auto secondWorkMode = scDeviceIDMap.lastKey();
        auto secondDeviceID = scDeviceIDMap.last();

        if (!firstDeviceID.isValid())
        {
            firstDeviceID = DeviceID(7, 0, 3);
        }

        // X低速和Ka低速同时存在使用同一个前端
        secondDeviceID = firstDeviceID;
        secondDeviceID.extenID = secondDeviceID.extenID == 3 ? 4 : 3;

        // X低速和Ka低速同时存在使用同一个前端
        if (scDeviceIDMap.contains(XDS) && scDeviceIDMap.contains(KaDS))
        {
            configMacroData.setCKQDDeviceID(firstWorkMode, firstDeviceID, 1);
            configMacroData.setCKQDDeviceID(secondWorkMode, firstDeviceID, 1);

            configMacroData.setCKQDDeviceID(firstWorkMode, secondDeviceID, 2);
            configMacroData.setCKQDDeviceID(secondWorkMode, secondDeviceID, 2);
        }
        else
        {
            configMacroData.setCKQDDeviceID(firstWorkMode, firstDeviceID, 1);
            configMacroData.setCKQDDeviceID(secondWorkMode, secondDeviceID, 1);
            configMacroData.setCKQDDeviceID(firstWorkMode, firstDeviceID, 2);
            configMacroData.setCKQDDeviceID(secondWorkMode, secondDeviceID, 2);
        }
    }
    else if (scDeviceIDMap.size() >= 3)
    {
        errorMsg = "当前测控前端数量超限";
        return false;
    }

    return true;
}

bool DeviceAlloter::allotGZ(ConfigMacroData& configMacroData, QString& errorMsg)
{
    // 单XDS不用管跟踪基带
    if (mManualMsg.linkLineMap.size() == 1 && mManualMsg.linkLineMap.contains(XDS))
    {
        return true;
    }
    mManualMsg.configMacroData = configMacroData;
    // 跟踪基带
    // 策略
    // A B 机主备 测控+高速
    // C D 机主备 测控+低速
    // 最多同时两种测控模式 其他的去掉
    // 先保证所有模式都有正确的数据
    /*
     * 标准 扩频 高速 标准+高速 扩频+高速
     * 标准 扩频 低速 标准+低速 扩频+低速
     * 无扩跳以上两行和每行取一种组合
     *
     * 扩跳 + 标准 扩频 高速 标准+高速 扩频+高速
     * 扩跳 + 标准 扩频 低速 标准+低速 扩频+低速
     * 有扩跳以上两种类型组合
     */

    /* 1.获取卫星的跟踪模式 */
    SatelliteManagementData satelliteManagementData;
    if (!GlobalData::getSatelliteManagementData(mManualMsg.masterTaskCode, satelliteManagementData))
    {
        errorMsg = QString("获取任务代号为%1的卫星数据错误").arg(mManualMsg.masterTaskCode);
        return false;
    }

    auto trackingInfoResult = BaseHandler::checkTrackingInfo(mManualMsg, satelliteManagementData.m_trackingMode);
    if (!trackingInfoResult)
    {
        errorMsg = trackingInfoResult.msg();
        return false;
    }
    /* 跟踪模式 获取所有的设备 */
    auto m_rackingMode = std::get<0>(trackingInfoResult.value());
    auto modeList = BaseHandler::trackingToSystemWorkModeList(m_rackingMode);
    QMap<SystemWorkMode, DeviceID> gzMasterDeviceMap;
    for (auto& item : modeList)
    {
        DeviceID deviceID;
        configMacroData.getGZQDDeviceID(item, deviceID);
        gzMasterDeviceMap.insert(item, deviceID);
    }

    QList<SystemWorkMode> gzckDeviceList;
    for (auto iter = gzMasterDeviceMap.begin(); iter != gzMasterDeviceMap.end(); ++iter)
    {
        auto tempMode = iter.key();
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(tempMode))
        {
            gzckDeviceList << iter.key();
        }
    }

    bool existKt = gzMasterDeviceMap.contains(SKT);
    // 凑数
    if (gzckDeviceList.size() < 2)
    {
        if (configMacroData.configMacroCmdModeMap.contains(KaKuo2) && !gzMasterDeviceMap.contains(KaKuo2))
        {
            gzckDeviceList << KaKuo2;
        }
        if (configMacroData.configMacroCmdModeMap.contains(Skuo2) && !gzMasterDeviceMap.contains(Skuo2))
        {
            gzckDeviceList << Skuo2;
        }
        if (configMacroData.configMacroCmdModeMap.contains(STTC) && !gzMasterDeviceMap.contains(STTC))
        {
            gzckDeviceList << STTC;
        }
        // 高速和低速同时存在则不能增加扩跳
        // 优先高低速
        if (!(gzMasterDeviceMap.contains(KaDS) && gzMasterDeviceMap.contains(KaGS)))
        {
            if (configMacroData.configMacroCmdModeMap.contains(SKT) && !gzMasterDeviceMap.contains(SKT))
            {
                gzckDeviceList << SKT;
            }
        }
    }

    auto gzjdResetFunc = [](ConfigMacroData& configMacroData, SystemWorkMode workMode) {
        if (configMacroData.configMacroCmdModeMap.contains(workMode))
        {
            DeviceID qdID;
            DeviceID jdID;
            auto res = configMacroData.getGZQDDeviceID(workMode, qdID);
            jdID = qdID;
            jdID.sysID = 4;
            jdID.devID = 2;
            if (!res)
            {
                jdID.extenID = 1;

                qdID.sysID = 7;
                qdID.devID = 1;
                qdID.extenID = jdID.extenID;
            }
            configMacroData.setGZJDDeviceID(workMode, jdID);
            configMacroData.setGZQDDeviceID(workMode, qdID);
        }
    };
    auto size = gzckDeviceList.size() > 2 ? 2 : gzckDeviceList.size();
    // 删除多余的
    while (gzckDeviceList.size() > size)
    {
        gzckDeviceList.removeLast();
    }
    /* 这个是清除了数传的数据的 */
    configMacroData.removeRedundantGZQD(gzckDeviceList.toSet());
    for (auto& item : gzckDeviceList)
    {
        gzjdResetFunc(configMacroData, item);
        if (item == SKT && !existKt)
        {
            existKt = true;
        }
    }

    // 截止20211023 扩跳不能快速校相
    // 扩跳一定是单模式 咋处理
    /* 如果扩跳主跟 且同时存在高速和低速 则去掉低速 */
    if (gzMasterDeviceMap.contains(KaGS))
    {
        configMacroData.setGZQDDeviceID(KaGS, gzMasterDeviceMap[KaGS]);
    }
    if (gzMasterDeviceMap.contains(KaDS))
    {
        configMacroData.setGZQDDeviceID(KaDS, gzMasterDeviceMap[KaDS]);
    }

    // 冲突检查 当存在扩跳时 可以加一个数传 优先高速
    // 20211109扩跳只能有一个不可能出现扩跳加低速的基带组合
    // 高速1 2
    bool gsEnable = false;
    if (gzMasterDeviceMap.contains(KaGS) || (configMacroData.configMacroCmdModeMap.contains(KaGS) /*&& !existKt*/))
    {
        gsEnable = true;
        gzjdResetFunc(configMacroData, KaGS);

        DeviceID qdID;
        DeviceID jdID;
        configMacroData.getGZQDDeviceID(KaGS, qdID);
        configMacroData.getGZJDDeviceID(KaGS, jdID);
        if (!(jdID.extenID == 1 || jdID.extenID == 2))
        {
            qdID.extenID = 1;
            jdID.extenID = 1;
            configMacroData.setGZJDDeviceID(KaGS, jdID);
            configMacroData.setGZQDDeviceID(KaGS, qdID);
        }
    }
    // 低速3 4
    bool dsEnable = false;
    if (gzMasterDeviceMap.contains(KaDS) || (configMacroData.configMacroCmdModeMap.contains(KaDS) && !existKt))
    {
        dsEnable = true;
        gzjdResetFunc(configMacroData, KaDS);

        DeviceID qdID;
        DeviceID jdID;
        configMacroData.getGZQDDeviceID(KaDS, qdID);
        configMacroData.getGZJDDeviceID(KaDS, jdID);
        if (!(jdID.extenID == 3 || jdID.extenID == 4))
        {
            qdID.extenID = 3;
            jdID.extenID = 3;
            configMacroData.setGZJDDeviceID(KaDS, jdID);
            configMacroData.setGZQDDeviceID(KaDS, qdID);
        }
    }

    // 高速和低速同时存在需要处理
    // 当主跟有哪个就使用哪个 否则 优先高速
    if (gzMasterDeviceMap.contains(KaGS))
    {
        // 0未定义 1是使用高速 2使用低速
        configMacroData.setDTGSOrDS(KaGS, 1);
        configMacroData.setDTGSOrDS(KaDS, 0);
    }
    else if (gzMasterDeviceMap.contains(KaDS))
    {
        configMacroData.setDTGSOrDS(KaGS, 0);
        configMacroData.setDTGSOrDS(KaDS, 2);
    }
    else if (gsEnable)
    {
        // 0未定义 1是使用高速 2使用低速
        configMacroData.setDTGSOrDS(KaGS, 1);
        configMacroData.setDTGSOrDS(KaDS, 0);
    }
    else if (dsEnable)
    {
        configMacroData.setDTGSOrDS(KaGS, 0);
        configMacroData.setDTGSOrDS(KaDS, 2);
    }
    else
    {
        configMacroData.setDTGSOrDS(KaGS, 0);
        configMacroData.setDTGSOrDS(KaDS, 0);
    }

    // 高速和扩跳冲突 冲突扩跳变成 3 4
    if (gsEnable && existKt)
    {
        DeviceID qdID;
        DeviceID jdID;
        configMacroData.getGZQDDeviceID(SKT, qdID);
        configMacroData.getGZJDDeviceID(SKT, jdID);
        if (!(jdID.extenID == 3 || jdID.extenID == 4))
        {
            qdID.extenID = 3;
            jdID.extenID = 3;
            configMacroData.setGZQDDeviceID(SKT, qdID);
            configMacroData.setGZJDDeviceID(SKT, jdID);
        }
    }
    // 低速和扩跳冲突 冲突扩跳变成 1 2
    if (dsEnable && existKt)
    {
        DeviceID qdID;
        DeviceID jdID;
        configMacroData.getGZQDDeviceID(SKT, qdID);
        configMacroData.getGZJDDeviceID(SKT, jdID);
        if (!(jdID.extenID == 1 || jdID.extenID == 2))
        {
            qdID.extenID = 1;
            jdID.extenID = 1;
            configMacroData.setGZQDDeviceID(SKT, qdID);
            configMacroData.setGZJDDeviceID(SKT, jdID);
        }
    }
    if (size > 1)
    {
        // 剩下两种测控第一个使用 A B 第二个使用 C D
        auto workMode1 = gzckDeviceList.at(0);
        DeviceID qdID1;
        DeviceID jdID1;
        configMacroData.getGZQDDeviceID(workMode1, qdID1);
        configMacroData.getGZJDDeviceID(workMode1, jdID1);

        auto workMode2 = gzckDeviceList.at(1);
        DeviceID qdID2;
        DeviceID jdID2;
        configMacroData.getGZQDDeviceID(workMode2, qdID2);
        configMacroData.getGZJDDeviceID(workMode2, jdID2);

        if ((qdID1.extenID == 1 || qdID1.extenID == 2) &&  //
            (qdID2.extenID == 1 || qdID2.extenID == 2))
        {
            if (workMode1 == SKT)
            {
                qdID2.extenID = 3;
                jdID2.extenID = 3;
                configMacroData.setGZJDDeviceID(workMode2, jdID2);
                configMacroData.setGZQDDeviceID(workMode2, qdID2);
            }
            else
            {
                qdID1.extenID = 3;
                jdID1.extenID = 3;
                configMacroData.setGZJDDeviceID(workMode1, jdID1);
                configMacroData.setGZQDDeviceID(workMode1, qdID1);
            }
        }

        if ((qdID1.extenID == 3 || qdID1.extenID == 4) &&  //
            (qdID2.extenID == 3 || qdID2.extenID == 4))
        {
            if (workMode1 == SKT)
            {
                qdID2.extenID = 1;
                jdID2.extenID = 1;
                configMacroData.setGZJDDeviceID(workMode2, jdID2);
                configMacroData.setGZQDDeviceID(workMode2, qdID2);
            }
            else
            {
                qdID1.extenID = 1;
                jdID1.extenID = 1;
                configMacroData.setGZJDDeviceID(workMode1, jdID1);
                configMacroData.setGZQDDeviceID(workMode1, qdID1);
            }
        }
    }
    /* 测控和数传冲突
     * 当前跟踪是测控以测控为准改数传
     * 当前跟踪是数传以数传为准改测控
     */
    QMap<DeviceID, QSet<SystemWorkMode>> ckModeDeviceIDMap;
    QMap<DeviceID, QSet<SystemWorkMode>> scModeDeviceIDMap;
    DeviceID gzjdDeviceID;
    for (auto& link : mManualMsg.linkLineMap)
    {
        if (configMacroData.getGZJDDeviceID(link.workMode, gzjdDeviceID))
        {
            if (SystemWorkModeHelper::isMeasureContrlWorkMode(link.workMode) && gzjdDeviceID.isValid())
            {
                ckModeDeviceIDMap[gzjdDeviceID] << link.workMode;
            }
            else
            {
                scModeDeviceIDMap[gzjdDeviceID] << link.workMode;
            }
        }
    }
    /*
     * 当用备机在另一种模式里面找到相关的数据 那就一定冲突了
     */
    auto scckConflictFunc = [&](const QMap<DeviceID, QSet<SystemWorkMode>>& baseMap, const QMap<DeviceID, QSet<SystemWorkMode>>& diffMap) {
        for (auto iter = diffMap.begin(); iter != diffMap.end(); ++iter)
        {
            auto deviceID = iter.key();
            auto modeList = iter.value();

            /* 找跟踪的备机 */
            auto bak = BaseHandler::getGZJDSlaveDeviceID(deviceID);
            if (!baseMap.contains(bak))
            {
                continue;
            }

            for (auto& itemMode : modeList)
            {
                DeviceID qdID;
                configMacroData.getGZQDDeviceID(itemMode, qdID);

                qdID.extenID = bak.extenID;
                configMacroData.setGZJDDeviceID(itemMode, bak);
                configMacroData.setGZQDDeviceID(itemMode, qdID);
            }
        }
    };

    auto baseMode = BaseHandler::trackingToSystemWorkMode(m_rackingMode);
    if (SystemWorkModeHelper::isMeasureContrlWorkMode(baseMode))
    {
        scckConflictFunc(ckModeDeviceIDMap, scModeDeviceIDMap);
    }
    else
    {
        scckConflictFunc(scModeDeviceIDMap, ckModeDeviceIDMap);
    }

    // 跟踪前端主备
    for (auto iter = configMacroData.configMacroCmdModeMap.begin(); iter != configMacroData.configMacroCmdModeMap.end(); ++iter)
    {
        auto workMode = iter.key();
        DeviceID qdID;
        if (configMacroData.getGZQDDeviceID(workMode, qdID) && qdID.isValid())
        {
            qdID = BaseHandler::getGZJDSlaveDeviceID(qdID);
            configMacroData.setGZQDDeviceID(workMode, qdID, 2);
        }

        // 这个跟踪模式让每一个都有
        configMacroData.setTrackMode(workMode, m_rackingMode);
    }

    return true;
}
bool DeviceAlloter::allotGSDC(ConfigMacroData& configMacroData, QString& /*errorMsg*/)
{
    auto gsdcIsOne = [](int unitID) { return (unitID == 1 || unitID == 2); };
    auto gsdcIsTwo = [](int unitID) { return (unitID == 3 || unitID == 4); };
    auto gsdcSlave1 = [](int unitID) { return unitID == 1 ? 2 : (unitID == 2 ? 1 : (unitID == 3 ? 4 : 3)); };
    auto gsdcSlave2 = [](int unitID) { return unitID == 1 ? 3 : (unitID == 3 ? 1 : (unitID == 2 ? 4 : 2)); };
    auto gsgpxDeviceID = DeviceID(3, 1, 3);
    auto workMode = KaGS;
    DeviceID deviceID;

    int dcNo1 = 0;
    int dcNo2 = 0;
    int dcNo3 = 0;
    int dcNo4 = 0;
    configMacroData.getKaGSXBBQInfo(workMode, deviceID, dcNo1, 1);
    configMacroData.getKaGSXBBQInfo(workMode, deviceID, dcNo2, 2);
    if (!deviceID.isValid())
    {
        deviceID = gsgpxDeviceID;
    }

    if (dcNo1 != 0 && (dcNo1 < 1 || dcNo1 > 4))
    {
        dcNo1 = 1;
    }
    if (dcNo2 != 0 && (dcNo2 < 1 || dcNo2 > 4))
    {
        dcNo2 = 1;
    }

    /*
     * 当下变频编号为0 或者单元ID相同时会把所有的下变频器设置为一样的频率
     */
    if (dcNo1 == 0 || dcNo2 == 0 || dcNo1 == dcNo2)
    {
        dcNo1 = 1;
        dcNo2 = 2;
        dcNo3 = 3;
        dcNo4 = 4;
    }
    else if ((gsdcIsOne(dcNo1) && gsdcIsTwo(dcNo2)) ||  //
             (gsdcIsOne(dcNo2) && gsdcIsTwo(dcNo1)))
    {
        dcNo3 = gsdcSlave1(dcNo1);
        dcNo4 = gsdcSlave1(dcNo2);
    }
    else
    {
        dcNo3 = gsdcSlave2(dcNo1);
        dcNo4 = gsdcSlave2(dcNo2);
    }

    configMacroData.setKaGSXBBQInfo(workMode, deviceID, dcNo1, 1);
    configMacroData.setKaGSXBBQInfo(workMode, deviceID, dcNo2, 2);
    configMacroData.setKaGSXBBQInfo(workMode, deviceID, dcNo3, 3);
    configMacroData.setKaGSXBBQInfo(workMode, deviceID, dcNo4, 4);
    return true;
}
bool DeviceAlloter::allotCCZF(ConfigMacroData& configMacroData, QString& /*errorMsg*/)
{
    // 没有数传基带直接跳过
    int scModeNum = static_cast<int>(mManualMsg.linkLineMap.contains(KaGS));
    scModeNum += static_cast<int>(mManualMsg.linkLineMap.contains(KaDS));
    scModeNum += static_cast<int>(mManualMsg.linkLineMap.contains(XDS));
    if (scModeNum <= 0)
    {
        return true;
    }

    DeviceID masterDeviceID;
    DeviceID slaveDeviceID;
    auto workMode = XDS;
    if (mManualMsg.linkLineMap.contains(workMode))
    {
        auto& cczf = mResourceDataMap[workMode];
        if (!cczf.deviceMap.isEmpty())
        {
            masterDeviceID = cczf.getDeviceId(BBEType::CCZF_BBE | Main | First);
            slaveDeviceID = cczf.getDeviceId(BBEType::CCZF_BBE | Bak | First);
        }
    }

    workMode = KaDS;
    if (mManualMsg.linkLineMap.contains(workMode))
    {
        auto& cczf = mResourceDataMap[workMode];
        if (!cczf.deviceMap.isEmpty())
        {
            masterDeviceID = cczf.getDeviceId(BBEType::CCZF_BBE | Main | First);
            slaveDeviceID = cczf.getDeviceId(BBEType::CCZF_BBE | Bak | First);
        }
    }

    workMode = KaGS;
    if (mManualMsg.linkLineMap.contains(workMode))
    {
        auto& cczf = mResourceDataMap[workMode];
        if (!cczf.deviceMap.isEmpty())
        {
            masterDeviceID = cczf.getDeviceId(BBEType::CCZF_BBE | Main | First);
            slaveDeviceID = cczf.getDeviceId(BBEType::CCZF_BBE | Bak | First);
        }
    }

    if (!masterDeviceID.isValid())
    {
        masterDeviceID = DeviceID(8, 1, 1);
    }
    if (!slaveDeviceID.isValid())
    {
        slaveDeviceID = DeviceID(8, 1, 2);
    }
    if (masterDeviceID == slaveDeviceID)
    {
        slaveDeviceID.extenID = masterDeviceID.extenID == 1 ? 2 : 1;
    }

    /* 写入 */
    workMode = XDS;
    if (mManualMsg.linkLineMap.contains(workMode))
    {
        configMacroData.setCCZFDeviceID(workMode, masterDeviceID, 1);
        configMacroData.setCCZFDeviceID(workMode, slaveDeviceID, 2);
    }

    workMode = KaDS;
    if (mManualMsg.linkLineMap.contains(workMode))
    {
        configMacroData.setCCZFDeviceID(workMode, masterDeviceID, 1);
        configMacroData.setCCZFDeviceID(workMode, slaveDeviceID, 2);
    }

    workMode = KaGS;
    if (mManualMsg.linkLineMap.contains(workMode))
    {
        configMacroData.setCCZFDeviceID(workMode, masterDeviceID, 1);
        configMacroData.setCCZFDeviceID(workMode, slaveDeviceID, 2);
    }

    return true;
}
