#include "CCZFParamMacroHandler.h"
#include "BusinessMacroCommon.h"
#include "GlobalData.h"
#include "RedisHelper.h"
#include "SatelliteManagementDefine.h"

#include "MessagePublish.h"

class CCZFParamMacroHandlerImpl
{
public:
    /* 是否是测试 */
    bool isTest{ false };
    /* 卫星信息 */
    SatelliteManagementDataMap satelliteManagementDataMap;
    TaskPlanData taskPlanData;
    QMap<SystemWorkMode, QMap<int, AutomaticRunningData>> dataMap;

public:
    /* 获取单元参数 */
    QVariantMap createCCZFSaveUnitMap(const AutomaticRunningData& taskItem, const DeviceID& scjd, int baseChannel);
    static QString createGSDeviceName(int extenID, int channel);
    static QString createDSDeviceName(int extenID, int channel);
    /* 获取命令参数 */
    QVariantMap createCCZFSaveCmdMap(const AutomaticRunningData& taskItem, const QString& dataCenter, int channel);
    OptionalNotValue getCCZFParam(const TargetInfo& taskItem, SystemWorkMode workMode, SatelliteManagementData& sate,
                                  SystemOrientation& systemOrientation, AutomaticRunningData& automaticRunningData);

    /* 获取基带的I/Q和路分路 */
    static int getGSIQStatus(const DeviceID& deviceID, int baseChannel);
};

QVariantMap CCZFParamMacroHandlerImpl::createCCZFSaveUnitMap(const AutomaticRunningData& taskItem, const DeviceID& scjd, int baseChannel)
{
    QString deviceName;
    if (baseChannel == 1 || baseChannel == 2)
    {
        deviceName = createGSDeviceName(scjd.extenID, baseChannel);
    }
    else
    {
        deviceName = createDSDeviceName(scjd.extenID, baseChannel);
    }

    QVariantMap saveDataMap;
    saveDataMap["BaseDev"] = deviceName;                                            // 设备名称
    saveDataMap["BaseChan"] = baseChannel;                                          // 通道号1
    saveDataMap["DateFormat"] = "B";                                                // 日期类型  北京时间
    saveDataMap["Date"] = GlobalData::currentDate().toString(DATE_DISPLAY_FORMAT);  // 日期
    saveDataMap["Time"] = GlobalData::currentTime().toString(TIME_DISPLAY_FORMAT);  // 时间
    saveDataMap["InformCat"] = 1;                                                   // 信息类别 默认1
    saveDataMap["TaskCircle"] = taskItem.circleNo;                                  // 圈次号
    saveDataMap["ObjectIden"] = taskItem.taskBZ;                                    // 对象标识 任务标识
    saveDataMap["DpNum"] = taskItem.dpNo;                                           // 目前没有使用，这里使用任务计划的点频
    saveDataMap["XuanXiang"] = static_cast<int>(taskItem.orientation);              // 旋向
    //获取基带当前是分路还是和路
    saveDataMap["IQOutPut"] = getGSIQStatus(scjd, baseChannel);  //和路输出还是分路输出，
    saveDataMap["IChannelNum"] = 312;                            // I通道号
    saveDataMap["QChannelNum"] = 311;                            // Q通道号

    //联调是TS，实战时OP，默认都是OP
    saveDataMap["PattIdent"] = isTest ? "TS" : "OP";  //模式标识
    return saveDataMap;
}
QString CCZFParamMacroHandlerImpl::createGSDeviceName(int extenID, int channel)
{
    return QString("HDD%1%2").arg(extenID, 1, 10, QChar(48)).arg(channel, 2, 10, QChar(48));
}
QString CCZFParamMacroHandlerImpl::createDSDeviceName(int extenID, int channel)
{
    return QString("LDD%1%2").arg(extenID, 1, 10, QChar(48)).arg(channel - 2, 2, 10, QChar(48));
}
QVariantMap CCZFParamMacroHandlerImpl::createCCZFSaveCmdMap(const AutomaticRunningData& taskItem, const QString& dataCenter, int channel)
{
    QDateTime tempDateTime;
    QVariantMap satPlanParamMap;
    if (taskItem.dtStartTime.isValid())  //数传时间有效就使用，无效就用任务开始时间
    {
        satPlanParamMap["StartTime"] = taskItem.dtStartTime.toString(DATETIME_DISPLAY_FORMAT3);  // 任务开始时间 yyyyMMddhhmmss
    }
    else if (taskItem.taskStartTime.isValid())
    {
        satPlanParamMap["StartTime"] = taskItem.taskStartTime.toString(DATETIME_DISPLAY_FORMAT3);  // 任务开始时间 yyyyMMddhhmmss
    }
    else
    {
        tempDateTime = GlobalData::currentDateTime().addSecs(20);
        satPlanParamMap["StartTime"] = tempDateTime.toString(DATETIME_DISPLAY_FORMAT3);  // 任务开始时间 yyyyMMddhhmmss
    }

    if (taskItem.dtEndTime.isValid())  //数传时间有效就使用，无效就用任务开始时间
    {
        satPlanParamMap["TEndTime"] = taskItem.dtEndTime.toString(DATETIME_DISPLAY_FORMAT3);  // 任务结束时间 yyyyMMddhhmmss
    }
    else if (taskItem.taskEndTime.isValid())
    {
        satPlanParamMap["TEndTime"] = taskItem.taskEndTime.toString(DATETIME_DISPLAY_FORMAT3);  // 任务结束时间 yyyyMMddhhmmss
    }
    else
    {
        satPlanParamMap["TEndTime"] = tempDateTime.addYears(3).toString(DATETIME_DISPLAY_FORMAT3);  // 任务结束时间 yyyyMMddhhmmss
    }

    // 事后传输时间 yyyyMMddhhmmss
    satPlanParamMap["PostTime"] = GlobalData::currentDateTime().addSecs(5 * 60).toString(DATETIME_DISPLAY_FORMAT3);
    satPlanParamMap["DataTran"] = dataCenter;

    if (taskItem.dtWorkingMode == DataTransMode::FirstRealTime)
    {
        satPlanParamMap["OperatMode"] = static_cast<int>(DataTransMode::RealTime);  //首次接收的事后 还是按照实时来处理
    }
    else
    {
        satPlanParamMap["OperatMode"] = static_cast<int>(taskItem.dtWorkingMode);
    }

    satPlanParamMap["TaskCode"] = taskItem.taskCode;                    // 任务代号
    satPlanParamMap["TaskMID"] = taskItem.taskBZ;                       // 任务标识
    satPlanParamMap["CircleNumb"] = taskItem.circleNo;                  // 圈次编号
    satPlanParamMap["Priority"] = static_cast<int>(taskItem.priority);  // 优先级
    satPlanParamMap["LocalNumb"] = channel;                             // 通道1数据

    satPlanParamMap["CtrlSign"] = 2;                                            // 启动
    satPlanParamMap["DataProtocal"] = static_cast<int>(taskItem.dataProtocal);  // 协议
    // TraceNum 先随便 给一个
    // Agrenment 协议是和基带的 这里在配置文件里面配置
    // TranSpeed 速率直接给最大存转会自适应 配置文件中配
    return satPlanParamMap;
}

OptionalNotValue CCZFParamMacroHandlerImpl::getCCZFParam(const TargetInfo& taskItem, SystemWorkMode workMode, SatelliteManagementData& sate,
                                                         SystemOrientation& systemOrientation, AutomaticRunningData& automaticRunningData)
{
    /* 获取旋向 */
    if (!satelliteManagementDataMap.contains(taskItem.taskCode))
    {
        auto msg = QString("存储转发参数配置:未能查找到任务代号为%1的卫星数据").arg(taskItem.taskCode);
        return OptionalNotValue(ErrorCode::InvalidArgument, msg);
    }

    sate = satelliteManagementDataMap.value(taskItem.taskCode);
    sate.getSReceivPolar(workMode, taskItem.pointFreqNo, systemOrientation);
    if (systemOrientation == SystemOrientation::Unkonwn)
    {
        auto msg = QString("存储转发参数配置:获取任务代号为%1的卫星接收旋向错误").arg(taskItem.taskCode);
        return OptionalNotValue(ErrorCode::InvalidArgument, msg);
    }

    auto modeData = dataMap.value(workMode);
    if (modeData.isEmpty())
    {
        automaticRunningData.circleNo = 1;
        automaticRunningData.dtWorkingMode = DataTransMode::RealTime;
        // 存转的开始时间不能小于当前时间
        automaticRunningData.dtStartTime = QDateTime();
        automaticRunningData.dtEndTime = QDateTime();
        automaticRunningData.taskStartTime = QDateTime();
        automaticRunningData.taskEndTime = QDateTime();
        automaticRunningData.priority = DataTransPriority::Urgent;
        automaticRunningData.dataProtocal = CenterProtocol::PDXP;
        automaticRunningData.centerName = sate.m_digitalCenterID;
        automaticRunningData.centerNameList = sate.m_digitalCenterID.split("+");
    }
    else
    {
        automaticRunningData = modeData.first();
        if (automaticRunningData.centerName.isEmpty())
        {
            automaticRunningData.centerName = sate.m_digitalCenterID;
            automaticRunningData.centerNameList = sate.m_digitalCenterID.split("+");
        }
        else
        {
            automaticRunningData.centerName = automaticRunningData.centerName;
            automaticRunningData.centerNameList = automaticRunningData.centerName.split("+");
        }
    }

    automaticRunningData.dpNo = taskItem.pointFreqNo;
    automaticRunningData.orientation = systemOrientation;
    automaticRunningData.taskCode = sate.m_satelliteCode;
    automaticRunningData.taskBZ = sate.m_satelliteIdentification;
    if (automaticRunningData.centerNameList.isEmpty())
    {
        auto msg = QString("没有匹配到对应的数传中心");
        return OptionalNotValue(ErrorCode::InvalidArgument, msg);
    }

    return OptionalNotValue();
}

int CCZFParamMacroHandlerImpl::getGSIQStatus(const DeviceID& deviceID, int baseChannel)
{
    auto value = GlobalData::getReportParamData(deviceID, baseChannel == 1 ? 2 : 6, "DemodOutputSelect").toInt();
    return value == 1 ? 1 : 2;
}

/*****************************************************************************************************/
/*****************************************************************************************************/
/*****************************************************************************************************/
/*****************************************************************************************************/

CCZFParamMacroHandler::CCZFParamMacroHandler(QObject* parent)
    : BaseParamMacroHandler(parent)
    , m_impl(new CCZFParamMacroHandlerImpl)
{
}
CCZFParamMacroHandler::~CCZFParamMacroHandler() {}
bool CCZFParamMacroHandler::handle()
{
    bool isExist = false;
    for (auto& item : mManualMsg.linkLineMap)
    {
        if (SystemWorkModeHelper::isDataTransmissionWorkMode(item.workMode))
        {
            isExist = true;
            break;
        }
    }
    if (!isExist)
    {
        emit signalInfoMsg("检测当前无数传模式跳过存转参数下发");
        return true;
    }
    if (!paramConvert())
    {
        return false;
    }

    /* 先获取所有的卫星数据 */
    m_impl->satelliteManagementDataMap = SatelliteManagementDataMap();
    if (!GlobalData::getSatelliteManagementData(m_impl->satelliteManagementDataMap))
    {
        emit signalErrorMsg("存储转发参数配置:获取系统已配置的卫星数据失败");
        return false;
    }

    configCCZFKaHS();
    configCCZFLS();

    return true;
}

SystemWorkMode CCZFParamMacroHandler::getSystemWorkMode() { return SystemWorkMode::NotDefine; }
void CCZFParamMacroHandler::setCCZFPlan(const TaskPlanData& taskPlanData) { m_impl->taskPlanData = taskPlanData; }

void CCZFParamMacroHandler::configCCZFKaHS()
{
    auto workMode = KaGS;
    if (!mManualMsg.linkLineMap.contains(workMode))
    {
        return;
    }
    auto& configMacroData = mManualMsg.configMacroData;
    auto& linkLineMap = mManualMsg.linkLineMap[workMode];
    auto& targetMap = linkLineMap.targetMap;

    DeviceID scMasterID;
    configMacroData.getGSJDDeviceID(workMode, scMasterID, 1);
    if (!scMasterID.isValid())
    {
        emit signalErrorMsg("无法查找到当前的高速基带主机信息,系统出错");
        return;
    }

    DeviceID czMasterID;
    DeviceID czSlaveID;
    configMacroData.getCCZFDeviceID(workMode, czMasterID, 1);
    configMacroData.getCCZFDeviceID(workMode, czSlaveID, 2);
    if (!czMasterID.isValid() || !czSlaveID.isValid())
    {
        emit signalErrorMsg("无法查找到高速分配的存储转发设备,系统出错");
        return;
    }

    /*
     * 高速有这么几种情况
     * 1.单点频单旋向
     * 2.单点频双旋向
     * 3.双点频单旋向
     * 4.双点频双旋向
     *
     * 123都是占用一个基带
     * 当使用1个中心的时候 是主机按照计划策略走 备机只接收 只连接主机
     * 2个中心的时候是主机按照计划策略走和中心1 备机按照计划策略走和中心2 只连接主机
     * 多中心直接主机按照计划策略走向多个中心发  备机只接收 只连接主机
     *
     * 4占用两个基带
     * 无论是那种情况都是 主机按照策略走向多个中心发 备机只接收
     */

    //单点频
    if (targetMap.size() == 1)
    {
        auto taskItem = targetMap.first();
        SatelliteManagementData sate;
        auto systemOrientation = SystemOrientation::Unkonwn;
        AutomaticRunningData automaticRunningData;
        auto result = m_impl->getCCZFParam(taskItem, workMode, sate, systemOrientation, automaticRunningData);
        if (!result)
        {
            emit signalErrorMsg(result.msg());
            return;
        }

        /* 单旋向默认通道1就行 */
        int gsChannelNo = 1;
        auto unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData, scMasterID, gsChannelNo);

        /* 主备下单元参数 */
        mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(gsChannelNo), mPackCommand, czMasterID, unitParam);
        waitExecSuccess(mPackCommand);

        unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData, scMasterID, gsChannelNo);
        mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(gsChannelNo), mPackCommand, czSlaveID, unitParam);
        waitExecSuccess(mPackCommand);

        /* 单点频双旋向
         * 双旋向的话再下一个通道二的数据
         */
        if (systemOrientation == SystemOrientation::LRCircumflex)
        {
            /* 通道二 */
            gsChannelNo = 2;
            unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData, scMasterID, gsChannelNo);
            mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(gsChannelNo), mPackCommand, czMasterID, unitParam);
            waitExecSuccess(mPackCommand);

            unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData, scMasterID, gsChannelNo);
            mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(gsChannelNo), mPackCommand, czSlaveID, unitParam);
            waitExecSuccess(mPackCommand);
        }

        /*****************************************************************/
        /*****************************************************************/
        /*****************************************************************/
        /*****************************************************************/
        /* 上面下宏 下面下计划 */
        clearQueue();
        if (automaticRunningData.centerNameList.size() == 1)
        {
            /*
             * 1个中心 单 点频
             * 主机按照计划走 备机只接收
             */
            if (systemOrientation == SystemOrientation::LRCircumflex)
            {
                /* 主机 */
                auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerNameList.first(), 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                appendExecQueue(mPackCommand);

                cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerNameList.first(), 2);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                appendExecQueue(mPackCommand);

                /* 备机 */
                automaticRunningData.dtWorkingMode = DataTransMode::OnlyRecv;
                cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerNameList.first(), 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                appendExecQueue(mPackCommand);

                cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerNameList.first(), 2);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                appendExecQueue(mPackCommand);
            }
            else
            {
                auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerNameList.first(), 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                appendExecQueue(mPackCommand);

                automaticRunningData.dtWorkingMode = DataTransMode::OnlyRecv;
                cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerNameList.first(), 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                appendExecQueue(mPackCommand);
            }
        }
        else if (automaticRunningData.centerNameList.size() == 2)
        {
            /*
             * 2个中心 单 点频
             * 主机按照计划走向中心1发
             * 备机按照计划走向中心2发
             */
            auto center1 = automaticRunningData.centerNameList.first();
            auto center2 = automaticRunningData.centerNameList.last();
            if (systemOrientation == SystemOrientation::LRCircumflex)
            {
                /* 主机 */
                auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, center1, 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                appendExecQueue(mPackCommand);

                cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, center1, 2);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                appendExecQueue(mPackCommand);

                /* 备机 */
                cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, center2, 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                appendExecQueue(mPackCommand);

                cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, center2, 2);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                appendExecQueue(mPackCommand);
            }
            else
            {
                /* 主机 */
                auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, center1, 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                appendExecQueue(mPackCommand);

                /* 备机 */
                cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, center2, 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                appendExecQueue(mPackCommand);
            }
        }
        else
        {
            /* 大于3个中心
             * 主机按照计划走向所有中心发
             * 备机只接收
             */
            if (systemOrientation == SystemOrientation::LRCircumflex)
            {
                /* 主机 */
                auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerName, 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                appendExecQueue(mPackCommand);

                cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerName, 2);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                appendExecQueue(mPackCommand);

                /* 备机 */
                automaticRunningData.dtWorkingMode = DataTransMode::OnlyRecv;
                cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerName, 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                appendExecQueue(mPackCommand);

                cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerName, 2);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                appendExecQueue(mPackCommand);
            }
            else
            {
                /* 主机 */
                auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerName, 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                appendExecQueue(mPackCommand);

                /* 备机 */
                automaticRunningData.dtWorkingMode = DataTransMode::OnlyRecv;
                cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerName, 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                appendExecQueue(mPackCommand);
            }
        }

        execQueue();
        return;
    }

    /* 双点频 */
    if (targetMap.size() == 2)
    {
        auto taskItem2 = targetMap.last();
        SatelliteManagementData sate2;
        auto systemOrientation2 = SystemOrientation::Unkonwn;
        AutomaticRunningData automaticRunningData2;
        auto result = m_impl->getCCZFParam(taskItem2, workMode, sate2, systemOrientation2, automaticRunningData2);
        if (!result)
        {
            emit signalErrorMsg(result.msg());
            return;
        }

        auto taskItem1 = targetMap.first();
        SatelliteManagementData sate1;
        auto systemOrientation1 = SystemOrientation::Unkonwn;
        AutomaticRunningData automaticRunningData1;
        result = m_impl->getCCZFParam(taskItem1, workMode, sate1, systemOrientation1, automaticRunningData1);
        if (!result)
        {
            emit signalErrorMsg(result.msg());
            return;
        }

        /* 任意一个是 双点频都是占用两个基带 */
        if (systemOrientation1 == SystemOrientation::LRCircumflex || systemOrientation2 == SystemOrientation::LRCircumflex)
        {
            DeviceID scSlaveID;
            configMacroData.getGSJDDeviceID(workMode, scSlaveID, 2);
            if (!scMasterID.isValid())
            {
                emit signalErrorMsg("无法查找到当前的高速基带备机信息,系统出错");
                return;
            }
            /* 每台的第一个通道 */
            int gsChannelNo = 1;
            auto unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData1, scMasterID, gsChannelNo);
            mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(gsChannelNo), mPackCommand, czMasterID, unitParam);
            waitExecSuccess(mPackCommand);

            unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData2, scSlaveID, gsChannelNo);
            mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(gsChannelNo), mPackCommand, czSlaveID, unitParam);
            waitExecSuccess(mPackCommand);

            /* 第二个通道 */
            gsChannelNo = 2;
            if (systemOrientation1 == SystemOrientation::LRCircumflex)
            {
                unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData1, scMasterID, gsChannelNo);
                mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(gsChannelNo), mPackCommand, czMasterID, unitParam);
                waitExecSuccess(mPackCommand);
            }
            if (systemOrientation2 == SystemOrientation::LRCircumflex)
            {
                unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData2, scSlaveID, gsChannelNo);
                mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(gsChannelNo), mPackCommand, czSlaveID, unitParam);
                waitExecSuccess(mPackCommand);
            }
        }
        /* 都是单点频 */
        else
        {
            /* 主备第一个通道 */
            int gsChannelNo = 1;
            auto unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData1, scMasterID, gsChannelNo);
            mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(gsChannelNo), mPackCommand, czMasterID, unitParam);
            waitExecSuccess(mPackCommand);

            unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData1, scMasterID, gsChannelNo);
            mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(gsChannelNo), mPackCommand, czSlaveID, unitParam);
            waitExecSuccess(mPackCommand);

            /* 主备第二个通道 */
            gsChannelNo = 2;
            unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData2, scMasterID, gsChannelNo);
            mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(gsChannelNo), mPackCommand, czMasterID, unitParam);
            waitExecSuccess(mPackCommand);

            unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData2, scMasterID, gsChannelNo);
            mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(gsChannelNo), mPackCommand, czSlaveID, unitParam);
            waitExecSuccess(mPackCommand);
        }

        /*****************************************************************/
        /*****************************************************************/
        /*****************************************************************/
        /*****************************************************************/
        /* 上面下宏 下面下计划 */
        clearQueue();
        if (systemOrientation1 == SystemOrientation::LRCircumflex || systemOrientation2 == SystemOrientation::LRCircumflex)
        {
            /* 任意一个是双极化双点频
             * 就会占用两个高速主机
             * 存转也只能两个是主机
             * 然后同时向所有中心发
             */
            /* 先把中心都合在一起 */
            auto tempList = automaticRunningData1.centerNameList;
            tempList.append(automaticRunningData2.centerNameList);
            auto centerName = tempList.toSet().toList().join("+");

            /* 然后都向这里发 */
            if (systemOrientation1 == SystemOrientation::LRCircumflex)
            {
                auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData1, centerName, 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                appendExecQueue(mPackCommand);

                cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData1, centerName, 2);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                appendExecQueue(mPackCommand);
            }
            else
            {
                auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData1, centerName, 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                appendExecQueue(mPackCommand);
            }

            if (systemOrientation2 == SystemOrientation::LRCircumflex)
            {
                auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData2, centerName, 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                appendExecQueue(mPackCommand);

                cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData2, centerName, 2);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                appendExecQueue(mPackCommand);
            }
            else
            {
                auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData2, centerName, 1);
                mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                appendExecQueue(mPackCommand);
            }
        }
        else
        {
            auto dsFunc = [&](AutomaticRunningData& automaticRunningData, int channel) {
                /* 都是 单点频
                 * 那每个都占用1个通道
                 * 然后如果是一个中心那么主机按照计划走 备机只接收
                 */
                if (automaticRunningData.centerNameList.size() == 1)
                {
                    auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerNameList.first(), channel);
                    mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                    appendExecQueue(mPackCommand);

                    automaticRunningData.dtWorkingMode = DataTransMode::OnlyRecv;
                    cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerNameList.first(), channel);
                    mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                    appendExecQueue(mPackCommand);
                }
                /* 两个中心则每个 基带向不同的中心发送 */
                else if (automaticRunningData.centerNameList.size() == 2)
                {
                    auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerNameList.first(), channel);
                    mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                    appendExecQueue(mPackCommand);

                    cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerNameList.last(), channel);
                    mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                    appendExecQueue(mPackCommand);
                }
                /* 中心很多就基带1向中心发 另一台只接收 */
                else
                {
                    auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerName, channel);
                    mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
                    appendExecQueue(mPackCommand);

                    automaticRunningData.dtWorkingMode = DataTransMode::OnlyRecv;
                    cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerName, channel);
                    mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
                    appendExecQueue(mPackCommand);
                }
            };

            dsFunc(automaticRunningData1, 1);
            dsFunc(automaticRunningData2, 2);
        }

        execQueue();
        return;
    }

    emit signalErrorMsg("当前高速点频过多超出程序支持范围");
    return;
}
bool CCZFParamMacroHandler::paramConvert()
{
    if (m_impl->taskPlanData.deviceWorkTask.m_totalTargetMap.isEmpty())
    {
        return true;
    }

    QMap<QString, DataTranWorkTask> dataTranWorkTaskMap;
    for (auto& item : m_impl->taskPlanData.dataTransWorkTasks)
    {
        dataTranWorkTaskMap.insert(item.m_dt_transNum, item);
    }

    for (auto& target : m_impl->taskPlanData.deviceWorkTask.m_totalTargetMap)
    {
        for (auto& link : target.m_linkMap)
        {
            AutomaticRunningData tempAutomaticRunningData;
            tempAutomaticRunningData.circleNo = target.m_circle_no;
            tempAutomaticRunningData.dpNo = link.working_point_frequency;

            if (dataTranWorkTaskMap.contains(target.m_dt_transNum))
            {
                auto dtData = dataTranWorkTaskMap.value(target.m_dt_transNum);

                tempAutomaticRunningData.dtWorkingMode = dtData.m_working_mode;
                tempAutomaticRunningData.taskStartTime = target.m_task_start_time;
                tempAutomaticRunningData.taskEndTime = target.m_task_end_time;
                tempAutomaticRunningData.dtStartTime = dtData.m_task_start_time;
                tempAutomaticRunningData.dtEndTime = dtData.m_task_end_time;
                tempAutomaticRunningData.priority = dtData.m_priority;
                tempAutomaticRunningData.centerName = dtData.m_datatrans_center;
                tempAutomaticRunningData.dataProtocal = dtData.m_center_protocol;
                m_impl->dataMap[link.m_work_system][tempAutomaticRunningData.dpNo] = tempAutomaticRunningData;
            }
            else
            {
                /* 没找到用设备计划的 */
                tempAutomaticRunningData.dtWorkingMode = DataTransMode::RealTime;
                tempAutomaticRunningData.taskStartTime = target.m_task_start_time;
                tempAutomaticRunningData.taskEndTime = target.m_task_end_time;
                tempAutomaticRunningData.dtStartTime = target.m_dt_starttime;
                tempAutomaticRunningData.dtEndTime = target.m_dt_endtime;
                tempAutomaticRunningData.priority = DataTransPriority::Urgent;
                tempAutomaticRunningData.centerName = QString();
                tempAutomaticRunningData.dataProtocal = CenterProtocol::PDXP;
                m_impl->dataMap[link.m_work_system][tempAutomaticRunningData.dpNo] = tempAutomaticRunningData;
            }
        }
    }

    return true;
}
#if 0
void CCZFParamMacroHandler::configCCZFLS()
{
    /*
     * 通道3 是Ka
     * 通道4 是X
     */
    /* 不存在低速 */
    if (!mManualMsg.linkLineMap.contains(KaDS) && !mManualMsg.linkLineMap.contains(XDS))
    {
        return;
    }

    /* 所有的数传获取到的存转设备是一样的 */
    auto& configMacroData = mManualMsg.configMacroData;
    DeviceID czMasterID;
    DeviceID czSlaveID;
    configMacroData.getCCZFDeviceID(KaDS, czMasterID, 1);
    configMacroData.getCCZFDeviceID(KaDS, czSlaveID, 2);
    if (!czMasterID.isValid() || !czSlaveID.isValid())
    {
        emit signalErrorMsg("无法查找到低速分配的存储转发设备,系统出错");
        return;
    }

    /*
     * 低速是单元34 通道12
     */
    auto dsParamFunc = [&](SystemWorkMode workMode, int channel) {
        if (mManualMsg.linkLineMap.contains(workMode))
        {
            DeviceID scMasterID;
            DeviceID scSlaveID;
            configMacroData.getDSJDDeviceID(workMode, scMasterID, 1);
            configMacroData.getDSJDDeviceID(workMode, scSlaveID, 2);
            if (!scMasterID.isValid() || !scSlaveID.isValid())
            {
                emit signalErrorMsg("无法查找到当前的低速基带信息,系统出错");
                return;
            }
            auto& linkLineMap = mManualMsg.linkLineMap[workMode];
            auto& targetMap = linkLineMap.targetMap;

            if (targetMap.size() != 1)
            {
                emit signalErrorMsg("当前系统只支持低速单点频");
                return;
            }

            auto taskItem = targetMap.first();
            SatelliteManagementData sate;
            auto systemOrientation = SystemOrientation::Unkonwn;
            AutomaticRunningData automaticRunningData;
            auto result = m_impl->getCCZFParam(taskItem, workMode, sate, systemOrientation, automaticRunningData);
            if (!result)
            {
                emit signalErrorMsg(result.msg());
                return;
            }

            auto unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData, scMasterID, channel);
            mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(channel), mPackCommand, czMasterID, unitParam);
            waitExecSuccess(mPackCommand);

            unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData, scSlaveID, channel);
            mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(channel), mPackCommand, czSlaveID, unitParam);
            waitExecSuccess(mPackCommand);

            /* 下计划
             * 低速计划不用向两个中心发 直接向所有中心发 另一台只接收
             */

            clearQueue();
            /* 主机 */
            auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerName, channel);
            mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
            appendExecQueue(mPackCommand);

            /* 备机 */
            automaticRunningData.dtWorkingMode = DataTransMode::OnlyRecv;
            cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerName, channel);
            mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
            appendExecQueue(mPackCommand);

            execQueue();
        }
    };

    dsParamFunc(KaDS, 3);
    dsParamFunc(XDS, 4);
}

#else
/* 20211130 他们又说低速基带支持一对多
 * 这样就变成了
 * 1个中心 存转主机按照计划 备机只接收 只连接主用基带
 * 2个中心 存转主机和备机每个分担一个中心 只连接主用基带
 * 多个中心 存转主机按照计划所有中心 备机只接收 只连接主用基带
 */
void CCZFParamMacroHandler::configCCZFLS()
{
    /*
     * 通道3 是Ka
     * 通道4 是X
     */
    /* 不存在低速 */
    if (!mManualMsg.linkLineMap.contains(KaDS) && !mManualMsg.linkLineMap.contains(XDS))
    {
        return;
    }

    /* 所有的数传获取到的存转设备是一样的 */
    auto& configMacroData = mManualMsg.configMacroData;
    auto workMode = configMacroData.configMacroCmdModeMap.contains(KaDS) ? KaDS : XDS;
    DeviceID czMasterID;
    DeviceID czSlaveID;
    configMacroData.getCCZFDeviceID(workMode, czMasterID, 1);
    configMacroData.getCCZFDeviceID(workMode, czSlaveID, 2);
    if (!czMasterID.isValid() || !czSlaveID.isValid())
    {
        emit signalErrorMsg("无法查找到低速分配的存储转发设备,系统出错");
        return;
    }

    /*
     * 低速是单元34 通道12
     */
    auto dsParamFunc = [&](SystemWorkMode workMode, int channel) {
        if (!mManualMsg.linkLineMap.contains(workMode))
        {
            return;
        }
        DeviceID scMasterID;
        configMacroData.getDSJDDeviceID(workMode, scMasterID, 1);
        if (!scMasterID.isValid())
        {
            emit signalErrorMsg("无法查找到当前的主用低速基带信息,系统出错");
            return;
        }
        auto& linkLineMap = mManualMsg.linkLineMap[workMode];
        auto& targetMap = linkLineMap.targetMap;

        if (targetMap.size() != 1)
        {
            emit signalErrorMsg("当前系统只支持低速单点频");
            return;
        }

        auto taskItem = targetMap.first();
        SatelliteManagementData sate;
        auto systemOrientation = SystemOrientation::Unkonwn;
        AutomaticRunningData automaticRunningData;
        auto result = m_impl->getCCZFParam(taskItem, workMode, sate, systemOrientation, automaticRunningData);
        if (!result)
        {
            emit signalErrorMsg(result.msg());
            return;
        }

        auto unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData, scMasterID, channel);
        mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(channel), mPackCommand, czMasterID, unitParam);
        waitExecSuccess(mPackCommand);

        unitParam = m_impl->createCCZFSaveUnitMap(automaticRunningData, scMasterID, channel);
        mSingleCommandHelper.packSingleCommand(QString("Step_SAT_UNIT%1_SAVE").arg(channel), mPackCommand, czSlaveID, unitParam);
        waitExecSuccess(mPackCommand);

        clearQueue();

        /* 1个中心 存转主机按照计划 备机只接收 只连接主用基带 */
        if (automaticRunningData.centerNameList.size() == 1)
        {
            /* 主机 */
            auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerNameList.first(), channel);
            mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
            appendExecQueue(mPackCommand);

            /* 备机 */
            automaticRunningData.dtWorkingMode = DataTransMode::OnlyRecv;
            cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerNameList.first(), channel);
            mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
            appendExecQueue(mPackCommand);
        }
        /* 2个中心 存转主机和备机每个分担一个中心 只连接主用基带 */
        else if (automaticRunningData.centerNameList.size() == 2)
        {
            /* 主机 */
            auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerNameList.first(), channel);
            mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
            appendExecQueue(mPackCommand);

            /* 备机 */
            cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerNameList.last(), channel);
            mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
            appendExecQueue(mPackCommand);
        }
        /* 多个中心 存转主机按照计划所有中心 备机只接收 只连接主用基带 */
        else
        {
            /* 主机 */
            auto cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerName, channel);
            mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czMasterID, cmdVarMap);
            appendExecQueue(mPackCommand);

            /* 备机 */
            automaticRunningData.dtWorkingMode = DataTransMode::OnlyRecv;
            cmdVarMap = m_impl->createCCZFSaveCmdMap(automaticRunningData, automaticRunningData.centerName, channel);
            mSingleCommandHelper.packSingleCommand("Step_DSFD_START", mPackCommand, czSlaveID, cmdVarMap);
            appendExecQueue(mPackCommand);
        }

        execQueue();
    };

    dsParamFunc(KaDS, 3);
    dsParamFunc(XDS, 4);
}

#endif
