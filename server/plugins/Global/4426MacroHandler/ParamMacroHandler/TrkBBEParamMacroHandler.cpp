#include "TrkBBEParamMacroHandler.h"
#include "AutorunPolicyMessageSerialize.h"
#include "BusinessMacroCommon.h"
#include "EquipmentCombinationHelper.h"
#include "GlobalData.h"
#include "PlatformInterface.h"
#include "SatelliteManagementDefine.h"
#include <QFile>

class TrkBBEParamMacroHandlerImpl
{
public:
    QMap<int, QList<TrkBBE::ParamInfo>> d_modeParamInfos;
    ParamMacroModeData m_ckParamMacroData;
    ParamMacroModeData m_scParamMacroData;

    TargetInfo ckTargetInfo;
    TargetInfo scTargetInfo;

    bool isKp = false;  //当前模式是否包含扩频模式
    // 跟踪基带
    QMap<DeviceID, QSet<SystemWorkMode>> gzjdDeviceIDMap;
    // 跟踪前端
    QMap<DeviceID, QSet<SystemWorkMode>> gzqdDeviceIDMap;
    /* 当前的跟踪模式 */
    SatelliteTrackingMode trackMode = SatelliteTrackingMode::TrackUnknown;

    // dealCKData处理测控数据   dealCKSpcialData处理包含有rule的特殊参数,下面同理
    void dealCKData(TrkBBE::ParamInfo& paramInfo, QMap<int, QMap<int, QMap<QString, QVariant>>> ckUnitTargetParamMap,
                    QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap);
    void dealCKSpcialData(QVariant& value, TrkBBE::ParamInfo& paramInfo, QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap);
    // dealCKData处理数传基带数据
    void dealSCData(TrkBBE::ParamInfo& paramInfo, QMap<int, QMap<QString, QVariant>> pointFreqUnitParamMap,
                    QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap);
    void dealGSSpcialData(QVariant& value, TrkBBE::ParamInfo& paramInfo, QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap);
    void dealDSSpcialData(QVariant& value, TrkBBE::ParamInfo& paramInfo, QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap);

    //处理跟踪基带低速的关联关系
    void dealWithDSRelationParam(QMap<int, QMap<int, QVariantMap>>& unitTargetParamMap);

    //处理跟踪旋向的值
    void dealTrackPolar(SystemWorkMode workMode, QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap);
};

TrkBBEParamMacroHandler::TrkBBEParamMacroHandler(QObject* parent)
    : BaseParamMacroHandler(parent)
    , m_impl(new TrkBBEParamMacroHandlerImpl)
{
    QString path = PlatformConfigTools::configBusiness("ParamMacroManager/TrkBBE.xml");
    TrkBBEXmlParser parser(path);
    m_impl->d_modeParamInfos = parser.getAllModesParamInfos();
}

TrkBBEParamMacroHandler::~TrkBBEParamMacroHandler() { delete m_impl; }

bool TrkBBEParamMacroHandler::handle()
{
    m_impl->gzjdDeviceIDMap.clear();
    m_impl->gzqdDeviceIDMap.clear();
    DeviceID gzjdDeviceID;
    for (auto workMode : mManualMsg.linkLineMap.keys())
    {
        if (mManualMsg.configMacroData.getGZJDDeviceID(workMode, gzjdDeviceID))
        {
            if (gzjdDeviceID.isValid())
            {
                m_impl->gzjdDeviceIDMap[gzjdDeviceID] << workMode;
                mManualMsg.configMacroData.getTrackMode(workMode, m_impl->trackMode);
            }
        }
        if (mManualMsg.configMacroData.getGZQDDeviceID(workMode, gzjdDeviceID))
        {
            if (gzjdDeviceID.isValid())
            {
                m_impl->gzqdDeviceIDMap[gzjdDeviceID] << workMode;
            }
        }
    }
    auto gzjdMasterNum = m_impl->gzjdDeviceIDMap.size();
    /*
     * 注：跟踪基带没有X频段
     * 注：跟踪基带主备对应情况目前为跟踪基带1和2互为主备机，跟踪基带3和4互为主备机
     * 注：S扩二和Ka扩二都是对应的跟踪基带的扩二，只是跟踪基带取的参数来自不同的模式而已
     * 注：规定好Ka高速只能走跟踪前端1/2  低速只能走跟踪前端3/4  测控则是可以选择跟踪前端1/2/3/4
     * 注：跟踪最终只会选择一个频段去跟踪，如一个天线跟踪一个卫星
     *
     *
     * 注：跟踪基带的下发包含主备机的下发，主机通过链路的配置宏取得跟踪前端的设备ID，根据跟踪前端和跟踪基带一一对应的特性再取得
     * 跟踪基带的设备ID；然后再根据跟踪基带12互为主备34互为主备的情况取得备机的设备ID，但是因为测控的跟踪前端它可以选择四个，如果你的主机是跟踪基带1
     * 那么备机的设备ID就得从剩下的跟踪基带2/3/4取，很有可能取了个4,4的主机是3而不是1，因此在有双模式的情况下我们通过数传的配置宏去取跟踪基带的主备机设备ID
     * 有一种双模式需要注意，扩跳+高速/低速，这种情况你就需要分配一台跟踪基带为扩跳模式，一种为高速/低速模式，根据主备都要下发的参数原则，这种情况你就
     * 要用到四台跟踪基带，这个时候你得先取扩跳的跟踪前端设备ID得到对应的跟踪基带ID，再取高速/低速的跟踪前端设备ID得到对应的跟踪基带ID，判断下从扩跳取得的
     * 跟踪基带ID是否和高速/低速取得的重复，如果重复就再分配，直至四台跟踪基带都对应上才可以下发参数
     */

    if (gzjdMasterNum > 2)
    {
        emit signalInfoMsg(QString("分配的跟踪基带的主机数量超过2,截取前两个主机进行处理"));
        int flag = 0;
        for (auto deviceID : m_impl->gzjdDeviceIDMap.keys())
        {
            flag++;
            if (flag > 2)
            {
                m_impl->gzjdDeviceIDMap.remove(deviceID);
            }
        }
    }

    /*
     * 注：跟踪基带里面的旋向没有在配置文件处理，在代码里统一处理
     */
    for (auto deviceID : m_impl->gzjdDeviceIDMap.keys())
    {
        ExitCheck(false);
        auto modeNum = m_impl->gzjdDeviceIDMap[deviceID].size();
        if (modeNum == 1)
        {
            singleModeDeal(deviceID);
        }
        if (modeNum == 2)
        {
            doubleModeDeal(deviceID);
        }
    }

    ExitCheck(false);
    /* 角误差控制 */
    configGZDeviceJWC();

    ExitCheck(false);
    /* 相位装订 */
    preBinding();
    return true;
}

bool TrkBBEParamMacroHandler::singleModeDeal(DeviceID gzDeviceID)
{
    auto modeList = m_impl->gzjdDeviceIDMap[gzDeviceID];
    auto needSwitchMode = convertGZJDMode(modeList);                   //跟踪基带需要切换的模式
    SystemWorkMode singleMode = SystemWorkMode::NotDefine;             //对应的单模式
    DeviceID deviceID;                                                 //单模式对应的设备ID
    auto gzDeviceIDSlave = getGZJDSlaveDeviceID(gzDeviceID);           //备机跟踪基带设备ID
    QMap<int, QMap<int, QMap<QString, QVariant>>> unitTargetParamMap;  //跟踪基带组包的Map
    //标准
    if (modeList.contains(STTC))
    {
        singleMode = STTC;
        mManualMsg.configMacroData.getCKJDDeviceID(STTC, deviceID);
    }
    // S扩频
    if (modeList.contains(Skuo2))
    {
        singleMode = Skuo2;
        mManualMsg.configMacroData.getCKJDDeviceID(Skuo2, deviceID);
        m_impl->isKp = true;
    }
    // Ka扩频
    if (modeList.contains(KaKuo2))
    {
        singleMode = KaKuo2;
        mManualMsg.configMacroData.getCKJDDeviceID(KaKuo2, deviceID);
        m_impl->isKp = true;
    }
    //扩跳
    if (modeList.contains(SKT))
    {
        singleMode = SKT;
        mManualMsg.configMacroData.getCKJDDeviceID(SKT, deviceID);
    }
    // Ka低速
    if (modeList.contains(KaDS))
    {
        singleMode = KaDS;
        mManualMsg.configMacroData.getDSJDDeviceID(KaDS, deviceID);
    }
    // Ka高速
    if (modeList.contains(KaGS))
    {
        singleMode = KaGS;
        mManualMsg.configMacroData.getGSJDDeviceID(KaGS, deviceID);
    }
    //设备ID校验、切换模式
    if (!deviceID.isValid())
    {
        emit signalErrorMsg("设备ID获取错误");
        return false;
    }
    //切换跟踪基带模式
    auto status = switchGZJDMasterAndSlaveWorkMode(gzDeviceID, gzDeviceIDSlave, static_cast<int>(needSwitchMode));  //主备机一起切换
    if (CKJDSwitchStatus::Failed == status)
    {
        emit signalErrorMsg(QString("跟踪基带主备机切换模式失败，预设工作模式与设备当前模式不符合"));
    }

    //    配置宏已经切换主备了
    //    //跟踪基带主机命令下发
    //    PackCommand packCommand;
    //    QMap<QString, QVariant> params;
    //    params["OnlineSelect"] = 1;
    //    mSingleCommandHelper.packSingleCommand(QString("Step_TrackBBE_OnlineSelect"), mPackCommand, gzDeviceID, params);
    //    appendExecQueue(mPackCommand);
    //    //备机命令下发
    //    params.clear();
    //    params["OnlineSelect"] = 2;
    //    mSingleCommandHelper.packSingleCommand(QString("Step_TrackBBE_OnlineSelect"), mPackCommand, gzDeviceIDSlave, params);
    //    appendExecQueue(mPackCommand);

    //组包
    if (SystemWorkModeHelper::isMeasureContrlWorkMode(singleMode))  // 测控模式参数宏都使用同一个
    {
        //获取参数宏数据
        auto ckLink = mManualMsg.linkLineMap[singleMode];
        m_impl->ckTargetInfo = ckLink.targetMap.value(ckLink.masterTargetNo);
        GlobalData::getParamMacroModeData(m_impl->ckTargetInfo.taskCode, singleMode, m_impl->m_ckParamMacroData);
        ckWorkDataSet(static_cast<int>(needSwitchMode), singleMode, deviceID, unitTargetParamMap);
    }
    else
    {
        //获取参数宏数据
        auto scLink = mManualMsg.linkLineMap[singleMode];
        m_impl->scTargetInfo = scLink.targetMap.value(scLink.masterTargetNo);
        GlobalData::getParamMacroModeData(m_impl->scTargetInfo.taskCode, singleMode, m_impl->m_scParamMacroData);
        scWorkDataSet(static_cast<int>(needSwitchMode), singleMode, deviceID, unitTargetParamMap);
    }

    //低速才会特殊处理码速率值
    if (modeList.contains(KaDS))
    {
        m_impl->dealWithDSRelationParam(unitTargetParamMap);
    }


    clearQueue();
    //参数下发 主机
    if (!packGroupParamSetData(gzDeviceID, static_cast<int>(needSwitchMode), unitTargetParamMap, mPackCommand))
    {
        emit signalErrorMsg(QString("%1组包失败").arg(GlobalData::getExtensionName(gzDeviceID)));
    }
    appendExecQueue(mPackCommand);

    //参数下发 备机
    if (!packGroupParamSetData(gzDeviceIDSlave, static_cast<int>(needSwitchMode), unitTargetParamMap, mPackCommand))
    {
        emit signalErrorMsg(QString("%1组包失败").arg(GlobalData::getExtensionName(gzDeviceIDSlave)));
    }
    appendExecQueue(mPackCommand);

    execQueue();
    return true;
}

bool TrkBBEParamMacroHandler::doubleModeDeal(DeviceID gzDeviceID)
{
    auto modeList = m_impl->gzjdDeviceIDMap[gzDeviceID];
    auto needSwitchMode = convertGZJDMode(modeList);  //跟踪基带需要切换的模式
    SystemWorkMode ckMode = SystemWorkMode::NotDefine;
    SystemWorkMode scMode = SystemWorkMode::NotDefine;
    DeviceID ckDeviceID;
    DeviceID scDeviceID;
    auto gzDeviceIDSlave = getGZJDSlaveDeviceID(gzDeviceID);
    QMap<int, QMap<int, QMap<QString, QVariant>>> unitTargetParamMap;
    //标准+Ka低速
    if (modeList.contains(STTC) && modeList.contains(KaDS))
    {
        ckMode = STTC;
        scMode = KaDS;
        mManualMsg.configMacroData.getCKJDDeviceID(STTC, ckDeviceID);
        mManualMsg.configMacroData.getDSJDDeviceID(KaDS, scDeviceID);
    }
    //标准+Ka高速
    if (modeList.contains(STTC) && modeList.contains(KaGS))
    {
        ckMode = STTC;
        scMode = KaGS;
        mManualMsg.configMacroData.getCKJDDeviceID(STTC, ckDeviceID);
        mManualMsg.configMacroData.getGSJDDeviceID(KaGS, scDeviceID);
    }
    // S扩频+Ka低速
    if (modeList.contains(Skuo2) && modeList.contains(KaDS))
    {
        ckMode = Skuo2;
        scMode = KaDS;
        mManualMsg.configMacroData.getCKJDDeviceID(Skuo2, ckDeviceID);
        mManualMsg.configMacroData.getDSJDDeviceID(KaDS, scDeviceID);
        m_impl->isKp = true;
    }
    // S扩频+Ka高速
    if (modeList.contains(Skuo2) && modeList.contains(KaGS))
    {
        ckMode = Skuo2;
        scMode = KaGS;
        mManualMsg.configMacroData.getCKJDDeviceID(Skuo2, ckDeviceID);
        mManualMsg.configMacroData.getGSJDDeviceID(KaGS, scDeviceID);
        m_impl->isKp = true;
    }
    // Ka扩频+Ka低速
    if (modeList.contains(KaKuo2) && modeList.contains(KaDS))
    {
        ckMode = KaKuo2;
        scMode = KaDS;
        mManualMsg.configMacroData.getCKJDDeviceID(KaKuo2, ckDeviceID);
        mManualMsg.configMacroData.getDSJDDeviceID(KaDS, scDeviceID);
        m_impl->isKp = true;
    }
    // Ka扩频+Ka高速
    if (modeList.contains(KaKuo2) && modeList.contains(KaGS))
    {
        ckMode = KaKuo2;
        scMode = KaGS;
        mManualMsg.configMacroData.getCKJDDeviceID(KaKuo2, ckDeviceID);
        mManualMsg.configMacroData.getGSJDDeviceID(KaGS, scDeviceID);
        m_impl->isKp = true;
    }
    //设备ID校验、切换模式
    if (!ckDeviceID.isValid() || !scDeviceID.isValid())
    {
        emit signalErrorMsg("基带设备ID获取错误");
        return false;
    }
    //切换跟踪基带模式
    //切换跟踪基带模式
    CKJDSwitchStatus status = switchGZJDMasterAndSlaveWorkMode(gzDeviceID, gzDeviceIDSlave, static_cast<int>(needSwitchMode));  //主备机一起切换
    if (CKJDSwitchStatus::Failed == status)
    {
        emit signalErrorMsg(QString("跟踪基带主备机切换模式失败"));
    }

    //    配置宏已经切换主备了
    //    //跟踪基带主机命令下发
    //    PackCommand packCommand;
    //    QMap<QString, QVariant> params;
    //    params["OnlineSelect"] = 1;
    //    mSingleCommandHelper.packSingleCommand(QString("Step_TrackBBE_OnlineSelect"), mPackCommand, gzDeviceID, params);
    //    appendExecQueue(mPackCommand);
    //    //备机命令下发
    //    params.clear();
    //    params["OnlineSelect"] = 2;
    //    mSingleCommandHelper.packSingleCommand(QString("Step_TrackBBE_OnlineSelect"), mPackCommand, gzDeviceIDSlave, params);
    //    appendExecQueue(mPackCommand);

    //获取参数宏数据
    auto ckLink = mManualMsg.linkLineMap[ckMode];
    auto scLink = mManualMsg.linkLineMap[scMode];
    m_impl->ckTargetInfo = ckLink.targetMap.value(ckLink.masterTargetNo);
    m_impl->scTargetInfo = scLink.targetMap.value(scLink.masterTargetNo);
    GlobalData::getParamMacroModeData(m_impl->ckTargetInfo.taskCode, ckMode, m_impl->m_ckParamMacroData);
    GlobalData::getParamMacroModeData(m_impl->scTargetInfo.taskCode, scMode, m_impl->m_scParamMacroData);
    //组包
    ckWorkDataSet(static_cast<int>(needSwitchMode), ckMode, ckDeviceID, unitTargetParamMap);
    scWorkDataSet(static_cast<int>(needSwitchMode), scMode, scDeviceID, unitTargetParamMap);

    //低速才会特殊处理码速率值
    if (modeList.contains(KaDS))
    {
        m_impl->dealWithDSRelationParam(unitTargetParamMap);
    }

    clearQueue();

    //参数下发 主机
    if (!packGroupParamSetData(gzDeviceID, static_cast<int>(needSwitchMode), unitTargetParamMap, mPackCommand))
    {
        emit signalErrorMsg(QString("%1组包失败").arg(GlobalData::getExtensionName(gzDeviceID)));
    }
    appendExecQueue(mPackCommand);

    //参数下发 备机
    if (!packGroupParamSetData(gzDeviceIDSlave, static_cast<int>(needSwitchMode), unitTargetParamMap, mPackCommand))
    {
        emit signalErrorMsg(QString("%1组包失败").arg(GlobalData::getExtensionName(gzDeviceIDSlave)));
    }
    appendExecQueue(mPackCommand);

    execQueue();
    return true;
}

void TrkBBEParamMacroHandler::configGZDeviceJWC()
{
    /* 这里的策略是配置基带和ACU链接的尽量都满足  角误差输出只输出有效的 */
    const auto& gzjdDeviceIDMap = m_impl->gzjdDeviceIDMap;
    if (gzjdDeviceIDMap.isEmpty())
    {
        return;
    }
    // 切换角误差切换单元
    // 获取当前的跟踪模式 没有的不显示
    bool sJWCEnable = false;
    bool kackJWCEnable = false;
    bool kascJWCEnable = false;
    /* 高速和低速不能共存的随便选一个就行 */
    bool containsGS = false;
    if (m_impl->trackMode != SatelliteTrackingMode::TrackUnknown)
    {
        auto modeList = BaseHandler::trackingToSystemWorkModeList(m_impl->trackMode);
        containsGS = modeList.toSet().contains(KaGS);
        for (auto& workMode : modeList)
        {
            if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SBand)
            {
                sJWCEnable = true;
            }
            if (workMode == KaKuo2)
            {
                kackJWCEnable = true;
            }
            if (workMode == KaDS || workMode == KaGS)
            {
                kascJWCEnable = true;
            }
        }
    }
    else
    {
        sJWCEnable = true;
        kackJWCEnable = true;
        kascJWCEnable = true;
    }

    QMap<QString, QVariant> gzJWCMap;
    gzJWCMap["SF_BANDCHECK"] = 0;
    gzJWCMap["KAF_BANDCHECK"] = 0;
    gzJWCMap["KADTF_BANDCHECK"] = 0;
    gzJWCMap["SF_FRAGCHECK"] = 0;
    gzJWCMap["KAF_FRAGCHECK"] = 0;
    gzJWCMap["KADTF_FRAGCHECK"] = 0;
    gzJWCMap["JWC_BANDSIGNAL1"] = 0;
    gzJWCMap["JWC_BANDSIGNAL2"] = 0;
    gzJWCMap["JWC_BANDSIGNAL3"] = 0;

    int sExtenID = 0;
    int kackExtenID = 0;
    int kascExtenID = 0;
    for (auto iter = gzjdDeviceIDMap.begin(); iter != gzjdDeviceIDMap.end(); ++iter)
    {
        auto workModelSet = iter.value();
        auto gzDeviceID = iter.key();

        for (auto& workMode : workModelSet)
        {
            if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SBand)
            {
                gzJWCMap["SF_BANDCHECK"] = gzDeviceID.extenID;
                sExtenID = gzDeviceID.extenID;
                gzJWCMap["SF_FRAGCHECK"] = 1;     // S:1 X:2 Ka:3
                gzJWCMap["JWC_BANDSIGNAL1"] = 1;  //测控:1 数传:2
            }
            if (workMode == KaKuo2)
            {
                gzJWCMap["KAF_BANDCHECK"] = gzDeviceID.extenID;
                kackExtenID = gzDeviceID.extenID;
                gzJWCMap["KAF_FRAGCHECK"] = 3;    // S:1 X:2 Ka:3
                gzJWCMap["JWC_BANDSIGNAL2"] = 1;  //测控:1 数传:2
            }
            /* 20211112 这里需要分别判断高低速的 */
            if ((workMode == KaDS && !containsGS) || (workMode == KaGS && containsGS))
            {
                gzJWCMap["KADTF_BANDCHECK"] = gzDeviceID.extenID;
                kascExtenID = gzDeviceID.extenID;
                gzJWCMap["KADTF_FRAGCHECK"] = 3;  // S:1 X:2 Ka:3
                gzJWCMap["JWC_BANDSIGNAL3"] = 2;  //测控:1 数传:2
            }
        }
    }
    if (sExtenID != 0)
    {
        emit signalInfoMsg(QString("跟踪基带%1,频段%2,模式%3")
                               .arg(gzJWCMap["SF_BANDCHECK"].toInt())
                               .arg(gzJWCMap["SF_FRAGCHECK"] == 1 ? "S" : "Ka", gzJWCMap["JWC_BANDSIGNAL1"] == 1 ? "测控" : "数传"));
    }

    if (kackExtenID != 0)
    {
        emit signalInfoMsg(QString("跟踪基带%1,频段%2,模式%3")
                               .arg(gzJWCMap["KAF_BANDCHECK"].toInt())
                               .arg(gzJWCMap["KAF_FRAGCHECK"] == 1 ? "S" : "Ka", gzJWCMap["JWC_BANDSIGNAL2"] == 1 ? "测控" : "数传"));
    }

    if (kascExtenID != 0)
    {
        emit signalInfoMsg(QString("跟踪基带%1,频段%2,模式%3")
                               .arg(gzJWCMap["KADTF_BANDCHECK"].toInt())
                               .arg(gzJWCMap["KADTF_FRAGCHECK"] == 1 ? "S" : "Ka", gzJWCMap["JWC_BANDSIGNAL3"] == 1 ? "测控" : "数传"));
    }

    clearQueue();
    auto gzjwcDeviceID = gzjdDeviceIDMap.begin().key();
    for (auto extendNum = 1; extendNum <= 4; extendNum++)
    {
        gzjwcDeviceID.extenID = extendNum;
        mSingleCommandHelper.packSingleCommand("Step_GZ_JWC", mPackCommand, gzjwcDeviceID, gzJWCMap);
        appendExecQueue(mPackCommand);
    }

    // 10 是关闭
    gzJWCMap.clear();
    /*
     * 1+1-1
     * 2+2-1
     * 3+3-1
     * 4+4-1
     */
    gzJWCMap["SBandSelectOut"] = ((sExtenID != 0 && sJWCEnable) ? (sExtenID + sExtenID - 1) : 10);             /* 1357 ABCD */
    gzJWCMap["XBandSelectOut"] = ((kackExtenID != 0 && kackJWCEnable) ? (kackExtenID + kackExtenID - 1) : 10); /* 1357 ABCD */

    /*
     * 1+1
     * 2+2
     * 3+3
     * 4+4
     */
    gzJWCMap["KaBandSelectOut"] = ((kascExtenID != 0 && kascJWCEnable) ? (kascExtenID + kascExtenID) : 10); /* 2468 ABCD */

    mSingleCommandHelper.packSingleCommand("StepJWCQHDY_Unit", mPackCommand, gzJWCMap);
    appendExecQueue(mPackCommand);
    execQueue();
}

void TrkBBEParamMacroHandler::preBinding()
{
    /* 默认值下发=1|使用基带当前值=2|历史值下发=3 */
    auto pcaBind = GlobalData::getAutorunPolicyData("PCaBind", "1").toInt();
    if (pcaBind == 2)
    {
        emit signalSpecificTipsMsg("跳过相位装订,使用当前基带默认值");
        return;
    }

    /* 获取当前的跟踪模式
     * 然后每种跟踪模式对应的模式都查询已经存储的相位然后下发
     * 查询失败会下发默认的数据
     */
    for (auto iter = m_impl->gzjdDeviceIDMap.begin(); iter != m_impl->gzjdDeviceIDMap.end(); ++iter)
    {
        auto deviceID = iter.key();
        auto modeList = iter.value();

        for (auto workMode : modeList)
        {
            auto linkLine = mManualMsg.linkLineMap.value(workMode);
            auto masterTargetNo = linkLine.masterTargetNo;
            masterTargetNo = masterTargetNo <= 0 ? 1 : masterTargetNo;
            auto targetInfo = linkLine.targetMap.value(masterTargetNo);
            auto pointFreqNo = targetInfo.pointFreqNo <= 0 ? 1 : targetInfo.pointFreqNo;
            quint64 devNum = 0;
            if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
            {
                EquipmentCombinationHelper::getCKDevNumber(workMode, mManualMsg.configMacroData, devNum);
            }
            else
            {
                EquipmentCombinationHelper::getDTDevNumber(workMode, mManualMsg.configMacroData, devNum);
            }

            auto pcInfo = BaseHandler::getCalibResultInfo(targetInfo.taskCode, pointFreqNo, workMode, devNum, pcaBind != 3);
            bindingToGZJD(pcInfo, deviceID);

            auto msg = QString("%1 %2模式相位下发 方位校相结果%3,俯仰校相结果%4,方位移相值%5,俯仰移相值%6")
                           .arg(GlobalData::getExtensionName(deviceID), SystemWorkModeHelper::systemWorkModeToDesc(pcInfo.workMode))
                           .arg(pcInfo.azge)
                           .arg(pcInfo.elge)
                           .arg(pcInfo.azpr)
                           .arg(pcInfo.elpr);

            emit signalSpecificTipsMsg(msg);
        }
    }
}
void TrkBBEParamMacroHandler::bindingToGZJD(const CalibResultInfo& pcInfo, const DeviceID& deviceID)
{
    // 下发给跟踪基带
    if (pcInfo.workMode == SKT)
    {
        QVariantMap replaceMap;
        replaceMap["AzimPhase1"] = pcInfo.azpr;
        replaceMap["PitchPhase1"] = pcInfo.elpr;
        replaceMap["AzimSlope"] = pcInfo.azge;
        replaceMap["PitchSlope"] = pcInfo.elge;
        mSingleCommandHelper.packSingleCommand("StepGZJD_PCUpdate_KT", mPackCommand, deviceID, replaceMap);
        waitExecSuccess(mPackCommand);
    }
    else if (SystemWorkModeHelper::isMeasureContrlWorkMode(pcInfo.workMode))
    {
        QVariantMap replaceMap;
        replaceMap["AzimPhase"] = pcInfo.azpr;
        replaceMap["PitchPhase"] = pcInfo.elpr;
        replaceMap["AzimSlope"] = pcInfo.azge;
        replaceMap["PitchSlope"] = pcInfo.elge;
        mSingleCommandHelper.packSingleCommand("StepGZJD_PCUpdate_YC", mPackCommand, deviceID, replaceMap);
        waitExecSuccess(mPackCommand);
    }
    else if (SystemWorkModeHelper::isDataTransmissionWorkMode(pcInfo.workMode))
    {
        QVariantMap replaceMap;
        replaceMap["AzimPhase"] = pcInfo.azpr;
        replaceMap["PitchPhase"] = pcInfo.elpr;
        replaceMap["AzimSlope"] = pcInfo.azge;
        replaceMap["PitchSlope"] = pcInfo.elge;
        mSingleCommandHelper.packSingleCommand("StepGZJD_PCUpdate_SC", mPackCommand, deviceID, replaceMap);
        waitExecSuccess(mPackCommand);
    }
}

SystemWorkMode TrkBBEParamMacroHandler::getSystemWorkMode() { return NotDefine; }

bool TrkBBEParamMacroHandler::ckWorkDataSet(const int modeID, SystemWorkMode ckMode, const DeviceID deviceId_CkBBE,
                                            QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap, MasterSlave masterSlave)
{
    // 获取点频参数转换的下发结构
    UnitTargetParamMap ckUnitTargetParamMap;
    if (!getUnitTargetParamMap(ckMode, mManualMsg.linkLineMap[ckMode], deviceId_CkBBE, ckUnitTargetParamMap))
    {
        emit signalErrorMsg(QString("获取%1基带参数宏数据失败").arg(SystemWorkModeHelper::systemWorkModeToDesc(ckMode)));
        return false;
    }

    // 需要获取测控基带设备参数，添加到下发的参数宏中
    QMap<int, QVariantMap> ckjdUnitParamMap;
    m_impl->m_ckParamMacroData.getDeviceUnitParamMap(deviceId_CkBBE, ckjdUnitParamMap);
    appendDeviceParam(ckjdUnitParamMap, ckUnitTargetParamMap);

    QList<TrkBBE::ParamInfo> paramInfos = m_impl->d_modeParamInfos[modeID];

    for (auto& paramInfo : paramInfos)
    {
        //基带参数映射,从测控基带、高速基带、低速基带中获取参数值并且映射到对应的跟踪基带中
        auto bbe = paramInfo.bbe;
        if (bbe == TrkBBE::CK)  //不加这个判断的话低速或高速的无效值就会覆盖测控的有效值
        {
            m_impl->dealCKData(paramInfo, ckUnitTargetParamMap, unitTargetParamMap);
        }
    }
    //处理旋向
    m_impl->dealTrackPolar(ckMode, unitTargetParamMap);
    //处理设备组合号   这里很无语，跟踪基带扩频的设备组合号是放在多目标里面的
    quint64 devNum = 0;
    EquipmentCombinationHelper::getCKDevNumber(ckMode, mManualMsg.configMacroData, devNum);
    auto devHex = QString::number(devNum, 16);
    if (ckMode == Skuo2 || ckMode == KaKuo2)
    {
        unitTargetParamMap[1][1]["SEquipCombNum"] = devHex;
        unitTargetParamMap[1][2]["SEquipCombNum"] = devHex;
        unitTargetParamMap[1][3]["SEquipCombNum"] = devHex;
        unitTargetParamMap[1][4]["SEquipCombNum"] = devHex;
    }
    else
    {
        unitTargetParamMap[1][0]["SEquipCombNum"] = devHex;
    }

    return true;
}

bool TrkBBEParamMacroHandler::scWorkDataSet(const int modeID, SystemWorkMode scMode, const DeviceID deviceId_ScBBE,
                                            QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap, MasterSlave masterSlave)
{
    //点频数据
    QMap<int, QMap<QString, QVariant>> pointFreqUnitParamMap;
    // 获取点频和设备单元参数 并将其按单元合并到一起
    if (!getPointFreqAndDeviceParamMap(m_impl->scTargetInfo.taskCode, scMode, m_impl->scTargetInfo.pointFreqNo, deviceId_ScBBE,
                                       pointFreqUnitParamMap))
    {
        emit signalErrorMsg(QString("获取%1基带参数宏数据失败").arg(SystemWorkModeHelper::systemWorkModeToDesc(scMode)));
        return false;
    }

    QList<TrkBBE::ParamInfo> paramInfos = m_impl->d_modeParamInfos[modeID];
    for (auto& paramInfo : paramInfos)
    {
        //基带参数映射,从测控基带、高速基带、低速基带中获取参数值并且映射到对应的跟踪基带中
        auto bbe = paramInfo.bbe;
        if (bbe == TrkBBE::GS || bbe == TrkBBE::DS)
        {
            m_impl->dealSCData(paramInfo, pointFreqUnitParamMap, unitTargetParamMap);
        }
    }

    //数传的点频里没有包含标识和代号，直接从卫星里获取
    SatelliteManagementData satelliteData;  //对应的卫星数据
    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(scMode);
    // 获取卫星数据
    if (!GlobalData::getSatelliteManagementData(m_impl->scTargetInfo.taskCode, satelliteData))
    {
        emit signalErrorMsg(QString("获取任务代号为：%1的卫星数据失败").arg(m_impl->scTargetInfo.taskCode, workModeDesc));
    }

    unitTargetParamMap[1][0]["XKaTaskIdentifier"] = satelliteData.m_satelliteIdentification;
    unitTargetParamMap[1][0]["XKaTaskCode"] = satelliteData.m_satelliteCode;

    //处理旋向
    m_impl->dealTrackPolar(scMode, unitTargetParamMap);

    //处理设备组合号
    quint64 devNum = 0;
    EquipmentCombinationHelper::getDTDevNumber(scMode, mManualMsg.configMacroData, devNum);
    auto devHex = QString::number(devNum, 16);
    unitTargetParamMap[1][0]["XKaEquipCombNum"] = devHex;


    // Ka低速和X低速在参数宏里取的ID不一样，但跟踪基带参数宏里的ID又只有一个，故需要特殊处理
    //    if (modeID == TrkBBE::KaDS || modeID == TrkBBE::STTC_KaDS)
    //    {
    //        QVariant downFreq;
    //        if (!MacroCommon::getDownFrequency(m_impl->m_scParamMacroData, m_impl->scTargetInfo.taskCode, scMode, m_impl->scTargetInfo.pointFreqNo,
    //                                           downFreq))
    //        {
    //            emit signalErrorMsg(QString("获取任务代号为：%1的参数宏%2模式的下行频率失败").arg(m_impl->scTargetInfo.taskCode, workModeDesc));
    //        }
    //        unitTargetParamMap[1][0]["XKaDownFreq"] = downFreq;
    //    }
    //    else if (modeID == TrkBBE::KP_KaDS)
    //    {
    //        QVariant downFreq;
    //        if (!MacroCommon::getDownFrequency(m_impl->m_scParamMacroData, m_impl->scTargetInfo.taskCode, scMode, m_impl->scTargetInfo.pointFreqNo,
    //                                           downFreq))
    //        {
    //            emit signalErrorMsg(QString("获取任务代号为：%1的参数宏%2模式的下行频率失败").arg(m_impl->scTargetInfo.taskCode, workModeDesc));
    //        }
    //        unitTargetParamMap[1][0]["XKANumberSimul"] = downFreq;
    //    }

    return true;
}

void TrkBBEParamMacroHandlerImpl::dealCKData(TrkBBE::ParamInfo& paramInfo, QMap<int, QMap<int, QMap<QString, QVariant>>> ckUnitTargetParamMap,
                                             QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap)
{
    int targetNum = 0;
    QVariant value;
    if (isKp)
    {
        targetNum = ckUnitTargetParamMap[1][0]["TargetNum"].toInt();
    }
    if (paramInfo.src_targetId > 0)
    {
        if (paramInfo.targetId > 0)
        {
            for (int targetId = 0; targetId < targetNum; targetId++)
            {
                value = ckUnitTargetParamMap[paramInfo.src_unitId][targetId + 1][paramInfo.src_paramId];
                if (!value.isValid())
                {
                    continue;
                }
                if (!paramInfo.rule.isEmpty())
                {
                    dealCKSpcialData(value, paramInfo, unitTargetParamMap);
                }
                else
                {
                    unitTargetParamMap[paramInfo.unitId][targetId + 1][paramInfo.id] = value;
                }
            }
        }
        else
        {
            for (int targetId = 0; targetId < targetNum; targetId++)
            {
                value = ckUnitTargetParamMap[paramInfo.src_unitId][targetId + 1][paramInfo.src_paramId];
            }

            if (!value.isValid())
            {
                return;
            }

            if (!paramInfo.rule.isEmpty())
            {
                dealCKSpcialData(value, paramInfo, unitTargetParamMap);
            }
            else
            {
                unitTargetParamMap[paramInfo.unitId][paramInfo.targetId][paramInfo.id] = value;
            }
        }
    }
    else
    {
        value = ckUnitTargetParamMap[paramInfo.src_unitId][0][paramInfo.src_paramId];

        if (!value.isValid())  //值无效就返回，取状态上报值
        {
            return;
        }

        if (paramInfo.targetId > 0)
        {
            for (int targetId = 0; targetId < targetNum; targetId++)
            {
                if (!paramInfo.rule.isEmpty())
                {
                    dealCKSpcialData(value, paramInfo, unitTargetParamMap);
                }
                else
                {
                    unitTargetParamMap[paramInfo.unitId][targetId + 1][paramInfo.id] = value;
                }
            }
        }
        else
        {
            if (!paramInfo.rule.isEmpty())
            {
                dealCKSpcialData(value, paramInfo, unitTargetParamMap);
            }
            else
            {
                unitTargetParamMap[paramInfo.unitId][paramInfo.targetId][paramInfo.id] = value;
            }
        }
    }
}

void TrkBBEParamMacroHandlerImpl::dealCKSpcialData(QVariant& value, TrkBBE::ParamInfo& paramInfo,
                                                   QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap)
{
    QVariant spcialValue;
    //跟踪基带 标准+高速和标准+低速载波环路带宽参数
    if (paramInfo.rule == "CK_ZBHLDK")
    {
        if (value == 3)  //测控基带的值3对应100Hz,对应跟踪基带的值1 100Hz
        {
            spcialValue = 1;
        }
        else if (value == 4)  // 500Hz
        {
            spcialValue = 2;
        }
        else if (value == 5)  // 1kHz
        {
            spcialValue = 3;
        }
        else if (value == 6)  // 2kHz
        {
            spcialValue = 4;
        }
        unitTargetParamMap[paramInfo.unitId][paramInfo.targetId][paramInfo.id] = spcialValue;
    }
    // Viterbi译码方式对应测控基带的译码方式
    else if (paramInfo.rule == "SKUO2_VTBYM")
    {
        if (value == 2 || value == 4)
        {
            spcialValue = 2;
        }
        else
        {
            spcialValue = 1;
        }
        unitTargetParamMap[paramInfo.unitId][paramInfo.targetId][paramInfo.id] = spcialValue;
    }
}

void TrkBBEParamMacroHandlerImpl::dealSCData(TrkBBE::ParamInfo& paramInfo, QMap<int, QMap<QString, QVariant>> pointFreqUnitParamMap,
                                             QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap)
{
    QVariant value;
    value = pointFreqUnitParamMap[paramInfo.src_unitId][paramInfo.src_paramId];

    if (!value.isValid())
    {
        return;
    }

    /*
     * 特殊处理,映射过程中需要特殊数据转换的处理，所有的特殊处理都在这里，后续可能需要新增。配置文件统一加了rule限定
     */
    if (!paramInfo.rule.isEmpty())
    {
        if (paramInfo.bbe == TrkBBE::DS)
        {
            dealDSSpcialData(value, paramInfo, unitTargetParamMap);
        }
        else if (paramInfo.bbe == TrkBBE::GS)
        {
            dealGSSpcialData(value, paramInfo, unitTargetParamMap);
        }
    }
    else
    {
        unitTargetParamMap[paramInfo.unitId][paramInfo.targetId][paramInfo.id] = value;
    }
}

void TrkBBEParamMacroHandlerImpl::dealGSSpcialData(QVariant& value, TrkBBE::ParamInfo& paramInfo,
                                                   QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap)
{
}

void TrkBBEParamMacroHandlerImpl::dealDSSpcialData(QVariant& value, TrkBBE::ParamInfo& paramInfo,
                                                   QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap)
{
    QVariant spcialValue;
    //跟踪基带 标准+低速和扩频+低速的双边环路带宽   对应低速基带的载波环路带宽
    if (paramInfo.rule == "DS_SBHLDK")
    {
        if (value == 1)  //低速基带的值1对应100Hz,对应跟踪基带的值100  100Hz
        {
            spcialValue = 100;
        }
        else if (value == 2)  // 300Hz
        {
            spcialValue = 300;
        }
        else if (value == 3)  // 1000Hz
        {
            spcialValue = 1000;
        }
        else if (value == 4)  // 3000Hz
        {
            spcialValue = 3000;
        }
    }
    //码型 跟踪基带0:NRZ-L 1:NRZ-M 2:NRZ-S 3:Biφ-L 4:Biφ-M 5:Biφ-S
    //低速基带1:NRZ-L 2:NRZ-M 3:NRZ-S 4:Biφ-L 5:Biφ-M 6:Biφ-S
    else if (paramInfo.rule == "DS_MX")
    {
        spcialValue = value.toInt() - 1;
    }
    //双基选择  低速基带1对偶 2常规 跟踪基带0常规1对偶
    else if (paramInfo.rule == "DS_SJXZ")
    {
        if (value == 1)  //低速基带的值1对
        {
            spcialValue = 1;
        }
        else if (value == 2)  // 300Hz
        {
            spcialValue = 0;
        }
    }
    //载波调制方式 跟踪基带2:BPSK 3:QPSK 4:OQPSK 5:UQPSK  低速基带1:BPSK 2:QPSK 4:UQPSK 5:OQPSK
    else if (paramInfo.rule == "DS_ZBTZFS")
    {
        if (value == 1)
        {
            spcialValue = 2;
        }
        else if (value == 2)
        {
            spcialValue = 3;
        }
        else if (value == 4)
        {
            spcialValue = 5;
        }
        else if (value == 5)
        {
            spcialValue = 4;
        }
    }
    // Viterbi译码 跟踪基带0:否 1:(7,1/2) 2:(7,2/3) 3:(7,3/4) 4:(7,5/6) 5:(7,7/8) 6:(7,6/7)
    //低速基带 1:(7,1/2) 2:(7,2/3) 3:(7,3/4) 4:(7,5/6) 5:(7,7/8)
    else if (paramInfo.rule == "DS_VTVYM")
    {
        if (value == 0 || value == 6)
        {
            spcialValue = 1;
        }
        else
        {
            spcialValue = value;
        }
    }
    // G2相位 跟踪基带0:正相 1:反相  低速基带1:正相 2:反相
    else if (paramInfo.rule == "DS_VTVXW")
    {
        if (value == 1)
        {
            spcialValue = 0;
        }
        else if (value == 2)
        {
            spcialValue = 1;
        }
    }
    // Turbo译码 跟踪基带0:无 1:有  低速基带1:关 2:Viterbi译码 3:R-S译码 4:R-S译码和Viterbi级联译码 5:LDPC 6:Turbo
    else if (paramInfo.rule == "DS_TURBOYM")
    {
        if (value == 6)
        {
            spcialValue = 1;
        }
        else
        {
            spcialValue = 0;
        }
    }
    // Turbo码率
    else if (paramInfo.rule == "DS_TURBOML")
    {
        if (value == 1 || value == 2 || value == 3 || value == 4)
        {
            spcialValue = 0;
        }
        else if (value == 5 || value == 6 || value == 7 || value == 8)
        {
            spcialValue = 1;
        }
        else if (value == 9 || value == 10 || value == 11 || value == 12)
        {
            spcialValue = 2;
        }
        else if (value == 13 || value == 14 || value == 15 || value == 16)
        {
            spcialValue = 3;
        }
    }
    // LDPC译码 跟踪基带0:无 1:有  低速基带1:关 2:Viterbi译码 3:R-S译码 4:R-S译码和Viterbi级联译码 5:LDPC 6:Turbo
    else if (paramInfo.rule == "DS_LDPC")
    {
        if (value == 5)
        {
            spcialValue = 1;
        }
        else
        {
            spcialValue = 0;
        }
    }
    // LDPC码率
    else if (paramInfo.rule == "DS_LDPCML")
    {
        if (value == 1 || value == 2)
        {
            spcialValue = 0;
        }
        else if (value == 3 || value == 4)
        {
            spcialValue = 1;
        }
        else if (value == 5 || value == 6)
        {
            spcialValue = 2;
        }
        else if (value == 7)
        {
            spcialValue = 3;
        }
    }
    unitTargetParamMap[paramInfo.unitId][paramInfo.targetId][paramInfo.id] = spcialValue;
}

void TrkBBEParamMacroHandlerImpl::dealWithDSRelationParam(QMap<int, QMap<int, QVariantMap>>& unitTargetParamMap)
{
    //通道1解调单元码速率处理
    auto& modulatSystem = unitTargetParamMap[4][0]["CarrierModulation"];  //调制体制
    auto& dataOutWay = unitTargetParamMap[4][0]["SignalModelChoose"];     // 数据输出方式
    auto& iCodeSpeed = unitTargetParamMap[5][0]["CodeSpeed"];             // I路码速率
    auto& qCodeSpeed = unitTargetParamMap[6][0]["CodeSpeed"];             // Q路码速率

    /*
     * BPSK 肯定是只有单I路 2 不用管
     * QPSK 可分可合 3 合路IQ相加=I  分路IQ相等
     * UQPSK 必须分路 5 各玩个的 不用管
     * OQPSK 可分可合 4 同 QPSK
     *
     * I/Q合 0
     * I/Q分 1
     * 2 独立
     */
    auto numModulatSystem = modulatSystem.toInt();
    if (numModulatSystem == 3 || numModulatSystem == 4)
    {
        if (dataOutWay.toInt() == 0)
        {
            iCodeSpeed = iCodeSpeed.toInt() + qCodeSpeed.toInt();
            qCodeSpeed = iCodeSpeed;
        }
        else
        {
            qCodeSpeed = iCodeSpeed;
        }
    }
}

void TrkBBEParamMacroHandlerImpl::dealTrackPolar(SystemWorkMode workMode, QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap)
{
    SatelliteManagementData satelliteData;                        //对应的卫星数据
    if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))  // 测控模式参数宏都使用同一个
    {
        // 获取卫星数据
        GlobalData::getSatelliteManagementData(ckTargetInfo.taskCode, satelliteData);
        SystemOrientation orientation;
        satelliteData.getSTrackPolar(workMode, ckTargetInfo.pointFreqNo, orientation);

        unitTargetParamMap[1][0]["SpecDireOutput"] = static_cast<int>(orientation);
    }
    else
    {
        // 获取卫星数据
        GlobalData::getSatelliteManagementData(scTargetInfo.taskCode, satelliteData);
        SystemOrientation orientation;
        satelliteData.getSTrackPolar(workMode, scTargetInfo.pointFreqNo, orientation);

        unitTargetParamMap[1][0]["SCSpecDireOutput"] = static_cast<int>(orientation);
    }
}

TrkBBEXmlParser::TrkBBEXmlParser(const QString& fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }

    QDomElement root = doc.documentElement();
    int modeID = -1;
    parseMainNode(root, modeID, d_modeParamInfos);
    file.close();
}

QList<TrkBBE::ParamInfo> TrkBBEXmlParser::getParamInfos(int modeId) const { return d_modeParamInfos[modeId]; }

QMap<int, QList<TrkBBE::ParamInfo>> TrkBBEXmlParser::getAllModesParamInfos() const { return d_modeParamInfos; }

void TrkBBEXmlParser::parseMainNode(const QDomElement& domEle, int& modeID, QMap<int, QList<TrkBBE::ParamInfo>>& modeParamInfos)
{
    auto domNode = domEle.firstChild();

    while (!domNode.isNull())
    {
        auto domEle = domNode.toElement();
        if (!domEle.isNull())
        {
            QString tagName = domEle.tagName();
            TrkBBE::ParamInfo paramInfo;
            if (tagName == "Mode")
            {
                modeID = domEle.attribute("modeId").toInt(nullptr, 16);
            }
            else if (tagName == "field")
            {
                parseFieldNode(domEle, paramInfo);
                modeParamInfos[modeID].append(paramInfo);
            }
            parseMainNode(domEle, modeID, modeParamInfos);
        }
        domNode = domNode.nextSibling();
    }
}

void TrkBBEXmlParser::parseFieldNode(const QDomElement& domEle, TrkBBE::ParamInfo& paramInfo)
{
    if (domEle.isNull())
    {
        return;
    }

    QStringList srcList, dstList;
    srcList = domEle.attribute("srcInfo").split(":");
    dstList = domEle.attribute("dstInfo").split(":");
    paramInfo.desc = domEle.attribute("desc").trimmed();
    paramInfo.rule = domEle.attribute("rule").trimmed();

    if (srcList.size() != 4 || dstList.size() != 3)
    {
        return;
    }

    paramInfo.unitId = dstList.at(0).toInt();
    paramInfo.targetId = dstList.at(1).toInt();
    paramInfo.id = dstList.at(2);

    QString bbe = srcList.at(0);
    if (bbe == "CK")
    {
        paramInfo.bbe = TrkBBE::CK;
    }
    if (bbe == "GS")
    {
        paramInfo.bbe = TrkBBE::GS;
    }
    if (bbe == "DS")
    {
        paramInfo.bbe = TrkBBE::DS;
    }

    paramInfo.src_unitId = srcList.at(1).toInt();
    paramInfo.src_targetId = srcList.at(2).toInt();
    paramInfo.src_paramId = srcList.at(3);
}
