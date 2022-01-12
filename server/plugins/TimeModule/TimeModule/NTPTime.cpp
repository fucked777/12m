#include "NTPTime.h"

#include <QUdpSocket>
#include <QtEndian>

unsigned int secs1970 = 2208988800;

NTPTime::NTPTime()
{
    initQueryData();

    mSocket = new QUdpSocket;
    mSocket->bind(9977, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    connect(mSocket, &QUdpSocket::readyRead, this, &NTPTime::slotDataReadyRead);
}

QDateTime NTPTime::getNTPTime() { return mLastDateTime; }

void NTPTime::stopNTP()
{
    if(mTimerID != -1)
    {
        killTimer(mTimerID);
        mTimerID = -1;
    }
}

void NTPTime::startNTP()
{
    mTimerID = startTimer(mTimerInterval);
}

void NTPTime::setNTPServerAddr(const QHostAddress& addr, quint16 port)
{
    mNTPServerAddr = addr;
    mNTPServerPort = port;

    mIsChangedNTPServerAddr = true;

    emit signalNTPServerAddrChanged(addr);
}

QHostAddress NTPTime::getNTPServerAddr() const { return mNTPServerAddr; }

void NTPTime::setMaxJumpInterval(int second)
{
    if (second < 0)
    {
        return;
    }
    mJumpIntervalSecond = second;
}

int NTPTime::getMaxJumpInterval() { return mJumpIntervalSecond; }

void NTPTime::setLongTimeNotUpdatedInterval(int second)
{
    if (second < 0)
    {
        return;
    }
    mLongTimeNotUpdateSecond = second;
}

int NTPTime::getLongTimeNotUpdatedInterval() { return mLongTimeNotUpdateSecond; }

void NTPTime::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event)
    sendTimeQuery();

    static int count = 0;
    static QDateTime oldDateTime;
    if (oldDateTime == mLastDateTime)
    {
        ++count;

        // NTP时间长时间没有改变（没有收到上报的时间，NTP服务器可能没了）
        if (count > mLongTimeNotUpdateSecond * 1000 / mTimerInterval)
        {
            emit signalLongTimeNotUpdated();
            count = 0;
        }
    }
    else
    {
        oldDateTime = mLastDateTime;
        count = 0;
    }
}

void NTPTime::initQueryData()
{
    mQueryBuf.resize(48);
    for (int i = 0; i < 48; i++)
    {
        mQueryBuf[i] = 0;
    }
    mQueryBuf[0] = 0x2B;
}

void NTPTime::sendTimeQuery() { mSocket->writeDatagram(mQueryBuf, mNTPServerAddr, mNTPServerPort); }

void NTPTime::slotDataReadyRead()
{
    QByteArray data;
    QHostAddress targetAddr;  // ip
    quint16 targetPort;       //端口号
    while (mSocket->hasPendingDatagrams())
    {
        auto size = mSocket->pendingDatagramSize();
        data.resize(size);
        mSocket->readDatagram(data.data(), data.size(), &targetAddr, &targetPort);
    }

    if (mNTPServerAddr.toIPv4Address() != targetAddr.toIPv4Address())
    {
        return;
    }

    memcpy(&ntpData, data.data(), data.size());

    // 最后一个的时间戳
    uint time = 0;
    ntpData.xmttime.coarse = qToBigEndian(ntpData.xmttime.coarse);
    time = ntpData.xmttime.coarse;
    time = time - secs1970;

    auto currentDateTime = QDateTime::fromTime_t(time);

    // 获取毫秒级
    uint msecond = 0;
    msecond = qToBigEndian(ntpData.xmttime.fine);

    auto msecondStr = QString::number(msecond / (429496.7296) / 10);
    auto list = msecondStr.split('.');
    if (list.length() == 2)
    {
        msecondStr = list[0];
    }
    msecondStr = QString("%1").arg(msecondStr.toInt(), 3, 10, QLatin1Char('0'));
    // 添加毫秒
    currentDateTime = currentDateTime.addMSecs(msecondStr.toInt());

    // NTP服务器地址改变后第一次的时间设置为当前时间
    if (mIsChangedNTPServerAddr)
    {
        mLastDateTime = currentDateTime;
        mIsChangedNTPServerAddr = false;
    }

    // 判断时间是否发生了时间跳跃
    if (qAbs(mLastDateTime.secsTo(currentDateTime)) > mJumpIntervalSecond)
    {
        // 发生时间跳跃
        emit signalTimeJump(mLastDateTime, currentDateTime);
    }

    mLastDateTime = currentDateTime;
}
