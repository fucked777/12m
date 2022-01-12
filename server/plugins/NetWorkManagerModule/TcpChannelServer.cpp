#include "TcpChannelServer.h"

TcpChannelServer::TcpChannelServer(const CommunicationInfo& info, QObject* parent)
    : BaseChannel(info, parent)
{
}

// bool TcpChannelServer::readData(const QByteArray &byteArray)
//{
//    Q_UNUSED(byteArray);
//    return false;
//}
TcpChannelServer::~TcpChannelServer() {}

OptionalNotValue TcpChannelServer::sendData(const QByteArray& /*byteArray*/) { return OptionalNotValue(ErrorCode::NotImplemented, "没写完"); }
bool TcpChannelServer::initSendSocket() { return false; }
bool TcpChannelServer::initRecvSocket() { return false; }
void TcpChannelServer::unInitSendSocket() {}
void TcpChannelServer::unInitRecvSocket() {}
void TcpChannelServer::onReadyRead() {}
