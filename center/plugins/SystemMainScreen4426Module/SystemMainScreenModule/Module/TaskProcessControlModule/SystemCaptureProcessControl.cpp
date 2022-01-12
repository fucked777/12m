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
        ui->macb->addItem("测控基带A", 0x4001);
        ui->macb->addItem("测控基带B", 0x4002);
    }
    else
    {
        ui->macb->addItem("测控基带A-S", 0x4001);
        ui->macb->addItem("测控基带A-Ka", 0x4002);
        ui->macb->addItem("测控基带B-S", 0x4003);
        ui->macb->addItem("测控基带B-Ka", 0x4004);
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

    //设备监控相应的命令不显示，因此不做该按钮
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
    case STTC:  // S标准TTC
    {
        ui->ttc->setVisible(true);
        ui->k2->setVisible(false);
        ui->skt->setVisible(false);
        ui->syth->setVisible(false);
        break;
    }
    case Skuo2:  // S扩二
    case KaKuo2:
    {
        ui->ttc->setVisible(false);
        ui->k2->setVisible(true);
        ui->skt->setVisible(false);
        ui->syth->setVisible(false);
        break;
    }
    case SYTHSMJ:        // 一体化上面级
    case SYTHWX:         // 一体化卫星
    case SYTHGML:        // 一体化高码率
    case SYTHSMJK2BGZB:  // 一体化上面级+扩二不共载波
    case SYTHSMJK2GZB:   // 一体化上面级+扩二共载波
    case SYTHWXSK2:      // 一体化卫星+扩二
    case SYTHGMLSK2:     // 一体化高码率+扩二
    {
        ui->ttc->setVisible(false);
        ui->k2->setVisible(false);
        ui->skt->setVisible(false);
        ui->syth->setVisible(true);
        break;
    }
    case SKT:  // 扩跳
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
    //这个函数是根据计时器不停的设置模式，如果xml对象不在，就再初始化一次
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
    //第一次初始化
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
        errorMsg = QString("%1设备离线，命令下发失败").arg(deviceName);
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
        return false;
    }
    return true;
}

bool SystemCaptureProcessControl::deviceIsMode(const DeviceID& deviceID, int modeId)
{
    QString errorMsg;
    if (!SystemWorkModeHelper::modeIdIsValid(modeId))
    {
        errorMsg = QString("当前下发命令的模式无效，命令下发失败");
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
    }

    int curModeId = GlobalData::getOnlineDeviceModeId(deviceID);
    if (modeId != curModeId)
    {
        auto workMode = SystemWorkModeHelper::modeIDToSystemWorkMode(curModeId);
        auto deviceName = GlobalData::getExtensionName(deviceID);
        auto curModeName = SystemWorkModeHelper::systemWorkModeToDesc(workMode);
        errorMsg = QString("%1设备当前模式为%2，与需要下发的命令模式不符，请重新选择模式").arg(deviceName).arg(curModeName);
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
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
        errorMsg = QString("%1设备为分控状态，命令下发失败").arg(deviceName);
        SystemLogPublish::warningMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
        return false;
    }

    return true;
}

bool SystemCaptureProcessControl::deviceIsMaster(const DeviceID& deviceID)
{
    auto value = GlobalData::getReportParamData(deviceID, 1, "OnLineMachine");
    // 测控基带  无效代表该设备没有主备机概念，或者该参数id错误，默认不存在参数错误的情况，所以无效就代表是主机
    if (!value.isValid())
    {
        return true;
    }

    // 测控基带1为主机  2为备机
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
    // 服务器在线状态检测
    SERVICEONLINECHECKNOPARENT();

    DeviceID deviceID(cmdRequest.m_systemNumb, cmdRequest.m_deviceNumb, cmdRequest.m_extenNumb);
    // 设备在线检测
    if (!deviceIsOnline(deviceID))
    {
        return;
    }
    // 设备模式检测
    if (!deviceIsMode(deviceID, cmdRequest.m_modeID))
    {
        return;
    }
    // 设备本分控检测
    if (!deviceIsSelfControl(deviceID))
    {
        return;
    }
    // 判断是否是主机
    auto isMaster = deviceIsMaster(deviceID);
    // 是否进行检测主备机情况，如果要检查，并且是备机就进行提示是否继续进行下发命令
    if (isCheckMasterSlave && !isMaster)
    {
        QString errorMsg;
        auto deviceName = GlobalData::getExtensionName(deviceID);
        errorMsg = QString("%1设备为备机，是否继续进行下发命令").arg(deviceName);
        SystemLogPublish::errorMsg(errorMsg);
        if (QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("是"), QString("否")) != 0)
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
    // 设备在线检测
    if (!deviceIsOnline(deviceID))
    {
        return;
    }
    // 设备模式检测
    if (!deviceIsMode(deviceID, unitParamRequest.m_modeID))
    {
        return;
    }
    // 设备本分控检测
    if (!deviceIsSelfControl(deviceID))
    {
        return;
    }
    // 判断是否是主机
    auto isMaster = deviceIsMaster(deviceID);
    // 是否进行检测主备机情况，如果要检查，并且是备机就进行提示是否继续进行下发命令
    if (isCheckMasterSlave && !isMaster)
    {
        QString errorMsg;
        auto deviceName = GlobalData::getExtensionName(deviceID);
        errorMsg = QString("%1设备为备机，是否继续进行下发命令").arg(deviceName);
        SystemLogPublish::errorMsg(errorMsg);
        if (QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("是"), QString("否")) != 0)
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

    // 1是自动 2是手动按钮
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
    // 1.如果redis对应的测控基带以及模式和快捷控制一样
    // 2.那么取出当前目标数,超出当前目标数的groupbox置灰
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
