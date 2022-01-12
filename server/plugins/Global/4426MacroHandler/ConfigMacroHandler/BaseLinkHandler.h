#ifndef BASELINKHANDLER_H
#define BASELINKHANDLER_H

#include "BusinessMacroCommon.h"
#include "PlanRunMessageDefine.h"
#include "ProtocolXmlTypeDefine.h"

#include "BaseHandler.h"

class BaseLinkHandler : public BaseHandler
{
    Q_OBJECT
public:
    explicit BaseLinkHandler(QObject* parent = nullptr);

    virtual LinkType getLinkType() = 0;

protected:
    // 设置测控基带和测控前端光口网络
    // bool setLightNetConfigCKJD_CKQD(const DeviceID& ckjdDeviceID, const DeviceID& ckqdDeviceID, bool isDoubleMode = false);
    bool setLightNetConfigCKJD_CKQD(const CKJDControlInfo& info, const DeviceID& ckqdDeviceID, bool isDoubleMode = false);

    // 设置低速基带和测控前端光口网络
    bool setLightNetConfigDSJD_CKQD(const DeviceID& dsjdDeviceID, const DeviceID& ckqdDeviceID);

    // 执行链路命令 根据动态设置参数后的配置宏进行组装链路
    void execLinkCommand();
    void execLinkCommand(const QSet<SystemWorkMode>&);

    // 测控前端切换
    enum class CKQDMode
    {
        SZHBH,  // 数字化闭环
        RW,     // 任务
    };

    bool ckqdLink(CKQDMode mode);

protected:
    MasterSlaveDeviceIDInfo m_ckqdDeviceInfo;
};

#endif  // BASELINKHANDLER_H
