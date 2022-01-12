#ifndef DIGITALCENTERMESSAGE_H
#define DIGITALCENTERMESSAGE_H

#include "JsonHelper.h"
#include <QMap>
#include <QVariant>
/*******************************************************************************************/
//添加数传中心的接口
/*******************************************************************************************/
struct AddDigitalCenterMessageRequest
{
    QString m_satellitID;               //卫星代号,任务代号
    QString m_satelliteIdentification;  //卫星标识
    QString m_satelliteName;            //卫星名称
    bool m_dateofEpoch;                 //瞬跟历元日期
    QString m_timeofEpoch;              //瞬跟历元时刻
    double m_semiMajorAxis;             //半长轴
    double m_eccentricityRatio;         //偏心率
    double m_dipAngle;                  //倾角（度）
    double m_rightAscension;            //升交点赤经（度）
    double m_argumentOfPerigee;         //近地点幅角（度）
    double m_meanAnomaly;               //平近点角（度）
};

template<typename Archiver>
Archiver& operator&(Archiver& ar, AddDigitalCenterMessageRequest& s)
{
    ar.StartObject();
    ar.Member("SatellitID") & s.m_satellitID;
    ar.Member("SatelliteIdentification") & s.m_satelliteIdentification;
    ar.Member("SatelliteName") & s.m_satelliteName;
    ar.Member("DateofEpoch") & s.m_dateofEpoch;
    ar.Member("TimeofEpoch") & s.m_timeofEpoch;
    ar.Member("SemiMajorAxis") & s.m_semiMajorAxis;
    ar.Member("EccentricityRatio") & s.m_eccentricityRatio;
    ar.Member("DipAngle") & s.m_dipAngle;
    ar.Member("RightAscension") & s.m_rightAscension;
    ar.Member("ArgumentOfPerigee") & s.m_argumentOfPerigee;
    ar.Member("MeanAnomaly") & s.m_meanAnomaly;

    return ar.EndObject();
}

struct AddDigitalCenterMessageResponse
{
    int m_returnCode;
    AddDigitalCenterMessageRequest m_data;
};

template<typename Archiver>
Archiver& operator&(Archiver& ar, AddDigitalCenterMessageResponse& s)
{
    ar.StartObject();
    ar.Member("ReturnCode") & s.m_returnCode;
    ar.Member("Data") & s.m_data;
    return ar.EndObject();
}

/*******************************************************************************************/
//删除数传中心的接口
/*******************************************************************************************/
struct DelDigitalCenterMessageRequest
{
    QString m_stationID;  //站ID
    QString m_workMode;   //工作模式 2个参数联合作为主键
};

template<typename Archiver>
Archiver& operator&(Archiver& ar, DelDigitalCenterMessageRequest& s)
{
    ar.StartObject();
    ar.Member("StationID") & s.m_stationID;
    ar.Member("WorkMode") & s.m_workMode;

    return ar.EndObject();
}

struct DelDigitalCenterMessageResponse
{
    int m_returnCode;
    QString m_DigitalCenterTaskCode;  //卫星代号  唯一标志 作为数据库主键
};

template<typename Archiver>
Archiver& operator&(Archiver& ar, DelDigitalCenterMessageResponse& s)
{
    ar.StartObject();
    ar.Member("ReturnCode") & s.m_returnCode;
    ar.Member("DigitalCenterTaskCode") & s.m_DigitalCenterTaskCode;
    return ar.EndObject();
}

/*******************************************************************************************/
//编辑数传中心的接口
/*******************************************************************************************/
struct EditDigitalCenterMessageRequest
{
    QString m_satellitID;               //卫星代号,任务代号
    QString m_satelliteIdentification;  //卫星标识
    QString m_satelliteName;            //卫星名称
    bool m_dateofEpoch;                 //瞬跟历元日期
    QString m_timeofEpoch;              //瞬跟历元时刻
    double m_semiMajorAxis;             //半长轴
    double m_eccentricityRatio;         //偏心率
    double m_dipAngle;                  //倾角（度）
    double m_rightAscension;            //升交点赤经（度）
    double m_argumentOfPerigee;         //近地点幅角（度）
    double m_meanAnomaly;               //平近点角（度）
};

template<typename Archiver>
Archiver& operator&(Archiver& ar, EditDigitalCenterMessageRequest& s)
{
    ar.StartObject();
    ar.Member("SatellitID") & s.m_satellitID;
    ar.Member("SatelliteIdentification") & s.m_satelliteIdentification;
    ar.Member("SatelliteName") & s.m_satelliteName;
    ar.Member("DateofEpoch") & s.m_dateofEpoch;
    ar.Member("TimeofEpoch") & s.m_timeofEpoch;
    ar.Member("SemiMajorAxis") & s.m_semiMajorAxis;
    ar.Member("EccentricityRatio") & s.m_eccentricityRatio;
    ar.Member("DipAngle") & s.m_dipAngle;
    ar.Member("RightAscension") & s.m_rightAscension;
    ar.Member("ArgumentOfPerigee") & s.m_argumentOfPerigee;
    ar.Member("MeanAnomaly") & s.m_meanAnomaly;
    return ar.EndObject();
}

struct EditDigitalCenterMessageResponse
{
    int m_returnCode;
    AddDigitalCenterMessageRequest m_data;
};

template<typename Archiver>
Archiver& operator&(Archiver& ar, EditDigitalCenterMessageResponse& s)
{
    ar.StartObject();
    ar.Member("ReturnCode") & s.m_returnCode;
    ar.Member("Data") & s.m_data;
    return ar.EndObject();
}

/*******************************************************************************************/
//获取数传中心的接口
/*******************************************************************************************/

struct AllDigitalCenterMessageRequest
{
};
template<typename Archiver>
Archiver& operator&(Archiver& ar, AllDigitalCenterMessageRequest&)
{
    return ar;
}

struct AllDigitalCenterMessageResponse
{
    int m_returnCode;
    QList<AddDigitalCenterMessageRequest> m_data;
};

template<typename Archiver>
Archiver& operator&(Archiver& ar, AllDigitalCenterMessageResponse& s)
{
    ar.StartObject();
    ar.Member("ReturnCode") & s.m_returnCode;
    ar.Member("Data") & s.m_data;
    return ar.EndObject();
}

#endif  // DIGITALCENTERMESSAGE_H
