#ifndef TIMESYNCUDPGB_H
#define TIMESYNCUDPGB_H

#include "Utility.h"
#include <QObject>

/* 此类需要删除
 * NetInterfaceHelper也要删除
 * 在修改Netmsg之后删除
 */
struct HeartbeatMessage;
struct TimeData;
class TimeSyncUDPGBImpl;
class TimeSyncUDPGB : public QObject
{
    Q_OBJECT
public:
    TimeSyncUDPGB();
    ~TimeSyncUDPGB();

    OptionalNotValue initSocket();

    /* 同步时间 */
    bool syncTime(const TimeData&);
signals:
    void sg_onReadyRead(const TimeData&);
    void sg_setLocalTimeData(const TimeData&);

private:
    void setLocalTimeData(const TimeData&);

private:
    TimeSyncUDPGBImpl* m_impl;
};

#endif  // TIMESYNCUDPGB_H
