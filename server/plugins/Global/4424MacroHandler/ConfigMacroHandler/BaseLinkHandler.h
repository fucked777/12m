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
    bool setLightNetConfigCKJD_CKQD(const DeviceID& ckjdDeviceID, const DeviceID& ckqdDeviceID);

    // 设置低速基带和测控前端光口网络
    bool setLightNetConfigDSJD_CKQD(const DeviceID& dsjdDeviceID, const DeviceID& ckqdDeviceID);

    // 执行链路命令 根据动态设置参数后的配置宏进行组装链路
    void execLinkCommand();
};

#endif  // BASELINKHANDLER_H
