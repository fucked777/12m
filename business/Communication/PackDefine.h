#ifndef PACKDEFINE_H
#define PACKDEFINE_H
#include <QByteArray>
#include <QDebug>
#include <QString>

/* 接收到的数据包结构 */
struct ProtocolPack
{
    /* 二级 */
    bool module{ false }; /* 模块间通讯 */
    QString srcID;        /* 发送端ID或者发送端地址 从哪来 */
    QString desID;        /* 接收端ID或者发送端地址 到哪去 */
    QString operation;    /* 发送选项 举个栗子remove add 开始校零等 */
    QString operationACK; /* 应答选项 注册接收端返回后的选项*/
    QString userID;       /* 发送者ID */
    QByteArray data;      /* 接收到的数据 */

    /* 一级 作为地址 */
    QString cascadeSrcID;   /* 级联的源地址 */
    QString cascadeDesID;   /* 级联的目的地址 */
    QString cascadeACKAddr; /* 作为数据 */
};

inline QDebug operator<<(QDebug os, const ProtocolPack& c)
{
    auto str = QString("*****************************************\n源地址:%1\n目标地址:%2\n发送选项:%"
                       "3\n应答选项:%4\n模块间通讯:%5\n发送者:%6\n数据:")
                   .arg(c.srcID, c.desID, c.operation, c.operationACK, c.module ? "是" : "否", c.userID);
    os.noquote() << str << c.data << "\n*****************************************\n";
    return os;
}
#endif  // PACKDEFINE_H
