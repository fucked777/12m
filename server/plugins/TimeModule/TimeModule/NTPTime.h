#ifndef NTPTIME_H
#define NTPTIME_H

#include <QDateTime>
#include <QHostAddress>

#pragma pack(1)
typedef struct
{
    unsigned int coarse;
    unsigned int fine;  // 表示秒以下的精度
} NTP_time;

typedef struct
{
    unsigned int head;
    unsigned int delay;
    unsigned int disp;
    unsigned int refid;
    NTP_time reftime;
    NTP_time orgtime;
    NTP_time rectime;
    NTP_time xmttime;

} NTPData;
#pragma pack()

class QUdpSocket;
class NTPTime : public QObject
{
    Q_OBJECT
public:
    NTPTime();

    QDateTime getNTPTime();

    // 设置NTP服务器地址
    void setNTPServerAddr(const QHostAddress& addr, quint16 port = 123);
    QHostAddress getNTPServerAddr() const;
    // 设置跳跃间隔
    void setMaxJumpInterval(int second);
    int getMaxJumpInterval();

    // 设置长时间未更新间隔
    void setLongTimeNotUpdatedInterval(int second);
    int getLongTimeNotUpdatedInterval();

    void stopNTP();
    void startNTP();

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    // 初始化查询数据
    void initQueryData();
    // 发送NTP时间查询数据
    void sendTimeQuery();

signals:
    // 当与上一次的时间发生跳跃时发送此信号
    void signalTimeJump(const QDateTime& lastDateTime, const QDateTime& currentDateTime);
    // 时间长时间未更新
    void signalLongTimeNotUpdated();
    // NTP服务器地址改变
    void signalNTPServerAddrChanged(const QHostAddress& addr);

private slots:
    void slotDataReadyRead();

private:
    QByteArray mQueryBuf;
    QUdpSocket* mSocket = nullptr;
    int mTimerInterval = 500;  // 定时器间隔
    int mTimerID{-1};

    NTPData ntpData;  // NTP数据

    QHostAddress mNTPServerAddr;    // NTP服务器地址
    quint16 mNTPServerPort{ 123 };  // NTP端口

    int mJumpIntervalSecond = 5;       // 最大时间跳跃的间隔
    int mLongTimeNotUpdateSecond = 5;  // 长时间未更新

    bool mIsChangedNTPServerAddr = false;  // 是否改变了NTP服务器地址
    QDateTime mLastDateTime;               // 上一次NTP时间
};

#endif  // NTPTIME_H
