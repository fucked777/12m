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
#include <atomic>

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
    ProtocolPack pack;                      /* 协议包 */
    AGCCalibrationItem item;                /* 曲线标定的参数 */
    std::atomic<bool> runningFlag{ false }; /* 运行标志 */
    std::atomic<bool> isCallStop{ false };  /* 是否调用停止函数 */

    SingleCommandHelper singleCmdHelper;
    DeviceID jdid;
    QString ktAGCKey;
    ControlFlowHandler controlFlowHandler; /* 配置宏参数宏控制 */
    ConfigMacroData configMacroData;
    double downFreq{ 0.0 };
    AGCCalibrationtSpin spin{ AGCCalibrationtSpin::Left }; /* 自动化测试的当前旋向 */

    AGCCalibrationServerImpl(AGCManage* parent_, AGCCalibrationServer* self_)
        : parent(parent_)
        , self(self_)
    {
    }
};

/***************************************************************************************************************/
/***************************************************************************************************************/
/***************************************************************************************************************/
/***************************************************************************************************************/
/***************************************************************************************************************/
/***************************************************************************************************************/
/***************************************************************************************************************/
/***************************************************************************************************************/
AGCCalibrationServer::AGCCalibrationServer(AGCManage* parent)
    : QThread(parent)
    , m_impl(new AGCCalibrationServerImpl(parent, this))
{
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
    m_impl->isCallStop = true;
    m_impl->runningFlag = true;
    pushLog("AGC曲线标定 开始");
    if (!pretreatment()) /* 参数预处理 */
    {
        emit sg_end();
        return;
    }
    if (isExit())
    {
        return;
    }

    /* 推送一下链路信息 */
    auto tempPushMsg = QString("当前曲线标定工作模式:%1 点频：%2 下行频率：%3 测试使用旋向: %4")
                           .arg(SystemWorkModeHelper::systemWorkModeToDesc(m_impl->item.workMode))
                           .arg(m_impl->item.dotDrequencyNum)
                           .arg(m_impl->downFreq, 0, 'f')
                           .arg(m_impl->spin == AGCCalibrationtSpin::Left ? "左旋" : "右旋");

    pushLog(tempPushMsg);

    if (!linkConfiguration()) /* 设置环路 */
    {
        emit sg_end();
        return;
    }
    if (isExit())
    {
        return;
    }

    if (!parameterSetting()) /* 参数设置 */
    {
        emit sg_end();
        return;
    }
    if (isExit())
    {
        return;
    }

    if (!testing()) /* 开始测试 */
    {
        emit sg_end();
        return;
    }
    if (isExit())
    {
        return;
    }

    if (!resultOperation()) /* 结果操作 */
    {
        emit sg_end();
        return;
    }
    if (isExit())
    {
        return;
    }

    endOfTest(); /* 结束测试(清理) 这个不能太耗时 不用等结果直接发指令就行 */

    m_impl->isCallStop = false;
    m_impl->runningFlag = false;
    emit sg_end();
}
bool AGCCalibrationServer::pretreatment() { return true; }
bool AGCCalibrationServer::linkConfiguration()
{
    /* 2.切换射频模拟源有线环路并且配置参数宏 */

    ManualMessage msg;
    msg.manualType = ManualType::ConfigMacroAndParamMacro;
    msg.isManualContrl = false;
    msg.isTest = true;
    // TODO 现在默认都是标校的主用的AGC曲线
    bool flagGetConfigMacro = GlobalData::getConfigMacroData(m_impl->item.workMode, msg.configMacroData);

    if (!flagGetConfigMacro)
    {
        pushLog("获取配置宏失败", true);
        return false;
    }
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
    linkLine.linkType = LinkType::SPMNYYXBH;
    msg.appendLine(linkLine);
    msg.masterTaskCode = m_impl->item.taskCode;

    auto allocResult = ControlFlowHandler::allocConfigMacro(msg);
    if (!allocResult)
    {
        this->pushLog(QString("资源查找错误:%1").arg(allocResult.msg()), true);
        return false;
    }

    /* 这里预先处理一下数据
     * 因为有的数据在所有测试中都可能用到
     */
    auto& test = m_impl->item;
    /* 配置宏,旋向 */
    auto scResult = AGCCalibrationHelper::getSpinAndConfigMacroData(test.taskCode, test.workMode, test.dotDrequencyNum);
    if (!scResult)
    {
        pushLog(scResult.msg(), true);
        return false;
    }
    SystemOrientation systemOrientation;
    bool flag = msg.configMacroData.getMasterReceivePolar(test.workMode, systemOrientation);
    if (!flag)
    {
        pushLog("获取极性出错", true);
        return false;
    }
    if (systemOrientation == SystemOrientation::LCircumflex)
    {
        m_impl->spin = AGCCalibrationtSpin::Left;
    }
    else if (systemOrientation == SystemOrientation::RCircumflex)
    {
        m_impl->spin = AGCCalibrationtSpin::Right;
    }
    else
    {
        m_impl->spin = AGCCalibrationtSpin::Left;
    }
    m_impl->configMacroData = msg.configMacroData;

    QVariant down;
    MacroCommon::getDownFrequency(test.taskCode, test.workMode, test.dotDrequencyNum, down);
    m_impl->downFreq = down.toDouble();

    if (qFuzzyIsNull(m_impl->downFreq))
    {
        pushLog("测试失败:参数宏数据解析失败", true);
        return false;
    }

    if (SystemWorkModeHelper::isMeasureContrlWorkMode(m_impl->item.workMode))
    {
        bool jdRes = m_impl->configMacroData.getCKJDDeviceID(m_impl->item.workMode, m_impl->jdid, 1);
        if (!jdRes)
        {
            auto msg = QString("%1获取当前主用测控基带错误").arg(SystemWorkModeHelper::systemWorkModeToDesc(m_impl->item.workMode));
            pushLog(msg, true);
            return false;
        }
    }
    else if (m_impl->item.workMode == SystemWorkMode::KaDS)
    {
        bool jdRes = m_impl->configMacroData.getDSJDDeviceID(m_impl->item.workMode, m_impl->jdid, 1);
        if (!jdRes)
        {
            pushLog("获取当前主用低速基带错误", true);
            return false;
        }
    }
    else if (m_impl->item.workMode == SystemWorkMode::KaGS)
    {
        bool jdRes = m_impl->configMacroData.getDSJDDeviceID(m_impl->item.workMode, m_impl->jdid, 1);
        if (!jdRes)
        {
            pushLog("获取当前主用高速基带错误", true);
            return false;
        }
    }

    bool result = true;
    connect(&(m_impl->controlFlowHandler), &ControlFlowHandler::signalSendToDevice, this, &AGCCalibrationServer::sendByteArrayToDevice);
    connect(&(m_impl->controlFlowHandler), &ControlFlowHandler::signalInfoMsg, [=](const QString& msg) { this->pushLog(msg); });
    connect(&(m_impl->controlFlowHandler), &ControlFlowHandler::signalWarningMsg, [=](const QString& msg) { this->pushLog(msg); });
    connect(&(m_impl->controlFlowHandler), &ControlFlowHandler::signalErrorMsg, [&](const QString& msg) {
        this->pushLog(msg, true);
        //result = false;
        emit sg_end();
    });
    m_impl->controlFlowHandler.setRunningFlag(&(m_impl->runningFlag));
    m_impl->controlFlowHandler.handle(msg);
    return result;
}
bool AGCCalibrationServer::parameterSetting()
{
    PackCommand packCommand;
    DeviceID jdid;
    /* 3.打开前端测试开关网络电源 2开1关 */
    //    m_impl->singleCmdHelper.packSingleCommand("StepQDMNY_ON_POWER", packCommand);
    //    if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
    //    {
    //        return false;
    //    }

    /* 4.将测试信号接入测试计算机
     */

    /* S频段 */
    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_impl->item.workMode) == SystemBandMode::SBand)
    {

        //设置前端模拟源出来 衰减 -58
        m_impl->singleCmdHelper.packSingleCommand("Step_QD1_STTC_AGC", packCommand);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
        m_impl->singleCmdHelper.packSingleCommand("Step_QD1_STTC_AGC", packCommand);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }

        /*
         * S频段 左旋的话是1接25 右旋是2接25
         * S频段接收开关矩阵
         */
        QVariantMap replace;
        replace["ChooseConnect_25"] = (m_impl->spin == AGCCalibrationtSpin::Left ? 1 : 2);
        m_impl->singleCmdHelper.packSingleCommand("Step_RFSM_S_DownlinkTest", packCommand, replace);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
    }
    else if (m_impl->item.workMode == SystemWorkMode::KaKuo2)
    {
        //设置前端模拟源出来 衰减 -58
        m_impl->singleCmdHelper.packSingleCommand("Step_QD1_KaKuo1_AGC", packCommand);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
        m_impl->singleCmdHelper.packSingleCommand("Step_QD1_KaKuo2_AGC", packCommand);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }

        /*
         * Ka遥测 左旋的话是6接25 右旋是7接25
         * S频段接收开关矩阵
         */
        QVariantMap replace;
        replace["ChooseConnect_25"] = (m_impl->spin == AGCCalibrationtSpin::Left ? 6 : 7);
        m_impl->singleCmdHelper.packSingleCommand("Step_RFSM_KaYC_DownlinkTest", packCommand, replace);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
    }
    else if (m_impl->item.workMode == SystemWorkMode::KaDS)
    {
        /*
         * Ka低速 左旋的话是12接25 右旋是13接25
         * S频段接收开关矩阵
         */
        QVariantMap replace;
        replace["ChooseConnect_25"] = (m_impl->spin == AGCCalibrationtSpin::Left ? 12 : 13);
        m_impl->singleCmdHelper.packSingleCommand("Step_RFSM_KaDS_DownlinkTest", packCommand, replace);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
    }
    else if (m_impl->item.workMode == SystemWorkMode::KaGS)
    {
        /*
         * Ka高速 没有走S频段接收开关矩阵是走的1.2g中频开关矩阵
         * 左旋 1接6 右旋 2接6
         */
        QVariantMap replace;
        replace["ChooseConnect_6"] = (m_impl->spin == AGCCalibrationtSpin::Left ? 1 : 2);
        m_impl->singleCmdHelper.packSingleCommand("Step_LSFBRFSMJ_DownlinkTest", packCommand, replace);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
    }

    /* 5.后端测试信号选择开关
     * 在S Ka遥测 Ka低速时需要把 开关打到S入1
     * 在高速时需要打到1.2G入
     */
    if (m_impl->item.workMode == SystemWorkMode::KaGS)
    {
        QVariantMap replace;
        replace["OscillChannSel1"] = 3;
        m_impl->singleCmdHelper.packSingleCommand("Step_BTHSN_BandwidthSwitching", packCommand, replace);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
    }
    else
    {
        QVariantMap replace;
        replace["OscillChannSel1"] = 1;
        m_impl->singleCmdHelper.packSingleCommand("Step_BTHSN_BandwidthSwitching", packCommand, replace);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
    }

    if (m_impl->item.workMode == SystemWorkMode::STTC)
    {
        m_impl->singleCmdHelper.packSingleCommand("StepMACBDC_CJ_UP", packCommand, m_impl->jdid);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
        m_impl->singleCmdHelper.packSingleCommand("StepMACBDC_YK_UP", packCommand, m_impl->jdid);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
    }
    else if (m_impl->item.workMode == SystemWorkMode::Skuo2)
    {
        m_impl->singleCmdHelper.packSingleCommand("StepMACBDC_SK2_YK_UP", packCommand, m_impl->jdid);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
    }
    else if (m_impl->item.workMode == SystemWorkMode::KaKuo2)
    {
        m_impl->singleCmdHelper.packSingleCommand("StepMACBDC_KaK2_YK_UP", packCommand, m_impl->jdid);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
    }
    else if (m_impl->item.workMode == SystemWorkMode::SKT)
    {
        m_impl->singleCmdHelper.packSingleCommand("StepMACBDC_SKT_YK_UP", packCommand, m_impl->jdid);
        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
        {
            return false;
        }
    }

    /* 7.前端测试开关网络切换输出 */
    //    if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_impl->item.workMode) == SystemBandMode::SBand)
    //    {
    //        m_impl->singleCmdHelper.packSingleCommand("StepSTTC_QDXH", packCommand);
    //        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
    //        {
    //            return false;
    //        }
    //    }
    //    else if (m_impl->item.workMode == SystemWorkMode::KaKuo2 ||  //
    //             m_impl->item.workMode == SystemWorkMode::KaDS ||    //
    //             m_impl->item.workMode == SystemWorkMode::KaGS)      //
    //    {
    //        m_impl->singleCmdHelper.packSingleCommand("StepKa_QDXH", packCommand);
    //        if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
    //        {
    //            return false;
    //        }
    //    }
    /*
     * 8.扩跳频长码短码获取
     * 20210816 wp 这里增加一个步骤
     * 当是扩跳模式的时候 需要获取他到底使用短码还是长码
     */
    if (m_impl->item.workMode == SystemWorkMode::SKT)
    {
        /* 扩跳在调制器单元有个扩跳输出方式 可以选择使用短码或者长码 */
        auto result = GlobalData::getReportParamData(m_impl->jdid, 0x03, "ExtendedJumpOutputMode");
        if (!result.isValid())
        {
            pushLog("扩跳模式获取当前使用长码或者短码错误", true);
            return false;
        }
        /* 1短 2长 3并行 */
        auto tempValue = result.toInt();
        if (tempValue == 1 || tempValue == 3)
        {
            m_impl->ktAGCKey = "Status5";
        }
        else if (tempValue == 2)
        {
            m_impl->ktAGCKey = "Status11";
        }
        else
        {
            pushLog("扩跳模式获取当前使用长码或者短码错误", true);
            return false;
        }
    }

    return true;
}

bool AGCCalibrationServer::testing()
{
    /* 9.开始标定agc */
    PackCommand packCommand;
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
    if (m_impl->item.workMode == SystemWorkMode::Skuo2)
    {
        tcsReplace["SignalSpan"] = 200000; /* 频率测试计算机是KHZ */
    }
    if (m_impl->item.workMode == SystemWorkMode::KaKuo2)
    {
        tcsReplace["SignalSpan"] = 2000000; /* 频率测试计算机是KHZ */
    }
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
    max = 0;
    min = 60;
    double agc = 0;
    double sbf = 0;
    int errorflag = 0;
    while (max < min)
    {
        QVariantMap shuaijianMap;
        /* 7.设置前端信号的衰减 */
        if (SystemWorkModeHelper::systemWorkModeToSystemBand(m_impl->item.workMode) == SystemBandMode::SBand)
        {
            shuaijianMap["SFreqDataContWeak"] = max;
            m_impl->singleCmdHelper.packSingleCommand("Step_QDCSKGWL_S", packCommand, shuaijianMap);
            if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
            {
                return false;
            }
        }
        else if (m_impl->item.workMode == SystemWorkMode::KaKuo2 ||  //
                 m_impl->item.workMode == SystemWorkMode::KaDS ||    //
                 m_impl->item.workMode == SystemWorkMode::KaGS)      //
        {
            shuaijianMap["KaFreqDataContWeak"] = max;
            m_impl->singleCmdHelper.packSingleCommand("Step_QDCSKGWL_Ka", packCommand, shuaijianMap);
            if (waitExecSuccess(packCommand, 40) != ExecStatus::Success)
            {
                return false;
            }
        }
        QThread::sleep(5);
        QApplication::processEvents();
        AGCResult agcResult;
        bool jdCarrierLockInd = false; /* 基带信号是否锁定 */
        /* 这里需要读取两个基带的agc 以及锁定情况 */
        /* 9.1 载噪比测试 */
        {
            tcsReplace["OutputLevecon"] = 0;

            m_impl->singleCmdHelper.packSingleCommand(testIndexKey, packCommand, tcsReplace);
            CmdResult cmdResult;
            if (waitExecResponceData(cmdResult, packCommand, 3, 60) != ExecStatus::Success)
            {
                return false;
            }

            /* 取载噪比的值 */
            auto curTestResultfind = cmdResult.m_paramdataMap.find("S/φRealValue");
            if (curTestResultfind == cmdResult.m_paramdataMap.end())
            {
                pushLog("测试错误:未能获取当前的S/φ值", true);
                return false;
            }
            agcResult.sbf = curTestResultfind->toDouble();
            pushLog(QString("当前S/φ : %1 前端信号衰减 %2 dbm").arg(agcResult.sbf, 0, 'f', 3).arg(max));
        }
        /* 9.2 检查基带信号锁定,以及获取agc值 */
        {
            double jdAGC = 0.0;
            {
                QMap<QString, QVariant> jdTempMap;
                if (m_impl->item.workMode == SystemWorkMode::STTC)
                {
                    jdTempMap = GlobalData::getReportParamData(m_impl->jdid, m_impl->spin == AGCCalibrationtSpin::Left ? 2 : 3,
                                                               QList<QString>() << "CarrierLockInd"
                                                                                << "AGCVoltage");
                    if (jdTempMap.isEmpty())
                    {
                        pushLog("当前测控基带AGC电压获取错误", true);
                        return false;
                    }
                    jdCarrierLockInd = (jdTempMap["CarrierLockInd"].toInt() == 1);
                    jdAGC = jdTempMap["AGCVoltage"].toDouble();
                }
                else if (m_impl->item.workMode == SystemWorkMode::Skuo2 ||  //
                         m_impl->item.workMode == SystemWorkMode::KaKuo2)
                {
                    jdTempMap = GlobalData::getReportParamData(m_impl->jdid, 2,
                                                               QList<QString>() << "YC_TeleCarrLock"
                                                                                << "YC_TeleAGCVol",
                                                               1);
                    if (jdTempMap.isEmpty())
                    {
                        pushLog("当前测控基带AGC电压获取错误", true);
                        return false;
                    }
                    jdCarrierLockInd = (jdTempMap["YC_TeleCarrLock"].toInt() == 1);
                    jdAGC = jdTempMap["YC_TeleAGCVol"].toDouble();
                }
                else if (m_impl->item.workMode == SystemWorkMode::KaDS)
                {
                    jdTempMap = GlobalData::getReportParamData(m_impl->jdid, 2,
                                                               QList<QString>() << "CarrierLock"
                                                                                << "AGCVoltage");
                    if (jdTempMap.isEmpty())
                    {
                        pushLog("当前低速基带AGC电压获取错误", true);
                        return false;
                    }
                    jdCarrierLockInd = (jdTempMap["CarrierLock"].toInt() == 1);
                    jdAGC = jdTempMap["AGCVoltage"].toDouble();
                }
                else if (m_impl->item.workMode == SystemWorkMode::KaGS)
                {
                    jdTempMap = GlobalData::getReportParamData(m_impl->jdid, 2,
                                                               QList<QString>() << "CarrierLockInd"
                                                                                << "AGCVoltage");
                    if (jdTempMap.isEmpty())
                    {
                        pushLog("当前高速基带AGC电压获取错误", true);
                        return false;
                    }
                    jdCarrierLockInd = (jdTempMap["CarrierLockInd"].toInt() == 1);
                    jdAGC = jdTempMap["AGCVoltage"].toDouble();
                }
                else if (m_impl->item.workMode == SystemWorkMode::SKT)
                {
                    jdTempMap = GlobalData::getReportParamData(m_impl->jdid, 2, QList<QString>() << "Status2" << m_impl->ktAGCKey);
                    if (jdTempMap.isEmpty())
                    {
                        pushLog("当前测控基带AGC电压获取错误", true);
                        return false;
                    }
                    jdCarrierLockInd = (jdTempMap["Status2"].toInt() == 1);
                    jdAGC = jdTempMap[m_impl->ktAGCKey].toDouble();
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
            if (!jdCarrierLockInd)
            {
                pushLog("当前测控基带失锁", true);
                break;
            }
            //            if(agc == 0 && sbf == 0){
            //                agc = agcResult.agc;
            //                sbf = agcResult.sbf;
            //                errorflag = 0;
            //            }
            //            else{
            //                if(agc > agcResult.agc && agc- 0.2 < agcResult.agc  && sbf > agcResult.sbf && sbf  - 5 < agcResult.sbf){
            //                    agc = agcResult.agc;
            //                    sbf = agcResult.sbf;
            //                    errorflag= 0;
            //                }
            //                else{
            //                    errorflag += 1;
            //                }
            //            }
            //            if(errorflag == 4){
            //                pushLog("SBF值连续不在范围，停止标校", true);
            //                break;
            //            }
            if (errorflag == 0)
            {
                m_impl->pack.data.clear();
                AGCCalibrationProcessACK ack;
                ack.result = AGCCalibrationStatus::Info;
                ack.value = agcResult;

                m_impl->pack.data << ack;
                m_impl->item.result << agcResult;

                emit sg_testProcessInfo(m_impl->pack);

                max += 2;
            }
        }
    }
    return true;
}
bool AGCCalibrationServer::resultOperation()
{
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
    return true;
}

void AGCCalibrationServer::endOfTest()
{
    PackCommand packCommand;
    DeviceID jdid;
    /* 关闭前端测试开关网络电源 2开1关 */
    m_impl->singleCmdHelper.packSingleCommand("StepQDMNY_OFF_POWER", packCommand);
    waitExecSuccess(packCommand, 0);
}

bool AGCCalibrationServer::isExit() const { return !m_impl->runningFlag.load(); }
void AGCCalibrationServer::startServer(const ProtocolPack& pack)
{
    m_impl->pack = pack;
    m_impl->pack.data >> m_impl->item;
    m_impl->item.result.clear();
    start();
}
void AGCCalibrationServer::stopServer()
{
    disconnect();
    m_impl->runningFlag = false;
    if (!isRunning())
    {
        return;
    }

    quit();
    while (isRunning() && !this->isFinished())
    {
        QThread::msleep(10);
        QApplication::processEvents();
    }
    wait();

    /* 善后处理 */
    if (m_impl->isCallStop)
    {
        endOfTest();
        m_impl->isCallStop = false;
    }
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

AGCCalibrationServer::ExecStatus AGCCalibrationServer::waitExecSuccessImpl(const PackCommand& packCommand, qint32 ttl, bool timeoutError)
{
    if (isExit())
    {
        return ExecStatus::Exit;
    }
    if (!packCommand.errorMsg.isEmpty())
    {
        pushLog(packCommand.errorMsg, true);
        return ExecStatus::Failed;
    }

    if (packCommand.data.isEmpty())
    {
        auto msg = QString("系统组包失败,请检查配置,当前的设备ID：0x%1,模式Id：0x%2,命令Id：%3,命令类型：%4")
                       .arg(packCommand.deviceID.toHex(), QString::number(packCommand.modeId, 16).toUpper())
                       .arg(packCommand.cmdId)
                       .arg(DevProtocolEnumHelper::devMsgTypeToDescStr(packCommand.cmdType));
        pushLog(msg, true);
        return ExecStatus::Failed;
    }

    // 当前命令操作信息
    auto currentCmdOperatorInfo = packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo;
    // 判断设备是否在线
    if (!GlobalData::getDeviceOnline(packCommand.deviceID))
    {
        auto msg = QString("%1设备离线,发送%2命令(%3)失败").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);
        pushLog(msg, true);
        return ExecStatus::Failed;
    }

    // 发送数据
    sendByteArrayToDevice(packCommand.data);

    // 写入日志当前命令执行的操作
    currentCmdOperatorInfo = QString("%1-%2: %3").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);
    pushLog(currentCmdOperatorInfo);

    // 不需要等待响应的命令
    if (ttl <= 0)
    {
        return ExecStatus::Success;
    }

    auto redisFindKey = DeviceProcessHelper::getCmdResponseRedisKey(packCommand.deviceID, packCommand.cmdId);
    while (ttl > 0)
    {
        if (isExit())
        {
            return ExecStatus::Exit;
        }
        QThread::sleep(1);
        --ttl;

        // 获取命令执行结果
        QString cmdResponceStr;
        if (!RedisHelper::getInstance().getData(redisFindKey, cmdResponceStr) || cmdResponceStr.isEmpty())
        {
            continue;
        }

        ControlCmdResponse cmdResponce;
        cmdResponceStr >> cmdResponce;

        // 如果是当前发送的命令响应
        if (cmdResponce.isValid() && cmdResponce.deviceID == packCommand.deviceID && cmdResponce.cmdId == packCommand.cmdId &&
            cmdResponce.cmdType == packCommand.cmdType)
        {
            auto cmdExecResultDesc = DevProtocolEnumHelper::controlCmdResponseToDescStr(cmdResponce.responseResult);
            auto message = QString("%1 (%2)").arg(currentCmdOperatorInfo, cmdExecResultDesc);
            pushLog(message);
            return ExecStatus::Success;
        }
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

    auto currentCmdOperatorInfo = packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo;
    currentCmdOperatorInfo = QString("%1-%2: %3").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);
    pushLog(QString("%1  等待状态切换中").arg(currentCmdOperatorInfo));

    /* 等待状态改变 */
    while (ttl > 0)
    {
        if (isExit())
        {
            return ExecStatus::Exit;
        }

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
    auto currentCmdOperatorInfo = packCommand.operatorInfo.isEmpty() ? packCommand.getSetParamDescValue() : packCommand.operatorInfo;
    currentCmdOperatorInfo = QString("%1-%2: %3").arg(packCommand.getCmdDeviceName(), packCommand.getCmdName(), currentCmdOperatorInfo);

    for (int i = 0; i < count; ++i)
    {
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
        auto redisFindKey = DeviceProcessHelper::getCmdResultRedisKey(packCommand.deviceID, packCommand.cmdId);
        while (timeout > 0)
        {
            if (isExit())
            {
                return ExecStatus::Exit;
            }
            QThread::sleep(1);
            QCoreApplication::processEvents();
            --timeout;

            // 获取命令执行结果
            QString cmdResultStr;
            if (!RedisHelper::getInstance().getData(redisFindKey, cmdResultStr) || cmdResultStr.isEmpty())
            {
                continue;
            }
            cmdResultStr >> cmdResult;

            // 如果是当前发送的命令响应
            if (cmdResult.m_systemNumb == packCommand.deviceID.sysID &&   //
                cmdResult.m_deviceNumb == packCommand.deviceID.devID &&   //
                cmdResult.m_extenNumb == packCommand.deviceID.extenID &&  //
                cmdResult.m_cmdID == packCommand.cmdId)
            {
                auto message = QString("%1 结果上报成功").arg(currentCmdOperatorInfo);
                pushLog(message);
                return ExecStatus::Success;
            }
        }
        pushLog(QString("%1: 未收到结果超时重试").arg(currentCmdOperatorInfo));
    }

    auto message = QString("%1 等待结果上报超时").arg(currentCmdOperatorInfo);
    pushLog(message, true);
    return ExecStatus::Timeout;
}
