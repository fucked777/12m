#ifndef AUTOTASK4426SERVICE_H
#define AUTOTASK4426SERVICE_H

#include "ControlFlowHandler.h"
#include "INetMsg.h"
#include <QScopedPointer>
#include <QThread>
#include <atomic>

namespace cppmicroservices
{
    class BundleContext;
}

struct ManualMessage;
class AutoTask4426ServiceImpl : public QObject
{
    Q_OBJECT
    DIS_COPY_MOVE(AutoTask4426ServiceImpl)
public:
    std::atomic<bool> runningFlag{ false };
    std::atomic<bool> endFlag{ true };
    ControlFlowHandler controlFlowHandler;
    ManualMessage manualMessage;
    QString userID;

public:
    AutoTask4426ServiceImpl(QObject* parent = nullptr);
    ~AutoTask4426ServiceImpl();

    void manualOperation(const ManualMessage& msg);
};
class AutoTask4426Service : public INetMsg
{
    Q_OBJECT
public:
    AutoTask4426Service(cppmicroservices::BundleContext context);
    ~AutoTask4426Service();

private:
    // 手动控制
    void manualOperation(const ProtocolPack& pack);

private slots:
    // 发送数据给设备
    void slotSendByteArrayToDevice(const QByteArray& data);
signals:
    void sg_manualOperation(const ManualMessage& msg);

public:
    // 推送提示消息
    void slotPublishInfoMsg(const QString& msg);
    // 推送警告消息
    void slotPublishWarningMsg(const QString& msg);
    // 推送错误消息
    void slotPublishErrorMsg(const QString& msg);
    // 推送明显提示的消息
    void slotPublishSpecificTipsMsg(const QString& msg);

private:
    QScopedPointer<AutoTask4426ServiceImpl> m_impl;
    QThread m_thread;
};

#endif  // AUTOTASK4426SERVICE_H
