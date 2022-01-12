#include "PhaseCalibration.h"
#include "AutorunPolicyMessageSerialize.h"
#include "BusinessMacroCommon.h"
#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "ParamMacroMessageSerialize.h"
#include "PhaseCalibrationModel.h"
#include "PhaseCalibrationSerialize.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "SCUtility.h"
#include "SatelliteManagementSerialize.h"
#include "ServiceCheck.h"
#include "StatusDataSource.h"
#include "SystemWorkMode.h"
#include "MessagePublish.h"
#include "Utility.h"
#include "SubscriberHelper.h"
#include "ui_PhaseCalibration.h"
#include <QButtonGroup>
#include <QMessageBox>
#include <QThread>

class PhaseCalibrationImpl
{
public:
    /* 所有的卫星数据 */
    SatelliteManagementDataMap satelliteManagementDataMap;
    SatelliteManagementData curSatelliteManagementData;
    cpp_redis::subscriber logSubscriber;
    PhaseCalibrationModel model;
    QButtonGroup* phaseCalibrationMethod{ nullptr }; /* 校相方式 */
    QButtonGroup* fbGroup{ nullptr };
    int timerID{ -1 };
    bool isReset{ false };
    /* 数据源 */
    StatusDataSource source;
    QThread dataSourceThread;
    QList<WidgetValueInfo> tfsInfo;

    void workModeChanged(QComboBox* dotDrequency, QComboBox* workMode);
    void dotDrequencyChanged(QComboBox* taskCode, QComboBox* dotDrequency, QComboBox* workMode, QLineEdit* downFreq);
    void calcPCFBChange(Ui::PhaseCalibration* ui);
    void refreshPCParam(Ui::PhaseCalibration* ui, int id);

    template<ACUPhaseCalibrationFreqBand ACUPCFB>
    Optional<PhaseCalibrationSaveParameter> getSingleGroupItem(const QString& taskCode, QComboBox* workMode, QComboBox* dotDrequency,
                                                               QLineEdit* downFreqQuick);
};
void PhaseCalibrationImpl::workModeChanged(QComboBox* dotDrequency, QComboBox* workMode)
{
    dotDrequency->clear();
    auto tempWorkMode = (SystemWorkMode)workMode->currentData().toInt();
    auto dpNum = curSatelliteManagementData.getDpNumByWorkMode(tempWorkMode);
    for (int i = 0; i < dpNum; ++i)
    {
        dotDrequency->addItem(QString::number(i + 1), i + 1);
    }
}

void PhaseCalibrationImpl::dotDrequencyChanged(QComboBox* taskCode, QComboBox* dotDrequency, QComboBox* workMode, QLineEdit* downFreq)
{
    downFreq->clear();
    auto tempDPData = dotDrequency->currentData();
    auto tempModeData = workMode->currentData();
    auto tempTaskCodeVar = taskCode->currentData();

    auto curTaskCode = tempTaskCodeVar.toString();
    auto curWorkMode = SystemWorkMode(tempModeData.toInt());
    auto curDP = tempDPData.toInt();

    QVariant down;
    if (MacroCommon::getDownFrequency(curTaskCode, curWorkMode, curDP, down))
    {
        downFreq->setText(QString::number(down.toDouble(), 'f', 6));
    }
}
void PhaseCalibrationImpl::refreshPCParam(Ui::PhaseCalibration* ui, int id)
{
    AutorunPolicyData autorunPolicyData;
    GlobalData::getAutorunPolicyData(autorunPolicyData);

    auto sCrossLowLimit = autorunPolicyData.policy.value("S_CrossLowLimit", 5).toInt();
    auto sStandSensit = autorunPolicyData.policy.value("S_StandSensit", 500).toInt();
    auto sDirectToler = autorunPolicyData.policy.value("S_DirectToler", 50).toInt();

    auto kaCrossLowLimit = autorunPolicyData.policy.value("KA_CrossLowLimit", 5).toInt();
    auto kaStandSensit = autorunPolicyData.policy.value("KA_StandSensit", 4800).toInt();
    auto kaDirectToler = autorunPolicyData.policy.value("KA_DirectToler", 400).toInt();

    auto kadtCrossLowLimit = autorunPolicyData.policy.value("KADT_CrossLowLimit", 5).toInt();
    auto kadtStandSensit = autorunPolicyData.policy.value("KADT_StandSensit", 4800).toInt();
    auto kadtDirectToler = autorunPolicyData.policy.value("KADT_DirectToler", 400).toInt();
    auto fb = ACUPhaseCalibrationFreqBand(id);

    if (fb == ACUPhaseCalibrationFreqBand::S ||      //
        fb == ACUPhaseCalibrationFreqBand::SKaYC ||  //
        fb == ACUPhaseCalibrationFreqBand::SKaSC ||  //
        fb == ACUPhaseCalibrationFreqBand::SKaYCKaSC)
    {
        ui->crossCouplingLowerLimitQuick->setValue(sCrossLowLimit);
        ui->directionalSensitivityQuick->setValue(sStandSensit);
        ui->directionalSensitivityToleranceQuick->setValue(sDirectToler);
    }
    else if (fb == ACUPhaseCalibrationFreqBand::KaYC)
    {
        ui->crossCouplingLowerLimitQuick->setValue(kaCrossLowLimit);
        ui->directionalSensitivityQuick->setValue(kaStandSensit);
        ui->directionalSensitivityToleranceQuick->setValue(kaDirectToler);
    }
    else if (fb == ACUPhaseCalibrationFreqBand::KaSC)
    {
        ui->crossCouplingLowerLimitQuick->setValue(kadtCrossLowLimit);
        ui->directionalSensitivityQuick->setValue(kadtStandSensit);
        ui->directionalSensitivityToleranceQuick->setValue(kadtDirectToler);
    }
}
/* 快速校相的频段选择 */
void PhaseCalibrationImpl::calcPCFBChange(Ui::PhaseCalibration* ui)
{
    bool sEnable = ui->s_workMode->count() != 0;
    bool kaYCEnable = ui->yc_workMode->count() != 0;
    bool kaSCEnable = ui->sc_workMode->count() != 0;

    ui->fb_1->setEnabled(sEnable);                            // s
    ui->fb_2->setEnabled(kaYCEnable);                         // ka遥测
    ui->fb_3->setEnabled(kaSCEnable);                         // ka数传
    ui->fb_4->setEnabled(sEnable & kaYCEnable);               // s+ka遥测
    ui->fb_5->setEnabled(sEnable & kaSCEnable);               // s+ka数传
    ui->fb_6->setEnabled(sEnable & kaYCEnable & kaSCEnable);  // s+ka遥测+ka数传
    if (sEnable)
    {
        ui->fb_1->setChecked(true);
        return;
    }
    if (kaYCEnable)
    {
        ui->fb_2->setChecked(true);
        return;
    }
    if (kaSCEnable)
    {
        ui->fb_3->setChecked(true);
        return;
    }
}

template<ACUPhaseCalibrationFreqBand ACUPCFB>
Optional<PhaseCalibrationSaveParameter> PhaseCalibrationImpl::getSingleGroupItem(const QString& taskCode, QComboBox* workMode,
                                                                                 QComboBox* dotDrequency, QLineEdit* downFreqQuick)
{
    /* 设备组合号 包含两个部分 一个是设备组合号 另一个是主备*/
    using ResType = Optional<PhaseCalibrationSaveParameter>;
    PhaseCalibrationSaveParameter result;
    result.freqBand = ACUPCFB;

    auto workModeData = workMode->currentData();
    if (!workModeData.isValid())
    {
        return ResType(ErrorCode::InvalidData, "当前工作模式不能为空");
    }
    result.systemWorkMode = SystemWorkMode(workModeData.toInt());

    auto dotDrequencyData = dotDrequency->currentData();
    if (!dotDrequencyData.isValid())
    {
        return ResType(ErrorCode::InvalidData, "当前点频不能为空");
    }
    result.dotDrequency = dotDrequencyData.toInt();

    auto frepVar = downFreqQuick->text();
    if (frepVar.isEmpty())
    {
        return ResType(ErrorCode::InvalidData, "当前频率不能为空");
    }
    result.downFreq = frepVar.toDouble();

    return ResType(result);
}

PhaseCalibration::PhaseCalibration(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PhaseCalibration)
    , m_impl(new PhaseCalibrationImpl)
{
    Q_UNUSED(context)
    ui->setupUi(this);
    ui->groupBox_9->setHidden(true);
    // 初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
    Utility::tableViewInit(ui->tableView, true);
    ui->tableView->setModel(&(m_impl->model));

    m_impl->calcPCFBChange(ui);
    /* 注册实时状态 */
    // SCUtility::registerValueInfo("STrackLockInst", ui->sInstructions, 2, &SetWidgetValue::setLampGGR, m_impl->tfsInfo);
    // SCUtility::registerValueInfo("XKaTrackLockInst", ui->xKaInstructions, 2, &SetWidgetValue::setLampGGR, m_impl->tfsInfo);
    // SCUtility::registerValueInfo("XKaSCTrackLockInst", ui->xKaSCInstructions, 2, &SetWidgetValue::setLampGGR, m_impl->tfsInfo);
    // SCUtility::registerValueInfo("STrackAzinVol", ui->s_fw, 2, &SetWidgetValue::setLabelText, m_impl->tfsInfo);
    // SCUtility::registerValueInfo("STrackPithVol", ui->s_fy, 2, &SetWidgetValue::setLabelText, m_impl->tfsInfo);
    // SCUtility::registerValueInfo("STrackAGCVol", ui->s_agc, 2, &SetWidgetValue::setLabelText, m_impl->tfsInfo);
    // SCUtility::registerValueInfo("XKaTrackAZVol", ui->yc_fw, 2, &SetWidgetValue::setLabelText, m_impl->tfsInfo);
    // SCUtility::registerValueInfo("XKaTrackPitchVol", ui->yc_fy, 2, &SetWidgetValue::setLabelText, m_impl->tfsInfo);
    // SCUtility::registerValueInfo("XKaTrackAGCVol", ui->yc_agc, 2, &SetWidgetValue::setLabelText, m_impl->tfsInfo);
    // SCUtility::registerValueInfo("XKaSCTrackAZVol", ui->sc_fw, 2, &SetWidgetValue::setLabelText, m_impl->tfsInfo);
    // SCUtility::registerValueInfo("XKaSCTrackPitchVol", ui->sc_fy, 2, &SetWidgetValue::setLabelText, m_impl->tfsInfo);
    // SCUtility::registerValueInfo("XKaSCTrackAGCVol", ui->sc_agc, 2, &SetWidgetValue::setLabelText, m_impl->tfsInfo);

    /* 频段 1s 2ka遥测 3Ka数传 4s+ka遥测 5s+ka数传 6S+Ka遥测+Ka数传 */
    m_impl->fbGroup = new QButtonGroup(this);
    m_impl->fbGroup->addButton(ui->fb_1, static_cast<int>(ACUPhaseCalibrationFreqBand::S));
    m_impl->fbGroup->addButton(ui->fb_2, static_cast<int>(ACUPhaseCalibrationFreqBand::KaYC));
    m_impl->fbGroup->addButton(ui->fb_3, static_cast<int>(ACUPhaseCalibrationFreqBand::KaSC));
    m_impl->fbGroup->addButton(ui->fb_4, static_cast<int>(ACUPhaseCalibrationFreqBand::SKaYC));
    m_impl->fbGroup->addButton(ui->fb_5, static_cast<int>(ACUPhaseCalibrationFreqBand::SKaSC));
    m_impl->fbGroup->addButton(ui->fb_6, static_cast<int>(ACUPhaseCalibrationFreqBand::SKaYCKaSC));
    connect(m_impl->fbGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [&](int id) { m_impl->refreshPCParam(ui, id); });

    /* 校相方式 1 对塔 2 对当前位置 3 对星 */
    m_impl->phaseCalibrationMethod = new QButtonGroup(this); /* 校相方式 */
    m_impl->phaseCalibrationMethod->addButton(ui->towerQuick, static_cast<int>(PhaseCalibrationCorrMeth::Tower));
    m_impl->phaseCalibrationMethod->addButton(ui->currentLocationQuick, static_cast<int>(PhaseCalibrationCorrMeth::Current));
    m_impl->phaseCalibrationMethod->addButton(ui->satelliteQuick, static_cast<int>(PhaseCalibrationCorrMeth::Satellite));

    /* 跟踪检查 0：无此参数（校相方式为对塔时有效） 1：检查 2：不检查 */
    ui->followUpInspectionQuick->addItem("无(对塔有效)", static_cast<int>(PhaseCalibrationFollowCheck::Not));
    ui->followUpInspectionQuick->addItem("检查", static_cast<int>(PhaseCalibrationFollowCheck::Check));
    ui->followUpInspectionQuick->addItem("不检查", static_cast<int>(PhaseCalibrationFollowCheck::NotCheck));

    ui->workMode->addItem("所有", static_cast<int>(NotDefine));
    ui->workMode->addItem(SystemWorkModeHelper::systemWorkModeToDesc(STTC), static_cast<int>(STTC));
    ui->workMode->addItem(SystemWorkModeHelper::systemWorkModeToDesc(Skuo2), static_cast<int>(Skuo2));
    ui->workMode->addItem(SystemWorkModeHelper::systemWorkModeToDesc(KaKuo2), static_cast<int>(KaKuo2));
    ui->workMode->addItem(SystemWorkModeHelper::systemWorkModeToDesc(SKT), static_cast<int>(SKT));
    ui->workMode->addItem(SystemWorkModeHelper::systemWorkModeToDesc(KaGS), static_cast<int>(KaGS));
    ui->workMode->addItem(SystemWorkModeHelper::systemWorkModeToDesc(KaDS), static_cast<int>(KaDS));

    SCUtility::bgBlackTextGreenLabel(ui->azpr);
    SCUtility::bgBlackTextGreenLabel(ui->azge);
    SCUtility::bgBlackTextGreenLabel(ui->elpr);
    SCUtility::bgBlackTextGreenLabel(ui->elge);
    SCUtility::bgBlackTextGreenLabel(ui->s_fw);
    SCUtility::bgBlackTextGreenLabel(ui->s_fy);
    SCUtility::bgBlackTextGreenLabel(ui->s_agc);

    SCUtility::bgBlackTextGreenLabel(ui->yc_fw);
    SCUtility::bgBlackTextGreenLabel(ui->yc_fy);
    SCUtility::bgBlackTextGreenLabel(ui->yc_agc);

    SCUtility::bgBlackTextGreenLabel(ui->sc_fw);
    SCUtility::bgBlackTextGreenLabel(ui->sc_fy);
    SCUtility::bgBlackTextGreenLabel(ui->sc_agc);

    SCUtility::bgBlackTextGreenLabel(ui->centerTemp);
    SCUtility::bgBlackTextGreenLabel(ui->centerHumidity);
    resetValue();

    m_impl->dataSourceThread.start();
    m_impl->source.moveToThread(&(m_impl->dataSourceThread));
    connect(&(m_impl->source), &StatusDataSource::sig_sendTFS, this, &PhaseCalibration::setTFSData);
    connect(ui->taskCode, &QComboBox::currentTextChanged, this, &PhaseCalibration::taskCodeChange);

    connect(ui->s_workMode, &QComboBox::currentTextChanged, this, &PhaseCalibration::sWorkModeChanged);
    connect(ui->yc_workMode, &QComboBox::currentTextChanged, this, &PhaseCalibration::ycWorkModeChanged);
    connect(ui->sc_workMode, &QComboBox::currentTextChanged, this, &PhaseCalibration::scWorkModeChanged);

    connect(ui->s_dotDrequency, &QComboBox::currentTextChanged, this, &PhaseCalibration::sDotDrequencyChanged);
    connect(ui->yc_dotDrequency, &QComboBox::currentTextChanged, this, &PhaseCalibration::ycDotDrequencyChanged);
    connect(ui->sc_dotDrequency, &QComboBox::currentTextChanged, this, &PhaseCalibration::scDotDrequencyChanged);

    connect(this, &PhaseCalibration::sg_addLog, this, &PhaseCalibration::addLog);

    connect(ui->start, &QPushButton::clicked, this, &PhaseCalibration::startPhaseCalibration);
    connect(ui->end, &QPushButton::clicked, this, &PhaseCalibration::stopPhaseCalibration);
    connect(ui->finishing, &QPushButton::clicked, this, &PhaseCalibration::clearHistry);
    connect(ui->remove, &QPushButton::clicked, this, &PhaseCalibration::deletePhaseCalibrationResult);
    connect(ui->query, &QPushButton::clicked, this, &PhaseCalibration::queryHistry);

    //时间显示修改为提前一个月到今天
    auto currentDate = GlobalData::currentDate();
    ui->beginDate->setDate(currentDate.addMonths(-1));
    ui->endDate->setDate(currentDate);

    connect(SubscriberHelper::getInstance(),&SubscriberHelper::sg_statusChange,this,&PhaseCalibration::masterStatusChange);
}

PhaseCalibration::~PhaseCalibration() { delete ui; }

void PhaseCalibration::reloadUIData()
{
    /* 先清空ui的数据 */
    ui->taskCode->clear();
    ui->taskCodeSearch->clear();
    m_impl->satelliteManagementDataMap.clear();

    /* 获取卫星管理数据 */
    if (!GlobalData::getSatelliteManagementData(m_impl->satelliteManagementDataMap))
    {
        return;
    }
    for (auto iter = m_impl->satelliteManagementDataMap.begin(); iter != m_impl->satelliteManagementDataMap.end(); ++iter)
    {
        /* 20210816 wp??  这里过滤一种情况
         * 只有X低速 排除
         * 只有S扩跳排除  这个东东需要单独写待定
         */
        auto& value = iter.value();
        if ((value.m_workModeParamMap.size() == 1 && (value.m_workModeParamMap.contains(XDS) || value.m_workModeParamMap.contains(SKT))) ||
            (value.m_workModeParamMap.size() == 2 && value.m_workModeParamMap.contains(XDS) && value.m_workModeParamMap.contains(SKT)))
        {
            continue;
        }
        auto key = iter.key();
        ui->taskCode->addItem(iter->m_satelliteCode, key);
        ui->taskCodeSearch->addItem(iter->m_satelliteCode, key);
    }
}
void PhaseCalibration::taskCodeChange()
{
    auto satelliteCode = ui->taskCode->currentData().toString();
    auto find = m_impl->satelliteManagementDataMap.find(satelliteCode);
    if (find == m_impl->satelliteManagementDataMap.end())
    {
        m_impl->calcPCFBChange(ui);
        m_impl->curSatelliteManagementData = SatelliteManagementData();
        return;
    }
    m_impl->curSatelliteManagementData = find.value();
    /* 刷新工作模式 */
    ui->s_workMode->clear();
    ui->yc_workMode->clear();
    ui->sc_workMode->clear();

    auto workModeMap = m_impl->curSatelliteManagementData.getworkMode();
    for (auto iter = workModeMap.begin(); iter != workModeMap.end(); ++iter)
    {
        auto key = iter.key();
        auto tempString = SystemWorkModeHelper::systemWorkModeToDesc(SystemWorkMode(key));
        switch (SystemWorkMode(key))
        {
        case NotDefine:  // 未定义
        case XGS:        // X高速
        case XDS:        // X低速
        {
            break;
        }
        /* 这玩意儿参数不一样,估计要单独的界面 */
        case SKT:  // 扩跳
        {
            break;
        }
        /* S */
        case STTC:           // S标准TTC
        case Skuo2:          // S扩二
        case SYTHSMJ:        // 一体化上面级
        case SYTHWX:         // 一体化卫星
        case SYTHGML:        // 一体化高码率
        case SYTHSMJK2GZB:   // 一体化上面级+扩二共载波
        case SYTHSMJK2BGZB:  // 一体化上面级+扩二不共载波
        case SYTHWXSK2:      // 一体化卫星+扩二
        case SYTHGMLSK2:     // 一体化高码率+扩二
        {
            ui->s_workMode->addItem(tempString, key);
            break;
        }
        /* Ka遥测 */
        case KaKuo2:  // Ka扩二
        {
            ui->yc_workMode->addItem(tempString, key);
            break;
        }
        /* Ka数传 */
        case KaDS:  // Ka低速
        case KaGS:  // Ka高速
        {
            ui->sc_workMode->addItem(tempString, key);
            break;
        }
        }
    }
    m_impl->calcPCFBChange(ui);
}
void PhaseCalibration::sWorkModeChanged() { m_impl->workModeChanged(ui->s_dotDrequency, ui->s_workMode); }
void PhaseCalibration::ycWorkModeChanged() { m_impl->workModeChanged(ui->yc_dotDrequency, ui->yc_workMode); }
void PhaseCalibration::scWorkModeChanged() { m_impl->workModeChanged(ui->sc_dotDrequency, ui->sc_workMode); }

void PhaseCalibration::sDotDrequencyChanged() { m_impl->dotDrequencyChanged(ui->taskCode, ui->s_dotDrequency, ui->s_workMode, ui->s_downFreqQuick); }
void PhaseCalibration::ycDotDrequencyChanged()
{
    m_impl->dotDrequencyChanged(ui->taskCode, ui->yc_dotDrequency, ui->yc_workMode, ui->yc_downFreqQuick);
}
void PhaseCalibration::scDotDrequencyChanged()
{
    m_impl->dotDrequencyChanged(ui->taskCode, ui->sc_dotDrequency, ui->sc_workMode, ui->sc_downFreqQuick);
}

void PhaseCalibration::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    emit m_impl->source.sg_stop();
}

void PhaseCalibration::showEvent(QShowEvent* event)
{
    /* 校零只能测控 sttc sk2 kak2 */
    QWidget::showEvent(event);
    reloadUIData();
    emit m_impl->source.sg_start();
}
void PhaseCalibration::masterStatusChange()
{
    if (m_impl->logSubscriber.is_connected())
    {
        m_impl->logSubscriber.disconnect();
    }
}
void PhaseCalibration::timerEvent(QTimerEvent* /*event*/)
{
    if (m_impl->logSubscriber.is_connected())
    {
        return;
    }
    if (!RedisHelper::getInstance().getSubscriber(m_impl->logSubscriber))
    {
        return;
    }

    m_impl->logSubscriber.subscribe("PhaseCalibrationLog", [=](const std::string& /*chanel*/, const std::string& msg) {
        auto qmsg = QString::fromStdString(msg);
        emit sg_addLog(qmsg);
    });
    m_impl->logSubscriber.commit();
}

void PhaseCalibration::resetValue()
{
    if (m_impl->isReset)
    {
        return;
    }
    m_impl->isReset = true;
    SCUtility::resetLedStatus(ui->sInstructions);
    SCUtility::resetLedStatus(ui->xKaInstructions);
    SCUtility::resetLedStatus(ui->xKaSCInstructions);

    ui->s_fw->setText("-");
    ui->s_fy->setText("-");
    ui->s_agc->setText("-");

    ui->yc_fw->setText("-");
    ui->yc_fy->setText("-");
    ui->yc_agc->setText("-");

    ui->sc_fw->setText("-");
    ui->sc_fy->setText("-");
    ui->sc_agc->setText("-");
}
void PhaseCalibration::setTFSData(bool isUpdate, const ExtensionStatusReportMessage& message)
{
    if (isUpdate)
    {
        m_impl->isReset = false;
        SCUtility::setDataS(message, m_impl->tfsInfo);
    }
    else
    {
        resetValue();
    }
}

void PhaseCalibration::startPhaseCalibration()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    if (isStart())
    {
        return;
    }

    auto runningTask = AutoRunTaskStepStatusPublish::getRunningTask();
    if(!runningTask.taskPlanData.deviceWorkTask.m_totalTargetMap.isEmpty())
    {
        if (QMessageBox::information(this, "当前有自动化任务正在执行", "当前操作会打乱任务的进行，是否继续?", QString("确定"), QString("取消")))
        {
            return;
        }
    }

    auto manualMsg = GlobalData::getTaskRunningInfoByObject();
    if (!manualMsg.linkLineMap.isEmpty())
    {
        if (QMessageBox::information(this, "当前有手动任务正在执行", "当前操作会打乱任务的进行，是否继续?", QString("确定"), QString("取消")))
        {
            return;
        }
    }

    ui->statusInfo->clear();
    StartPhaseCalibration startPhaseCalibration;

    startPhaseCalibration.parameter.projectCode = ExtendedConfig::curTKID();
    startPhaseCalibration.parameter.taskIdent = m_impl->curSatelliteManagementData.m_satelliteIdentification;
    startPhaseCalibration.parameter.taskCode = m_impl->curSatelliteManagementData.m_satelliteCode;

    if (startPhaseCalibration.parameter.taskCode.isEmpty())
    {
        QMessageBox::information(this, "提示", "当前卫星代号不能为空", "确定");
        return;
    }

    startPhaseCalibration.parameter.freqBand = ACUPhaseCalibrationFreqBand(m_impl->fbGroup->checkedId());
    startPhaseCalibration.parameter.corrMeth = PhaseCalibrationCorrMeth(m_impl->phaseCalibrationMethod->checkedId());

    auto followData = ui->followUpInspectionQuick->currentData();
    Q_ASSERT(followData.isValid());
    startPhaseCalibration.parameter.followCheck = PhaseCalibrationFollowCheck(followData.toInt());
    startPhaseCalibration.parameter.crossLowLimit = quint32(ui->crossCouplingLowerLimitQuick->value());
    startPhaseCalibration.parameter.standSensit = ui->directionalSensitivityQuick->value();
    startPhaseCalibration.parameter.crossLowLimit = quint32(ui->directionalSensitivityToleranceQuick->value());

    auto tempPCFreqBand = startPhaseCalibration.parameter.freqBand;
    auto taskCode = startPhaseCalibration.parameter.taskCode;
    if (tempPCFreqBand == ACUPhaseCalibrationFreqBand::S || tempPCFreqBand == ACUPhaseCalibrationFreqBand::SKaYC ||
        tempPCFreqBand == ACUPhaseCalibrationFreqBand::SKaSC || tempPCFreqBand == ACUPhaseCalibrationFreqBand::SKaYCKaSC)
    {
        auto result = m_impl->getSingleGroupItem<ACUPhaseCalibrationFreqBand::S>(taskCode, ui->s_workMode, ui->s_dotDrequency, ui->s_downFreqQuick);
        if (!result)
        {
            QMessageBox::information(this, "提示", result.msg(), "确定");
            return;
        }
        startPhaseCalibration.saveParameter.insert(ACUPhaseCalibrationFreqBand::S, result.value());
    }
    if (tempPCFreqBand == ACUPhaseCalibrationFreqBand::KaYC || tempPCFreqBand == ACUPhaseCalibrationFreqBand::SKaYC ||
        tempPCFreqBand == ACUPhaseCalibrationFreqBand::SKaYCKaSC)
    {
        auto result =
            m_impl->getSingleGroupItem<ACUPhaseCalibrationFreqBand::KaYC>(taskCode, ui->yc_workMode, ui->yc_dotDrequency, ui->yc_downFreqQuick);
        if (!result)
        {
            QMessageBox::information(this, "提示", result.msg(), "确定");
            return;
        }
        startPhaseCalibration.saveParameter.insert(ACUPhaseCalibrationFreqBand::KaYC, result.value());
    }

    if (tempPCFreqBand == ACUPhaseCalibrationFreqBand::KaSC || tempPCFreqBand == ACUPhaseCalibrationFreqBand::SKaSC ||
        tempPCFreqBand == ACUPhaseCalibrationFreqBand::SKaYCKaSC)
    {
        auto result =
            m_impl->getSingleGroupItem<ACUPhaseCalibrationFreqBand::KaSC>(taskCode, ui->sc_workMode, ui->sc_dotDrequency, ui->sc_downFreqQuick);
        if (!result)
        {
            QMessageBox::information(this, "提示", result.msg(), "确定");
            return;
        }
        startPhaseCalibration.saveParameter.insert(ACUPhaseCalibrationFreqBand::KaSC, result.value());
    }

    start();

    QByteArray json;
    json << startPhaseCalibration;
    emit sg_startPhaseCalibration(json);
}
void PhaseCalibration::stopPhaseCalibration()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    stop();
    emit sg_stopPhaseCalibration(QByteArray());
}
void PhaseCalibration::deletePhaseCalibrationResult()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    PhaseCalibrationItem item;
    if (!m_impl->model.itemAt(ui->tableView->currentIndex(), item))
    {
        QMessageBox::information(this, "提示", "请选择需要删除的数据", "确定");
        return;
    }
    QByteArray json;
    json << item;
    emit sg_deletePhaseCalibrationResult(json);
}
void PhaseCalibration::clearHistry()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    if (1 == QMessageBox::warning(this, "是否继续?", "此操作会清空90天之前的数据,不可恢复", "继续", "取消"))
    {
        return;
    }
    QByteArray json;
    ClearPhaseCalibrationHistry item;
    json << item;
    emit sg_clearHistry(json);
}
void PhaseCalibration::queryHistry()
{
    QueryPhaseCalibrationHistry query;
    /* 20210617 wp??
     * 这里认为查询不需要这么精确 精确到秒没什么意义 你基本都不会去动后面的秒的到天足够了
     * 接口是完整的包括服务器的都是OK的 如果实在是需要精确到秒直接界面改成datetime就OK
     */
    query.beginTime = ui->beginDate->dateTime();
    query.endTime = ui->endDate->dateTime();
    query.workMode = SystemWorkMode(ui->workMode->currentData().toInt());

    auto satelliteCode = ui->taskCodeSearch->currentData().toString();
    query.taskCode = satelliteCode;
    if (query.beginTime > query.endTime)
    {
        QMessageBox::critical(this, "提示", "开始时间不能晚于结束时间", "确定");
        return;
    }
    if (satelliteCode.isEmpty())
    {
        QMessageBox::critical(this, "提示", "任务代号不能为空", "确定");
        return;
    }

    m_impl->model.clear();
    QByteArray json;
    json << query;
    emit sg_queryHistry(json);
}

void PhaseCalibration::start() { Utility::startTimer(this, m_impl->timerID, 1000); }
void PhaseCalibration::stop() { Utility::killTimer(this, m_impl->timerID); }
bool PhaseCalibration::isStart() { return m_impl->timerID != -1; }
void PhaseCalibration::addLog(const QString& qmsg) { ui->statusInfo->append(qmsg); }

void PhaseCalibration::startPhaseCalibrationACK(const QByteArray& data)
{
    auto ack = OptionalNotValue::emptyOptional();
    data >> ack;
    stop();
    ui->statusInfo->append("校相结束");
    ui->statusInfo->append(ack.msg());
}
void PhaseCalibration::stopPhaseCalibrationACK(const QByteArray& data) {}
void PhaseCalibration::deletePhaseCalibrationResultACK(const QByteArray& data)
{
    auto bak = data;
    auto result = Optional<PhaseCalibrationItem>::emptyOptional();
    bak >> result;
    if (!result)
    {
        QMessageBox::information(this, "删除失败", result.msg(), "确定");
        return;
    }
    m_impl->model.removeValue(result.value());
    QMessageBox::information(this, "提示", "删除成功", "确定");
}
void PhaseCalibration::clearHistryACK(const QByteArray& data)
{
    OptionalNotValue result;
    data >> result;
    if (!result)
    {
        QMessageBox::information(this, "清除错误", result.msg(), "确定");
        return;
    }
    QMessageBox::information(this, "提示", "清除成功");
}
void PhaseCalibration::queryHistryACK(const QByteArray& data)
{
    auto result = Optional<PhaseCalibrationItemList>::emptyOptional();
    data >> result;
    if (!result)
    {
        QMessageBox::information(this, "提示", result.msg());
        return;
    }
    m_impl->model.setValue(result.value());
}
