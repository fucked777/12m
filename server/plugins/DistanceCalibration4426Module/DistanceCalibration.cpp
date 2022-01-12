#include "DistanceCalibration.h"
#include "BusinessMacroCommon.h"
#include "ConfigMacroDispatcher.h"
#include "ConfigMacroMessageDefine.h"
#include "CppMicroServicesUtility.h"
#include "DeviceAlloter.h"
#include "DistanceCalibrationFlow.h"
#include "DistanceCalibrationSerialize.h"
#include "EquipmentCombinationHelper.h"
#include "GlobalData.h"
#include "MacroHelper.h"
#include "MessagePublish.h"
#include "OnekeyXLHandler.h"
#include "ParamMacroDispatcher.h"
#include "RedisHelper.h"
#include "SqlDistanceCalibration.h"
#include "Utility.h"
#include <QString>
class DistanceCalibrationImpl
{
public:
    cppmicroservices::BundleContext context;
    // SqlDistanceCalibration sql;
    static Optional<quint64> createDevNumber(SystemWorkMode workMode);

    QString userID; /* 发送者ID */
};

Optional<quint64> DistanceCalibrationImpl::createDevNumber(SystemWorkMode workMode)
{
    using ResType = Optional<quint64>;
    /* 生成设备组合号 */
    /* 1.获取当前配置宏数据 */
    ConfigMacroData configMacroData;
    if (!GlobalData::getConfigMacroData(workMode, configMacroData))
    {
        auto message = "查找当前模式对应的配置宏数据失败";
        return ResType(ErrorCode::NotFound, message);
    }

    /* 2.获取当前测控基带 */
    DeviceID ckDeviceID;
    if (!MacroCommon::getMasterCKJDDeviceID(workMode, ckDeviceID))
    {
        auto message = "获取主用测控基带失败";
        return ResType(ErrorCode::NotFound, message);
    }

    /* 3.获取当前跟踪基带 */
    DeviceID gzDeviceID;
    if (!MacroCommon::getGZQDDeviceID(workMode, gzDeviceID))
    {
        auto message = "获取主用跟踪基带失败";
        return ResType(ErrorCode::NotFound, message);
    }
    quint64 equipComb = 0;
    if (!EquipmentCombinationHelper::getCKDevNumber(workMode, configMacroData, ckDeviceID, gzDeviceID, equipComb))
    {
        auto message = "获取当前设备组合号失败";
        return ResType(ErrorCode::NotSupported, message);
    }
    return ResType(equipComb);
}

DistanceCalibration::DistanceCalibration(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new DistanceCalibrationImpl)
{
    m_impl->context = context;

    auto initDBRes = SqlDistanceCalibration::initDB();
    if (!initDBRes)
    {
        auto msg = QString("校零初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("startDistanceCalibration", &DistanceCalibration::startDistanceCalibration, this, false);
    registerSubscribe("stopDistanceCalibration", &DistanceCalibration::stopDistanceCalibration, this);
    registerSubscribe("deleteDistanceCalibrationResult", &DistanceCalibration::deleteDistanceCalibrationResult, this);
    registerSubscribe("clearHistry", &DistanceCalibration::clearHistry, this);
    registerSubscribe("queryHistry", &DistanceCalibration::queryHistry, this);
    registerSubscribe("AcceptDeviceCMDResultMessage", &DistanceCalibration::sg_deviceCMDResult, this);
}

DistanceCalibration::~DistanceCalibration() { delete m_impl; }

void DistanceCalibration::startDistanceCalibration(const ProtocolPack& pack)
{
    auto bak = pack;
    DistanceCalibrationItem item;
    bak.data >> item;
    bak.data.clear();

    auto equipComb = DistanceCalibrationImpl::createDevNumber(item.workMode);
    if (!equipComb)
    {
        DistanceCalibrationFlow::pushLog(equipComb.msg());
        bak.data << OptionalNotValue(equipComb.errorCode(), equipComb.msg());
        return;
    }

    //记录发送者ID
    m_impl->userID = bak.userID;
    SystemLogPublish::infoMsg(QString("开始%1测距校零").arg(item.workMode), bak.userID, MACRSTR(MODULE_NAME));

    slotPublishInfoMsg("校零环路开始配置");
    //    ConfigMacroDispatcher configMacroDispatcher;
    //    connect(&configMacroDispatcher, &ConfigMacroDispatcher::signalSendToDevice, this, &DistanceCalibration::sendByteArrayToDevice);
    //    connect(&configMacroDispatcher, &ConfigMacroDispatcher::signalInfoMsg, this, &DistanceCalibration::slotPublishInfoMsg);
    //    connect(&configMacroDispatcher, &ConfigMacroDispatcher::signalWarningMsg, this, &DistanceCalibration::slotPublishWarningMsg);
    //    connect(&configMacroDispatcher, &ConfigMacroDispatcher::signalErrorMsg, this, &DistanceCalibration::slotPublishErrorMsg);

    //    ParamMacroDispatcher paramMacroDispatcher;
    //    connect(&paramMacroDispatcher, &ParamMacroDispatcher::signalSendToDevice, this, &DistanceCalibration::sendByteArrayToDevice);
    //    connect(&paramMacroDispatcher, &ParamMacroDispatcher::signalInfoMsg, this, &DistanceCalibration::slotPublishInfoMsg);
    //    connect(&paramMacroDispatcher, &ParamMacroDispatcher::signalWarningMsg, this, &DistanceCalibration::slotPublishWarningMsg);
    //    connect(&paramMacroDispatcher, &ParamMacroDispatcher::signalErrorMsg, this, &DistanceCalibration::slotPublishErrorMsg);

    OnekeyXLHandler onekeyXLHandler;
    connect(&onekeyXLHandler, &BaseHandler::signalSendToDevice, this, &DistanceCalibration::sendByteArrayToDevice);
    connect(&onekeyXLHandler, &BaseHandler::signalInfoMsg, this, &DistanceCalibration::slotPublishInfoMsg);
    connect(&onekeyXLHandler, &BaseHandler::signalWarningMsg, this, &DistanceCalibration::slotPublishWarningMsg);
    connect(&onekeyXLHandler, &BaseHandler::signalErrorMsg, this, &DistanceCalibration::slotPublishErrorMsg);

    ManualMessage msg;
    msg.manualType = ManualType::ConfigMacro;
    LinkLine linkLine;
    linkLine.linkType = LinkType::PKXLBPQBH;
    linkLine.workMode = item.workMode;
    {
        TargetInfo targetInfo;

        targetInfo.pointFreqNo = item.dpNum;  // 该目标对应的点频号
        targetInfo.targetNo = 1;              // 目标号
        targetInfo.workMode = item.workMode;  //该目标的工作模式
        targetInfo.taskCode = item.taskCode;  // 该目标对应的任务代号
        linkLine.targetMap[0] = targetInfo;   //目标信息 这里默认使用0
        linkLine.masterTargetNo = 0;          //主用目标号 这里默认使用0
    }
    msg.appendLine(linkLine);

    //    configMacroDispatcher.handle(msg);

    //    paramMacroDispatcher.handle(msg);
    ConfigMacroData configMacroData;
    if (msg.isManualContrl)
    {
        configMacroData = msg.configMacroData;
    }
    else
    {
        DeviceAlloter alloter;
        QString errorMsg;
        if (!alloter.allot(msg, errorMsg))
        {
            slotPublishErrorMsg(errorMsg);
            return;
        }
    }

    for (const auto& link : msg.linkLineMap)
    {
        onekeyXLHandler.setLinkLine(link);
        onekeyXLHandler.handle();
    }

    double distZeroMean = 0;
    auto isSuccess = onekeyXLHandler.getResult(distZeroMean);
    if (isSuccess)
    {
        slotPublishInfoMsg("校零成功");
        item.distZeroVar = 0.0;
        item.distZeroMean = distZeroMean;
        auto insertResult = SqlDistanceCalibration::insertItem(item);
        if (insertResult)
        {
            slotPublishInfoMsg("校零数据保存数据库成功");
        }
        else
        {
            auto msg = QString("校零数据保存数据库失败:%1").arg(insertResult.msg());
            slotPublishInfoMsg(msg);
        }
    }
    else
    {
        slotPublishInfoMsg(QString("校零失败"));
    }

    /* 这里只是返回结束所有的信息都由redis推送 */
    bak.data << OptionalNotValue();
    silenceSendACK(bak);
}
void DistanceCalibration::stopDistanceCalibration(const ProtocolPack& pack)
{
    auto bak = pack;
    DistanceCalibrationItem item;
    bak.data >> item;
    bak.data.clear();

    DistanceCalibrationFlow flow(this);
    switch (item.workMode)
    {
    case STTC:
    {
        flow.sttcEnd(item);
        break;
    }
    case SKT:
    {
        flow.sktEnd(item);
        break;
    }
    case Skuo2:
    {
        flow.skuo2End(item);
        break;
    }
    case SYTHSMJ:
    {
        flow.sythgmlEnd(item);
        break;
    }
    case SYTHWX:
    {
        flow.sythwxEnd(item);
        break;
    }
    case SYTHGML:
    {
        flow.sythgmlEnd(item);
        break;
    }
    case SYTHSMJK2GZB:
    {
        flow.sythsmjk2gzbEnd(item);
        break;
    }
    case SYTHSMJK2BGZB:
    {
        flow.sythsmjk2bgzbEnd(item);
        break;
    }
    case SYTHWXSK2:
    {
        flow.sythwxsk2End(item);
        break;
    }
    case SYTHGMLSK2:
    {
        flow.sythgmlsk2End(item);
        break;
    }
    default:
    {
        /* 停止的时候无所谓 */
        break;
    }
    }

    SystemLogPublish::infoMsg(QString("停止%1测距校零").arg(item.workMode), bak.userID, MACRSTR(MODULE_NAME));

    /* 这里只是返回结束所有的信息都由redis推送 */
    bak.data << OptionalNotValue();
    silenceSendACK(bak);
}
void DistanceCalibration::deleteDistanceCalibrationResult(const ProtocolPack& pack)
{
    auto bak = pack;
    DistanceCalibrationItem item;
    bak.data >> item;
    bak.data.clear();

    auto ans = SqlDistanceCalibration::deleteItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除%1校零结果成功").arg(item.workMode), bak.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除%1校零结果失败").arg(item.workMode), bak.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;
    silenceSendACK(bak);
}

void DistanceCalibration::clearHistry(const ProtocolPack& pack)
{
    auto bak = pack;
    ClearDistanceCalibrationHistry item;
    bak.data >> item;
    bak.data.clear();
    auto ans = SqlDistanceCalibration::deleteItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("清理90天内校零数据成功"), bak.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("清理90天内校零数据失败"), bak.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;
    silenceSendACK(bak);
}
void DistanceCalibration::queryHistry(const ProtocolPack& pack)
{
    auto bak = pack;
    QueryDistanceCalibrationHistry item;
    bak.data >> item;
    bak.data.clear();

    bak.data << SqlDistanceCalibration::selectItem(item);
    silenceSendACK(bak);
}

void DistanceCalibration::sendByteArrayToDevice(const QByteArray& data)
{
    ProtocolPack protocal;
    protocal.srcID = "";
    protocal.desID = "NetWorkManager";
    protocal.module = true;
    protocal.operation = "sendDataToDevice";
    protocal.data = data;
    this->silenceSend(protocal);
}

void DistanceCalibration::slotPublishInfoMsg(const QString& msg)
{
    RedisHelper::getInstance().publish("DistanceCalibrationLog", msg);
    SystemLogPublish::infoMsg(msg, m_impl->userID, MACRSTR(MODULE_NAME));
}

void DistanceCalibration::slotPublishWarningMsg(const QString& msg)
{
    RedisHelper::getInstance().publish("DistanceCalibrationLog", msg);
    SystemLogPublish::warningMsg(msg, m_impl->userID, MACRSTR(MODULE_NAME));
}

void DistanceCalibration::slotPublishErrorMsg(const QString& msg)
{
    RedisHelper::getInstance().publish("DistanceCalibrationLog", msg);
    SystemLogPublish::errorMsg(msg, m_impl->userID, MACRSTR(MODULE_NAME));
}
