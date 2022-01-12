#ifndef TIMEMESSAGEDEFINE_H
#define TIMEMESSAGEDEFINE_H

#include <QDateTime>
#include <QHostAddress>

enum class TimeType
{
    Unknown = 0,
    BCodeTime,  // B码时间
    NTPTime,    // NTP时间
    SystemTime  // 系统时间
};

struct TimeData
{
    TimeType usedTimeType{TimeType::Unknown};  // 服务器使用的时间类型

    QDateTime bCodeTime;  // B码时间

    QDateTime ntpTime;           // NTP时间
    QHostAddress ntpServerAddr;  // NTP服务器地址

    QDateTime serverSystemTime{ QDateTime::currentDateTime() };  // 服务器操作系统时间
};

Q_DECLARE_METATYPE(TimeData)

// 时间配置文件信息
struct TimeConfigInfo
{
    TimeType usedTimeType{ TimeType::Unknown };  // 使用的时间类型

    QHostAddress ntpMasterServerAddr;  // NTP主用服务器
    QHostAddress ntpSlaveServerAddr;   // NTP备用服务器

    int maxJumpInterval;             // 时间跳跃最大间隔 NTP服务器当前上报的时间和上一次上报的时间间隔超过此值
    int longTimeNotUpdatedInterval;  // 长时间未更新 NTP的时间长时间未改变，可能NTP服务器没了
};

class TimeDataHelper
{
public:
    static QString getTimeTypeString(TimeType type)
    {
        switch (type)
        {
        case TimeType::BCodeTime: return QString("B码时间");
        case TimeType::NTPTime: return QString("NTP时间");
        case TimeType::SystemTime: return QString("系统时间");
        case TimeType::Unknown: break;;
        }
        return QString("未知时间类型");
    }
};

#endif  // TIMEMESSAGEDEFINE_H
