#include "BlockDiagramDataSource.h"
#include "BlockDiagramDefine.h"
#include "CustomPacketMessageDefine.h"
#include "GlobalData.h"
#include <QDebug>

class BlockDiagramDataSourceImpl
{
public:
    QMutex* refreshMutex = nullptr;
    DeviceReportMessageMap* currentDataMap = nullptr;
    SystemBlockInfo* blockInfo = nullptr;

    static bool findCmd(const ExtensionStatusReportMessage& extenReportMap, int unit, const QString& key, QVariant& value);
    /**
     * @brief devMainPreparationCheck
     * @param info
     * @param reportMsg
     * @return
     * -1表示无主备信息
     * 1表示主机
     * 0表示备机
     */
    static int devMainPreparationCheck(DeviceStatusTypeInfo& info, ExtensionStatusReportMessage& reportMsg);
};
bool BlockDiagramDataSourceImpl::findCmd(const ExtensionStatusReportMessage& extenReportMap, int unit, const QString& key, QVariant& value)
{
    if (extenReportMap.unitReportMsgMap.isEmpty())
    {
        return false;
    }
    auto find = extenReportMap.unitReportMsgMap.find(unit);
    if (find == extenReportMap.unitReportMsgMap.end())
    {
        return false;
    }
    auto& reportMap = find->paramMap;
    auto valueIter = reportMap.find(key);
    if (valueIter == reportMap.end())
    {
        return false;
    }
    value = valueIter.value();
    return true;
}
/* 主备检查 */
int BlockDiagramDataSourceImpl::devMainPreparationCheck(DeviceStatusTypeInfo& info, ExtensionStatusReportMessage& reportMsg)
{
    /* 主备信息不存在,直接忽略 */
    auto& mainPreparationInfo = info.mainPreparationList;
    if (mainPreparationInfo.isEmpty())
    {
        return -1;
    }

    QVariant value;
    for (auto& item : mainPreparationInfo)
    {
        /* 1. 先根据redisID获取设备信息检查主备 */
        reportMsg = GlobalData::getExtenStatusReportData(item.redisKey);

        if (!BlockDiagramDataSourceImpl::findCmd(reportMsg, item.unitID, item.indexKey, value))
        {
            continue;
        }

        /* 相等认为他就是主机 */
        if (value.toInt() == item.mainValue)
        {
            info.redisKey = item.mainRedisKey;
            return 1;
        }
    }
    return 0;
}

/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/

BlockDiagramDataSource::BlockDiagramDataSource()
    : m_impl(new BlockDiagramDataSourceImpl)
{
    qRegisterMetaType<SystemBlockInfo>("SystemBlockInfo");
    qRegisterMetaType<SystemBlockInfo>("const SystemBlockInfo&");
    qRegisterMetaType<SystemBlockInfo>("SystemBlockInfo&");
}
BlockDiagramDataSource::~BlockDiagramDataSource() { delete m_impl; }
void BlockDiagramDataSource::setSystemBlockInfo(SystemBlockInfo* blockInfo, DeviceReportMessageMap* currentDataMap, QMutex* refreshMutex)
{
    m_impl->refreshMutex = refreshMutex;
    m_impl->currentDataMap = currentDataMap;
    m_impl->blockInfo = blockInfo;
}

/* 主备检查 */
void devMainPreparationCheck(DeviceStatusTypeInfo& info, ExtensionStatusReportMessage& reportMsg)
{
    /* 主备信息不存在,直接忽略 */
    auto& mainPreparationInfo = info.mainPreparationList;
    if (mainPreparationInfo.isEmpty())
    {
        return;
    }

    QVariant value;
    for (auto& item : mainPreparationInfo)
    {
        /* 1. 先根据redisID获取设备信息检查主备 */
        reportMsg = GlobalData::getExtenStatusReportData(item.redisKey);

        if (!BlockDiagramDataSourceImpl::findCmd(reportMsg, item.unitID, item.indexKey, value))
        {
            continue;
        }

        /* 相等认为他就是主机 */
        if (value.toInt() == item.mainValue)
        {
            info.redisKey = item.mainRedisKey;
            return;
        }
    }
}

void BlockDiagramDataSource::slotRefreshData()
{
    QMutexLocker locker(m_impl->refreshMutex);
    slotRefresh();
}

void BlockDiagramDataSource::slotRefresh()
{
    /* 设备状态 */
    /*
     * 1. 先根据redisID获取设备信息检查主备 如果不存在主备信息就直接忽略 开关无此状态
     * 2. 判断设备是否离线 离线的话开关会默认选择一个,然后设备状态的话会变成默认颜色
     * 3. 获取设备的开关电状态 如果没有则直接下一步 开关没有这个
     * 4. 获取设备的综合状态--开关则获取当前的开关状态
     * 5. 判断本分控--分控则显示分控的颜色
     */
    ExtensionStatusReportMessage reportMsg;
    QVariant value;
    /* 设备状态 */
    for (auto& item : m_impl->blockInfo->deviceStatusTypeMap)
    {
        /* 1. 先根据redisID获取设备信息检查主备 */
        int devMainInfo = BlockDiagramDataSourceImpl::devMainPreparationCheck(item, reportMsg);

        /* 2. 判断设备是否离线 离线的话开关会默认选择一个,然后设备状态的话会变成默认颜色 */
        //        reportMsg = GlobalData::getExtenStatusReportData(item.redisKey);
        if (m_impl->currentDataMap->contains(item.redisKey))
        {
            reportMsg = (*(m_impl->currentDataMap))[item.redisKey];
        }

        /* 离线 */
        if (reportMsg.unitReportMsgMap.isEmpty())
        {
            //需要的时候删掉,测控基带自身的问题导致这样写
            if (item.systemID == "4001")
            {
                item.systemID = "4002";
                item.redisKey = "4002";
            }
            else if (item.systemID == "4002")
            {
                item.systemID = "4001";
                item.redisKey = "4001";
            }
            else if (item.systemID == "4003")
            {
                item.systemID = "4004";
                item.redisKey = "4004";
            }
            else if (item.systemID == "4004")
            {
                item.systemID = "4003";
                item.redisKey = "4003";
            }

            emit sg_sendParamData(item.graphicsID, m_impl->blockInfo->rgbOffLine, DeviceStatusType);
            //只有具备主备信息的设备离线时,才会清理边框的状态
            if (item.mainPreparationList.size() == 1)
            {
                if(item.systemID == "4001")
                {
                    item.mainPreparationList[0].redisKey = "4001";
                    item.mainPreparationList[0].mainRedisKey = "4001";
                }
                else if(item.systemID == "4002")
                {
                    item.mainPreparationList[0].redisKey = "4002";
                    item.mainPreparationList[0].mainRedisKey = "4002";
                }
                else if (item.systemID == "4003")
                {
                    item.mainPreparationList[0].redisKey = "4003";
                    item.mainPreparationList[0].mainRedisKey = "4003";
                }
                else if (item.systemID == "4004")
                {
                    item.mainPreparationList[0].redisKey = "4004";
                    item.mainPreparationList[0].mainRedisKey = "4004";
                }
                emit sg_sendParamData(item.graphicsID, m_impl->blockInfo->rgbDefault, BorderColorType);
            }
            continue;
        }

        /* 3. 获取设备的开关电状态 如果没有则直接下一步 开关没有这个 */
        auto& shutdownInfo = item.shutdownInfo;
        if (!shutdownInfo.indexKey.isEmpty())
        {
            if (BlockDiagramDataSourceImpl::findCmd(reportMsg, shutdownInfo.unitID, shutdownInfo.indexKey, value))
            {
                /* 相等认为关电了 */
                if (qFuzzyCompare(1.0 + value.toDouble(), 1.0 + shutdownInfo.shutdownValue))
                {
                    emit sg_sendParamData(item.graphicsID, m_impl->blockInfo->rgbShutdown, DeviceStatusType);
                    continue;
                }
                /* 不相等直接忽略 */
            }
            /* 没有获取到直接忽略 */
        }

        /*如果配置文件中没有配置本分控信息，则判断完设备是否去电，就直接得出结论。针对链路中的LNA做的特殊处理*/
        auto& controlInfo = item.controlInfo;
        if (controlInfo.indexKey.isEmpty())
        {
            emit sg_sendParamData(item.graphicsID, m_impl->blockInfo->rgbNormal, DeviceStatusType);
            continue;
        }

        /* 4. 获取设备的综合状态 */
        auto& comprehensiveStateInfo = item.comprehensiveStateInfo;
        if (!BlockDiagramDataSourceImpl::findCmd(reportMsg, comprehensiveStateInfo.unitID, comprehensiveStateInfo.indexKey, value))
        {
            /* 获取状态失败 */
            emit sg_sendParamData(item.graphicsID, m_impl->blockInfo->rgbNotFound, DeviceStatusType);
            continue;
        }
        /* 获取状态成功,不是正常认为错误 */
        if (value.toInt() != comprehensiveStateInfo.normal)
        {
            emit sg_sendParamData(item.graphicsID, m_impl->blockInfo->rgbFault, DeviceStatusType);
            continue;
        }

        /* 5. 获取设备的主备信息状态 */
        switch (devMainInfo)
        {
        case 1:
        case -1:  //没有主备信息的设备显示为绿色
        {
            emit sg_sendParamData(item.graphicsID, m_impl->blockInfo->rgbCurLink, DeviceStatusType);
            break;
        }
        default:
        {
            emit sg_sendParamData(item.graphicsID, m_impl->blockInfo->rgbNotCurLink, DeviceStatusType);
            break;
        }
        }

        /* 6.判断本分控--分控则显示分控的颜色 */

        /* 本分控获取失败 */
        if (!BlockDiagramDataSourceImpl::findCmd(reportMsg, controlInfo.unitID, controlInfo.indexKey, value))
        {
            /* 获取状态失败 */
            emit sg_sendParamData(item.graphicsID, m_impl->blockInfo->rgbNotFound, DeviceStatusType);
            continue;
        }
        /* 7.分控状态和正常状态 本分控显示 */
        emit sg_sendParamData(item.graphicsID,
                              value.toInt() == controlInfo.subControl ? m_impl->blockInfo->rgbSubControl : m_impl->blockInfo->rgbDefault,
                              BorderColorType);
    }

    /* 二值开关状态 */
    for (auto& item : m_impl->blockInfo->switchTypeMap)
    {
        /* 1. 先根据redisID获取设备信息检查主备 开关无此状态 */
        /* 2. 判断设备是否离线 离线的话开关会默认选择一个,然后设备状态的话会变成默认颜色 */
        //        reportMsg = GlobalData::getExtenStatusReportData(item.redisKey);
        if (m_impl->currentDataMap->contains(item.redisKey))
        {
            reportMsg = (*(m_impl->currentDataMap))[item.redisKey];
        }
        /* 离线 */
        if (reportMsg.unitReportMsgMap.isEmpty())
        {
            emit sg_sendParamData(item.graphicsID, std::get<1>(item.twoValueRelational[0]), TwoValueSwitchType);
            continue;
        }
        /* 3. 获取设备的开关电状态 如果没有则直接下一步 开关没有这个 */
        /* 4. 获取设备的综合状态--开关则获取当前的开关状态 */
        if (!BlockDiagramDataSourceImpl::findCmd(reportMsg, item.unitID, item.statusKey, value))
        {
            /* 获取状态失败 */
            emit sg_sendParamData(item.graphicsID, std::get<1>(item.twoValueRelational[0]), TwoValueSwitchType);
            continue;
        }
        for (int i = 0; i < 2; ++i)
        {
            if (std::get<0>(item.twoValueRelational[i]) == value.toInt())
            {
                emit sg_sendParamData(item.graphicsID, std::get<1>(item.twoValueRelational[i]), TwoValueSwitchType);
                break;
            }
        }
    }
}
