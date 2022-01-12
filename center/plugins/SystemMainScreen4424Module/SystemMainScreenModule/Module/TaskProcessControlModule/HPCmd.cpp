#include "HPCmd.h"

#include "CustomPacketMessageDefine.h"
#include "CustomPacketMessageSerialize.h"
#include "DeviceProcessMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "ProtocolXmlTypeDefine.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include <QDebug>
#include <QMessageBox>

void HPCmd::Control(const int deviceId, const int modeId, const int cmdIdOrUnitId, QVariantMap params, HPCmd::CommandType type, QByteArray& data)
{
    SERVICEONLINECHECKNOPARENT();  //判断服务器是否离线

    //先判断设备是否在线
    if (!GlobalData::getDeviceOnline(deviceId))
    {
        QMessageBox::warning(nullptr, QString("提示"), QString("当前设备离线,命令下发失败"), QString("确定"));
        return;
    }
    //在判断设备是否是本分控
    bool isRemoteCtrl = GlobalData::getDeviceSelfControl(deviceId);
    if (!isRemoteCtrl)
    {
        QMessageBox::warning(nullptr, QString("提示"), QString("当前设备为分控状态,命令下发失败"), QString("确定"));
        return;
    }

    DeviceID id;
    int tmp_deviceId = deviceId;
    id << tmp_deviceId;
    if (type == UnitParameter)
    {
        UnitParamRequest unitParamRequest;

        unitParamRequest.m_systemNumb = id.sysID;
        unitParamRequest.m_deviceNumb = id.devID;
        unitParamRequest.m_extenNumb = id.extenID;
        unitParamRequest.m_modeID = modeId;
        unitParamRequest.m_unitID = cmdIdOrUnitId;
        unitParamRequest.m_paramdataMap = params;

        JsonWriter writer;
        writer& unitParamRequest;
        RedisHelper::getInstance().setData("单元发送数据", writer.GetQString());
        data = writer.GetByteArray();
        // qDebug() << writer.GetString();
    }
    if (type == ProcessControl)
    {
        CmdRequest cmdRequest;
        cmdRequest.m_systemNumb = id.sysID;
        cmdRequest.m_deviceNumb = id.devID;
        cmdRequest.m_extenNumb = id.extenID;
        cmdRequest.m_modeID = modeId;
        cmdRequest.m_cmdID = cmdIdOrUnitId;
        cmdRequest.m_paramdataMap = params;

        JsonWriter writer;
        writer& cmdRequest;
        RedisHelper::getInstance().setData("过程控制命令发送数据", writer.GetQString());
        data = writer.GetByteArray();
        // qDebug() << writer.GetString();
    }
}
