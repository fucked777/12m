#include "BaseChannel.h"
#include "BaseChannelImpl.h"
#include "CommunicationDefine.h"
#include "DevProtocolSerialize.h"
#include <QDebug>
#include <QThread>

BaseChannel::BaseChannel(const CommunicationInfo& info, QObject* parent)
    : QObject(parent)
    , m_baseImpl(new BaseChannelImpl)
{
    m_baseImpl->channelInfo = info;
    //    connect(this, &BaseChannel::sg_start, this, &BaseChannel::startImpl);
    //    connect(this, &BaseChannel::sg_stop, this, &BaseChannel::stopImpl);
}
BaseChannel::~BaseChannel()
{
    stopImpl();
    delete m_baseImpl;
    m_baseImpl = nullptr;
}
// void BaseChannel::start() { emit sg_start(); }
// void BaseChannel::stop() { emit sg_stop(); }
void BaseChannel::start() { startImpl(); }
void BaseChannel::stop() { stopImpl(); }

bool BaseChannel::initSendSocket() { return false; }
bool BaseChannel::initRecvSocket() { return false; }

void BaseChannel::unInitSendSocket() { m_baseImpl->sendStatus.reset(); }
void BaseChannel::unInitRecvSocket() { m_baseImpl->recvStatus.reset(); }

void BaseChannel::setCompareHead(const DevProtocolHeader& data) { m_baseImpl->compareHead = data; }

CommunicationInfo BaseChannel::channelInfo() { return m_baseImpl->channelInfo; }
quint64 BaseChannel::sendCount() { return m_baseImpl->sendStatus.count; }
quint64 BaseChannel::sendFailCount() { return m_baseImpl->sendStatus.failCount; }
quint64 BaseChannel::recvCount() { return m_baseImpl->sendStatus.count; }
quint64 BaseChannel::recvFailCount() { return m_baseImpl->sendStatus.count; }
void BaseChannel::stopImpl()
{
    if (m_baseImpl->timerID != -1)
    {
        killTimer(m_baseImpl->timerID);
        m_baseImpl->timerID = -1;
    }

    unInitSendSocket();
    unInitRecvSocket();
}
void BaseChannel::startImpl()
{
    /* 小于0证明根本就没有初始化编号 */
    Q_ASSERT(m_baseImpl->sendStatus.index >= 0);
    if (m_baseImpl->timerID != -1)
    {
        return;
    }
    if (recvVaild())
    {
        unInitRecvSocket();
    }
    if (sendVaild())
    {
        unInitSendSocket();
    }
    m_baseImpl->timerID = startTimer(1000);
}
void BaseChannel::timerEvent(QTimerEvent* /*event*/)
{
    initRecvSocket();
    initSendSocket();
    if (recvVaild() && sendVaild())
    {
        killTimer(m_baseImpl->timerID);
        m_baseImpl->timerID = -1;
    }
}

void BaseChannel::setPackMax(qint32 maxSize)
{
    if (maxSize <= 0)
    {
        return;
    }
    m_baseImpl->packMax = maxSize;
}
void BaseChannel::setChanelNum(qint32 index)
{
    m_baseImpl->sendStatus.index = index;
    m_baseImpl->recvStatus.index = index;
}
qint32 BaseChannel::chanelNum() const { return m_baseImpl->sendStatus.index; }
