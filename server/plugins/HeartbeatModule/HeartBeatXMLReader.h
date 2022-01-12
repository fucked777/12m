#ifndef CONFIGMACROREADER_H
#define CONFIGMACROREADER_H

#include <QDomElement>

struct HeartBeatInfo
{
    enum HostType
    {
        Master,
        Slave
    };

    bool isUsed = false;    // 配置主备是否启用
    QString vIP;            // 如果包含了虚拟IP 就代表当前是主用的设备
    QString aIP;            // A的IP
    QString bIP;            // B的IP
    QString masterToSlave;  //主机切备机脚本
    QString slaveToMaster;  //备机切主机脚本
};

// 读取生成配置宏界面和主界面手动控制的配置文件数据(配置宏界面和主界面手动控制界面共用同一个配置文件)
class HeartBeatXMLReader
{
public:
    HeartBeatXMLReader();

    bool getHeartbeatConfig(HeartBeatInfo& heartbeatInfo);
};

#endif  // ConfigMacroCONFIG_H
