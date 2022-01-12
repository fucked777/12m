/*
 * 描述： 远控协议消息发送线程(定时上报)
 * 作者： daixb
 * 日期： 2018-11-6
 */
#ifndef HEARTBEATTHREAD_H4
#define HEARTBEATTHREAD_H4

#include "INetMsg.h"
#include <QScopedPointer>
#include <QDateTime>
#include <QThread>

namespace cppmicroservices
{
    class BundleContext;
}
class HeartbeatThreadImpl;
class HeartbeatThread: public QThread
{
    Q_OBJECT
private:
public:
    explicit HeartbeatThread(cppmicroservices::BundleContext context);
    ~HeartbeatThread();
    void stopHeartbeat();
    void startHeartbeat();

private:
    void run() override;

private:
    QScopedPointer<HeartbeatThreadImpl> m_impl;
};

#endif  // DEVICEMESSAGESENDTHREAD_H
