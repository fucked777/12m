#include "TimeSyncThread.h"

#include "GlobalData.h"
#include "GlobalDefine.h"
#include "MessagePublish.h"
#include "NTPTime.h"
#include "TimeMessageDefine.h"
#include "TimeSyncUDPGB.h"
#include <QDateTime>
#include <QApplication>

TimeSyncThread::TimeSyncThread()
{
    // 通过配置获取主备用NTP服务器ip
    auto timeConfigInfo = GlobalData::getTimeConfigInfo();
    mMasterNTPServerAddr = timeConfigInfo.ntpMasterServerAddr;
    mSlaveNTPServerAddr = timeConfigInfo.ntpSlaveServerAddr;

    mNTPTime = new NTPTime;
    mNTPTime->setNTPServerAddr(mMasterNTPServerAddr);
    mNTPTime->setMaxJumpInterval(timeConfigInfo.maxJumpInterval);
    mNTPTime->setLongTimeNotUpdatedInterval(timeConfigInfo.longTimeNotUpdatedInterval);
    // 时间发生跳跃
    connect(mNTPTime, &NTPTime::signalTimeJump, this, &TimeSyncThread::slotNTPDateTimeJump);
    // NTP时间长时间未改变
    connect(mNTPTime, &NTPTime::signalLongTimeNotUpdated, this, &TimeSyncThread::slotLongTimeNotUpdated);
}

void TimeSyncThread::stopSync()
{
    mIsRunning = false;
    quit();
    while (isRunning() && !isFinished())
    {
        QThread::msleep(10);
        QApplication::processEvents();
    }
    wait();
    mNTPTime->stopNTP();
}
void TimeSyncThread::startSync()
{
    auto timeConfigInfo = GlobalData::getTimeConfigInfo();
    auto type = GlobalData::getUsedTimeTypeRedis();
    if(type == TimeType::Unknown)
    {
        type = timeConfigInfo.usedTimeType;
        GlobalData::setUsedTimeTypeRedis(type);
    }

    GlobalData::setUsedTimeType(type);
    mNTPTime->startNTP();
    start();
}

void TimeSyncThread::run()
{
    mIsRunning = true;
    TimeSyncUDPGB sync;
    auto result = sync.initSocket();
    if (!result)
    {
        SystemLogPublish::errorMsg(result.msg());
        return;
    }
    while (mIsRunning)
    {
        TimeData timeData;
        timeData.usedTimeType = GlobalData::getUsedTimeType();  // 获取使用的时间类型
        timeData.ntpTime = mNTPTime->getNTPTime();              // 获取NTP时间
        timeData.ntpServerAddr = mNTPTime->getNTPServerAddr();

#ifdef Q_OS_LINUX
        timeData.bCodeTime = getBCodeTime();  // 获取B码时间
#endif

        sync.syncTime(timeData);
        // 触发信号
        // emit signalTimeUpdated(timeData);

        msleep(500);
    }
}

#ifdef Q_OS_LINUX
#include <timeserv.h>
#endif
QDateTime TimeSyncThread::getBCodeTime()
{
#ifdef Q_OS_LINUX
    ABSTIME abstime;
    int status = getAbsTime(&abstime);

    auto dateTime = QDateTime(QDate(abstime.year, abstime.month, abstime.day), QTime(abstime.hour, abstime.minute, abstime.second));
    dateTime = dateTime.addMSecs(abstime.microsecond / 1000);

    return dateTime;
#endif
    return QDateTime();
}

void TimeSyncThread::slotNTPDateTimeJump(const QDateTime& /*lastDateTime*/, const QDateTime& /*currentDateTime*/)
{
    if (mNTPTime->getNTPServerAddr() == mMasterNTPServerAddr)
    {
        mNTPTime->setNTPServerAddr(mSlaveNTPServerAddr);
    }
    else
    {
        mNTPTime->setNTPServerAddr(mMasterNTPServerAddr);
    }

//    auto msg = QString("NTP时间跳跃间隔超过%1秒，上一次NTP时间：%2，当前NTP时间：%3。切换NTP服务器为：%4")
//                   .arg(mNTPTime->getMaxJumpInterval())
//                   .arg(lastDateTime.toString(DATETIME_DISPLAY_FORMAT))
//                   .arg(currentDateTime.toString(DATETIME_DISPLAY_FORMAT))
//                   .arg(mNTPTime->getNTPServerAddr().toString());
//    SystemLogPublish::warningMsg(msg);
}

void TimeSyncThread::slotLongTimeNotUpdated()
{
    auto oldNTPServerAddr = mNTPTime->getNTPServerAddr();
    QHostAddress newNTPServerAddr;

    if (oldNTPServerAddr == mMasterNTPServerAddr)
    {
        newNTPServerAddr = mSlaveNTPServerAddr;
    }
    else
    {
        newNTPServerAddr = mMasterNTPServerAddr;
    }

    mNTPTime->setNTPServerAddr(newNTPServerAddr);

//    auto msg = QString("NTP时间超过%1秒未更新，NTP服务器：%2可能异常。切换NTP服务器为：%3")
//                   .arg(mNTPTime->getLongTimeNotUpdatedInterval())
//                   .arg(oldNTPServerAddr.toString())
//                   .arg(mNTPTime->getNTPServerAddr().toString());
//    SystemLogPublish::warningMsg(msg);
}
