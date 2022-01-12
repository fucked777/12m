#ifndef LOCALCOMMUNICATIONADDR_H
#define LOCALCOMMUNICATIONADDR_H
#include "DevProtocol.h"
#include <QString>

/* 获取平台配置文件中的扩展配置部分 */
class LocalCommunicationAddr
{
public:
    static void setDevAddrMessage(const MessageAddress&);
    static void setDevAddrProtocol(quint64);
    /* 基地ID */
    static quint8 baseID();
    /* 站ID */
    static quint8 stationID();
    /* 设备ID */
    static quint8 equipmentID();
    /* 分机名称 */
    static quint8 extenName();
    /* 系统ID 分机名称的一部分 */
    static quint8 systemID();
    /* 这个设备ID也是分机名称的一部分 */
    static quint8 deviceID();
    /* 分机编号 */
    static quint8 extenNumb();
    /* 地址前缀 */
    static quint32 addrPrefix();
    /* 当前的设备ID 就是分机名称+编号 */
    static quint32 devID();
    /* 设备地址,全部合起来 */
    static quint64 devAddrProtocol();
    static MessageAddress devAddrMessage();
};

#endif  // LOCALCOMMUNICATIONADDR_H
