#ifndef TIMESYNCTHREAD_H
#define TIMESYNCTHREAD_H

#include <QHostAddress>
#include <QThread>
#include <atomic>

struct TimeData;
class NTPTime;
// 时间同步 获取B码时间、NTP时间、服务器本地时间
class TimeSyncThread : public QThread
{
    Q_OBJECT
public:
    TimeSyncThread();

    void stopSync();
    void startSync();

protected:
    void run() override;

private:
    QDateTime getBCodeTime();

// signals:
//     void signalTimeUpdated(const TimeData& timeData);

private slots:
    // NTP时间发生跳跃
    void slotNTPDateTimeJump(const QDateTime& lastDateTime, const QDateTime& currentDateTime);
    // NTP时间长时间未更新
    void slotLongTimeNotUpdated();

private:
    std::atomic<bool> mIsRunning { true };
    NTPTime* mNTPTime { nullptr };

    QHostAddress mMasterNTPServerAddr;  // 主用NTP服务器
    QHostAddress mSlaveNTPServerAddr;   // 备用NTP服务器
};

#endif  // TIMESYNCTHREAD_H
