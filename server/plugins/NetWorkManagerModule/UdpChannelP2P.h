#ifndef UDPCHANNELP2P_H
#define UDPCHANNELP2P_H

#include "BaseChannel.h"
#include <QList>
#include <QMutex>
#include <QUdpSocket>

class UdpChannelP2PImpl;
class UdpChannelP2P : public BaseChannel
{
public:
    UdpChannelP2P(const CommunicationInfo& info, QObject* parent = nullptr);
    virtual ~UdpChannelP2P() override;
    bool recvVaild() override;
    bool sendVaild() override;

    virtual OptionalNotValue sendData(const QByteArray& byteArray) override;

protected:
    virtual bool initSendSocket() override;
    virtual bool initRecvSocket() override;
    virtual void unInitSendSocket() override;
    virtual void unInitRecvSocket() override;
    void onReadyRead() override;

private:
    UdpChannelP2PImpl* m_impl;
};

#endif  // UDPCHANNELP2P_H
