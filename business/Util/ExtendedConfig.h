#ifndef EXTENDEDCONFIG_H
#define EXTENDEDCONFIG_H
#include <QString>

/* 获取平台配置文件中的扩展配置部分 */
class ExtendedConfig
{
public:
    /*
     * 20210502
     * 这里做一个说明
     * 关于这个通讯IP地址
     * 我们这里不是客户端直接发送给服务器
     * 是在两者之间加了一个Router 现在这个路由是和服务器一起启动的
     * 所以其实客户端和服务器全部都是连接的Router服务
     * 因此对于现在来讲其实服务器的通讯IP和客户端的通讯IP都应该填写Router服务IP
     * 这个IP恰好都是服务器的IP
     */
    /* 获取配置文件中服务器通讯IP */
    static QString serverCommunicationIP();
    /* 获取配置文件中服务器通讯端口 */
    static quint16 serverCommunicationPort();
    /* 获取配置文件中服务器通讯文件 */
    static QString serverCommunicationFile();
    /* 获取配置文件中客户端通讯IP */
    static QString clientCommunicationIP();
    /* 获取配置文件中客户端通讯端口 */
    static quint16 clientCommunicationPort();
    /* 获取通讯的instanceID
     * 2021052 再多客户端情况下 现有配置是所有客户端的所有模块的源ID是一致的,所以导致多客户端连接不上
     * 现在将当前程序的instanceID拼接在ID里面就能区分多个客户端了同理服务器用的是完全一致的代码
     * 所以发送给服务器时也需要加上服务器的instanceID
     * 因此添加此配置来配置服务器的instanceID已达到效果
     *
     * 此时服务器就不能主动推送数据给客户端了,因为客户端有很多,ID都不一样了,所有服务端的主动推送都是未定义的
     */
    static QString communicationInstanceID();

    /* 当前项目ID */
    static QString curProjectID();
    /* 当前天线ID */
    static QString curTKID();
};

#endif  // EXTENDEDCONFIG_H
