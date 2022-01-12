#ifndef RECVMSGPOLL_H
#define RECVMSGPOLL_H

#include "Utility.h"
#include <QThread>

/*
 * 此类的作用就是接收消息
 * 然后发信号出去
 *
 * 接受的是所有需要通讯的模块的消息
 */
class INetMsg;
class RecvMsgPollImpl;
class RecvMsgPoll : public QThread
{
public:
    RecvMsgPoll();
    ~RecvMsgPoll();
    /* 添加一个需要接收消息的对象 */
    OptionalNotValue registerPollItem(QString recvID, QString addr, INetMsg* netMsg);
    void removePollItem(INetMsg* netMsg);
    void stopServer();
    void startServer();

private:
    void run() override;

private:
    RecvMsgPollImpl* m_impl;
};

#endif  // RECVMSGPOLL_H
