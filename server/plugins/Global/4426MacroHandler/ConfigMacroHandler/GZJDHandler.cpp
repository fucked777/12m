#include "GZJDHandler.h"

GZJDHandler::GZJDHandler(QObject* parent)
    : BaseLinkHandler(parent)
{
}

LinkType GZJDHandler::getLinkType() { return LinkType::Unknown; }

bool GZJDHandler::handle()
{
    // 跟踪基带要单独配置 因为跟踪基带是集合所有模式的
    /* 1.获取跟踪基带 */
    QMap<DeviceID, QSet<SystemWorkMode>> gzjdModeMap;
    for (const auto& link : mManualMsg.linkLineMap)
    {
        DeviceID gzjdDeviceID;
        if (mManualMsg.configMacroData.getGZJDDeviceID(link.workMode, gzjdDeviceID))
        {
            if (gzjdDeviceID.isValid())
            {
                gzjdModeMap[gzjdDeviceID] << link.workMode;
            }
        }
    }
    for (auto iter = gzjdModeMap.begin(); iter != gzjdModeMap.end(); ++iter)
    {
        auto masterDeviceID = iter.key();
        auto modeSet = iter.value();
        ExitCheck(false);
        if (modeSet.isEmpty())
        {
            continue;
        }
        // 需要转换的目标模式
        QStringList desModeList;
        for (auto& item : modeSet)
        {
            desModeList << SystemWorkModeHelper::systemWorkModeToDesc(item);
        }
        auto gzjdModeID = convertGZJDMode(modeSet);
        if (gzjdModeID == TBBEModeID::Unknown)
        {
            auto msg = QString("获取%1的目标模式失败 目标模式：%2").arg(GlobalData::getExtensionName(masterDeviceID), desModeList.join("+"));
            emit signalErrorMsg(msg);
            continue;
        }
        // 获取备机
        auto slaveDeviceID = getGZJDSlaveDeviceID(masterDeviceID);

        // 切换跟踪基带的工作模式
        auto status = switchGZJDMasterAndSlaveWorkMode(masterDeviceID, slaveDeviceID, static_cast<int>(gzjdModeID));
        if (CKJDSwitchStatus::Master == status)
        {
            auto msg = QString("备机：%1 切换模式 %2 失败，预设工作模式与设备当前模式不符")
                           .arg(GlobalData::getExtensionName(slaveDeviceID), desModeList.join("+"));
            emit signalErrorMsg(msg);
        }
        if (CKJDSwitchStatus::Slave == status)
        {
            auto msg = QString("主机：%1 切换模式 %2 失败，预设工作模式与设备当前模式不符")
                           .arg(GlobalData::getExtensionName(masterDeviceID), desModeList.join("+"));
            emit signalErrorMsg(msg);
        }
        if (CKJDSwitchStatus::Failed == status)
        {
            auto msg = QString("跟踪基带主机 %1 备机 %2 切换模式%3失败，预设工作模式与设备当前模式不符")
                           .arg(GlobalData::getExtensionName(masterDeviceID), GlobalData::getExtensionName(slaveDeviceID), desModeList.join("+"));
            emit signalErrorMsg(msg);
            continue;
        }

        // 设置跟踪基带主/备机
        switchJDMasterSlave(masterDeviceID, MasterSlave::Master);
        switchJDMasterSlave(slaveDeviceID, MasterSlave::Slave);

        // 切换跟踪基带的工作方式为任务方式
        clearQueue();

        mSingleCommandHelper.packSingleCommand(QString("StepGZJD_RWFS"), mPackCommand, masterDeviceID, static_cast<int>(gzjdModeID));
        appendExecQueue(mPackCommand);
        mSingleCommandHelper.packSingleCommand(QString("StepGZJD_RWFS"), mPackCommand, slaveDeviceID, static_cast<int>(gzjdModeID));
        appendExecQueue(mPackCommand);

        execQueue();
    }

    return true;
}
