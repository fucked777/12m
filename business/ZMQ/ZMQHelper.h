#ifndef ZMQHELPER_H
#define ZMQHELPER_H
#include <QString>

class ZMQHelper
{
public:
    static QString errMsg();
    /*
     * 合成zmq地址的函数
     * 此函数不会检查输入参数是否合法
     * 就是一股脑拼接起来
     * protocol:  tcp udp 等
     * ip:  *  localhost  192.10.45.55等
     * port:端口
     */
    static QString syntheticAddress(const QString& protocol, const QString& ip, quint16 port);
    static QString syntheticAddress(const QString& protocol, const QString& file);
    static QString procAddress(const QString& name);
    static QString deaultLocalAddress();
    static QString defaultContextObjName();
};

#endif  // ZMQHELPER_H
