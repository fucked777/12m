#include "PackDataBuilder.h"
#include "CustomPacketMessageDefine.h"
#include "DeviceProcessMessageDefine.h"
#include "GlobalData.h"
#include "ProtocolXmlTypeDefine.h"

Optional<QMap<QString, QVariant>> PackDataBuilder::cmdToVarMap(int sysNum, int devNum, int extNum, int modeNum, int cmdID)
{
    using ResType = Optional<QMap<QString, QVariant>>;
    auto modeCtrl = GlobalData::getMode(sysNum, devNum, extNum, modeNum);
    if (modeCtrl.cmdMap.isEmpty())
    {
        auto msg = QString("查找设备失败:0x%1%2%3").arg(sysNum, 0, 16).arg(devNum, 0, 16).arg(extNum, 0, 16);
        return ResType(ErrorCode::NotFound, msg);
    }

    auto find = modeCtrl.cmdMap.find(cmdID);
    if (find == modeCtrl.cmdMap.end())
    {
        auto msg = QString("0x%1%2%3查找命令失败:%4").arg(sysNum, 0, 16).arg(devNum, 0, 16).arg(extNum, 0, 16).arg(modeNum, 0, 16);
        return ResType(ErrorCode::NotFound, msg);
    }
    QMap<QString, QVariant> result;
    auto& cmdAttr = find.value();
    for (auto& item : cmdAttr.requestList)
    {
        result[item.id] = item.initValue;
    }

    return ResType(result);
}

Optional<QMap<QString, QVariant>> PackDataBuilder::unitToVarMap(int sysNum, int devNum, int extNum, int modeNum, int unitID)
{
    using ResType = Optional<QMap<QString, QVariant>>;
    auto statusReportMsg = GlobalData::getExtenStatusReportData(sysNum, devNum, extNum);
    if (statusReportMsg.unitReportMsgMap.isEmpty())
    {
        auto msg = QString("0x%1%2%3查找单元参数命令失败:%4").arg(sysNum, 0, 16).arg(devNum, 0, 16).arg(extNum, 0, 16).arg(unitID, 0, 16);
        return ResType(ErrorCode::NotFound, msg);
    }
    auto unitReportMsgMap = statusReportMsg.unitReportMsgMap;
    QMap<QString, QVariant> result = unitReportMsgMap[unitID].paramMap;

    return ResType(result);
}

Optional<CmdRequest> PackDataBuilder::cmdToCmdRequest(int sysNum, int devNum, int extNum, int modeNum, int cmdID,
                                                      const QMap<QString, QVariant>& replace)
{
    using ResType = Optional<CmdRequest>;
    auto resMap = PackDataBuilder::cmdToVarMap(sysNum, devNum, extNum, modeNum, cmdID);
    if (!resMap.success())
    {
        return ResType(resMap.errorCode(), resMap.msg());
    }
    CmdRequest cmdRequest;
    cmdRequest.m_systemNumb = sysNum;
    cmdRequest.m_deviceNumb = devNum;
    cmdRequest.m_extenNumb = extNum;
    cmdRequest.m_modeID = modeNum;
    cmdRequest.m_cmdID = cmdID;
    cmdRequest.m_paramdataMap = resMap.value();
    for (auto iter = replace.begin(); iter != replace.end(); ++iter)
    {
        cmdRequest.m_paramdataMap[iter.key()] = iter.value();
    }
    return ResType(cmdRequest);
}

Optional<UnitParamRequest> PackDataBuilder::unitToCmdRequest(int sysNum, int devNum, int extNum, int modeNum, int unitID,
                                                             const QMap<QString, QVariant>& replace)
{
    using ResType = Optional<UnitParamRequest>;
    auto resMap = PackDataBuilder::unitToVarMap(sysNum, devNum, extNum, modeNum, unitID);
    if (!resMap.success())
    {
        return ResType(resMap.errorCode(), resMap.msg());
    }

    UnitParamRequest cmdRequest;
    cmdRequest.m_systemNumb = sysNum;
    cmdRequest.m_deviceNumb = devNum;
    cmdRequest.m_extenNumb = extNum;
    cmdRequest.m_modeID = modeNum;
    cmdRequest.m_unitID = unitID;
    //不需要再把单元里面所有的参数都取出来了,服务端会自动补全
    //    cmdRequest.m_paramdataMap = resMap.value();
    for (auto iter = replace.begin(); iter != replace.end(); ++iter)
    {
        cmdRequest.m_paramdataMap[iter.key()] = iter.value();
    }
    return ResType(cmdRequest);
}
