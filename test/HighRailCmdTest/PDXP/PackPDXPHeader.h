#ifndef PACKPDXPHEADER_H
#define PACKPDXPHEADER_H

#include "PDXPHeader.h"
#include "Utility.h"
#include <QObject>
#include <QTimer>

/**
 * @className PackPDXPHeader 这里组装PDXP的协议
 * @brief brief
 * @author Wangjy
 * @createTime 2019-09-25
 */
class PackPDXPHeader : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief packHead 组包PDXP协议头  只有30个字节 在组装完数据包后 添加数据包之前将ushort的字节保存到具体的类型里面
     * @param message
     * @param header
     */
    static QByteArray packHead(const PDXPHeader& header);
    static Optional<PDXPHeader> unpackHead(const QByteArray& message);

private:
    /**
     * @brief packVer 封装协议版本 1个字节
     * @param message
     * @param version
     */
    static void packVer(QByteArray& message, short version);

    /**
     * @brief packVer 任务标识 2个字节
     * @param message
     * @param mid
     */
    static void packMID(QByteArray& message, ushort mid);
    /**
     * @brief packSIDAndDID 信源地址 心宿地址 4个字节和4个字节
     * @param message
     * @param address
     */
    static void packSIDAndDID(QByteArray& message, int address);
    /**
     * @brief packBID 定义这条消息是类型 4个字节
     * @param message
     * @param messageType
     */
    static void packBID(QByteArray& message, int messageType);
    /**
     * @brief packNo 定义包序号 4个字节
     * @param message
     * @param index
     */
    static void packNo(QByteArray& message, int index);

    /**
     * @brief packFlag 处理标志位 1个字节 默认缺省0x00
     * @param message
     * @param index
     */
    static void packFlag(QByteArray& message, int index = 0);

    /**
     * @brief packReserve 处理保留字段  4个字节
     * @param message
     */
    static void packReserve(QByteArray& message);
    /**
     * @brief packDate 为发送该数据包的日期 相对于2000年1月1日的积日 2000年1月1日计为第一天
     * @param message
     */
    static void packDate(QByteArray& message);
    /**
     * @brief packTime 该数据包相对于当日零时的积秒， 单位为0.1ms。
     * @param message
     */
    static void packTime(QByteArray& message);
};

#endif  // PACK_H
