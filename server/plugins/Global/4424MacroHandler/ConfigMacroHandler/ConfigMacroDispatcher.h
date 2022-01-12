#ifndef CONFIGMACRODISPATCHER_H
#define CONFIGMACRODISPATCHER_H

#include "PlanRunMessageDefine.h"

#include <QObject>
#include <atomic>

/*
 * 20210926 wp??
 * 经验证terminate会导致内存泄漏且还会导致崩溃
 * 所以才改造配置宏和参数宏的下发类
 * 让其可以优雅的停止
 */

class BaseLinkHandler;
class ConfigMacroDispatcher : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ConfigMacroDispatcher)
public:
    explicit ConfigMacroDispatcher(QObject* parent = nullptr);
    ~ConfigMacroDispatcher();

    void handle(const ManualMessage& msg);

    void setRunningFlag(std::atomic<bool>* runningFlag);
    bool isRunning();
    bool isExit();

private:
    void appendHandle(BaseLinkHandler* linkHandler);

signals:
    // 发送数据到设备
    void signalSendToDevice(const QByteArray& data);
    // 提示信息
    void signalInfoMsg(const QString& msg);
    // 警告信息
    void signalWarningMsg(const QString& msg);
    // 错误信息
    void signalErrorMsg(const QString& msg);

private:
    QMap<LinkType, BaseLinkHandler*> mLinkhandleMap;
    /* 20210926 wp?? 退出的标志 */
    std::atomic<bool>* mRunningFlag;
};

#endif  // CONFIGMACRODISPATCHER_H
