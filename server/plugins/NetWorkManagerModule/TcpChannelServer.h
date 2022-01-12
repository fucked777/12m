#ifndef TCPCHANNELSERVER_H
#define TCPCHANNELSERVER_H

#include "BaseChannel.h"

class TcpChannelServer : public BaseChannel
{
public:
    TcpChannelServer(const CommunicationInfo& info, QObject* parent = nullptr);
    virtual ~TcpChannelServer() override;

private:
    virtual OptionalNotValue sendData(const QByteArray& byteArray) override;
    virtual bool initSendSocket() override;
    virtual bool initRecvSocket() override;
    virtual void unInitSendSocket() override;
    virtual void unInitRecvSocket() override;
    void onReadyRead() override;
};

#endif  // TCPCHANNELSERVER_H
