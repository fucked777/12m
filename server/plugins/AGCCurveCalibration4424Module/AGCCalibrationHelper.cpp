#include "AGCCalibrationHelper.h"
#include "GlobalData.h"
#include "LocalCommunicationAddr.h"
#include "PacketHandler.h"
#include "SatelliteManagementSerialize.h"

Optional<int> AGCCalibrationHelper::getCurSpin(const QString& taskCode, SystemWorkMode workMode, int dpNum)
{
    using ResType = Optional<int>;
    /* 旋向 */
    SystemOrientation orientation = SystemOrientation::Unkonwn;
    SatelliteManagementData satelliteTemp;
    if (!GlobalData::getSatelliteManagementData(taskCode, satelliteTemp))
    {
        return ResType(ErrorCode::InvalidArgument, "获取旋向错误:卫星数据获取错误");
    }
    auto getDataResult = satelliteTemp.getSReceivPolar(workMode, dpNum, orientation);
    if (!getDataResult)
    {
        return ResType(ErrorCode::InvalidData, QString("获取当前卫星旋向错误:%1").arg(satelliteTemp.m_satelliteName));
    }

    if (orientation == SystemOrientation::LCircumflex ||  //
        orientation == SystemOrientation::LRCircumflex)
    {
        return ResType(1);
    }
    else if (orientation == SystemOrientation::RCircumflex)
    {
        return ResType(2);
    }

    return ResType(ErrorCode::InvalidData, QString("获取当前卫星旋向错误:%1").arg(satelliteTemp.m_satelliteName));
}

Optional<QByteArray> AGCCalibrationHelper::cmdPack(const CmdRequest& cmdRequest)
{
    using ResType = Optional<QByteArray>;
    ProcessControlCmdMessage controlCtrlCmdMsg;
    controlCtrlCmdMsg.mode = cmdRequest.m_modeID;
    controlCtrlCmdMsg.cmdId = cmdRequest.m_cmdID;
    controlCtrlCmdMsg.settingParamMap = cmdRequest.m_paramdataMap;
    controlCtrlCmdMsg.multiParamMap = cmdRequest.multiParamMap;

    MessageAddress sourceAddr = LocalCommunicationAddr::devAddrMessage();

    MessageAddress targetAddr = sourceAddr;
    targetAddr.systemNumb = cmdRequest.m_systemNumb;
    targetAddr.deviceNumb = cmdRequest.m_deviceNumb;
    targetAddr.extenNumb = cmdRequest.m_extenNumb;

    Version version;
    version.mainVersion = 0x2;
    version.subVersionOne = 0x1;
    version.subVersionTwo = 0x3;

    PackMessage packMsg;
    packMsg.header.msgType = DevMsgType::ProcessControlCmd;
    packMsg.header.sourceAddr = sourceAddr;
    packMsg.header.targetAddr = targetAddr;
    packMsg.header.version = version;
    packMsg.processCtrlCmdMsg = controlCtrlCmdMsg;

    QByteArray packData;
    QString erroMsg;
    bool flag = PacketHandler::instance().pack(packMsg, packData, erroMsg);
    if (!flag)
    {
        auto msg = QString("组包出错：%1").arg(erroMsg);
        return ResType(ErrorCode::InvalidArgument, msg);
    }
    return ResType(packData);
}
