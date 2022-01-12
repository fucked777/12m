#ifndef SATELLITEMESSAGE_H
#define SATELLITEMESSAGE_H

#include "JsonHelper.h"
#include "SystemWorkMode.h"
#include "TaskPlanMessageDefine.h"
#include <QDebug>
#include <QMap>
#include <QVariant>
/* 轨道类型 */
enum class SatelliteRailType
{
    Unknown = 0,
    HighOrbit, /* 高轨卫星 */
    LowOrbit,  /* 低轨卫星 */
};

//每个点频下的参数
struct SatelliteManagementParamItem
{
    QMap<QString, QVariant> m_itemMap;  //每种模式对应的卫星参数
};

struct SatelliteManagementDpParamItem
{
    int defalutNum = 0;  //默认点频的配置
    QMap<int, SatelliteManagementParamItem> m_dpInfoMap;
};
struct SatelliteManagementData
{
    QString m_satelliteName;                                                      // 卫星名称
    QString m_satelliteCode;                                                      // 卫星代号  唯一标志 作为数据库主键
    QString m_satelliteIdentification;                                            // 卫星标志 16进制的数据
    QString m_stationID;                                                          // 站ID信息
    QString m_taskCenterID;                                                       // 任务中心
    QString m_digitalCenterID;                                                    // 数传中心
    SatelliteRailType m_railType{ SatelliteRailType::Unknown };                   // 轨道类型
    SatelliteTrackingMode m_trackingMode{ SatelliteTrackingMode::TrackUnknown };  // 卫星跟踪方式
    bool m_sPhasing{ true };                                                      // S频段校相
    bool m_kaPhasing{ true };                                                     // Ka 频段校相
    QString m_desc;                                                               // 描述
    QString m_workMode;  //当前卫星的工作模式  默认采用1+2+3+4形式代替，具体的值由配置决定

    QMap<SystemWorkMode, SatelliteManagementDpParamItem> m_workModeParamMap;
    CarrierType m_isCarrierType{ CarrierType::Unknown };  //用于共载波，不共载波区分

    inline QString railTypeString()
    {
        switch (m_railType)
        {
        case SatelliteRailType::Unknown: return "未知类型"; break;

        case SatelliteRailType::HighOrbit: return "高轨卫星"; break;

        case SatelliteRailType::LowOrbit: return "低轨卫星"; break;
        default: return "未知类型"; break;
        }
    }

    inline QString trackingModeString(const QString& currentProject = "4424") const
    {
        if (currentProject == "4424")
        {
            switch (m_trackingMode)
            {
            case SatelliteTrackingMode::TrackUnknown: return "未知类型"; break;
            case SatelliteTrackingMode::_4424_S: return "S"; break;
            case SatelliteTrackingMode::_4424_KaCk: return "Ka遥测"; break;
            case SatelliteTrackingMode::_4424_KaDSDT: return "Ka低速数传"; break;
            case SatelliteTrackingMode::_4424_KaGSDT: return "X/Ka高速"; break;
            case SatelliteTrackingMode::_4424_S_KaCk: return "S+Ka遥测"; break;
            case SatelliteTrackingMode::_4424_S_KaDSDT: return "S+Ka低速数传"; break;
            case SatelliteTrackingMode::_4424_S_KaGSDT: return "S+X/Ka高速"; break;
            case SatelliteTrackingMode::_4424_S_KaCk_KaDSDT: return "S+Ka遥测+Ka低速"; break;
            case SatelliteTrackingMode::_4424_S_KaCK_KaGSDT: return "S+Ka遥测+X/Ka高速"; break;
            default: return "未知类型"; break;
            }
        }
        else
        {
            switch (m_trackingMode)
            {
            case SatelliteTrackingMode::TrackUnknown: return "未知类型"; break;
            case SatelliteTrackingMode::_4426_STTC: return "STTC"; break;
            case SatelliteTrackingMode::_4426_SK2: return "S扩二"; break;
            case SatelliteTrackingMode::_4426_SKT: return "S扩跳"; break;
            case SatelliteTrackingMode::_4426_KaCk: return "Ka遥测"; break;
            case SatelliteTrackingMode::_4426_KaDSDT: return "Ka低速数传"; break;
            case SatelliteTrackingMode::_4426_KaGSDT: return "Ka高速数传"; break;

            case SatelliteTrackingMode::_4426_STTC_KaCk: return "STTC+Ka遥测"; break;
            case SatelliteTrackingMode::_4426_STTC_KaDSDT: return "STTC+Ka低速数传"; break;
            case SatelliteTrackingMode::_4426_STTC_KaGSDT: return "STTC+Ka高速数传"; break;
            case SatelliteTrackingMode::_4426_STTC_KaCk_KaDSDT: return "STTC+Ka遥测+Ka低速数传"; break;
            case SatelliteTrackingMode::_4426_STTC_KaCK_KaGSDT: return "STTC+Ka遥测+Ka高速数传"; break;

            case SatelliteTrackingMode::_4426_SK2_KaCk: return "S扩二+Ka遥测"; break;
            case SatelliteTrackingMode::_4426_SK2_KaDSDT: return "S扩二+Ka低速数传"; break;
            case SatelliteTrackingMode::_4426_SK2_KaGSDT: return "S扩二+Ka高速数传"; break;
            case SatelliteTrackingMode::_4426_SK2_KaCk_KaDSDT: return "S扩二+Ka遥测+Ka低速数传"; break;
            case SatelliteTrackingMode::_4426_SK2_KaCK_KaGSDT:
                return "S扩二+Ka遥测+Ka高速数传";
                break;

                // 截止20211023 扩跳不能快速校相
                // case SatelliteTrackingMode::_4426_SKT_KaCk: return "S扩跳+Ka遥测"; break;
                // case SatelliteTrackingMode::_4426_SKT_KaDSDT: return "S扩跳+Ka低速数传"; break;
                // case SatelliteTrackingMode::_4426_SKT_KaGSDT: return "S扩跳+Ka高速数传"; break;
                // case SatelliteTrackingMode::_4426_SKT_KaCk_KaDSDT: return "S扩跳+Ka遥测+Ka低速数传"; break;
                // case SatelliteTrackingMode::_4426_SKT_KaCK_KaGSDT: return "S扩跳+Ka遥测+Ka高速数传"; break;
            default: return "未知类型"; break;
            }
        }
    }
    inline QStringList workMode() const { return m_workMode.split("+"); }

    inline QStringList getWorkModeDesc() const
    {
        QStringList list;
        for (auto key : m_workModeParamMap.keys())
        {
            list << SystemWorkModeHelper::systemWorkModeToDesc(key);
        }

        return list;
    }

    inline QMap<int, QString> getworkMode() const
    {
        QMap<int, QString> out;
        auto list = m_workMode.split("+");
        for (auto& item : list)
        {
            out[item.toUInt()] = m_satelliteCode;
        }
        return out;
    }

    // 获取指定工作模式的点频数量
    inline int getDpNumByWorkMode(SystemWorkMode workMode) const
    {
        if (m_workModeParamMap.contains(workMode))
        {
            return m_workModeParamMap[workMode].m_dpInfoMap.size();
        }
        return 0;
    }

    // 获取指定工作模式的默认点频
    inline int getDefaultDpNumByWorkMode(SystemWorkMode workMode) const
    {
        if (m_workModeParamMap.contains(workMode))
        {
            return m_workModeParamMap[workMode].defalutNum;
        }
        return 0;
    }

    // 获取每个工作模式下的点频
    inline QMap<SystemWorkMode, int> getDpNumMap() const
    {
        QMap<SystemWorkMode, int> map;
        for (auto workModeItem : m_workModeParamMap.keys())
        {
            map[workModeItem] = m_workModeParamMap[workModeItem].m_dpInfoMap.size();
        }
        return map;
    }

    // 获取卫星发射极化
    inline bool getLaunchPolar(SystemWorkMode workMode, int dpNum, SystemOrientation& orientation)
    {
        auto value = getValue(workMode, dpNum, QString("SEmissPolar"));
        if (!value.isValid())
        {
            return false;
        }

        orientation = SystemOrientation(value.toInt());
        return true;
    }

    // 获取卫星接收极化
    inline bool getSReceivPolar(SystemWorkMode workMode, int dpNum, SystemOrientation& orientation)
    {
        auto value = getValue(workMode, dpNum, QString("SReceivPolar"));
        if (!value.isValid())
        {
            orientation = SystemOrientation::Unkonwn;
            return false;
        }

        orientation = SystemOrientation(value.toInt());
        return true;
    }

    // 获取卫星跟踪极化
    inline bool getSTrackPolar(SystemWorkMode workMode, int dpNum, SystemOrientation& orientation)
    {
        auto value = getValue(workMode, dpNum, QString("STrackPolar"));
        if (!value.isValid())
        {
            return false;
        }

        orientation = SystemOrientation(value.toInt());
        return true;
    }

    // 获取选择的动态参数组号
    inline bool getSelectedGroupNo(SystemWorkMode workMode, int dpNum, int& groupNo, QString type = "")
    {
        if (this->m_workModeParamMap.contains(workMode) && this->m_workModeParamMap.value(workMode).m_dpInfoMap.contains(dpNum))
        {
            for (auto item : m_workModeParamMap[workMode].m_dpInfoMap[dpNum].m_itemMap.keys())
            {
                if (item.endsWith(type))
                {
                    groupNo = (m_workModeParamMap[workMode].m_dpInfoMap[dpNum].m_itemMap[item].toInt());
                    return true;
                }
            }
        }
        return false;
    }

    // 获取卫星中配置的值
    inline QVariant getValue(SystemWorkMode workMode, int dpNum, const QString& type)
    {
        if (this->m_workModeParamMap.contains(workMode) && this->m_workModeParamMap.value(workMode).m_dpInfoMap.contains(dpNum))
        {
            for (auto item : m_workModeParamMap[workMode].m_dpInfoMap[dpNum].m_itemMap.keys())
            {
                if (item.endsWith(type))
                {
                    return m_workModeParamMap[workMode].m_dpInfoMap[dpNum].m_itemMap[item];
                }
            }
        }
        return QVariant();
    }
};

/*******************************************************************************************/
//获取所有卫星的接口
/*******************************************************************************************/

struct AllSatelliteMessageRequest
{
};
template<typename Archiver>
Archiver& operator&(Archiver& ar, AllSatelliteMessageRequest&)
{
    return ar;
}

struct AllSatelliteMessageResponse
{
    int m_returnCode;
    QList<SatelliteManagementData> m_data;
};

template<typename Archiver>
Archiver& operator&(Archiver& ar, AllSatelliteMessageResponse& s)
{
    ar.StartObject();
    ar.Member("ReturnCode") & s.m_returnCode;
    ar.Member("Data") & s.m_data;
    return ar.EndObject();
}

Q_DECLARE_METATYPE(AllSatelliteMessageResponse)
Q_DECLARE_METATYPE(SatelliteManagementData)

#endif  // SATELLITEMESSAGE_H
