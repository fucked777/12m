#include "HPCmd.h"

#include "CustomPacketMessageDefine.h"
#include "CustomPacketMessageSerialize.h"
#include "DeviceProcessMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "ProtocolXmlTypeDefine.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include <QDebug>
#include <QMessageBox>

void HPCmd::Control(const int deviceId, const int modeId, const int cmdIdOrUnitId, QVariantMap params, HPCmd::CommandType type, QByteArray& data,
                    QWidget* widget)
{
    if (widget == nullptr)
        return;

    DeviceID devID(deviceId);
    auto deviceName = GlobalData::getDeviceName(devID.sysID, devID.devID);
    QString erroMsg;

    if (!ServiceCheck::serviceOnline())
    {
        erroMsg = QString("当前监控服务器离线");
        QMessageBox::warning(widget, QString("提示"), erroMsg, QString("确定"));
        SystemLogPublish::errorMsg(erroMsg);
        return;
    }

    //先判断设备是否在线
    if (!GlobalData::getDeviceOnline(deviceId))
    {
        erroMsg = QString("%1设备离线,命令下发失败").arg(deviceName);
        SystemLogPublish::errorMsg(erroMsg);
        QMessageBox::warning(widget, QString("提示"), erroMsg, QString("确定"));
        return;
    }
    //在判断设备是否是本分控
    bool isRemoteCtrl = GlobalData::getDeviceSelfControl(deviceId);
    if (!isRemoteCtrl)
    {
        erroMsg = QString("%1设备为分控状态,命令下发失败").arg(deviceName);
        SystemLogPublish::warningMsg(erroMsg);
        QMessageBox::warning(widget, QString("提示"), erroMsg, QString("确定"));
        return;
    }

    if (type == UnitParameter)
    {
        UnitParamRequest unitParamRequest;

        unitParamRequest.m_systemNumb = devID.sysID;
        unitParamRequest.m_deviceNumb = devID.devID;
        unitParamRequest.m_extenNumb = devID.extenID;
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
        cmdRequest.m_systemNumb = devID.sysID;
        cmdRequest.m_deviceNumb = devID.devID;
        cmdRequest.m_extenNumb = devID.extenID;
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
