#ifndef ZWZXREMOTECONTROLCMDHELPER_H
#define ZWZXREMOTECONTROLCMDHELPER_H
#include "ProtocolXmlTypeDefine.h"
#include "StationNetCenterType.h"
#include "Utility.h"

class MeasurementContrlCmd;
class ZwzxRemoteControlCmdHelper
{
public:
    /* 上行载波输出,当前如果已经输出则不作操作,否则打开输出
     * 这里考虑的情况是,重复发送载波输出开可能会让载波输出停了再开,导致链路出错
     */
    // static bool upCarrOutputOn(ZwzxCmdHelperInfo& info, int devModeID, MeasurementContrlCmd& cmd, QString& errMsg);
    /* 获取测控基带数据 */
    static Optional<QVariant> getMACBKPValue(const DeviceID& deviceID, SystemWorkMode workMode, int target, int unit, const QString& key);
    static Optional<QVariant> getMACBBZValue(const DeviceID& deviceID, SystemWorkMode workMode, int unit, const QString& key);
    /* 获取测控基带当前目标数 */
    static Optional<int> getMACBCurTargetNum(const DeviceID& deviceID, SystemWorkMode workMode);
    /* 获取测控基带的输出电平 */
    static Optional<int> getMACBOutputLev(const DeviceID& deviceID, SystemWorkMode workMode);
    /* 获取测控基带的加调状态 */
    static Optional<bool> getMACBKPJT(const DeviceID& deviceID, SystemWorkMode workMode, int target, const QString& key);
    static Optional<bool> getMACBBZJT(const DeviceID& deviceID, SystemWorkMode workMode, const QString& key);
    /* 扩频模式 获取遥测伪码加调 */
    static Optional<bool> getMACBKPYKWMJT(const DeviceID& deviceID, SystemWorkMode workMode, int target)
    {
        return getMACBKPJT(deviceID, workMode, target, "YC_PNCodeAdd");
    }
    /* 扩频模式 获取遥测数据加调 */
    static Optional<bool> getMACBKPYKSJJT(const DeviceID& deviceID, SystemWorkMode workMode, int target)
    {
        return getMACBKPJT(deviceID, workMode, target, "YC_DataTuning");
    }
    /* 扩频模式 获取测量伪码加调 */
    static Optional<bool> getMACBKPCLWMJT(const DeviceID& deviceID, SystemWorkMode workMode, int target)
    {
        return getMACBKPJT(deviceID, workMode, target, "CL_PNCodeAdd");
    }
    /* 扩频模式 获取测量数据加调 */
    static Optional<bool> getMACBKPCLSJJT(const DeviceID& deviceID, SystemWorkMode workMode, int target)
    {
        return getMACBKPJT(deviceID, workMode, target, "CL_DataTuning");
    }
    /* 标准模式 获取遥控加调 */
    static Optional<bool> getMACBBZYKJT(const DeviceID& deviceID, SystemWorkMode workMode)
    {
        return getMACBBZJT(deviceID, workMode, "RemotePlusControl");
    }
    /* 标准模式 获取测距加调 */
    static Optional<bool> getMACBBZCJJT(const DeviceID& deviceID, SystemWorkMode workMode) { return getMACBBZJT(deviceID, workMode, "RangPlusCtrl"); }

    /* 获取测控基带当前上行载波输出状态 */
    static Optional<bool> getMACBUpCarrOutput(const DeviceID& deviceID, SystemWorkMode workMode);
    /* 获取测控基带通道输出控制 */
    static Optional<bool> getMACBYKTDSCKZ(const DeviceID& deviceID, SystemWorkMode workMode, int target);
    static Optional<bool> getMACBCLTDSCKZ(const DeviceID& deviceID, SystemWorkMode workMode, int target);
    /* 获取在线的DTE */
    static Optional<DeviceID> getOnlineDTE();
};

#endif  // ZWZXREMOTECONTROLCMDHELPER_H
