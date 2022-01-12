#ifndef ZMQROUTORTHREAD_H
#define ZMQROUTORTHREAD_H

#include <QByteArray>
#include <QString>
#include <QThread>
/*
 *                      Router
 *         接收方----------------------发送方
 * frame1   源地址                     目标地址
 * frame2   空帧                       空帧
 * frame3   目标地址                   源地址
 * frame4   空帧                       空帧
 * frame5   发送选项                    发送选项
 * frame6   空帧                       空帧
 * frame7   应答选项                    应答选项
 * frame8   空帧                       空帧
 * frame9   是否模块间通讯              是否模块间通讯
 * frame10  空帧                       空帧
 * frame11  数据                       数据
 */

namespace cppmicroservices
{
    class BundleContext;
}

class ZMQContext;
class ZMQRouterServerImpl;
class ZMQRouterServer : public QThread
{
    Q_OBJECT
public:
    ZMQRouterServer(cppmicroservices::BundleContext context);
    ~ZMQRouterServer();

    /*
     * 协议://然后跟着一个address
     * 例如
     * tcp://localhost:555
     * udp://localhost:555
     *
     * 参数1和参数2 没有区别的
     * 都是两端对发
     */
    bool startServer(const QString& frontend, const QString& backend, QString* errMsg = nullptr);
    void stopServer();

protected:
    void run();

private:
    ZMQRouterServerImpl* m_impl;
};

#endif  // ZMQROUTORTHREAD_H
