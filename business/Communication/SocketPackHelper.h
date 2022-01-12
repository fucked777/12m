#ifndef SOCKETPACKHELPER_H
#define SOCKETPACKHELPER_H
#include "PackDefine.h"
#include "Utility.h"
#include "ZMQContext.h"
#include "ZMQSocket.h"

#define SEND_ID_SUFFIX "_send"
#define RECV_ID_SUFFIX "_recv"
class SocketPackHelper
{
public:
    /* 这里是把级联的地址数据当做数据发送和接收 */
    static ZMQSocketRes dealerRecv(ZMQSocket& zmqSocket, ProtocolPack& pack, QString& errMsg);
    static ZMQSocketRes dealerSend(ZMQSocket& zmqSocket, const ProtocolPack& pack, QString& errMsg, int flags = 0);
    static ZMQSocketRes routerRecv(ZMQSocket& zmqSocket, ProtocolPack& pack, QString& errMsg);
    static ZMQSocketRes routerSend(ZMQSocket& zmqSocket, const ProtocolPack& pack, QString& errMsg);

    /* 这里是把级联的地址数据当做地址发送和接收 */
    static ZMQSocketRes routerSendTotalData(ZMQSocket& zmqSocket, const ProtocolPack& pack, QString& errMsg);
    static ZMQSocketRes routerRecvTotalData(ZMQSocket& zmqSocket, ProtocolPack& pack, QString& errMsg);
    static ZMQSocketRes dealerSendTotalData(ZMQSocket& zmqSocket, const ProtocolPack& pack, QString& errMsg);
    static ZMQSocketRes dealerRecvTotalData(ZMQSocket& zmqSocket, ProtocolPack& pack, QString& errMsg);

    static OptionalNotValue initDealerSocket(ZMQContext& zmqContext, ZMQSocket& zmqSocket, const QString& addr, const QString& id);
    static OptionalNotValue initRouterSocket(ZMQContext& zmqContext, ZMQSocket& zmqSocket, const QString& addr, const QString& id = QString());

    static constexpr int getTimeo() { return 1000; }
    static constexpr int getLinger() { return 1000; }
    static constexpr int getSendHWM() { return 1000; }
    static constexpr int getRecvHWM() { return 1000; }

    static constexpr const char* getSendIDSuffix() { return SEND_ID_SUFFIX; }
    static constexpr const char* getRecvIDSuffix() { return RECV_ID_SUFFIX; }
    static constexpr int getSendIDSuffixLen() { return sizeof(SEND_ID_SUFFIX) - 1; } /* 去掉\0 */
    static constexpr int getRecvIDSuffixLen() { return sizeof(RECV_ID_SUFFIX) - 1; } /* 去掉\0 */
    static QString removeSendIDSuffix(const QString& src);

    static Optional<ZMQContext*> registerZMQContext();
    static Optional<ZMQContext*> getZMQContext();
};

#endif  // SOCKETPACKHELPER_H
