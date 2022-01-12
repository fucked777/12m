#include "SocketPackHelper.h"
#include "PlatformInterface.h"
#include "ZMQUtility.h"
#include <array>

/*
 * dealer recv                  send
 * 0 隐藏源地址                  0 隐藏源地址
 * 1 空帧                       1 空帧
 * 2 目标地址                   2 目标地址
 * 3 发送选项                   3 发送选项
 * 4 应答选项                   4 应答选项
 * 5 模块间通讯                 5 模块间通讯
 * 6 发送者                     6 发送者
 * 7 数据                      7 数据
 *
 *
 * router recv                send
 * 1 源地址                    1 目标地址
 * 2 空帧                      2 空帧
 * 3 目标地址                  3 源地址
 * 4 发送选项                   4 发送选项
 * 5 应答选项                  5 应答选项
 * 6 模块间通讯                6 模块间通讯
 * 7 发送者                   7 发送者
 * 8 数据                     8 数据
 */
ZMQSocketRes SocketPackHelper::dealerRecv(ZMQSocket& zmqSocket, ProtocolPack& pack, QString& errMsg)
{
    std::array<QByteArray, 8> recvArray;
    ZMQSocketRes res;
    for (auto& item : recvArray)
    {
        res = zmqSocket.recv(item, 0);
        if (!res.success())
        {
            errMsg = zmqSocket.getErrMsg();
            break;
        }
        /* 加这句话是防止协议被破坏后一直等待 */
        if (!res.isMore())
        {
            break;
        }
    }
    // recvArray[0];       /* 1 空帧 */
    pack.srcID = recvArray[1];                                        /* 2 源地址 */
    pack.operation = recvArray[2];                                    /* 3 发送选项 */
    pack.operationACK = recvArray[3];                                 /* 4 应答选项 */
    pack.module = recvArray[4].isEmpty() ? false : !!recvArray[4][0]; /* 5 模块间通讯 */
    pack.userID = recvArray[5];                                       /* 6 发送者 */
    pack.cascadeACKAddr = recvArray[6];                               /* 7 级联的源地址 */
    pack.data = recvArray[7];                                         /* 8 数据 */

    return res;
}

ZMQSocketRes SocketPackHelper::dealerSend(ZMQSocket& zmqSocket, const ProtocolPack& pack, QString& errMsg, int flags)
{
    std::array<std::tuple<QByteArray, int>, 8> sendArray{
        std::make_tuple(QByteArray(), ZMQ_SNDMORE),                                  /* 1 空帧 */
        std::make_tuple(pack.desID.toUtf8(), ZMQ_SNDMORE),                           /* 2 目标地址 */
        std::make_tuple(pack.operation.toUtf8(), ZMQ_SNDMORE),                       /* 3 发送选项 */
        std::make_tuple(pack.operationACK.toUtf8(), ZMQ_SNDMORE),                    /* 4 应答选项 */
        std::make_tuple(QByteArray(1, static_cast<char>(pack.module)), ZMQ_SNDMORE), /* 5 模块间通讯 */
        std::make_tuple(pack.userID.toUtf8(), ZMQ_SNDMORE),                          /* 6 发送者 */
        std::make_tuple(pack.cascadeACKAddr.toUtf8(), ZMQ_SNDMORE),                  /* 7 级联的源地址 */
        std::make_tuple(pack.data, flags)                                            /* 8 数据 */
    };

    ZMQSocketRes res;
    for (auto& item : sendArray)
    {
        res = zmqSocket.send(std::get<0>(item), std::get<1>(item));
        if (!res.success())
        {
            errMsg = zmqSocket.getErrMsg();
            break;
        }
    }

    return res;
}

ZMQSocketRes SocketPackHelper::routerRecv(ZMQSocket& zmqSocket, ProtocolPack& pack, QString& errMsg)
{
    std::array<QByteArray, 9> recvArray;
    ZMQSocketRes res;
    for (auto& item : recvArray)
    {
        res = zmqSocket.recv(item, 0);
        if (!res.success())
        {
            errMsg = zmqSocket.getErrMsg();
            break;
        }
        /* 加这句话是防止协议被破坏后一直等待 */
        if (!res.isMore())
        {
            break;
        }
    }
    pack.srcID = recvArray[0]; /* 1 源地址 */
    // recvArray[1];       /* 2 空帧 */
    pack.desID = recvArray[2];                                        /* 3 目标地址 */
    pack.operation = recvArray[3];                                    /* 4 发送选项 */
    pack.operationACK = recvArray[4];                                 /* 5 应答选项 */
    pack.module = recvArray[5].isEmpty() ? false : !!recvArray[5][0]; /* 6 模块间通讯 */
    pack.userID = recvArray[6];                                       /* 7 发送者 */
    pack.cascadeACKAddr = recvArray[7];                               /* 8 级联的源地址 */
    pack.data = recvArray[8];                                         /* 9 数据 */

    return res;
}
ZMQSocketRes SocketPackHelper::routerSend(ZMQSocket& zmqSocket, const ProtocolPack& pack, QString& errMsg)
{
    std::array<std::tuple<QByteArray, int>, 9> sendArray{
        std::make_tuple(pack.desID.toUtf8(), ZMQ_SNDMORE),                           /* 1 目标地址 */
        std::make_tuple(QByteArray(), ZMQ_SNDMORE),                                  /* 2 空帧 */
        std::make_tuple(pack.srcID.toUtf8(), ZMQ_SNDMORE),                           /* 3 目标地址 */
        std::make_tuple(pack.operation.toUtf8(), ZMQ_SNDMORE),                       /* 4 发送选项 */
        std::make_tuple(pack.operationACK.toUtf8(), ZMQ_SNDMORE),                    /* 5 应答选项 */
        std::make_tuple(QByteArray(1, static_cast<char>(pack.module)), ZMQ_SNDMORE), /* 6 模块间通讯 */
        std::make_tuple(pack.userID.toUtf8(), ZMQ_SNDMORE),                          /* 7 发送者 */
        std::make_tuple(pack.cascadeACKAddr.toUtf8(), ZMQ_SNDMORE),                  /* 8 级联的源地址 */
        std::make_tuple(pack.data, 0)                                                /* 9 数据 */
    };

    ZMQSocketRes res;
    for (auto& item : sendArray)
    {
        res = zmqSocket.send(std::get<0>(item), std::get<1>(item));
        if (!res.success())
        {
            errMsg = zmqSocket.getErrMsg();
            break;
        }
    }

    return res;
}

ZMQSocketRes SocketPackHelper::dealerSendTotalData(ZMQSocket& zmqSocket, const ProtocolPack& pack, QString& errMsg)
{
    std::array<std::tuple<QByteArray, int>, 11> sendArray{
        std::make_tuple(QByteArray(), ZMQ_SNDMORE),               /* 1 空帧 */
        std::make_tuple(pack.cascadeDesID.toUtf8(), ZMQ_SNDMORE), /* 2 目标地址 */

        std::make_tuple(pack.srcID.toUtf8(), ZMQ_SNDMORE),                           /* 1 目标地址 */
        std::make_tuple(QByteArray(), ZMQ_SNDMORE),                                  /* 2 空帧 */
        std::make_tuple(pack.desID.toUtf8(), ZMQ_SNDMORE),                           /* 3 目标地址 */
        std::make_tuple(pack.operation.toUtf8(), ZMQ_SNDMORE),                       /* 4 发送选项 */
        std::make_tuple(pack.operationACK.toUtf8(), ZMQ_SNDMORE),                    /* 5 应答选项 */
        std::make_tuple(QByteArray(1, static_cast<char>(pack.module)), ZMQ_SNDMORE), /* 6 模块间通讯 */
        std::make_tuple(pack.userID.toUtf8(), ZMQ_SNDMORE),                          /* 7 发送者 */
        std::make_tuple(pack.cascadeACKAddr.toUtf8(), ZMQ_SNDMORE),                  /* 8 级联的源地址 */
        std::make_tuple(pack.data, 0)                                                /* 9 数据 */
    };

    ZMQSocketRes res;
    for (auto& item : sendArray)
    {
        res = zmqSocket.send(std::get<0>(item), std::get<1>(item));
        if (!res.success())
        {
            errMsg = zmqSocket.getErrMsg();
            break;
        }
    }

    return res;
}
ZMQSocketRes SocketPackHelper::dealerRecvTotalData(ZMQSocket& zmqSocket, ProtocolPack& pack, QString& errMsg)
{
    std::array<QByteArray, 11> recvArray;
    ZMQSocketRes res;
    for (auto& item : recvArray)
    {
        res = zmqSocket.recv(item, 0);
        if (!res.success())
        {
            errMsg = zmqSocket.getErrMsg();
            break;
        }
        /* 加这句话是防止协议被破坏后一直等待 */
        if (!res.isMore())
        {
            break;
        }
    }
    // recvArray[0];       /* 1 空帧 */
    pack.cascadeSrcID = recvArray[1]; /* 2.远端的发送的源地址 接收的目的地址 */

    pack.srcID = recvArray[2]; /* 3 源地址 */
    // recvArray[3];       /* 4 空帧 */
    pack.desID = recvArray[4];                                        /* 5 目标地址 */
    pack.operation = recvArray[5];                                    /* 6 发送选项 */
    pack.operationACK = recvArray[6];                                 /* 7 应答选项 */
    pack.module = recvArray[7].isEmpty() ? false : !!recvArray[7][0]; /* 8 模块间通讯 */
    pack.userID = recvArray[8];                                       /* 9 发送者 */
    pack.cascadeACKAddr = recvArray[9];                               /* 10 级联的源地址 */
    pack.data = recvArray[10];                                        /* 11 数据 */

    return res;
}

ZMQSocketRes SocketPackHelper::routerSendTotalData(ZMQSocket& zmqSocket, const ProtocolPack& pack, QString& errMsg)
{
    std::array<std::tuple<QByteArray, int>, 12> sendArray{
        std::make_tuple(pack.cascadeDesID.toUtf8(), ZMQ_SNDMORE), /* 1 目标地址 */
        std::make_tuple(QByteArray(), ZMQ_SNDMORE),               /* 2 空帧 */
        std::make_tuple(pack.cascadeSrcID.toUtf8(), ZMQ_SNDMORE), /* 3 目标地址 */

        std::make_tuple(pack.srcID.toUtf8(), ZMQ_SNDMORE),                           /* 1 目标地址 */
        std::make_tuple(QByteArray(), ZMQ_SNDMORE),                                  /* 2 空帧 */
        std::make_tuple(pack.desID.toUtf8(), ZMQ_SNDMORE),                           /* 3 目标地址 */
        std::make_tuple(pack.operation.toUtf8(), ZMQ_SNDMORE),                       /* 4 发送选项 */
        std::make_tuple(pack.operationACK.toUtf8(), ZMQ_SNDMORE),                    /* 5 应答选项 */
        std::make_tuple(QByteArray(1, static_cast<char>(pack.module)), ZMQ_SNDMORE), /* 6 模块间通讯 */
        std::make_tuple(pack.userID.toUtf8(), ZMQ_SNDMORE),                          /* 7 发送者 */
        std::make_tuple(pack.cascadeACKAddr.toUtf8(), ZMQ_SNDMORE),                  /* 8 级联的源地址 */
        std::make_tuple(pack.data, 0)                                                /* 9 数据 */
    };

    ZMQSocketRes res;
    for (auto& item : sendArray)
    {
        res = zmqSocket.send(std::get<0>(item), std::get<1>(item));
        if (!res.success())
        {
            errMsg = zmqSocket.getErrMsg();
            break;
        }
    }

    return res;
}
ZMQSocketRes SocketPackHelper::routerRecvTotalData(ZMQSocket& zmqSocket, ProtocolPack& pack, QString& errMsg)
{
    std::array<QByteArray, 12> recvArray;
    ZMQSocketRes res;
    for (auto& item : recvArray)
    {
        res = zmqSocket.recv(item, 0);
        if (!res.success())
        {
            errMsg = zmqSocket.getErrMsg();
            break;
        }
        /* 加这句话是防止协议被破坏后一直等待 */
        if (!res.isMore())
        {
            break;
        }
    }

    pack.cascadeSrcID = recvArray[0]; /* 1 级联的源地址 */
    //                  recvArray[1]; /* 2 空帧 */
    pack.cascadeDesID = recvArray[2]; /* 3 级联的目的地址 */

    pack.srcID = recvArray[3]; /* 4 源地址 */
    //     recvArray[4];       /* 5 空帧 */
    pack.desID = recvArray[5];                                        /* 6 目标地址 */
    pack.operation = recvArray[6];                                    /* 7 发送选项 */
    pack.operationACK = recvArray[7];                                 /* 8 应答选项 */
    pack.module = recvArray[8].isEmpty() ? false : !!recvArray[8][0]; /* 9 模块间通讯 */
    pack.userID = recvArray[9];                                       /* 10 发送者 */
    pack.cascadeACKAddr = recvArray[10];                              /* 11 级联的源地址 */
    pack.data = recvArray[11];                                        /* 12 数据 */

    return res;
}

OptionalNotValue SocketPackHelper::initDealerSocket(ZMQContext& zmqContext, ZMQSocket& zmqSocket, const QString& addr, const QString& id)
{
    if (!zmqSocket.init(zmqContext, ZMQ_DEALER))
    {
        return OptionalNotValue(ErrorCode::ZMQSocketInitFailed, zmqSocket.getErrMsg());
    }

    /* recv超时时间,单位毫秒 */
    zmqSocket.set(ZMQ_RCVTIMEO, getTimeo());
    /* send超时时间,单位毫秒 */
    zmqSocket.set(ZMQ_SNDTIMEO, getTimeo());
    /*
     * 默认属性值是0，表示会静默的丢弃不能路由到的消息。属性值是1时，
     * 如果消息不能路由，会返回一个EHOSTUNREACH错误代码
     * 此参数 只是ZMQ_ROUTER 有效
     */
    // zmqSocket.set(ZMQ_ROUTER_MANDATORY, 1);
    /*设置调用zmq_disconnect或zmq_close后尚未接收消息的等待时间。该选项将进一步影响zmq_ctx_term的阻塞时间。
     * 设置为-1表示不丢弃任何消息，zmq_ctx_term将会一直阻塞直到消息全部接收；
     * 设置为0表示丢弃消息并立刻返回；设置为正值x表示，x毫秒后将返回，默认是30000毫秒
     */
    zmqSocket.set(ZMQ_LINGER, getLinger());

    /*ZMQ_SNDHWM属性将会设置socket参数指定的socket对外发送的消息的高水位。
     * 高水位是一个硬限制，它会限制每一个与此socket相连的在内存中排队的未处理的消息数目的最大值。0值代表着没有限制。
     * 如果已经到达了规定的限制，socket就需要进入一种异常的状态，
     * 表现形式因socket类型而异。socket会进行适当的调节，比如阻塞或者丢弃已发送的消息
     */
    zmqSocket.set(ZMQ_SNDHWM, getSendHWM());
    /*ZMQ_RCVHWM属性将会设置socket参数指定的socket进入的消息的高水位。
     * 高水位是一个硬限制，它会限制每一个与此socket相连的在内存中排队的未处理的消息数目的最大值。0值代表着没有限制。
     * 如果已经到达了规定的限制，socket就需要进入一种异常的状态，
     * 表现形式因socket类型而异。socket会进行适当的调节，比如阻塞或者丢弃被发送的消息。
     */
    zmqSocket.set(ZMQ_RCVHWM, getRecvHWM());

    /* 设置ID */
    zmqSocket.set(ZMQ_IDENTITY, id);

    /* 开启TCP保活机制，防止网络连接因长时间无数据而被中断 */
    zmqSocket.set(ZMQ_TCP_KEEPALIVE, 1);

    /* 网络连接空闲1min即发送保活包 */
    zmqSocket.set(ZMQ_TCP_KEEPALIVE_IDLE, 3);

    /* 连接 */
    if (!zmqSocket.connect(addr))
    {
        return OptionalNotValue(ErrorCode::ZMQConnectFailed, zmqSocket.getErrMsg());
    }
    return OptionalNotValue();
}
OptionalNotValue SocketPackHelper::initRouterSocket(ZMQContext& zmqContext, ZMQSocket& zmqSocket, const QString& addr, const QString& id)
{
    if (!zmqSocket.init(zmqContext, ZMQ_ROUTER))
    {
        return OptionalNotValue(ErrorCode::ZMQSocketInitFailed, zmqSocket.getErrMsg());
    }

    /* recv超时时间,单位毫秒 */
    zmqSocket.set(ZMQ_RCVTIMEO, getTimeo());
    /* send超时时间,单位毫秒 */
    zmqSocket.set(ZMQ_SNDTIMEO, getTimeo());
    /*
     * 默认属性值是0，表示会静默的丢弃不能路由到的消息。属性值是1时，
     * 如果消息不能路由，会返回一个EHOSTUNREACH错误代码
     * 此参数 只是ZMQ_ROUTER 有效
     */
    zmqSocket.set(ZMQ_ROUTER_MANDATORY, 1);
    /*设置调用zmq_disconnect或zmq_close后尚未接收消息的等待时间。该选项将进一步影响zmq_ctx_term的阻塞时间。
     * 设置为-1表示不丢弃任何消息，zmq_ctx_term将会一直阻塞直到消息全部接收；
     * 设置为0表示丢弃消息并立刻返回；设置为正值x表示，x毫秒后将返回，默认是30000毫秒
     */
    zmqSocket.set(ZMQ_LINGER, getLinger());

    /*ZMQ_SNDHWM属性将会设置socket参数指定的socket对外发送的消息的高水位。
     * 高水位是一个硬限制，它会限制每一个与此socket相连的在内存中排队的未处理的消息数目的最大值。0值代表着没有限制。
     * 如果已经到达了规定的限制，socket就需要进入一种异常的状态，
     * 表现形式因socket类型而异。socket会进行适当的调节，比如阻塞或者丢弃已发送的消息
     */
    zmqSocket.set(ZMQ_SNDHWM, getSendHWM());
    /*ZMQ_RCVHWM属性将会设置socket参数指定的socket进入的消息的高水位。
     * 高水位是一个硬限制，它会限制每一个与此socket相连的在内存中排队的未处理的消息数目的最大值。0值代表着没有限制。
     * 如果已经到达了规定的限制，socket就需要进入一种异常的状态，
     * 表现形式因socket类型而异。socket会进行适当的调节，比如阻塞或者丢弃被发送的消息。
     */
    zmqSocket.set(ZMQ_RCVHWM, getRecvHWM());

    /* 设置ID */
    if (!id.isEmpty())
    {
        zmqSocket.set(ZMQ_IDENTITY, id);
    }

    /* 开启TCP保活机制，防止网络连接因长时间无数据而被中断 */
    zmqSocket.set(ZMQ_TCP_KEEPALIVE, 1);

    /* 网络连接空闲1min即发送保活包 */
    zmqSocket.set(ZMQ_TCP_KEEPALIVE_IDLE, 3);

    /* 绑定 */
    if (!zmqSocket.bind(addr))
    {
        return OptionalNotValue(ErrorCode::ZMQBindFailed, zmqSocket.getErrMsg());
    }

    return OptionalNotValue();
}

QString SocketPackHelper::removeSendIDSuffix(const QString& bak)
{
    auto src = bak;
    if (src.endsWith(getSendIDSuffix()))
    {
        src.chop(getSendIDSuffixLen());
    }
    return src;
}
Optional<ZMQContext*> SocketPackHelper::registerZMQContext()
{
    using ResType = Optional<ZMQContext*>;
    auto platformObjectTools = PlatformObjectTools::instance();
    auto zmqContext = platformObjectTools->addObj<ZMQContext>(ZMQHelper::defaultContextObjName());
    if (zmqContext == nullptr)
    {
        return ResType(ErrorCode::InvalidData, "注册ZMQContext错误");
    }

    /* 未初始化的情况才去初始化context */
    if (!zmqContext->valid() && !zmqContext->init())
    {
        return ResType(ErrorCode::ZMQContextInitFailed, zmqContext->getErrMsg());
    }
    return ResType(zmqContext);
}
Optional<ZMQContext*> SocketPackHelper::getZMQContext()
{
    using ResType = Optional<ZMQContext*>;
    auto platformObjectTools = PlatformObjectTools::instance();
    auto zmqContext = platformObjectTools->getObj<ZMQContext>(ZMQHelper::defaultContextObjName());
    if (zmqContext == nullptr)
    {
        return ResType(ErrorCode::InvalidData, "获取ZMQContext错误");
    }
    return ResType(zmqContext);
}
