#include "SystemCaptureProcessControl.h"
#include "ui_SystemCaptureProcessControl.h"

#include "CustomPacketMessageSerialize.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "ProtocolXmlTypeDefine.h"
#include "ServiceCheck.h"
#include "SystemCaptureProcessCmd.h"

#include <QButtonGroup>
#include <QMessageBox>

class SystemCaptureProcessControlImpl
{
public:
    SystemCaptureProcessControl* self{ nullptr };

    SystemCaptureProcessXml* m_xml{ nullptr };

    QComboBox* curDeviceComBox{ nullptr };

    SystemWorkMode currentMode;

    quint64 deviceID{ 0 };
    QButtonGroup* captureMode{ nullptr };
    QButtonGroup* blindControl{ nullptr };

    QButtonGroup* target1_r{ nullptr };
    QButtonGroup* target1_m{ nullptr };
    QButtonGroup* target2_r{ nullptr };
    QButtonGroup* target2_m{ nullptr };
    QButtonGroup* target3_r{ nullptr };
    QButtonGroup* target3_m{ nullptr };
    QButtonGroup* target4_r{ nullptr };
    QButtonGroup* target4_m{ nullptr };
    QButtonGroup* k2_adjunct{ nullptr };
    QButtonGroup* k2_carrier{ nullptr };

    QButtonGroup* sttc_carrier{ nullptr };

public:
    SystemCaptureProcessControlImpl(SystemCaptureProcessControl* self_)
        : self(self_)
    {
    }
};

/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
SystemCaptureProcessControl::SystemCaptureProcessControl(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SystemCaptureProcessControl)
    , m_impl(new SystemCaptureProcessControlImpl(this))
{
    ui->setupUi(this);
    m_impl->captureMode = new QButtonGroup(this);
    m_impl->blindControl = new QButtonGroup(this);

    m_impl->captureMode->addButton(ui->captureModeAuto, 1);
    m_impl->captureMode->addButton(ui->captureModeManual, 2);
    m_impl->blindControl->addButton(ui->blindControlOn, 1);
    m_impl->blindControl->addButton(ui->blindControlOff, 2);
    ui->captureModeManual->setChecked(true);
    ui->blindControlOn->setChecked(true);
    ui->switchSlave->hide();

    setShowWidget(STTC);

    auto currentProject = ExtendedConfig::curProjectID();
    if (currentProject == "4424")
    {
        ui->macb->addItem("????????????A", 0x4001);
        ui->macb->addItem("????????????B", 0x4002);
    }
    else
    {
        ui->macb->addItem("????????????A-S", 0x4001);
        ui->macb->addItem("????????????A-Ka", 0x4002);
        ui->macb->addItem("????????????B-S", 0x4003);
        ui->macb->addItem("????????????B-Ka", 0x4004);
    }

    m_impl->target1_r = new QButtonGroup(this);
    m_impl->target1_m = new QButtonGroup(this);
    m_impl->target2_r = new QButtonGroup(this);
    m_impl->target2_m = new QButtonGroup(this);
    m_impl->target3_r = new QButtonGroup(this);
    m_impl->target3_m = new QButtonGroup(this);
    m_impl->target4_r = new QButtonGroup(this);
    m_impl->target4_m = new QButtonGroup(this);
    m_impl->k2_adjunct = new QButtonGroup(this);
    m_impl->k2_carrier = new QButtonGroup(this);
    m_impl->sttc_carrier = new QButtonGroup(this);

    m_impl->target1_r->addButton(ui->target1_r_on, 1);
    m_impl->target1_r->addButton(ui->target1_r_off, 2);
    m_impl->target2_r->addButton(ui->target2_r_on, 1);
    m_impl->target2_r->addButton(ui->target2_r_off, 2);
    m_impl->target3_r->addButton(ui->target3_r_on, 1);
    m_impl->target3_r->addButton(ui->target3_r_off, 2);
    m_impl->target4_r->addButton(ui->target4_r_on, 1);
    m_impl->target4_r->addButton(ui->target4_r_off, 2);

    m_impl->target1_m->addButton(ui->target1_m_on, 1);
    m_impl->target1_m->addButton(ui->target1_m_off, 2);
    m_impl->target2_m->addButton(ui->target2_m_on, 1);
    m_impl->target2_m->addButton(ui->target2_m_off, 2);
    m_impl->target3_m->addButton(ui->target3_m_on, 1);
    m_impl->target3_m->addButton(ui->target3_m_off, 2);
    m_impl->target4_m->addButton(ui->target4_m_on, 1);
    m_impl->target4_m->addButton(ui->target4_m_off, 2);

    m_impl->k2_adjunct->addButton(ui->k2_adjunct_on, 1);
    m_impl->k2_adjunct->addButton(ui->k2_adjunct_off, 2);
    m_impl->k2_carrier->addButton(ui->k2_carrier_on, 1);
    m_impl->k2_carrier->addButton(ui->k2_carrier_off, 2);

    m_impl->sttc_carrier->addButton(ui->sttc_carrier_on, 1);
    m_impl->sttc_carrier->addButton(ui->sttc_carrier_off, 2);

    ui->target1_r_on->setChecked(true);
    ui->target2_r_on->setChecked(true);
    ui->target3_r_on->setChecked(true);
    ui->target4_r_on->setChecked(true);
    ui->target1_m_on->setChecked(true);
    ui->target2_m_on->setChecked(true);
    ui->target3_m_on->setChecked(true);
    ui->target4_m_on->setChecked(true);
    ui->k2_adjunct_on->setChecked(true);
    ui->k2_carrier_on->setChecked(true);
    ui->sttc_carrier_on->setChecked(true);

    //????????????????????????????????????????????????????????????
    ui->syth_dopplerPresets->setHidden(true);
    ui->syth_backToZero->setHidden(true);
    ui->syth_carrier->setHidden(true);
    ui->k2_dopplerPresets->setHidden(true);

    connect(m_impl->captureMode, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this,
            &SystemCaptureProcessControl::slotAutoManualBtnSwitch);
    startTimer(1000);
}

SystemCaptureProcessControl::~SystemCaptureProcessControl()
{
    delete ui;

    if (m_impl->m_xml != nullptr)
    {
        delete m_impl->m_xml;
        m_impl->m_xml = nullptr;
    }
    delete m_impl;
    m_impl = nullptr;
}

void SystemCaptureProcessControl::setShowWidget(SystemWorkMode workMode)
{
    m_impl->currentMode = workMode;

    setCurDeviceComBox();

    switch (workMode)
    {
    case STTC:  // S??????TTC
    {
        ui->ttc->setVisible(true);
        ui->k2->setVisible(false);
        ui->skt->setVisible(false);
        ui->syth->setVisible(false);
        break;
    }
    case Skuo2:  // S??????
    case KaKuo2:
    {
        ui->ttc->setVisible(false);
        ui->k2->setVisible(true);
        ui->skt->setVisible(false);
        ui->syth->setVisible(false);
        break;
    }
    case SYTHSMJ:        // ??????????????????
    case SYTHWX:         // ???????????????
    case SYTHGML:        // ??????????????????
    case SYTHSMJK2BGZB:  // ??????????????????+??????????????????
    case SYTHSMJK2GZB:   // ??????????????????+???????????????
    case SYTHWXSK2:      // ???????????????+??????
    case SYTHGMLSK2:     // ??????????????????+??????
    {
        ui->ttc->setVisible(false);
        ui->k2->setVisible(false);
        ui->skt->setVisible(false);
        ui->syth->setVisible(true);
        break;
    }
    case SKT:  // ??????
    {
        ui->ttc->setVisible(false);
        ui->k2->setVisible(false);
        ui->skt->setVisible(true);
        ui->syth->setVisible(false);
        break;
    }
    default:
    {
        ui->ttc->setVisible(false);
        ui->k2->setVisible(false);
        ui->skt->setVisible(false);
        ui->syth->setVisible(false);
        break;
    }
    }
}

void SystemCaptureProcessControl::setCurDeviceComBox()
{
    //????????????????????????????????????????????????????????????xml????????????????????????????????????
    auto modeID = SystemWorkModeHelper::systemWorkModeConverToModeId(m_impl->currentMode);
    if (m_impl->m_xml != nullptr)
    {
        m_impl->m_xml->setModeID(modeID);
    }
    if (m_impl->m_xml == nullptr)
    {
        m_impl->m_xml = new SystemCaptureProcessXml();
        connect(m_impl->m_xml, &SystemCaptureProcessXml::signalsCmdRequest, this, &SystemCaptureProcessControl::slotCmdRequest);
        connect(m_impl->m_xml, &SystemCaptureProcessXml::signalsUnitParamRequest, this, &SystemCaptureProcessControl::slotUnitParamRequest);

        connect(m_impl->m_xml, &SystemCaptureProcessXml::signalsCmdDeviceJson, this, &SystemCaptureProcessControl::signalsCmdDeviceJson);
        connect(m_impl->m_xml, &SystemCaptureProcessXml::signalsUnitDeviceJson, this, &SystemCaptureProcessControl::signalsUnitDeviceJson);
        m_impl->m_xml->setModeID(modeID);
        m_impl->m_xml->start(ui->macb, this, "SystemCaptureProcessControl");
    }
}

void SystemCaptureProcessControl::setXMLSlot()
{
    //??????????????????
    if (m_impl->m_xml != nullptr)
    {
        delete m_impl->m_xml;
        m_impl->m_xml = nullptr;
    }
    if (m_impl->m_xml == nullptr)
    {
        m_impl->m_xml = new SystemCaptureProcessXml();
        connect(m_impl->m_xml, &SystemCaptureProcessXml::signalsCmdRequest, this, &SystemCaptureProcessControl::slotCmdRequest);
        connect(m_impl->m_xml, &SystemCaptureProcessXml::signalsUnitParamRequest, this, &SystemCaptureProcessControl::slotUnitParamRequest);

        connect(m_impl->m_xml, &SystemCaptureProcessXml::signalsCmdDeviceJson, this, &SystemCaptureProcessControl::signalsCmdDeviceJson);
        connect(m_impl->m_xml, &SystemCaptureProcessXml::signalsUnitDeviceJson, this, &SystemCaptureProcessControl::signalsUnitDeviceJson);
        auto modeID = SystemWorkModeHelper::systemWorkModeConverToModeId(m_impl->currentMode);
        m_impl->m_xml->setModeID(modeID);
        m_impl->m_xml->start(ui->macb, this, "SystemCaptureProcessControl");
    }
}

bool SystemCaptureProcessControl::deviceIsOnline(const DeviceID& deviceID)
{
    QString errorMsg;
    if (!GlobalData::getDeviceOnline(deviceID))
    {
        auto deviceName = GlobalData::getExtensionName(deviceID);
        errorMsg = QString("%1?????????????????????????????????").arg(deviceName);
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("??????"), errorMsg, QString("??????"));
        return false;
    }
    return true;
}

bool SystemCaptureProcessControl::deviceIsMode(const DeviceID& deviceID, int modeId)
{
    QString errorMsg;
    if (!SystemWorkModeHelper::modeIdIsValid(modeId))
    {
        errorMsg = QString("??????????????????????????????????????????????????????");
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("??????"), errorMsg, QString("??????"));
    }

    int curModeId = GlobalData::getOnlineDeviceModeId(deviceID);
    if (modeId != curModeId)
    {
        auto workMode = SystemWorkModeHelper::modeIDToSystemWorkMode(curModeId);
        auto deviceName = GlobalData::getExtensionName(deviceID);
        auto curModeName = SystemWorkModeHelper::systemWorkModeToDesc(workMode);
        errorMsg = QString("%1?????????????????????%2???????????????????????????????????????????????????????????????").arg(deviceName).arg(curModeName);
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("??????"), errorMsg, QString("??????"));
        return false;
    }
    return true;
}

bool SystemCaptureProcessControl::deviceIsSelfControl(const DeviceID& deviceID)
{
    QString errorMsg;
    if (!GlobalData::getDeviceSelfControl(deviceID))
    {
        auto deviceName = GlobalData::getExtensionName(deviceID);
        errorMsg = QString("%1??????????????????????????????????????????").arg(deviceName);
        SystemLogPublish::warningMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("??????"), errorMsg, QString("??????"));
        return false;
    }

    return true;
}

bool SystemCaptureProcessControl::deviceIsMaster(const DeviceID& deviceID)
{
    auto value = GlobalData::getReportParamData(deviceID, 1, "OnLineMachine");
    // ????????????  ????????????????????????????????????????????????????????????id??????????????????????????????????????????????????????????????????????????????
    if (!value.isValid())
    {
        return true;
    }

    // ????????????1?????????  2?????????
    return value.toInt() != 1 ? false : true;
}

int SystemCaptureProcessControl::getCurrentDeviceId()
{
    int deviceId = ui->macb->currentData().toInt();
    return deviceId;
}

int SystemCaptureProcessControl::getCurrentModeID()
{
    auto modeID = SystemWorkModeHelper::systemWorkModeConverToModeId(m_impl->currentMode);
    return modeID;
}

void SystemCaptureProcessControl::resetUIData() { ui->macb->setCurrentIndex(0); }

void SystemCaptureProcessControl::slotCmdRequest(const CmdRequest& cmdRequest, bool isCheckMasterSlave)
{
    // ???????????????????????????
    SERVICEONLINECHECKNOPARENT();

    DeviceID deviceID(cmdRequest.m_systemNumb, cmdRequest.m_deviceNumb, cmdRequest.m_extenNumb);
    // ??????????????????
    if (!deviceIsOnline(deviceID))
    {
        return;
    }
    // ??????????????????
    if (!deviceIsMode(deviceID, cmdRequest.m_modeID))
    {
        return;
    }
    // ?????????????????????
    if (!deviceIsSelfControl(deviceID))
    {
        return;
    }
    // ?????????????????????
    auto isMaster = deviceIsMaster(deviceID);
    // ??????????????????????????????????????????????????????????????????????????????????????????????????????????????????
    if (isCheckMasterSlave && !isMaster)
    {
        QString errorMsg;
        auto deviceName = GlobalData::getExtensionName(deviceID);
        errorMsg = QString("%1????????????????????????????????????????????????").arg(deviceName);
        SystemLogPublish::errorMsg(errorMsg);
        if (QMessageBox::warning(nullptr, QString("??????"), errorMsg, QString("???"), QString("???")) != 0)
        {
            return;
        }
    }

    JsonWriter writer;
    writer& cmdRequest;
    emit signalsCmdDeviceJson(writer.GetString());
}

void SystemCaptureProcessControl::slotUnitParamRequest(const UnitParamRequest& unitParamRequest, bool isCheckMasterSlave)
{
    SERVICEONLINECHECKNOPARENT();

    DeviceID deviceID(unitParamRequest.m_systemNumb, unitParamRequest.m_deviceNumb, unitParamRequest.m_extenNumb);
    // ??????????????????
    if (!deviceIsOnline(deviceID))
    {
        return;
    }
    // ??????????????????
    if (!deviceIsMode(deviceID, unitParamRequest.m_modeID))
    {
        return;
    }
    // ?????????????????????
    if (!deviceIsSelfControl(deviceID))
    {
        return;
    }
    // ?????????????????????
    auto isMaster = deviceIsMaster(deviceID);
    // ??????????????????????????????????????????????????????????????????????????????????????????????????????????????????
    if (isCheckMasterSlave && !isMaster)
    {
        QString errorMsg;
        auto deviceName = GlobalData::getExtensionName(deviceID);
        errorMsg = QString("%1????????????????????????????????????????????????").arg(deviceName);
        SystemLogPublish::errorMsg(errorMsg);
        if (QMessageBox::warning(nullptr, QString("??????"), errorMsg, QString("???"), QString("???")) != 0)
        {
            return;
        }
    }

    JsonWriter writer;
    writer& unitParamRequest;
    emit signalsUnitDeviceJson(writer.GetString());
}

void SystemCaptureProcessControl::slotAutoManualBtnSwitch(int id)
{
    auto btn = m_impl->captureMode->button(id);
    if (btn == nullptr)
    {
        return;
    }

    // 1????????? 2???????????????
    if (id == 1)
    {
        ui->ttc_doubleCatchStart->setEnabled(false);
        ui->ttc_openAndSweep->setEnabled(false);
        ui->ttc_backToZero->setEnabled(false);
        ui->ttc_rangeCapture->setEnabled(false);
        ui->ttc_AFStart->setEnabled(false);
        ui->ttc_AFEnd->setEnabled(false);
    }
    else if (id == 2)
    {
        ui->ttc_doubleCatchStart->setEnabled(true);
        ui->ttc_openAndSweep->setEnabled(true);
        ui->ttc_backToZero->setEnabled(true);
        ui->ttc_rangeCapture->setEnabled(true);
        ui->ttc_AFStart->setEnabled(true);
        ui->ttc_AFEnd->setEnabled(true);
    }
}

void SystemCaptureProcessControl::timerEvent(QTimerEvent* /*event*/)
{
    // 1.??????redis??????????????????????????????????????????????????????
    // 2.???????????????????????????,????????????????????????groupbox??????
    if (m_impl->currentMode == Skuo2 || m_impl->currentMode == KaKuo2)
    {
        int deviceID = ui->macb->currentData().toInt();
        int modeId = GlobalData::getOnlineDeviceModeId(deviceID);
        if (modeId == SystemWorkModeHelper::systemWorkModeConverToModeId(m_impl->currentMode))
        {
            ExtensionStatusReportMessage reportMessage = GlobalData::getExtenStatusReportData(deviceID);
            int targetNum = reportMessage.unitReportMsgMap[1].paramMap["TargetNum"].toInt();
            switch (targetNum)
            {
            case 1:
            {
                //                ui->groupBox_1->setEnabled(true);
                ui->groupBox_2->setEnabled(false);
                ui->groupBox_3->setEnabled(false);
                ui->groupBox_4->setEnabled(false);
                break;
            }
            case 2:
            {
                //                ui->groupBox_1->setEnabled(true);
                ui->groupBox_2->setEnabled(true);
                ui->groupBox_3->setEnabled(false);
                ui->groupBox_4->setEnabled(false);
                break;
            }
            case 3:
            {
                //                ui->groupBox_1->setEnabled(true);
                ui->groupBox_2->setEnabled(true);
                ui->groupBox_3->setEnabled(true);
                ui->groupBox_4->setEnabled(false);
                break;
            }
            default:
            {
                //                ui->groupBox_1->setEnabled(true);
                ui->groupBox_2->setEnabled(true);
                ui->groupBox_3->setEnabled(true);
                ui->groupBox_4->setEnabled(true);
                break;
            }
            }
        }
    }
}
