#ifndef FEPHELPER_H
#define FEPHELPER_H

#include <QTcpSocket>
#include <QThread>
enum class RecvState;
class FepHelper
{
public:
    /* 接收发送请求包 */
    static RecvState recvSendREQ(QString& errMsg, RecvContent, const QByteArray& rawData);
};

#endif  // TcpSocket
