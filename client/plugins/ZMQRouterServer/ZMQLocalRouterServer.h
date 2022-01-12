#ifndef ZMQLOCALROUTERSERVER_H
#define ZMQLOCALROUTERSERVER_H

#include <QByteArray>
#include <QString>
#include <QThread>
/*
 *
 *          ZMQ_DEALER                ZMQ_ROUTER
 *         自己的ID                     源地址
 * 附加帧1  空帧                        空帧
 * 附加帧2  远端转换目的地址              远端转换目的地址
 * frame1   源地址                     源地址
 * frame2   空帧                       空帧
 * frame3   目标地址                   目标地址
 * frame4   发送选项                    发送选项
 * frame5   应答选项                    应答选项
 * frame6   是否模块间通讯              是否模块间通讯
 * frame7  数据                       数据
 *
 */

/*
 * 对于模块间通讯 客户端服务器都一样 无论接收还是发送
 * 直接源地址目的地址互换一下发送就行 走的进程间通讯
 *
 * 对外通讯 分情况了 不能统一处理了
 * 对于客户端
 * 对外发送 目的地址是配置文件读取的 源地址是自己 将这两个组在包的前面发送就行
 * 对外接收 能收到那就是自己的包 然后通过本地router分发即可
 *
 * 对于服务器
 * 对外发送 不能主动发必须先接收数据然后恢复
 * 源地址是自己 目的地址在接收数据的数据包里面有
 *
 * 对外接收 收的到就是给自己的然后通过本地router分发即可但是一定要带上接收的源地址这个对于发送是目标地址
 */

namespace cppmicroservices
{
    class BundleContext;
}

class ZMQContext;
class ZMQLocalRouterServerImpl;
class ZMQLocalRouterServer : public QThread
{
    Q_OBJECT
public:
    ZMQLocalRouterServer(cppmicroservices::BundleContext context);
    ~ZMQLocalRouterServer();

    /*
     * 协议://然后跟着一个address
     * 例如
     * tcp://localhost:555
     * udp://localhost:555
     *
     * 参数1和参数2 没有区别的
     * 都是两端对发
     */
    bool startServer(const QString& convertAddr, QString* errMsg = nullptr);
    void stopServer();

private:
    void run();

private:
    ZMQLocalRouterServerImpl* m_impl;
};

#endif  // ZMQLOCALROUTERSERVER_H
