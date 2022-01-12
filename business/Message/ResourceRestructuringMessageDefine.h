#ifndef RESOURCERESTRUCTURINGMESSAGEDEFINE_H
#define RESOURCERESTRUCTURINGMESSAGEDEFINE_H

#include <QMap>
#include <QVariant>

#include "DevProtocol.h"
#include "ProtocolXmlTypeDefine.h"
#include "SystemWorkMode.h"

enum BBEType
{
    CK_BBE = 0x0100,
    GS_BBE = 0x0200,
    TRK_BBE = 0x0300,
    DS_BBE = 0x0400,
    CCZF_BBE = 0x0500,
    GSDDP_BBE = 0x0600,  // 4424高速基带单点频
    GSSDP_BBE = 0x0700   // 4424高速基带双点频
};

enum MainBak
{
    Main = 0x0000,
    Bak = 0x0010
};

enum No
{
    First = 0x0000,
    Second = 0x0001
};

// 资源分配数据
struct ResourceRestructuringData
{
    QString resourceRestructuringID;                       // 用于标识每个资源分配唯一ID
    QString name;                                          // 资源分配名称
    SystemWorkMode workMode{ SystemWorkMode::NotDefine };  // 工作模式
    MasterSlave isMaster{ MasterSlave::Master };           // 主备模式
    QString desc;                                          // 资源分配描述
    QMap<int, DeviceID> deviceMap;                         // 基带的资源分配详细情况

    DeviceID getDeviceId(int indexInfo) const
    {
        if (deviceMap.contains(indexInfo))
        {
            return deviceMap[indexInfo];
        }
        else
        {
            return DeviceID();
        }
    }
};

using ResourceRestructuringDataMap = QMap<QString, ResourceRestructuringData>;
using ResourceRestructuringDataList = QList<ResourceRestructuringData>;

#endif  // ResourceRestructuringMESSAGEDEFINE_H
