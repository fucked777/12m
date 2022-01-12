#ifndef EPHEMERISDATADEFINE_H
#define EPHEMERISDATADEFINE_H
#include <QDate>
#include <QMap>
#include <QObject>
#include <QTime>

struct EphemerisDataData
{
    QString satellitCode;             //卫星代号,任务代号
    QDate dateOfEpoch;                //瞬跟历元日期
    QTime timeOfEpoch;                //瞬跟历元时刻
    double semimajorAxis{ 0.0 };      //半长轴
    double eccentricity{ 0.0 };       //偏心率
    double dipAngle{ 0.0 };           //倾角(度)
    double rightAscension{ 0.0 };     //升交点赤经(度)
    double argumentOfPerigee{ 0.0 };  //近地点幅角(度)
    double meanAnomaly{ 0.0 };        //平近点角(度)
};

using EphemerisDataMap = QMap<QString, EphemerisDataData>;
using EphemerisDataList = QList<EphemerisDataData>;

Q_DECLARE_METATYPE(EphemerisDataData);

#endif  // EPHEMERISDATADEFINE_H
