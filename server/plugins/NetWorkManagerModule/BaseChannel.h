#ifndef BASECHANNEL_H
#define BASECHANNEL_H

//#include "CommunicationDefine.h"
#include "NetStatusInterface.h"
#include "Utility.h"
#include <QMutex>
#include <QObject>
#include <QVariantList>
/*
 * 通道基类
 */
struct DevProtocolHeader;
struct CommunicationInfo;
class QIODevice;
class BaseChannelImpl;
class BaseChannel : public QObject
{
    Q_OBJECT
public:
    explicit BaseChannel(const CommunicationInfo& info, QObject* parent = nullptr);
    virtual ~BaseChannel();

    void stop();
    void start();
    virtual OptionalNotValue sendData(const QByteArray& byteArray) = 0;
    /*
     * 设置包的最大大小
     * 这里是防止乱包导致包大小很大
     * 内存疯涨
     * 只要包大小超出这个值就直接丢弃了
     * 默认是10000
     * 小于等于0是无效的不采取任何操作
     */
    void setPackMax(qint32 maxSize);
    /* 设置和获取通道编号,没什么含义,就是一个数字,从0开始依次递增,每个通道都是不一样的 */
    void setChanelNum(qint32);
    qint32 chanelNum() const;
    /*读写通道是否有效 */
    virtual bool recvVaild() = 0;
    virtual bool sendVaild() = 0;

    /* 初始化用于比较的数据
     * 这里的用户数据一般是固定的
     * 一般对包处理检查筛选的时候使用
     *
     * 注意此处的协议头是以自己为发送方来定义的
     * 意思是
     * 协议头中的源地址指的是自己
     * 目的地址指的是对方
     * 所以比较的时候要注意
     */
    void setCompareHead(const DevProtocolHeader&);

    /* 获取当前的通道信息 */
    CommunicationInfo channelInfo();
    /* 获取包计数 */
    quint64 sendCount();     /* 发送数据的总数 */
    quint64 sendFailCount(); /* 发送失败的数据 */
    quint64 recvCount();     /* 接收数据的总数 */
    quint64 recvFailCount(); /* 接收错误的总数 */
signals:
    /* 此接口是发送的有效的数据,包含协议头的所有数据,进行过基本的数据检查了的 */
    void sg_recvPack(const CommunicationInfo &channelInfo, const QByteArray&);
    /* 有问题的数据 这个是原始数据 */
    void sg_recvErrorPack(const QByteArray&, const QString&);
    /* 通道数据改变 */
    void sg_channelRecvDataChange(const NetStatusItemInfo&);
    void sg_channelSendDataChange(const NetStatusItemInfo&);

    //    /* 用于多线程使用 */
    //    void sg_start();
    //    void sg_stop();

protected:
    virtual void onReadyRead() = 0;
    /*
     * 初始化会在定时器中进行,失败后会一直初始化
     * 每秒1次
     */
    void timerEvent(QTimerEvent* event) override;
    void stopImpl();
    void startImpl();
    /* 这两个函数在构造函数中调用不能为纯虚函数 */
    virtual bool initSendSocket();
    virtual bool initRecvSocket();
    virtual void unInitSendSocket();
    virtual void unInitRecvSocket();

public:
    BaseChannelImpl* m_baseImpl;
};

#endif  // BASECHANNEL_H
