#ifndef STATIONNETCENTERTYPE_H
#define STATIONNETCENTERTYPE_H

#include "PDXPHeader.h"
#include <QtGlobal>

enum class ZwzxRemoteControlCmdReplyResult
{
    Error = 0x33,
    Correct = 0xCC,
    Refuse = 0xFF,
};

// 站网中心远程控制命令  数据标志：0x60261000
struct ZwzxRemoteControlCMD
{
    PDXPHeader header;  // PDXP头

    quint8 v{ 0 };     // 版本号
    quint8 st{ 0 };    // 执行方式
    quint8 pa{ 0 };    // 状态参数
    quint8 jg{ 0 };    // 执行结果
    quint32 res{ 0 };  // 保留
};

// 站网中心远程控制命令应答 数据标志：0x60050101
struct ZwzxRemoteControlCmdReply
{
    PDXPHeader header;  // PDXP头

    quint32 infoType{ 0 };  // 信息类型
    quint8 result{ 0 };     // 结果   0x33:错误  0x:cc正确
};

// 站网中心远程控制命令执行结果 数据标志：0x60261001
struct ZwzxRemoteControlCmdResult
{
    PDXPHeader header;  // PDXP头
    quint32 uac{ 0 };
    quint32 t{ 0 };    /* 24小时之内的时间0.1ms */
    quint8 pa{ 0 };    // 远控参数
    quint8 jg{ 0 };    // 结果
    quint8 sjjg{ 0 };  // 实际结果
};

Q_DECLARE_METATYPE(ZwzxRemoteControlCmdResult);
Q_DECLARE_METATYPE(ZwzxRemoteControlCmdReply);
Q_DECLARE_METATYPE(ZwzxRemoteControlCMD);
#endif  // STATIONNETCENTERTYPE_H
