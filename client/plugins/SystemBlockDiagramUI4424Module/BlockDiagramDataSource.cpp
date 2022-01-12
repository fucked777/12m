#include "BlockDiagramDataSource.h"
#include "BlockDiagramDefine.h"
#include "CustomPacketMessageDefine.h"
#include "GlobalData.h"
#include <QtConcurrent>

class BlockDiagramDataSourceImpl
{
public:
    int timerID{ -1 };
    /* 默认的颜色显示 */
    quint32 rgbOffLine{ 0 };    /* 离线 */
    quint32 rgbSubControl{ 0 }; /* 分控 */
    quint32 rgbFault{ 0 };      /* 错误 */
    quint32 rgbNormal{ 0 };     /* 正常 */
    quint32 rgbCurLink{ 0 };    /* 当前链路 */
    quint32 rgbNotCurLink{ 0 }; /* 不是当前链路 */
    quint32 rgbShutdown{ 0 };   /* 关电 */
    quint32 rgbSystem{ 0 };     /* 分系统 */
    quint32 rgbNotFound{ 0 };   /* 未查询到状态 */

    QMap<QString, TwoValueSwitchTypeInfo> switchTypeMap;
    QMap<QString, DeviceStatusTypeInfo> deviceStatusTypeMap;

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
    connect(this, &BlockDiagramDataSource::sg_start, this, &BlockDiagramDataSource::start);
    connect(this, &BlockDiagramDataSource::sg_stop, this, &BlockDiagramDataSource::stop);
    connect(this, &BlockDiagramDataSource::sg_refresh, this, &BlockDiagramDataSource::slotRefresh);
}
BlockDiagramDataSource::~BlockDiagramDataSource() { delete m_impl; }
void BlockDiagramDataSource::start(const SystemBlockInfo& info)
{
    m_impl->rgbOffLine = info.rgbOffLine;       /* 离线 */
    m_impl->rgbSubControl = info.rgbSubControl; /* 分控 */
    m_impl->rgbFault = info.rgbFault;           /* 错误 */
    m_impl->rgbNormal = info.rgbNormal;         /* 正常 */
    m_impl->rgbCurLink = info.rgbCurLink;       /* 当前链路 */
    m_impl->rgbNotCurLink = info.rgbNotCurLink; /* 不是当前链路 */
    m_impl->rgbShutdown = info.rgbShutdown;     /* 关电 */
    m_impl->rgbSystem = info.rgbSystem;         /* 分系统 */
    m_impl->rgbNotFound = info.rgbNotFound;     /* 未查询到状态 */
    m_impl->switchTypeMap = info.switchTypeMap;
    m_impl->deviceStatusTypeMap = info.deviceStatusTypeMap;

    if (m_impl->timerID == -1)
    {
        m_impl->timerID = startTimer(1000);
    }
}
void BlockDiagramDataSource::stop()
{
    if (m_impl->timerID != -1)
    {
        killTimer(m_impl->timerID);
        m_impl->timerID = -1;
    }
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

void BlockDiagramDataSource::timerEvent(QTimerEvent* /*event*/)
{
    QtConcurrent::run([=]() {
        QMutexLocker locker(&m_refreshMutex);
        currentDataMap.clear();
        for (auto& item : m_impl->deviceStatusTypeMap)
        {
            if (!currentDataMap.contains(item.redisKey))
            {
                currentDataMap[item.redisKey] = GlobalData::getExtenStatusReportData(item.redisKey);
            }
        }
        emit sg_refresh();
    });
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
    for (auto& item : m_impl->deviceStatusTypeMap)
    {
        /* 1. 先根据redisID获取设备信息检查主备 */
        int devMainInfo = BlockDiagramDataSourceImpl::devMainPreparationCheck(item, reportMsg);

        /* 2. 判断设备是否离线 离线的话开关会默认选择一个,然后设备状态的话会变成默认颜色 */
        if (currentDataMap.contains(item.redisKey))
        {
            reportMsg = currentDataMap[item.redisKey];
        }
        /* 离线 */
        if (reportMsg.unitReportMsgMap.isEmpty())
        {
            emit sg_sendParamData(item.graphicsID, m_impl->rgbOffLine, DeviceStatusType);
            //只有具备主备信息的设备离线时,才会清理边框的状态
            if (item.mainPreparationList.size())
            {
                emit sg_sendParamData(item.graphicsID, m_impl->rgbNotCurLink, BorderColorType);
            }
            continue;
        }

        //主备信息边框显示
        switch (devMainInfo)
        {
        case 1:
        {
            emit sg_sendParamData(item.graphicsID, m_impl->rgbCurLink, BorderColorType);
            break;
        }
        default:
        {
            emit sg_sendParamData(item.graphicsID, m_impl->rgbNotCurLink, BorderColorType);
            break;
        }
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
                    emit sg_sendParamData(item.graphicsID, m_impl->rgbShutdown, DeviceStatusType);
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
            emit sg_sendParamData(item.graphicsID, m_impl->rgbNormal, DeviceStatusType);
            continue;
        }

        /* 4. 获取设备的综合状态 */
        auto& comprehensiveStateInfo = item.comprehensiveStateInfo;
        if (!BlockDiagramDataSourceImpl::findCmd(reportMsg, comprehensiveStateInfo.unitID, comprehensiveStateInfo.indexKey, value))
        {
            /* 获取状态失败 */
            emit sg_sendParamData(item.graphicsID, m_impl->rgbNotFound, DeviceStatusType);
            continue;
        }
        /* 获取状态成功,不是正常认为错误 */
        if (value.toInt() != comprehensiveStateInfo.normal)
        {
            emit sg_sendParamData(item.graphicsID, m_impl->rgbFault, DeviceStatusType);
            continue;
        }

        /* 5.判断本分控--分控则显示分控的颜色 */

        /* 本分控获取失败 */
        if (!BlockDiagramDataSourceImpl::findCmd(reportMsg, controlInfo.unitID, controlInfo.indexKey, value))
        {
            /* 获取状态失败 */
            emit sg_sendParamData(item.graphicsID, m_impl->rgbNotFound, DeviceStatusType);
            continue;
        }
        /* 本分控显示 */
        emit sg_sendParamData(item.graphicsID, value.toInt() == controlInfo.subControl ? m_impl->rgbSubControl : m_impl->rgbNormal, DeviceStatusType);
    }

    /* 二值开关状态 */
    for (auto& item : m_impl->switchTypeMap)
    {
        /* 1. 先根据redisID获取设备信息检查主备 开关无此状态 */
        /* 2. 判断设备是否离线 离线的话开关会默认选择一个,然后设备状态的话会变成默认颜色 */
        if (currentDataMap.contains(item.redisKey))
        {
            reportMsg = currentDataMap[item.redisKey];
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
