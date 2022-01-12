#ifndef EPHEMERISMESSAGE_H
#define EPHEMERISMESSAGE_H

#include "JsonHelper.h"
#include <QMap>
#include <QVariant>
/*******************************************************************************************/
//添加星历的接口
/*******************************************************************************************/
struct AddEphemerisMessageRequest
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
Archiver& operator&(Archiver& ar, AddEphemerisMessageRequest& s)
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

struct AddEphemerisMessageResponse
{
    int m_returnCode;
    AddEphemerisMessageRequest m_data;
};

template<typename Archiver>
Archiver& operator&(Archiver& ar, AddEphemerisMessageResponse& s)
{
    ar.StartObject();
    ar.Member("ReturnCode") & s.m_returnCode;
    ar.Member("Data") & s.m_data;
    return ar.EndObject();
}

/*******************************************************************************************/
//删除星历的接口
/*******************************************************************************************/
struct DelEphemerisMessageRequest
{
    QString m_stationID;  //站ID
    QString m_workMode;   //工作模式 2个参数联合作为主键
};

template<typename Archiver>
Archiver& operator&(Archiver& ar, DelEphemerisMessageRequest& s)
{
    ar.StartObject();
    ar.Member("StationID") & s.m_stationID;
    ar.Member("WorkMode") & s.m_workMode;

    return ar.EndObject();
}

struct DelEphemerisMessageResponse
{
    int m_returnCode;
    QString m_EphemerisTaskCode;  //卫星代号  唯一标志 作为数据库主键
};

template<typename Archiver>
Archiver& operator&(Archiver& ar, DelEphemerisMessageResponse& s)
{
    ar.StartObject();
    ar.Member("ReturnCode") & s.m_returnCode;
    ar.Member("EphemerisTaskCode") & s.m_EphemerisTaskCode;
    return ar.EndObject();
}

/*******************************************************************************************/
//编辑星历的接口
/*******************************************************************************************/
struct EditEphemerisMessageRequest
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
Archiver& operator&(Archiver& ar, EditEphemerisMessageRequest& s)
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

struct EditEphemerisMessageResponse
{
    int m_returnCode;
    AddEphemerisMessageRequest m_data;
};

template<typename Archiver>
Archiver& operator&(Archiver& ar, EditEphemerisMessageResponse& s)
{
    ar.StartObject();
    ar.Member("ReturnCode") & s.m_returnCode;
    ar.Member("Data") & s.m_data;
    return ar.EndObject();
}

/*******************************************************************************************/
//获取星历的接口
/*******************************************************************************************/

struct AllEphemerisMessageRequest
{
};
template<typename Archiver>
Archiver& operator&(Archiver& ar, AllEphemerisMessageRequest&)
{
    return ar;
}

struct AllEphemerisMessageResponse
{
    int m_returnCode;
    QList<AddEphemerisMessageRequest> m_data;
};

template<typename Archiver>
Archiver& operator&(Archiver& ar, AllEphemerisMessageResponse& s)
{
    ar.StartObject();
    ar.Member("ReturnCode") & s.m_returnCode;
    ar.Member("Data") & s.m_data;
    return ar.EndObject();
}

#endif  // EPHEMERISMESSAGE_H
