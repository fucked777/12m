#ifndef DEVPROTOCOLSERIALIZE_H
#define DEVPROTOCOLSERIALIZE_H

#include "DevProtocol.h"
#include "Utility.h"

/* 一定要注意这里的协议头转换不是转成JSON,是直接与协议相关的字节流 */

QByteArray& operator<<(QByteArray& self, const DevProtocolHeader& value);
QByteArray& operator>>(QByteArray& self, DevProtocolHeader& value);
QByteArray& operator<<(QByteArray& self, const DevMessageHeader& value);
QByteArray& operator>>(QByteArray& self, DevMessageHeader& value);
QByteArray& operator<<(QByteArray& self, const MessageAddress& value);
QByteArray& operator>>(QByteArray& self, MessageAddress& value);
QByteArray& operator<<(QByteArray& self, const Version& value);
QByteArray& operator>>(QByteArray& self, Version& value);

class ProtocolHeaderBit
{
public:
    /* 直接从字节流中得到指定的原始数据 */
    static quint64 sidRaw(const QByteArray&);
    static quint64 didRaw(const QByteArray&);
    static quint32 bidRaw(const QByteArray&);
    static quint16 verRaw(const QByteArray&);
    static quint16 reserveRaw(const QByteArray&);
    static quint32 dateRaw(const QByteArray&);
    static quint32 timeRaw(const QByteArray&);
    static qint32 dataLenRaw(const QByteArray&);
    /* 从字节流中获取,解析之后的数据 */
    static MessageAddress sidParse(const QByteArray&);
    static MessageAddress didParse(const QByteArray&);
    static DevMsgType bidParse(const QByteArray&);
    static Version verParse(const QByteArray&);
    static quint16 reserveParse(const QByteArray&);
    static QDate dateParse(const QByteArray&);
    static QTime timeParse(const QByteArray&);
    static qint32 dataLenParse(const QByteArray&);

    /* 根据协议目的地址与信源地址中包含一个设备名与设备号组成的id
     * ps: 我把这认为是ID,用来区分不同的设备
     */
    static quint32 didDevID(const QByteArray&);
    static quint32 sidDevID(const QByteArray&);
};

class DevMsgTypeHelper
{
public:
    static QString toStr(DevMsgType);
    static DevMsgType fromStr(const QString&);
    static QString toDescStr(DevMsgType);
};

class ControlCmdResponseTypeHelper
{
public:
    static QString toDescStr(ControlCmdResponseType type);
};

#endif  // DEVPROTOCOLSERIALIZE_H
