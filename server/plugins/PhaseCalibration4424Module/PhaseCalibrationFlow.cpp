//#include "PhaseCalibrationFlow.h"
//#include "BusinessMacroCommon.h"
//#include "DevProtocolSerialize.h"
//#include "DeviceProcessMessageSerialize.h"
//#include "GlobalData.h"
//#include "INetMsg.h"
//#include "PhaseCalibration.h"
//#include "PhaseCalibrationSerialize.h"
//#include "RedisHelper.h"
//#include "SingleCommandHelper.h"
//#include "SqlPhaseCalibration.h"

//#include <QCoreApplication>
//#include <QThread>
//#include <QUuid>

// class PhaseCalibrationFlowImpl
//{
// public:
//    PhaseCalibration* parent;
//    CmdResult curResult;
//    int calibrationTimeout{ 40 };
//    bool isRecvCmdResult{ false };

//    PhaseCalibrationFlowImpl(PhaseCalibration* parent_)
//        : parent(parent_)
//    {
//    }

//    void resetRecv()
//    {
//        curResult = CmdResult();
//        isRecvCmdResult = false;
//    }
//};

// PhaseCalibrationFlow::PhaseCalibrationFlow(PhaseCalibration* dc)
//    : QObject(nullptr)
//    , m_impl(new PhaseCalibrationFlowImpl(dc))
//{
//    connect(m_impl->parent, &PhaseCalibration::sg_deviceCMDResult, this, &PhaseCalibrationFlow::acceptDeviceCMDResultMessage);
//    /* 获取校相超时时间 */
//    m_impl->calibrationTimeout = GlobalData::getAutorunPolicyData("PCaT", 40).toInt();
//}
// PhaseCalibrationFlow::~PhaseCalibrationFlow() { delete m_impl; }
// void PhaseCalibrationFlow::sendByteArrayToDevice(const QByteArray& data)
//{
//    ProtocolPack protocal;
//    protocal.srcID = "";
//    protocal.desID = "NetWorkManager";
//    protocal.module = true;
//    protocal.operation = "sendDataToDevice";
//    protocal.data = data;
//    m_impl->parent->silenceSend(protocal);
//}

// void PhaseCalibrationFlow::pushLog(const QString& msg) { RedisHelper::getInstance().publish("PhaseCalibrationLog", msg); }

// bool PhaseCalibrationFlow::waitExecSuccess(const PackCommand& packCommand, uint ttl)
//{
//    if (!packCommand.errorMsg.isEmpty())
//    {
//        pushLog(packCommand.errorMsg);
//        return false;
//    }

//    if (packCommand.data.isEmpty())
//    {
//        pushLog("系统组包失败，请检查配置");
//        return false;
//    }

//    // 发送数据
//    sendByteArrayToDevice(packCommand.data);

//    // 写入日志当前命令执行的操作
//    QString currentCmdOperatorInfo;
//    if (packCommand.operatorInfo.isEmpty())
//    {
//        currentCmdOperatorInfo = QString("%1：%2").arg(GlobalData::getExtensionName(packCommand.deviceID), packCommand.getSetParamDescValue());
//    }
//    else
//    {
//        currentCmdOperatorInfo = packCommand.operatorInfo;
//    }
//    pushLog(currentCmdOperatorInfo);

//    while (ttl > 0)
//    {
//        // 获取命令执行结果
//        QString cmdRespnceStr;
//        if (RedisHelper::getInstance().getData("ControlCmdResponse", cmdRespnceStr))
//        {
//            ControlCmdResponse cmdRespnce;
//            cmdRespnceStr >> cmdRespnce;

//            // 如果是当前发送的命令响应
//            if (cmdRespnce.isValid() && cmdRespnce.deviceID == packCommand.deviceID && cmdRespnce.cmdId == packCommand.cmdId &&
//                cmdRespnce.cmdType == packCommand.cmdType)
//            {
//                auto cmdExecResultDesc = DevProtocolEnumHelper::controlCmdResponseToDescStr(cmdRespnce.responseResult);
//                QString message = QString("%1 (%2)").arg(currentCmdOperatorInfo).arg(cmdExecResultDesc);

//                if (cmdRespnce.responseResult == ControlCmdResponseType::Success)
//                {
//                    pushLog(message);
//                }
//                else
//                {
//                    pushLog(message);
//                }
//                // 接收到响应就直接返回
//                return true;
//            }
//        }

//        QThread::sleep(1);
//        QCoreApplication::processEvents();
//        --ttl;
//    }
//    // 超时未响应
//    pushLog(QString("%1，超时未响应").arg(currentCmdOperatorInfo));
//    return false;
//}
// bool PhaseCalibrationFlow::waitResultSuccess(const QString& operatorInfo, int ttl, bool (*checkFunc)(const CmdResult&))
//{
//    m_impl->resetRecv();
//    while (ttl > 0)
//    {
//        QThread::sleep(1);
//        QCoreApplication::processEvents();
//        --ttl;
//        /* 为true,代表已经接收到结果 */
//        if (!m_impl->isRecvCmdResult)
//        {
//            continue;
//        }
//        if (checkFunc(m_impl->curResult))
//        {
//            return true;
//        }
//        m_impl->resetRecv();
//    }
//    // 超时未响应
//    pushLog(QString("接收%1结果超时").arg(operatorInfo));
//    return false;
//}
// void PhaseCalibrationFlow::acceptDeviceCMDResultMessage(const ProtocolPack& pack)
//{
//    auto bak = pack;
//    bak.data >> m_impl->curResult;
//    m_impl->isRecvCmdResult = true;
//}
// void PhaseCalibrationFlow::waitDCResult(const PhaseCalibrationItem& item)
//{
//    /* 等待校零结果 */
//    if (!waitResultSuccess("校零", m_impl->calibrationTimeout))
//    {
//        return;
//    }

//    auto bak = item;
//    bak.distZeroMean = m_impl->curResult.m_paramdataMap.value("DistZeroMean").toDouble(); /* 距离零值均值 */
//    bak.distZeroVar = m_impl->curResult.m_paramdataMap.value("DistZeroVar").toDouble();   /* 距离零值方差 */
//    bak.id = Utility::createTimeUuid();
//    auto msg = QString("校零完成:\n距离零值均值:bak.distZeroMean\n距离零值方差:%2").arg(bak.distZeroMean, 0, 'f').arg(bak.distZeroVar, 0, 'f');
//    pushLog(msg);

//    /* 存数据 */
//    auto insertResult = SqlPhaseCalibration::insertItem(bak);
//    if (!insertResult)
//    {
//        pushLog("校零数据保存数据库成功");
//    }
//    else
//    {
//        msg = QString("校零数据保存数据库失败:%1").arg(insertResult.msg());
//        pushLog(msg);
//    }
//}

// void PhaseCalibrationFlow::sttcStart(const PhaseCalibrationItem& item)
//{
//    pushLog("STTC校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(STTC, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 1;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_STTC_Start", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//    waitDCResult(item);
//}
// void PhaseCalibrationFlow::sktStart(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SKT, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 1;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SKT_Start", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//    waitDCResult(item);
//}
// void PhaseCalibrationFlow::skuo2Start(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(Skuo2, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 1;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SK2_Start", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//    waitDCResult(item);
//}
// void PhaseCalibrationFlow::sythsmjStart(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SYTHSMJ, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 1;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJ_Start", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//    waitDCResult(item);
//}
// void PhaseCalibrationFlow::sythwxStart(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SYTHWX, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 1;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHWX_Start", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//    waitDCResult(item);
//}
// void PhaseCalibrationFlow::sythgmlStart(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SYTHGML, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 1;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSXGML_Start", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//    waitDCResult(item);
//}
// void PhaseCalibrationFlow::sythsmjk2gzbStart(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SYTHSMJK2GZB, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 1;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJK2GZB_Start", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//    waitDCResult(item);
//}
// void PhaseCalibrationFlow::sythsmjk2bgzbStart(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SYTHSMJK2BGZB, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 1;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJK2BGZB_Start", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//    waitDCResult(item);
//}
// void PhaseCalibrationFlow::sythwxsk2Start(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SYTHWXSK2, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 1;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHWXK2_Start", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//    waitDCResult(item);
//}
// void PhaseCalibrationFlow::sythgmlsk2Start(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SYTHGMLSK2, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 1;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSXGMLK2_Start", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//    waitDCResult(item);
//}

// void PhaseCalibrationFlow::sttcEnd(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(STTC, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 2;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_STTC_Stop", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//}
// void PhaseCalibrationFlow::sktEnd(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SKT, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 2;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SKT_Stop", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//}
// void PhaseCalibrationFlow::skuo2End(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(Skuo2, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 2;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SK2_Stop", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//}
// void PhaseCalibrationFlow::sythsmjEnd(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SYTHSMJ, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 2;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJ_Stop", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//}
// void PhaseCalibrationFlow::sythwxEnd(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SYTHWX, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 2;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHWX_Stop", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//}
// void PhaseCalibrationFlow::sythgmlEnd(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SYTHGML, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 2;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSXGML_Stop", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//}
// void PhaseCalibrationFlow::sythsmjk2gzbEnd(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SYTHSMJK2GZB, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 2;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJK2GZB_Stop", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//}
// void PhaseCalibrationFlow::sythsmjk2bgzbEnd(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SYTHSMJK2BGZB, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 2;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSMJK2BGZB_Stop", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//}
// void PhaseCalibrationFlow::sythwxsk2End(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SYTHWXSK2, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 2;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHWXK2_Stop", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//}
// void PhaseCalibrationFlow::sythgmlsk2End(const PhaseCalibrationItem& item)
//{
//    pushLog("校零开始");
//    /*
//     * 20210424  这里校零其实要判断载波判断基带什么的
//     * 这里比较急 直接发命令 流程能过就ok 剩下的后面再说
//     */
//    SingleCommandHelper singleCmdHelper;
//    PackCommand packCommand;
//    DeviceID deviceID;

//    if (!MacroCommon::getMasterCKJDDeviceID(SYTHGMLSK2, deviceID))
//    {
//        pushLog("获取主用测控基带失败");
//        return;
//    }
//    QVariantMap replaceParamMap;
//    /*
//     * 20210424 wp??
//     * 现在这里设备组合号 旋向  没有拿到
//     */

//    replaceParamMap["StartUpCtrl"] = 2;
//    replaceParamMap["TaskIdent"] = item.taskIdent.toUInt();
//    replaceParamMap["EquipComb"] = item.equipComb;
//    replaceParamMap["ChannelSel"] = item.channel;
//    replaceParamMap["UplinkFreq"] = item.uplinkFreq;
//    replaceParamMap["DownFreq"] = item.downFreq;
//    replaceParamMap["CorrectValue"] = item.correctValue;
//    replaceParamMap["StatisPoint"] = item.statisPoint;
//    replaceParamMap["SignalRot"] = item.signalRot;

//    /*
//     * 这里和下链路不一样
//     * 这里错误直接就返回错误就行了
//     */
//    singleCmdHelper.packSingleCommand("StepMACBDC_SYTHSXGMLK2_Stop", packCommand, deviceID, replaceParamMap);
//    if (!waitExecSuccess(packCommand))
//    {
//        return;
//    }
//}
