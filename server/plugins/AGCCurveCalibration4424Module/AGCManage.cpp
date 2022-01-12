#include "AGCManage.h"
#include "AGCCalibrationHelper.h"
#include "AGCCalibrationServer.h"
#include "AGCSerialize.h"
#include "AutoRunTaskMessageDefine.h"
#include "BusinessMacroCommon.h"
#include "ConfigMacroMessageDefine.h"
#include "CppMicroServicesUtility.h"
#include "DeviceProcessMessageSerialize.h"
#include "EquipmentCombinationHelper.h"
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
    ControlCmdResponse curResponce; /* 命令响应 */
    DeviceID curDevID;
    int curCMDID{ -1 };
    bool isRecvResponce{ false };
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
    connect(this, &AGCManage::sg_deviceCMDResult, this, &AGCManage::acceptDeviceCMDResponceMessage);

    registerSubscribe("selectAGCData", &AGCManage::selectAGCData, this);
    registerSubscribe("saveAGCData", &AGCManage::saveAGCData, this);
    registerSubscribe("startAGCData", &AGCManage::startAGCData, this);
    registerSubscribe("endAGCData", &AGCManage::endAGCData, this);
    registerSubscribe("deleteAGCData", &AGCManage::deleteAGCData, this);
    registerSubscribe("finishingItem", &AGCManage::finishingItem, this);
    registerSubscribe("sendToMACB", &AGCManage::sendToMACB, this);
    registerSubscribe("sendToACU", &AGCManage::sendToACU, this);

    registerSubscribe("AcceptDeviceCMDResultMessage", &AGCManage::sg_deviceCMDResult, this);
    registerSubscribe("AcceptDeviceCMDResponceMessage", &AGCManage::sg_deviceCMDResponce, this);
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
void AGCManage::acceptDeviceCMDResponceMessage(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data >> m_impl->curResponce;
    if (m_impl->curDevID == m_impl->curResponce.deviceID &&  //
        m_impl->curCMDID == m_impl->curResponce.cmdId)
    {
        m_impl->isRecvResponce = true;
    }
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

    /* 1.获取当前旋向,配置宏,跟踪基带,测控基带,因为这里设备组合号创建要用到 */
    ConfigMacroData configMacroData;
    DeviceID gzjd;
    DeviceID ckjd;
    int spin = -1; /* 1左旋 2 右旋 */
    {
        /* 旋向 */
        auto spinResult = AGCCalibrationHelper::getCurSpin(item.taskCode, item.workMode, item.dotDrequencyNum);
        if (!spinResult)
        {
            bak.data << OptionalNotValue(spinResult.errorCode(), spinResult.msg());
            silenceSendACK(bak);
            return;
        }
        spin = spinResult.value();

        /* 配置宏 */
        auto getDataResult = GlobalData::getConfigMacroData(item.workMode, configMacroData, MasterSlave::Master);
        if (!getDataResult)
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidArgument, "获取配置宏数据错误");
            silenceSendACK(bak);
            return;
        }

        bool getJDResult = configMacroData.getCKJDDeviceID(item.workMode, ckjd, 1);
        if (!getJDResult)
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidArgument, "获取当前使用测控基带错误");
            silenceSendACK(bak);
            return;
        }
        /* 跟踪基带 */
        DeviceID gzqd;
        getJDResult = configMacroData.getGZQDDeviceID(item.workMode, gzqd);
        if (!getJDResult)
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidArgument, "获取当前使用跟踪基带错误");
            silenceSendACK(bak);
            return;
        }

        gzjd.sysID = 0x04;
        gzjd.devID = 0x02;
        gzjd.extenID = gzqd.extenID;
    }

    /* 组包命令 */
    QByteArray array;
    {
        CmdRequest cmdRequest;
        cmdRequest.m_systemNumb = ckjd.sysID;                                                     //系统ID
        cmdRequest.m_deviceNumb = ckjd.devID;                                                     //设备ID
        cmdRequest.m_extenNumb = ckjd.extenID;                                                    //分机ID
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
        m_impl->curDevID = ckjd;
        m_impl->curCMDID = cmdRequest.m_cmdID;

        quint64 devNum = 0;
        auto devNumResult = EquipmentCombinationHelper::getCKDevNumber(item.workMode, configMacroData, ckjd, gzjd, devNum);
        if (!devNumResult)
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidData, "获取当前设备组合号错误");
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

        cmdRequest.m_paramdataMap["EquipComb"] = devNum;
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
        cmdRequest.m_paramdataMap["ReceSelect"] = spin;
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

        auto packResult = AGCCalibrationHelper::cmdPack(cmdRequest);
        if (!packResult)
        {
            bak.data << OptionalNotValue(packResult.errorCode(), packResult.msg());
            silenceSendACK(bak);
            return;
        }
        array = packResult.value();
    }

    /* 发送命令 */
    {
        ProtocolPack sendMACBPack;
        sendMACBPack.desID = "NetWorkManager";
        sendMACBPack.module = true;
        sendMACBPack.operation = "sendDataToDevice";
        sendMACBPack.operationACK.clear();
        sendMACBPack.data = array;
        silenceSend(sendMACBPack);
    }

    /* 等待设备应答 */
    {
        int ttl = 20;
        m_impl->isRecvResponce = false;

        while (ttl > 0)
        {
            QThread::sleep(1);
            QCoreApplication::processEvents();
            --ttl;

            /* 判断是否接收响应成功 */
            if (!m_impl->isRecvResponce)
            {
                continue;
            }

            /* 判断是否执行成功 */
            if (ControlCmdResponseType::Success == m_impl->curResponce.responseResult)
            {
                bak.data << OptionalNotValue();
            }
            else
            {
                auto cmdExecResultDesc = DevProtocolEnumHelper::controlCmdResponseToDescStr(m_impl->curResponce.responseResult);
                bak.data << OptionalNotValue(ErrorCode::InstructionExecutionError, cmdExecResultDesc);
            }
            silenceSendACK(bak);
            return;
        }

        // 超时未响应
        bak.data << OptionalNotValue(ErrorCode::Timeout, "设备超时未响应");
        silenceSendACK(bak);
        return;
    }
}

void AGCManage::sendToACU(const ProtocolPack& pack)
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

    /* 1.获取当前主用ACU */
    DeviceID mainACU;
    {
        //获取基带的在线状态和本控状态
        DeviceID acuADeviceID(ACU_A);
        DeviceID acuBDeviceID(ACU_B);

        if (!GlobalData::getDeviceOnline(acuADeviceID) && GlobalData::getDeviceOnline(acuADeviceID))
        {
            bak.data << OptionalNotValue(ErrorCode::DevOffline, "当前ACU全部离线");
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
            bak.data << OptionalNotValue(ErrorCode::UnknownError, "未找到当前在线ACU");
            silenceSendACK(bak);
            return;
        }
    }

    /* 2.获取当前配置宏,跟踪基带,因为这里设备组合号创建要用到 */
    ConfigMacroData configMacroData;
    DeviceID gzjd;
    DeviceID ckjd;
    {
        /* 配置宏 */
        auto getDataResult = GlobalData::getConfigMacroData(item.workMode, configMacroData, MasterSlave::Master);
        if (!getDataResult)
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidArgument, "获取配置宏数据错误");
            silenceSendACK(bak);
            return;
        }

        bool getJDResult = configMacroData.getCKJDDeviceID(item.workMode, ckjd, 1);
        if (!getJDResult)
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidArgument, "获取当前使用测控基带错误");
            silenceSendACK(bak);
            return;
        }
        /* 跟踪基带 */
        DeviceID gzqd;
        getJDResult = configMacroData.getGZQDDeviceID(item.workMode, gzqd);
        if (!getJDResult)
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidArgument, "获取当前使用跟踪基带错误");
            silenceSendACK(bak);
            return;
        }

        gzjd.sysID = 0x04;
        gzjd.devID = 0x02;
        gzjd.extenID = gzqd.extenID;
    }

    /* 组包命令 */
    QByteArray array;
    {
        CmdRequest cmdRequest;
        cmdRequest.m_systemNumb = mainACU.sysID;   //系统ID
        cmdRequest.m_deviceNumb = mainACU.devID;   //设备ID
        cmdRequest.m_extenNumb = mainACU.extenID;  //分机ID
        cmdRequest.m_modeID = 0XFFFF;              //当前设备模式
        cmdRequest.m_cmdID = 0x14;
        m_impl->curDevID = mainACU;
        m_impl->curCMDID = cmdRequest.m_cmdID;

        quint64 devNum = 0;
        auto devNumResult = EquipmentCombinationHelper::getCKDevNumber(item.workMode, configMacroData, ckjd, gzjd, devNum);
        if (!devNumResult)
        {
            bak.data << OptionalNotValue(ErrorCode::InvalidData, "获取当前设备组合号错误");
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

        cmdRequest.m_paramdataMap["EquipNumb"] = devNum;
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
    }

    /* 发送命令 */
    {
        ProtocolPack sendMACBPack;
        sendMACBPack.desID = "NetWorkManager";
        sendMACBPack.module = true;
        sendMACBPack.operation = "sendDataToDevice";
        sendMACBPack.operationACK.clear();
        sendMACBPack.data = array;
        silenceSend(sendMACBPack);
    }

    /* 等待设备应答 */
    {
        int ttl = 20;
        m_impl->isRecvResponce = false;

        while (ttl > 0)
        {
            QThread::sleep(1);
            QCoreApplication::processEvents();
            --ttl;

            /* 判断是否接收响应成功 */
            if (!m_impl->isRecvResponce)
            {
                continue;
            }

            /* 判断是否执行成功 */
            if (ControlCmdResponseType::Success == m_impl->curResponce.responseResult)
            {
                bak.data << OptionalNotValue();
            }
            else
            {
                auto cmdExecResultDesc = DevProtocolEnumHelper::controlCmdResponseToDescStr(m_impl->curResponce.responseResult);
                bak.data << OptionalNotValue(ErrorCode::InstructionExecutionError, cmdExecResultDesc);
            }
            silenceSendACK(bak);
            return;
        }

        // 超时未响应
        bak.data << OptionalNotValue(ErrorCode::Timeout, "设备超时未响应");
        silenceSendACK(bak);
        return;
    }
}
