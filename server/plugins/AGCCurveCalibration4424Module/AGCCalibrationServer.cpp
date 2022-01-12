#include "AGCCalibrationServer.h"
#include "AGCCalibrationHelper.h"
#include "AGCManage.h"
#include "AGCSerialize.h"
#include "BusinessMacroCommon.h"
#include "CConverter.h"
#include "ControlFlowHandler.h"
#include "DeviceProcessMessageSerialize.h"
#include "PlanRunMessageDefine.h"
#include "RedisHelper.h"
#include "SatelliteManagementSerialize.h"
#include "SingleCommandHelper.h"
#include "SqlAGC.h"
#include <QApplication>
#include <QMap>
#include <QVariant>

class AGCCalibrationServerAutoStop
{
public:
    AGCCalibrationServerAutoStop(AGCCalibrationServer* self_)
        : self(self_)
    {
    }
    ~AGCCalibrationServerAutoStop() { emit self->sg_end(); }

private:
    AGCCalibrationServer* self;
};

class AGCCalibrationServerImpl
{
public:
    AGCManage* parent;
    AGCCalibrationServer* self;
    ProtocolPack pack;              /* 协议包 */
    AGCCalibrationItem item;        /* 曲线标定的参数 */
    CmdResult curResult;            /* 命令结果 */
    ControlCmdResponse curResponce; /* 命令响应 */
    bool isRecvResult{ false };     /* 成功接收命令结果 */
    bool isRecvResponce{ false };   /* 成功接收命令响应 */
    PackCommand currentPackCmd;     /* 用于上报结果对比 */

    double downFreq{ 0.0 };
    AGCCalibrationServerImpl(AGCManage* parent_, AGCCalibrationServer* self_)
        : parent(parent_)
        , self(self_)
    {
    }

    /* 切换场放到测试信号 */
    OptionalNotValue lnaSwitchToTest(ConfigMacroData& configMacroData);
};

OptionalNotValue AGCCalibrationServerImpl::lnaSwitchToTest(ConfigMacroData& configMacroData)
{
    if (!configMacroData.configMacroCmdModeMap.contains(item.workMode))
    {
        return OptionalNotValue(ErrorCode::InvalidArgument, "当前配置宏数据错误,未能找到相应的工作模式");
    }

    /*
     * STTC_LAN_L_SUM
     * STTC_LAN_R_SUM
     * STTC_LAN_DIFF
     * KA_LAN_L_SUM
     * KA_LAN_R_SUM
     * KA_LAN_L_DIFF
     * KA_LAN_R_DIFF
     * 有效值只有1和2
     *
     * 但是 XDS_LAN_L_SUM  XDS_LAN_R_SUM  不需要处理
     */

    auto& configMacroCmdList = configMacroData.configMacroCmdModeMap[item.workMode];
    for (auto& item : configMacroCmdList.configMacroCmdMap)
    {
        /* X低速不用管 */
        if (item.id.startsWith("XDS"))
        {
            continue;
        }
        if (!item.id.contains("_LAN_"))
        {
            continue;
        }

        auto tempValue = item.value.toInt();
        item.value = (tempValue == 1 ? 2 : 1);
    }
    return OptionalNotValue();
}

AGCCalibrationServer::AGCCalibrationServer(AGCManage* parent)
    : QThread(parent)
    , m_impl(new AGCCalibrationServerImpl(parent, this))
{
    /* 没测试过这个东东连接信号槽行不行 现在只是能编译通过 不行就加一个函数做中转 20210616 */
    connect(m_impl->parent, &AGCManage::sg_deviceCMDResult, this, &AGCCalibrationServer::acceptDeviceCMDResultMessage);
    connect(m_impl->parent, &AGCManage::sg_deviceCMDResponce, this, &AGCCalibrationServer::acceptDeviceCMDResponceMessage);
}
AGCCalibrationServer::~AGCCalibrationServer() { delete m_impl; }

void AGCCalibrationServer::run()
{
    /*
     * 20210816 wp??
     * 理论上标校的数据通过跟踪基带的和(测控/低速/高速)都是一样的这个
     * 数据 理论上是可以直接给给基带和给ACU的
     * 所以这里删除跟踪基带相关的数据
     */
    /* 退出函数后自动发送结束信号 */
    AGCCalibrationServerAutoStop autoStop(this);

    /* 1.数据获取
     * 旋向 配置宏 下行频率
     */
    int spin = -1; /* 1左旋 2 右旋 */
    ConfigMacroData configMacroData;
    {
        /* 旋向 */
        auto spinResult = AGCCalibrationHelper::getCurSpin(m_impl->item.taskCode, m_impl->item.workMode, m_impl->item.dotDrequencyNum);
        if (!spinResult)
        {
            pushLog(spinResult.msg(), true);
            return;
        }
        spin = spinResult.value();

        /* 配置宏 */
        auto getDataResult = GlobalData::getConfigMacroData(m_impl->item.workMode, configMacroData, MasterSlave::Master);
        if (!getDataResult)
        {
            pushLog(QString("获取当前配置宏错误"), true);
            return;
        }

        QVariant down;
        MacroCommon::getDownFrequency(m_impl->item.taskCode, m_impl->item.workMode, m_impl->item.dotDrequencyNum, down);
        m_impl->downFreq = down.toDouble();

        if (qFuzzyIsNull(m_impl->downFreq))
        {
            pushLog("测试失败:参数宏数据解析失败", true);
            return;
        }
    }

    /* 推送一下链路信息 */
    auto tempPushMsg = QString("当前曲线标定工作模式:%1 点频：%2 下行频率：%3 测试使用旋向: %4")
                           .arg(SystemWorkModeHelper::systemWorkModeToDesc(m_impl->item.workMode))
                           .arg(m_impl->item.dotDrequencyNum)
                           .arg(m_impl->downFreq, 0, 'f')
                           .arg(spin == 1 ? "左旋" : "右旋");

    pushLog(tempPushMsg);

    /* 2.切换为任务环路并且配置参数宏 */
    {
        /* 场放切换
         * 场放一条线上一般有两个场放
         * 是互为主备的关系
         * 此函数功能就是 比如当前是用的是场放1 那就切换到场放2
         * 如果当前是用的是场放2 那就切换到场放1
         * 因为任务环路和测试环路场放刚好是反向的
         */
        auto switchResult = m_impl->lnaSwitchToTest(configMacroData);
        if (!switchResult)
        {
            pushLog(switchResult.msg(), true);
            return;
        }

        ManualMessage msg;
        msg.manualType = ManualType::ConfigMacroAndParamMacro;
        msg.isManualContrl = true;
        msg.configMacroData = configMacroData;

        LinkLine linkLine;
        auto index = 1;
        linkLine.workMode = m_impl->item.workMode;
        linkLine.masterTargetNo = index;
        TargetInfo targetInfo;
        targetInfo.targetNo = index;
        targetInfo.workMode = m_impl->item.workMode;
        targetInfo.taskCode = m_impl->item.taskCode;
        targetInfo.pointFreqNo = m_impl->item.dotDrequencyNum;
        linkLine.targetMap[index] = targetInfo;
        //配置宏任务环路
        linkLine.linkType = LinkType::RWFS;
        msg.appendLine(linkLine);

        ControlFlowHandler controlFlowHandler;
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalSendToDevice, this, &AGCCalibrationServer::sendByteArrayToDevice);
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalInfoMsg, [=](const QString& msg) { this->pushLog(msg); });
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalWarningMsg, [=](const QString& msg) { this->pushLog(msg); });
        QObject::connect(&controlFlowHandler, &ControlFlowHandler::signalErrorMsg, [=](const QString& msg) {
            this->pushLog(msg, true);
            emit this->sg_end();
        });
        controlFlowHandler.handle(msg);
    }

    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    DeviceID ckjd;
    DeviceID dsjd;
    DeviceID gsjd;
    /* 3.打开前端测试开关网络电源 2开1关 */
    {
        singleCmdHelper.packSingleCommand("StepQDMNY_ON_POWER", packCommand);
        auto execStatus = waitExecSuccess(packCommand, 40);
        if (execStatus != ExecStatus::Success)
        {
            return;
        }
    }

    /* 4.将测试信号接入测试计算机
     */
    {
        /* S频段 */
        if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_impl->item.workMode) == SystemBandMode::SBand)
        {
            /*
             * S频段 左旋的话是1接25 右旋是2接25
             * S频段接收开关矩阵
             */
            QVariantMap replace;
            replace["ChooseConnect_25"] = (spin == 1 ? 1 : 2);
            singleCmdHelper.packSingleCommand("Step_RFSM_S_DownlinkTest", packCommand, replace);
            auto execStatus = waitExecSuccess(packCommand, 40);
            if (execStatus != ExecStatus::Success)
            {
                return;
            }
        }
        else if (m_impl->item.workMode == SystemWorkMode::KaKuo2)
        {
            /*
             * Ka遥测 左旋的话是6接25 右旋是7接25
             * S频段接收开关矩阵
             */
            QVariantMap replace;
            replace["ChooseConnect_25"] = (spin == 1 ? 6 : 7);
            singleCmdHelper.packSingleCommand("Step_RFSM_KaYC_DownlinkTest", packCommand, replace);
            auto execStatus = waitExecSuccess(packCommand, 40);
            if (execStatus != ExecStatus::Success)
            {
                return;
            }
        }
        else if (m_impl->item.workMode == SystemWorkMode::KaDS)
        {
            /*
             * Ka低速 左旋的话是12接25 右旋是13接25
             * S频段接收开关矩阵
             */
            QVariantMap replace;
            replace["ChooseConnect_25"] = (spin == 1 ? 12 : 13);
            singleCmdHelper.packSingleCommand("Step_RFSM_KaDS_DownlinkTest", packCommand, replace);
            auto execStatus = waitExecSuccess(packCommand, 40);
            if (execStatus != ExecStatus::Success)
            {
                return;
            }
        }
        else if (m_impl->item.workMode == SystemWorkMode::KaGS)
        {
            /*
             * Ka高速 没有走S频段接收开关矩阵是走的1.2g中频开关矩阵
             * 左旋 1接6 右旋 2接6
             */
            QVariantMap replace;
            replace["ChooseConnect_6"] = (spin == 1 ? 1 : 2);
            singleCmdHelper.packSingleCommand("Step_LSFBRFSMJ_DownlinkTest", packCommand, replace);
            auto execStatus = waitExecSuccess(packCommand, 40);
            if (execStatus != ExecStatus::Success)
            {
                return;
            }
        }
    }

    /* 5.后端测试信号选择开关
     * 在S Ka遥测 Ka低速时需要把 开关打到S入1
     * 在高速时需要打到1.2G入
     */
    {
        if (m_impl->item.workMode == SystemWorkMode::KaGS)
        {
            QVariantMap replace;
            replace["OscillChannSel1"] = 3;
            singleCmdHelper.packSingleCommand("Step_BTHSN_BandwidthSwitching", packCommand, replace);
            auto execStatus = waitExecSuccess(packCommand, 40);
            if (execStatus != ExecStatus::Success)
            {
                return;
            }
        }
        else
        {
            QVariantMap replace;
            replace["OscillChannSel1"] = 1;
            singleCmdHelper.packSingleCommand("Step_BTHSN_BandwidthSwitching", packCommand, replace);
            auto execStatus = waitExecSuccess(packCommand, 40);
            if (execStatus != ExecStatus::Success)
            {
                return;
            }
        }
    }

    /* 6.如果为扩频模式需要
     * 更改基带的 载波加调开 伪码开关关闭 扩频就变单载波信号了
     * 20210816 wp?? 暂时缺少接口........
     * 缺少一个 接收通道伪码开关控制
     */
    {
        if (m_impl->item.workMode == SystemWorkMode::STTC)
        {
            bool ckjdRes = configMacroData.getCKJDDeviceID(m_impl->item.workMode, ckjd, 1);
            if (!ckjdRes)
            {
                pushLog("STTC获取当前主用测控基带错误", true);
                return;
            }
        }
        else if (m_impl->item.workMode == SystemWorkMode::Skuo2)
        {
            bool ckjdRes = configMacroData.getCKJDDeviceID(m_impl->item.workMode, ckjd, 1);
            if (!ckjdRes)
            {
                pushLog("S扩二获取当前主用测控基带错误", true);
                return;
            }
            singleCmdHelper.packSingleCommand("StepMACBDC_SK2_YK_DOWN", packCommand, ckjd);
            auto execStatus = waitExecSuccess(packCommand, 40);
            if (execStatus != ExecStatus::Success)
            {
                return;
            }
        }
        else if (m_impl->item.workMode == SystemWorkMode::KaKuo2)
        {
            bool ckjdRes = configMacroData.getCKJDDeviceID(m_impl->item.workMode, ckjd, 1);
            if (!ckjdRes)
            {
                pushLog("Ka扩二获取当前主用测控基带错误", true);
                return;
            }
            singleCmdHelper.packSingleCommand("StepMACBDC_KaK2_YK_DOWN", packCommand, ckjd);
            auto execStatus = waitExecSuccess(packCommand, 40);
            if (execStatus != ExecStatus::Success)
            {
                return;
            }
        }
        else if (m_impl->item.workMode == SystemWorkMode::KaDS)
        {
            bool ckjdRes = configMacroData.getDSJDDeviceID(m_impl->item.workMode, dsjd, 1);
            if (!ckjdRes)
            {
                pushLog("获取当前主用低速基带错误", true);
                return;
            }
        }
        else if (m_impl->item.workMode == SystemWorkMode::KaGS)
        {
            bool ckjdRes = configMacroData.getDSJDDeviceID(m_impl->item.workMode, gsjd, 1);
            if (!ckjdRes)
            {
                pushLog("获取当前主用高速基带错误", true);
                return;
            }
        }
    }

    /* 7.前端测试开关网络切换输出 */
    {
        if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_impl->item.workMode) == SystemBandMode::SBand)
        {
            singleCmdHelper.packSingleCommand("StepSTTC_QDXH", packCommand);
            auto execStatus = waitExecSuccess(packCommand, 40);
            if (execStatus != ExecStatus::Success)
            {
                return;
            }
        }
        else if (m_impl->item.workMode == SystemWorkMode::KaKuo2 ||  //
                 m_impl->item.workMode == SystemWorkMode::KaDS ||    //
                 m_impl->item.workMode == SystemWorkMode::KaGS)      //
        {
            singleCmdHelper.packSingleCommand("StepKa_QDXH", packCommand);
            auto execStatus = waitExecSuccess(packCommand, 40);
            if (execStatus != ExecStatus::Success)
            {
                return;
            }
        }
    }
    /*
     * 8.扩跳频长码短码获取
     * 20210816 wp 这里增加一个步骤
     * 当是扩跳模式的时候 需要获取他到底使用短码还是长码
     */
    QString ktAGCKey;
    if (m_impl->item.workMode == SystemWorkMode::SKT)
    {
        /* 扩跳在调制器单元有个扩跳输出方式 可以选择使用短码或者长码 */
        auto result = GlobalData::getReportParamData(ckjd, 0x03, "ExtendedJumpOutputMode");
        if (!result.isValid())
        {
            pushLog("扩跳模式获取当前使用长码或者短码错误", true);
            return;
        }
        /* 1短 2长 3并行 */
        auto tempValue = result.toInt();
        if (tempValue == 1 || tempValue == 3)
        {
            ktAGCKey = "Status5";
        }
        else if (tempValue == 2)
        {
            ktAGCKey = "Status11";
        }
        else
        {
            pushLog("扩跳模式获取当前使用长码或者短码错误", true);
            return;
        }
    }

    /* 9.开始标定agc */
    /* 衰减的范围 20210806 wp??
     * 这个信号源的频率是-15-12
     * 这里给12db不会太高
     * 额先给5db先凑够20组数据
     * 然后不够再说
     *
     * 13在站上的时候给过几次比较大的值 很容易烧掉场放切记增加输出功率要小心
     */
    int min = -15;
    int max = 5;
    /*
     * 可能的问题 因为这里到频谱仪的频率有两种 一种是S的 另一种是1.2G
     * 频谱仪的参数可能不一样 然后这里写死不咋行 所以在ini里面加一个配置项
     * 将这两种频率对应的数据分开 各玩儿各的
     */
    QString testIndexKey = "Step_TC_SBF";

    QVariantMap tcsReplace;
    tcsReplace["TestCtrl"] = 1;
    tcsReplace["SignalSelect"] = 2; /* 前端信号源 */
    tcsReplace["SpectSelect"] = 1;  /* 后端频谱仪 */
    tcsReplace["OutputSwitch"] = 0; /* 信号源开 */
    /* 信号源是原始信号是他转换后才是到测试计算机的频率 */
    tcsReplace["OutputFrequence"] = m_impl->downFreq * 1000.0; /* 信号源输出频率 */
    /*
     * 如果是S频段则下行频率就是设置给测试计算机的频率因为S没有变过频是直接过去的
     * Ka遥测和Ka低速是经过变频器了的给测试计算机的频率是固定的 2250 wp?? 这个好像是这么多忘了需要确认一下
     * Ka高速也是固定的频率是1.2G
     */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_impl->item.workMode) == SystemBandMode::SBand)
    {
        tcsReplace["SpectCenter"] = m_impl->downFreq * 1000.0; /* 频率测试计算机是KHZ */
    }
    else if (m_impl->item.workMode == SystemWorkMode::KaGS)
    {
        testIndexKey = "Step_TC_SBF_1_2G";
        tcsReplace["SpectCenter"] = 1200000; /* 频率测试计算机是KHZ */
    }
    else
    {
        tcsReplace["SpectCenter"] = 2250000; /* 频率测试计算机是KHZ */
    }

    for (int i = max; i >= min; --i)
    {
        AGCResult agcResult;
        bool jdCarrierLockInd = false; /* 基带信号是否锁定 */
        /* 这里需要读取两个基带的agc 以及锁定情况 */
        /* 9.1 载噪比测试 */
        {
            tcsReplace["OutputLevecon"] = i;

            singleCmdHelper.packSingleCommand(testIndexKey, packCommand, tcsReplace);
            CmdResult cmdResult;
            auto execStatus = waitExecResponceData(cmdResult, packCommand, 3, 60);
            if (execStatus != ExecStatus::Success)
            {
                return;
            }

            /* 取载噪比的值 */
            auto curTestResultfind = cmdResult.m_paramdataMap.find("S/φRealValu");
            if (curTestResultfind == cmdResult.m_paramdataMap.end())
            {
                pushLog("测试错误:未能获取当前的S/φ值", true);
                return;
            }
            agcResult.sbf = curTestResultfind->toDouble();
            pushLog(QString("当前S/φ : %1 信号源输出电频 %2 dbm").arg(agcResult.sbf, 0, 'f', 3).arg(i));
        }
        /* 9.2 检查基带信号锁定,以及获取agc值 */
        {
            double jdAGC = 0.0;
            {
                QMap<QString, QVariant> jdTempMap;
                if (m_impl->item.workMode == SystemWorkMode::STTC)
                {
                    jdTempMap = GlobalData::getReportParamData(ckjd, spin == 1 ? 2 : 3,
                                                               QList<QString>() << "CarrierLockInd"
                                                                                << "AGCVoltage");
                    if (jdTempMap.isEmpty())
                    {
                        pushLog("当前测控基带AGC电压获取错误", true);
                        return;
                    }
                    jdCarrierLockInd = (jdTempMap["CarrierLockInd"].toInt() == 1);
                    jdAGC = jdTempMap["AGCVoltage"].toDouble();
                }
                else if (m_impl->item.workMode == SystemWorkMode::Skuo2 ||  //
                         m_impl->item.workMode == SystemWorkMode::KaKuo2)
                {
                    jdTempMap = GlobalData::getReportParamData(ckjd, 2,
                                                               QList<QString>() << "YC_TeleCarrLock"
                                                                                << "YC_TeleAGCVol",
                                                               1);
                    if (jdTempMap.isEmpty())
                    {
                        pushLog("当前测控基带AGC电压获取错误", true);
                        return;
                    }
                    jdCarrierLockInd = (jdTempMap["YC_TeleCarrLock"].toInt() == 1);
                    jdAGC = jdTempMap["YC_TeleAGCVol"].toDouble();
                }
                else if (m_impl->item.workMode == SystemWorkMode::KaDS)
                {
                    jdTempMap = GlobalData::getReportParamData(dsjd, 2,
                                                               QList<QString>() << "CarrierLock"
                                                                                << "AGCVoltage");
                    if (jdTempMap.isEmpty())
                    {
                        pushLog("当前低速基带AGC电压获取错误", true);
                        return;
                    }
                    jdCarrierLockInd = (jdTempMap["CarrierLock"].toInt() == 1);
                    jdAGC = jdTempMap["AGCVoltage"].toDouble();
                }
                else if (m_impl->item.workMode == SystemWorkMode::KaGS)
                {
                    jdTempMap = GlobalData::getReportParamData(gsjd, 2,
                                                               QList<QString>() << "CarrierLockInd"
                                                                                << "AGCVoltage");
                    if (jdTempMap.isEmpty())
                    {
                        pushLog("当前高速基带AGC电压获取错误", true);
                        return;
                    }
                    jdCarrierLockInd = (jdTempMap["CarrierLockInd"].toInt() == 1);
                    jdAGC = jdTempMap["AGCVoltage"].toDouble();
                }
                else if (m_impl->item.workMode == SystemWorkMode::SKT)
                {
                    jdTempMap = GlobalData::getReportParamData(ckjd, 2, QList<QString>() << "Status2" << ktAGCKey);
                    if (jdTempMap.isEmpty())
                    {
                        pushLog("当前测控基带AGC电压获取错误", true);
                        return;
                    }
                    jdCarrierLockInd = (jdTempMap["Status2"].toInt() == 1);
                    jdAGC = jdTempMap[ktAGCKey].toDouble();
                }
            }

            if (jdCarrierLockInd)
            {
                agcResult.agc = jdAGC;
            }
        }
        /* 9.3 检查锁定情况失锁则结束,否则上报当前的值 */
        {
            /* 失锁 */
            if (jdCarrierLockInd)
            {
                break;
            }
            m_impl->pack.data.clear();
            AGCCalibrationProcessACK ack;
            ack.result = AGCCalibrationStatus::Info;
            ack.value = agcResult;

            m_impl->pack.data << ack;
            m_impl->item.result << agcResult;

            emit sg_testProcessInfo(m_impl->pack);
        }
    }

    m_impl->pack.data.clear();
    AGCCalibrationProcessACK ack;

    /* 到这里就参数肯定是对的了只判断是否有数据就行 */
    if (!m_impl->item.result.isEmpty())
    {
        SqlAGC sql;
        auto result = sql.saveAGCItem(m_impl->item);
        if (result)
        {
            ack.result = AGCCalibrationStatus::End;
            ack.item = m_impl->item;
        }
        else
        {
            ack.result = AGCCalibrationStatus::Failed;
            ack.msg = QString("数据保存错误:%1").arg(result.msg());
        }
    }
    else
    {
        ack.result = AGCCalibrationStatus::Failed;
        ack.msg = "当前测得有效点数为空";
    }

    m_impl->pack.data << ack;
    emit sg_testProcessInfo(m_impl->pack);

    return;
}
void AGCCalibrationServer::startServer(const ProtocolPack& pack)
{
    m_impl->pack = pack;
    m_impl->pack.data >> m_impl->item;
    m_impl->item.result.clear();
    start();
}
void AGCCalibrationServer::stopServer()
{
    if (!isRunning())
    {
        return;
    }
    this->terminate();
    wait();
}

void AGCCalibrationServer::pushLog(const QString& msg, bool error)
{
    RedisHelper::getInstance().publish("AGC", msg);
    /* 日志记录 */
    if (error)
    {
        /* 停止当前的标定 */
        AGCCalibrationProcessACK ack;
        ack.msg = msg;
        ack.result = AGCCalibrationStatus::Failed;

        m_impl->pack.data.clear();
        m_impl->pack.data << ack;
        m_impl->parent->silenceSendACK(m_impl->pack);
    }
}

void AGCCalibrationServer::sendByteArrayToDevice(const QByteArray& data)
{
    ProtocolPack protocal;
    protocal.srcID = "";
    protocal.desID = "NetWorkManager";
    protocal.module = true;
    protocal.operation = "sendDataToDevice";
    protocal.data = data;
    m_impl->parent->silenceSend(protocal);
}

void AGCCalibrationServer::acceptDeviceCMDResultMessage(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data >> m_impl->curResult;
    if (m_impl->currentPackCmd.deviceID.sysID == m_impl->curResult.m_systemNumb &&   //
        m_impl->currentPackCmd.deviceID.devID == m_impl->curResult.m_deviceNumb &&   //
        m_impl->currentPackCmd.deviceID.extenID == m_impl->curResult.m_extenNumb &&  //
        m_impl->currentPackCmd.cmdId == m_impl->curResult.m_cmdID)
    {
        m_impl->isRecvResult = true;
    }
}
void AGCCalibrationServer::acceptDeviceCMDResponceMessage(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data >> m_impl->curResponce;
    if (m_impl->currentPackCmd.deviceID == m_impl->curResponce.deviceID &&  //
        m_impl->currentPackCmd.cmdId == m_impl->curResponce.cmdId)
    {
        m_impl->isRecvResponce = true;
    }
}

AGCCalibrationServer::ExecStatus AGCCalibrationServer::waitExecSuccessImpl(const PackCommand& packCommand, qint32 ttl, bool timeoutError)
{
    if (!packCommand.errorMsg.isEmpty())
    {
        pushLog(packCommand.errorMsg, true);
        return ExecStatus::Failed;
    }

    if (packCommand.data.isEmpty())
    {
        pushLog("系统组包失败，请检查配置", true);
        return ExecStatus::Failed;
    }
    m_impl->isRecvResponce = false;

    // 发送数据
    sendByteArrayToDevice(packCommand.data);

    // 写入日志当前命令执行的操作
    QString currentCmdOperatorInfo;
    if (packCommand.operatorInfo.isEmpty())
    {
        currentCmdOperatorInfo = QString("%1：%2").arg(GlobalData::getExtensionName(packCommand.deviceID), packCommand.getSetParamDescValue());
    }
    else
    {
        currentCmdOperatorInfo = packCommand.operatorInfo;
    }
    pushLog(currentCmdOperatorInfo);

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

        auto cmdExecResultDesc = DevProtocolEnumHelper::controlCmdResponseToDescStr(m_impl->curResponce.responseResult);
        auto message = QString("%1 (%2)").arg(currentCmdOperatorInfo, cmdExecResultDesc);

        /* 判断是否执行成功 */
        if (ControlCmdResponseType::Success == m_impl->curResponce.responseResult)
        {
            pushLog(message);
            return ExecStatus::Success;
        }

        pushLog(message, true);
        return ExecStatus::Failed;
    }

    // 超时未响应
    if (timeoutError)
    {
        pushLog(QString("%1，超时未响应").arg(currentCmdOperatorInfo), true);
    }
    return ExecStatus::Timeout;
}

AGCCalibrationServer::ExecStatus AGCCalibrationServer::waitExecSuccess(const PackCommand& packCommand, qint32 ttl)
{
    return waitExecSuccessImpl(packCommand, ttl, true);
}
AGCCalibrationServer::ExecStatus AGCCalibrationServer::waitSingleStatusChange(const PackCommand& packCommand, const DeviceID& devID, int unitID,
                                                                              const QString& paramID, const QVariant& value, qint32 ttl)
{
    auto execStatus = waitExecSuccessImpl(packCommand, ttl, true);
    if (execStatus != ExecStatus::Success)
    {
        return execStatus;
    }

    QString currentCmdOperatorInfo;
    if (packCommand.operatorInfo.isEmpty())
    {
        currentCmdOperatorInfo = QString("%1：%2").arg(GlobalData::getExtensionName(packCommand.deviceID), packCommand.getSetParamDescValue());
    }
    else
    {
        currentCmdOperatorInfo = packCommand.operatorInfo;
    }
    pushLog(QString("%1: 等待状态切换中").arg(currentCmdOperatorInfo));

    /* 等待状态改变 */
    while (ttl > 0)
    {
        QThread::sleep(1);
        QCoreApplication::processEvents();
        --ttl;

        auto statusValue = GlobalData::getReportParamData(devID, unitID, paramID);
        if (statusValue.isValid() && statusValue == value)
        {
            auto message = QString("%1: 等待状态切换成功").arg(currentCmdOperatorInfo);
            pushLog(message);
            return ExecStatus::Success;
        }
    }

    auto message = QString("%1 等待状态切换超时").arg(currentCmdOperatorInfo);
    pushLog(message, true);
    return ExecStatus::Timeout;
}

AGCCalibrationServer::ExecStatus AGCCalibrationServer::waitExecResponceData(CmdResult& cmdResult, const PackCommand& packCommand, qint32 count,
                                                                            qint32 ttl)
{
    m_impl->currentPackCmd = packCommand;
    QString currentCmdOperatorInfo;
    if (packCommand.operatorInfo.isEmpty())
    {
        currentCmdOperatorInfo = QString("%1：%2").arg(GlobalData::getExtensionName(packCommand.deviceID), packCommand.getSetParamDescValue());
    }
    else
    {
        currentCmdOperatorInfo = packCommand.operatorInfo;
    }

    for (int i = 0; i < count; ++i)
    {
        m_impl->isRecvResult = false;   /* 成功接收命令结果 */
        m_impl->isRecvResponce = false; /* 成功接收命令响应 */

        /* 等待命令响应 */
        auto execStatus = waitExecSuccessImpl(packCommand, ttl, false);
        if (execStatus == ExecStatus::Timeout)
        {
            pushLog(QString("%1: 未收到响应超时重试").arg(currentCmdOperatorInfo));
            continue;
        }
        else if (execStatus != ExecStatus::Success)
        {
            return execStatus;
        }

        pushLog(QString("%1: 等待结果上报中").arg(currentCmdOperatorInfo));

        auto timeout = ttl;
        /* 等待结果 */
        while (timeout > 0)
        {
            QThread::sleep(1);
            QCoreApplication::processEvents();
            --timeout;

            /* 判断是否接收结果成功 */
            if (!m_impl->isRecvResult)
            {
                continue;
            }

            auto message = QString("%1 结果上报成功").arg(currentCmdOperatorInfo);
            pushLog(message);
            cmdResult = m_impl->curResult;
            return ExecStatus::Success;
        }
        pushLog(QString("%1: 未收到结果超时重试").arg(currentCmdOperatorInfo));
    }

    auto message = QString("%1 等待结果上报超时").arg(currentCmdOperatorInfo);
    pushLog(message, true);
    return ExecStatus::Timeout;
}
