#ifndef STATIONRESMANAGEMENTDEFINE_H
#define STATIONRESMANAGEMENTDEFINE_H

#include <QMap>
#include <QObject>
#include <QString>

struct StationResManagementData
{
    QString stationIdentifying;     /* 站标识    */
    QString stationName;            /* 站名     */
    QString ttcUACAddr;             /* 测控UAC地址 */
    QString dataTransUACAddr;       /* 数传UAC地址 */
    QString descText;               /* 站描述     */
    double stationLongitude{ 0.0 }; /* 站址(经度) */
    double stationLatitude{ 0.0 };  /* 站址(纬度) */
    double stationHeight{ 0.0 };    /* 站址(高度) */
    bool isUsable{ true };          /* 是否可用    */
};

using StationResManagementMap = QMap<QString, StationResManagementData>;
using StationResManagementList = QList<StationResManagementData>;

Q_DECLARE_METATYPE(StationResManagementData);

#endif  // STATIONRESMANAGEMENTDEFINE_H
