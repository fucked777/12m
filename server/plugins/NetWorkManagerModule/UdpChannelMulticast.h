#ifndef UDPCHANNEL_H
#define UDPCHANNEL_H
#include "BaseChannel.h"
#include <QUdpSocket>

class UdpChannelMulticastImpl;
class UdpChannelMulticast : public BaseChannel
{
public:
    UdpChannelMulticast(const CommunicationInfo& info, QObject* parent = nullptr);
    virtual ~UdpChannelMulticast() override;
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
    UdpChannelMulticastImpl* m_impl;
};

#endif  // UDPCHANNEL_H
