#ifndef INETMSG_H
#define INETMSG_H

#include "PackDefine.h"
#include "Utility.h"
#include <QByteArray>
#include <QDebug>
#include <QString>
#include <functional>

/*
 * 所有需要使用网络通讯的类都需要继承此类
 * 这里的处理方式是每个模块都会有一个socket所以理论数据量不会很大
 * 所以这里公用一个socket来收发
 */
struct ProtocolPack;
class ZMQContext;
class INetMsgImpl;
class INetMsg;
class INetMsg : public QObject
{
    Q_OBJECT
    /* 拷贝和赋值是故意禁用的,如果不是特别需要就不要打开,一定记住打开后一定要手动重写
     * 移动是懒得写所以禁用了
     * wp??
     */
    DIS_COPY_MOVE(INetMsg)

public:
    /*
     * 订阅的函数
     * pack 当前收到的数据包信息
     */
    using SubscribeFunc = void(const ProtocolPack& pack);
    using SubscribeFunction = std::function<INetMsg::SubscribeFunc>;

public:
    /*
     * 两个构造函数
     * 传入threadName则会在单独的线程中启动线程
     * 否则会统一在一个默认的线程中执行
     */
    explicit INetMsg(QObject* parent = nullptr);
    explicit INetMsg(const QString& threadName, QObject* parent = nullptr);
    virtual ~INetMsg();

    /*
     * 设置源地址
     * 就是自己的身份牌
     * 就是发送方的目的地址
     * 默认是随机生成的,所以肯定收不到的
     */
    void setSrcAddr(const QString& id);
    QString sendAddr() const;
    QString recvAddr() const;

    /*
     * 连接地址
     * 默认连接地址是
     * linux: ipc://localmsg.ipc
     * windows:tcp://localhost:4000
     */
    void setConnectAddr(const QString& addr);
    QString connectAddr() const;

    bool startServer(QString* errMsg = nullptr);
    void stopServer();

    /* 发送数据 srcID写与不写都会被填充为当前的源地址
     * 用户名会自动填充为当前的
     */
    OptionalNotValue send(const ProtocolPack& recvValue);
    /* 发送数据 srcID写与不写都会被填充为当前的源地址
     * 除此以外都需要手动填充
     */
    OptionalNotValue sendManual(const ProtocolPack& recvValue);

    /*
     * 此方法与send的唯一区别是
     * 此方法会交换发送选项与接收选项
     */
    OptionalNotValue sendACK(const ProtocolPack& recvValue);

    inline void silenceSend(const ProtocolPack& recvValue)
    {
        auto sendRes = send(recvValue);
        if (!sendRes.success())
        {
            qWarning() << recvValue << sendRes.msg();
        }
    }

    inline void silenceSendACK(const ProtocolPack& recvValue)
    {
        auto sendRes = sendACK(recvValue);
        if (!sendRes.success())
        {
            qWarning() << recvValue << sendRes.msg();
        }
    }

protected:
    /*
     * 注册订阅的函数
     * key  为需要订阅的类型与ProtocolPack::operation  内容一致就会触发调用
     * func 为收到消息后触发的函数
     * 一个key只能对应一个触发函数,多次注册只会保留最后一次
     */
    void registerSubscribe(const QString& key, const std::function<SubscribeFunc>& func, bool localThread = true);
    void unregisterSubscribe(const QString& key);

    template<typename Func, typename T>
    void registerSubscribe(const QString& key, Func&& func, T* self, bool localThread = true)
    {
        registerSubscribe(key, std::bind(func, self, std::placeholders::_1), localThread);
    }
signals:
    void sg_recvMsg(const ProtocolPack&);

private:
    void recvMsg(const ProtocolPack&);

private:
    INetMsgImpl* m_baseImpl;
};

#endif  // IZMQSERVER_H
