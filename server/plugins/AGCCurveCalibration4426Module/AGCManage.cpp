#include "AGCManage.h"
#include "AGCCalibrationHelper.h"
#include "AGCCalibrationServer.h"
#include "AGCSerialize.h"
#include "AutoRunTaskMessageDefine.h"
#include "BusinessMacroCommon.h"
#include "ConfigMacroMessageDefine.h"
#include "CppMicroServicesUtility.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "MacroHelper.h"
#include "MessagePublish.h"
#include "PacketHandler.h"
#include "ProcessControlCmdPacker.h"
#include "RedisHelper.h"
#include "SatelliteManagementSerialize.h"
#include "SqlAGC.h"
#include <QString>
#include <QThread>

class AGCManageImpl
{
public:
    AGCCalibrationServer* agcCalibrationServer{ nullptr };
    cppmicroservices::BundleContext context;
    SqlAGC sql;
};

AGCManage::AGCManage(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new AGCManageImpl)
{
    m_impl->context = context;

    auto initDBRes = m_impl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("AGC曲线标定初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    PacketHandler::instance().appendPacker(new ProcessControlCmdPacker());

    registerSubscribe("selectAGCData", &AGCManage::selectAGCData, this);
    registerSubscribe("saveAGCData", &AGCManage::saveAGCData, this);
    registerSubscribe("startAGCData", &AGCManage::startAGCData, this);
    registerSubscribe("endAGCData", &AGCManage::endAGCData, this);
    registerSubscribe("deleteAGCData", &AGCManage::deleteAGCData, this);
    registerSubscribe("finishingItem", &AGCManage::finishingItem, this);
    registerSubscribe("sendToMACB", &AGCManage::sendToMACB, this);
    registerSubscribe("sendToACU", &AGCManage::sendToACU, this);
}

AGCManage::~AGCManage()
{
    if (m_impl->agcCalibrationServer != nullptr)
    {
        m_impl->agcCalibrationServer->stopServer();
        delete m_impl->agcCalibrationServer;
    }
    delete m_impl;
}

void AGCManage::selectAGCData(const ProtocolPack& pack)
{
    auto bak = pack;
    AGCResultQuery agcResultQuery;
    bak.data >> agcResultQuery;
    bak.data.clear();

    auto ans = m_impl->sql.selectAGCItem(agcResultQuery);
    bak.data << ans;
    silenceSendACK(bak);
}
void AGCManage::saveAGCData(const ProtocolPack& pack)
{
    auto bak = pack;
    AGCCalibrationItem item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.saveAGCItem(item);
    bak.data << ans;
    silenceSendACK(bak);

    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("保存AGC曲线成功"), bak.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("保存AGC曲线失败"), bak.userID, MACRSTR(MODULE_NAME));
    }
}
void AGCManage::deleteAGCData(const ProtocolPack& pack)
{
    auto bak = pack;
    AGCCalibrationItem item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.deleteAGCItem(item);
    bak.data << ans;
    silenceSendACK(bak);

    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除AGC曲线成功"), bak.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除AGC曲线失败"), bak.userID, MACRSTR(MODULE_NAME));
    }
}
void AGCManage::finishingItem(const ProtocolPack& pack)
{
    auto bak = pack;
    ClearAGCCalibrationHistry item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.deleteAGCItem(item);
    bak.data << ans;
    silenceSendACK(bak);

    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("清理AGC曲线历史数据成功"), bak.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("清理AGC曲线历史数据失败"), bak.userID, MACRSTR(MODULE_NAME));
    }
}

void AGCManage::startAGCData(const ProtocolPack& pack)
{
    if (m_impl->agcCalibrationServer != nullptr)
    {
        auto bak = pack;
        AGCCalibrationProcessACK ack;
        ack.msg = "当前已有标定正在进行";
        ack.result = AGCCalibrationStatus::Busy;

        bak.data.clear();
        bak.data << ack;
        silenceSend(bak);
        return;
    }
    m_impl->agcCalibrationServer = new AGCCalibrationServer(this);
    connect(m_impl->agcCalibrationServer, &AGCCalibrationServer::sg_testProcessInfo, this, &AGCManage::silenceSendACK);
    connect(m_impl->agcCalibrationServer, &AGCCalibrationServer::sg_end, this, &AGCManage::endCalibrationServer);
    m_impl->agcCalibrationServer->startServer(pack);
}
void AGCManage::endCalibrationServer()
{
    if (m_impl->agcCalibrationServer != nullptr)
    {
        m_impl->agcCalibrationServer->stopServer();
        delete m_impl->agcCalibrationServer;
        m_impl->agcCalibrationServer = nullptr;
    }
}
void AGCManage::endAGCData(const ProtocolPack& /*pack*/) { endCalibrationServer(); }

void AGCManage::sendToMACB(const ProtocolPack& pack)
{
    auto bak = pack;
    AGCCalibrationItem item;
    bak.data >> item;
    bak.data.clear();

    if (!SystemWorkModeHelper::isMeasureContrlWorkMode(item.workMode))
    {
        bak.data << OptionalNotValue(ErrorCode::NotFound, "只有测控模式的才能向测控基带装订");
        silenceSendACK(bak);
        return;
    }

    /* 1.设备组合号与模式有关 与设备有关 所以就算我下发可能基带也不能正确读取 直接去掉 */
    AGCCalibrationtSpin spin = AGCCalibrationtSpin::Left; /* 1左旋 2 右旋 */
    {
        /* 旋向 */
        auto spinResult = AGCCalibrationHelper::getSpin(item.taskCode, item.workMode, item.dotDrequencyNum);
        if (!spinResult)
        {
            bak.data << OptionalNotValue(spinResult.errorCode(), spinResult.msg());
            silenceSendACK(bak);
            return;
        }
        spin = spinResult.value();
    }

    /* 组包命令 */
    CmdRequest cmdRequest;
    {
        cmdRequest.m_modeID = SystemWorkModeHelper::systemWorkModeConverToModeId(item.workMode);  //当前设备模式
        if (item.workMode == STTC)
        {
            cmdRequest.m_cmdID = 0x3B;
        }
        else if (item.workMode == Skuo2 || item.workMode == KaKuo2)
        {
            cmdRequest.m_cmdID = 0x21;
        }
        else if (item.workMode == SKT)
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidData, "扩跳待定....");
            silenceSendACK(bak);
            return;
        }
        else
        {
            bak.data << OptionalNotValue(ErrorCode::NotFound, "只有测控模式的才能向测控基带装订");
            silenceSendACK(bak);
            return;
        }

        QVariant down;
        MacroCommon::getDownFrequency(item.taskCode, item.workMode, item.dotDrequencyNum, down);
        auto downFreq = down.toDouble();

        if (qFuzzyIsNull(downFreq))
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidData, "参数宏数据解析失败:下行频率获取错误");
            silenceSendACK(bak);
            return;
        }

        cmdRequest.m_paramdataMap["EquipComb"] = 0;
        cmdRequest.m_paramdataMap["UplinkFreq"] = downFreq;
        /* 待定这里先给通道1 */
        if (item.workMode != STTC)
        {
            /* 通道 1 2 3 4 先默认给通道1吧 */
            cmdRequest.m_paramdataMap["ChanNumb"] = 1;
            /* 1是遥测 2是测量 先默认给遥测
             * 因为到20210816为止 还没使用过测量
             */
            cmdRequest.m_paramdataMap["Chan"] = 1;
        }

        /* 接收机 1 左 2右 */
        cmdRequest.m_paramdataMap["ReceSelect"] = (spin == AGCCalibrationtSpin::Left ? 1 : 2);
        /* 曲线表点数 */
        cmdRequest.m_paramdataMap["PointCurve"] = item.result.size();

        QList<QPair<QString, QVariant>> tempList;
        int num = 0;
        for (auto& point : item.result)
        {
            tempList << QPair<QString, QVariant>(QString("AGC%1").arg(num), point.agc);
            tempList << QPair<QString, QVariant>(QString("SBF%1").arg(num++), point.sbf);
        }
        cmdRequest.multiParamMap[1] = tempList;
    }
    /* 循环下发 */
    QList<DeviceID> deviceIDList;
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(item.workMode) == SBand)
    {
        deviceIDList << DeviceID(4, 0, 1) << DeviceID(4, 0, 3);
    }
    else
    {
        deviceIDList << DeviceID(4, 0, 2) << DeviceID(4, 0, 4);
    }
    int successCount = 0;
    int errorCount = 0;
    QStringList msg;
    for (auto& deviceID : deviceIDList)
    {
        /* 在线状态 */
        if (!GlobalData::getDeviceOnline(deviceID))
        {
            msg << QString("%1设备离线").arg(GlobalData::getExtensionName(deviceID));
            ++errorCount;
            continue;
        }

        if (!GlobalData::getDeviceSelfControl(deviceID))
        {
            msg << QString("%1设备分控").arg(GlobalData::getExtensionName(deviceID));
            ++errorCount;
            continue;
        }

        cmdRequest.m_systemNumb = deviceID.sysID;   //系统ID
        cmdRequest.m_deviceNumb = deviceID.devID;   //设备ID
        cmdRequest.m_extenNumb = deviceID.extenID;  //分机ID
        auto packResult = AGCCalibrationHelper::cmdPack(cmdRequest);
        if (!packResult)
        {
            bak.data << OptionalNotValue(packResult.errorCode(), packResult.msg());
            silenceSendACK(bak);
            return;
        }

        ProtocolPack sendMACBPack;
        sendMACBPack.desID = "NetWorkManager";
        sendMACBPack.module = true;
        sendMACBPack.operation = "sendDataToDevice";
        sendMACBPack.operationACK.clear();
        sendMACBPack.data = packResult.value();

        auto sendRes = send(sendMACBPack);
        if (!sendRes.success())
        {
            msg << QString("%1设备%2").arg(GlobalData::getExtensionName(deviceID), sendRes.msg());
            ++errorCount;
            continue;
        }
        else
        {
            msg << QString("%1设备AGC曲线已下发").arg(GlobalData::getExtensionName(deviceID));
            ++successCount;
        }
    }

    if (successCount == deviceIDList.size())
    {
        bak.data << OptionalNotValue(ErrorCode::Success, msg.join('\n'));
        silenceSendACK(bak);
    }
    else
    {
        bak.data << OptionalNotValue(ErrorCode::InvalidArgument, msg.join('\n'));
        silenceSendACK(bak);
    }
}

void AGCManage::sendToACU(const ProtocolPack& pack)
{
    auto bak = pack;
    AGCCalibrationItem item;
    bak.data >> item;
    bak.data.clear();

    if (item.workMode == XDS)
    {
        bak.data << OptionalNotValue(ErrorCode::NotFound, "X不能向ACU装订曲线");
        silenceSendACK(bak);
        return;
    }

    /* 1.获取当前主用ACU */
    DeviceID mainACU;
    {
        //获取基带的在线状态和本控状态
        DeviceID acuADeviceID(ACU_A);
        DeviceID acuBDeviceID(ACU_B);

        if (!GlobalData::getDeviceOnline(acuADeviceID) && GlobalData::getDeviceOnline(acuADeviceID))
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidData, "检测到当前ACU都离线,曲线下发失败");
            silenceSendACK(bak);
            return;
        }

        /*
         * 1 在线 表示当前主机
         * 2 离线 表示当前备机
         * 根据4413的情况ACU的主备是绝对的主备 不能两个同时使用的
         */
        auto tempVarA = GlobalData::getReportParamData(acuADeviceID, 0x01, "OnLineMachine");
        auto tempVarB = GlobalData::getReportParamData(acuBDeviceID, 0x01, "OnLineMachine");
        if (tempVarA.toInt() == 1)
        {
            mainACU = acuADeviceID;
        }
        else if (tempVarB.toInt() == 1)
        {
            mainACU = acuBDeviceID;
        }
        else
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidData, "未能找到当前主用ACU,曲线下发失败");
            silenceSendACK(bak);
            return;
        }
        if (!GlobalData::getDeviceSelfControl(mainACU))
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidData, "当前主用ACU分控,曲线下发失败");
            silenceSendACK(bak);
            return;
        }
    }

    /* 组包命令 */
    QByteArray array;
    QString redisFindKey;
    CmdRequest cmdRequest;
    {
        cmdRequest.m_systemNumb = mainACU.sysID;   //系统ID
        cmdRequest.m_deviceNumb = mainACU.devID;   //设备ID
        cmdRequest.m_extenNumb = mainACU.extenID;  //分机ID
        cmdRequest.m_modeID = 0XFFFF;              //当前设备模式
        cmdRequest.m_cmdID = 0x14;

        QVariant down;
        MacroCommon::getDownFrequency(item.taskCode, item.workMode, item.dotDrequencyNum, down);
        auto downFreq = down.toDouble();

        if (qFuzzyIsNull(downFreq))
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidData, "参数宏数据解析失败:下行频率获取错误");
            silenceSendACK(bak);
            return;
        }

        cmdRequest.m_paramdataMap["EquipNumb"] = 0;
        cmdRequest.m_paramdataMap["DownRadFreq"] = downFreq;

        /* 1Ka遥测 2S 3Ka数传 */
        if (SystemWorkModeHelper::systemWorkModeToSystemBand(item.workMode) == SystemBandMode::SBand)
        {
            cmdRequest.m_paramdataMap["ReceSelect"] = 2;
        }
        else if (item.workMode == SystemWorkMode::KaKuo2)
        {
            cmdRequest.m_paramdataMap["ReceSelect"] = 1;
        }
        else if (item.workMode == SystemWorkMode::KaDS || item.workMode == SystemWorkMode::KaGS)
        {
            cmdRequest.m_paramdataMap["ReceSelect"] = 3;
        }
        else
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidData, "当前工作模式错误,无法配ACU模式");
            silenceSendACK(bak);
            return;
        }

        /* 曲线表点数 */
        cmdRequest.m_paramdataMap["PointCurve"] = item.result.size();

        /* 这里根据13的情况 ACU的点数是固定数量60组数据 所以需要先凑单  */
        QList<QPair<QString, QVariant>> tempList;
        int num = 1;
        for (auto& point : item.result)
        {
            tempList << QPair<QString, QVariant>(QString("AGC%1").arg(num), point.agc);
            tempList << QPair<QString, QVariant>(QString("SBF%1").arg(num++), point.sbf);
        }
        for (int i = num; i <= 60; ++i)
        {
            tempList << QPair<QString, QVariant>(QString("AGC%1").arg(i), 0.0);
            tempList << QPair<QString, QVariant>(QString("SBF%1").arg(i), 0.0);
        }
        cmdRequest.multiParamMap[1] = tempList;

        auto packResult = AGCCalibrationHelper::cmdPack(cmdRequest);
        if (!packResult)
        {
            bak.data << OptionalNotValue(packResult.errorCode(), packResult.msg());
            silenceSendACK(bak);
            return;
        }
        array = packResult.value();
        redisFindKey = DeviceProcessHelper::getCmdResponseRedisKey(mainACU, cmdRequest.m_cmdID);
    }

    /* 发送命令 */
    {
        ProtocolPack sendMACBPack;
        sendMACBPack.desID = "NetWorkManager";
        sendMACBPack.module = true;
        sendMACBPack.operation = "sendDataToDevice";
        sendMACBPack.operationACK.clear();
        sendMACBPack.data = array;
        auto sendRes = send(sendMACBPack);
        if (!sendRes.success())
        {
            auto msg = QString("%1设备%2").arg(GlobalData::getExtensionName(mainACU), sendRes.msg());
            bak.data << OptionalNotValue(ErrorCode::InvalidData, msg);
            silenceSendACK(bak);
        }
        else
        {
            auto msg = QString("%1设备AGC曲线已下发").arg(GlobalData::getExtensionName(mainACU));
            bak.data << OptionalNotValue(ErrorCode::InvalidData, msg);
            silenceSendACK(bak);
        }
    }
}
