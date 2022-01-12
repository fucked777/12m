#ifndef BASECHANNELIMPL_H
#define BASECHANNELIMPL_H

#include "CommunicationDefine.h"
#include "DevProtocol.h"
#include "NetStatusInterface.h"
#include "Utility.h"
#include <QAbstractSocket>
#include <QObject>
#include <QVariantList>

class BaseChannelImpl
{
    DIS_COPY_MOVE(BaseChannelImpl)
public:
    inline BaseChannelImpl() {}
    inline ~BaseChannelImpl() {}
    CommunicationInfo channelInfo;
    NetStatusItemInfo sendStatus;
    NetStatusItemInfo recvStatus;

    qint32 packMax{ 100000 };      /* 单个包的最大大小,超出大小后认为是数据包有问题,直接丢弃 */
    DevProtocolHeader compareHead; /* 用于比较的数据协议头 */

    int timerID{ -1 }; /* 定时器 */
};

#endif  // BASECHANNELIMPL_H
