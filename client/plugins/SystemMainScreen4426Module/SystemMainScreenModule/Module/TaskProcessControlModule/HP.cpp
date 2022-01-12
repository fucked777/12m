#include "HP.h"
#include "ControlFactory.h"
#include "CustomPacketMessageDefine.h"
#include "GlobalData.h"
#include "HPCmd.h"
#include "ui_HP.h"
#include <ExtendedConfig.h>
#include <QComboBox>
#include <QMessageBox>

namespace HPInfo
{
    static const int DEVICEID_SHPA = 0x2001;
    static const int DEVICEID_SHPB = 0x2002;
    static const int DEVICEID_SHQHCX = 0x2101;
    static const int DEVICEID_KACKHPA = 0x2003;
    static const int DEVICEID_KACKHPB = 0x2004;
    static const int DEVICEID_KASCHPA = 0x2005;
    static const int DEVICEID_KASCHPB = 0x2006;
    static const int DEVICEID_CKBBEA = 0x4001;
    static const int DEVICEID_CKBBEB = 0x4002;
    static const int DEVICEID_CKBBEC = 0x4003;
    static const int DEVICEID_CKBBED = 0x4004;
    static const int MODEID = 0xFFFF;

    static int getMasterSHP()
    {
        if (GlobalData::getDeviceOnlineByObject(DEVICEID_SHQHCX))
        {
            ExtensionStatusReportMessage reportMessage = GlobalData::getExtenStatusReportDataByObject(DEVICEID_SHQHCX);
            int SwitcherSet = reportMessage.unitReportMsgMap[1].paramMap["SwitcherSet"].toInt();
            if (SwitcherSet == 1)
            {
                return DEVICEID_SHPA;
            }
            else
            {
                return DEVICEID_SHPB;
            }
        }
        else
        {
            return -1;
        }
    }
    static int getMasterKaCKHP()
    {
        if (GlobalData::getDeviceOnlineByObject(DEVICEID_KACKHPA))
        {
            ExtensionStatusReportMessage reportMessage = GlobalData::getExtenStatusReportDataByObject(DEVICEID_KACKHPA);
            int OnlineDevice = reportMessage.unitReportMsgMap[1].paramMap["PrevenStatus"].toInt();
            if (OnlineDevice == 1)
            {
                return DEVICEID_KACKHPA;
            }
            else
            {
                return DEVICEID_KACKHPB;
            }
        }
        else
        {
            return -1;
        }
    }
    static int getMasterKaSCHP()
    {
        if (GlobalData::getDeviceOnlineByObject(DEVICEID_KASCHPA))
        {
            ExtensionStatusReportMessage reportMessage = GlobalData::getExtenStatusReportDataByObject(DEVICEID_KASCHPA);
            int OnlineDevice = reportMessage.unitReportMsgMap[1].paramMap["OnlineDevice"].toInt();
            if (OnlineDevice == 1)
            {
                return DEVICEID_KASCHPA;
            }
            else
            {
                return DEVICEID_KASCHPB;
            }
        }
        else
        {
            return -1;
        }
    }
    static int getModeID(const int deviceID)
    {
        if (GlobalData::getDeviceOnlineByObject(deviceID))
        {
            ExtensionStatusReportMessage reportMessage = GlobalData::getExtenStatusReportDataByObject(deviceID);
            return reportMessage.modeId;
        }
        else
        {
            return -1;
        }
    }

}  // namespace HPInfo

HP::HP(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::HP)
    , m_currentProject(ExtendedConfig::curProjectID())
    , d_power_S(nullptr)
    , d_power_KaCKA(nullptr)
    , d_power_KaCKB(nullptr)
    , d_power_KaSCA(nullptr)
    , d_power_KaSCB(nullptr)
{
    ui->setupUi(this);

    initUI();
}

void HP::initUI()
{
    ui->toACU_S->addItem("A上天线B去负载", HPInfo::DEVICEID_SHPA);
    ui->toACU_S->addItem("B上天线A去负载", HPInfo::DEVICEID_SHPB);
    ui->output_S->addItem("加激励", 1);
    ui->output_S->addItem("去激励", 2);
    ui->alc_S->addItem("开环", 1);
    ui->alc_S->addItem("闭环", 2);
    d_power_S = createFreqContol(HPInfo::DEVICEID_SHPA, HPInfo::MODEID, 1, "TransmPower");
    ui->horizontalLayout_S->addWidget(d_power_S);

    if (m_currentProject == "4424") {}
    else
    {
        ui->toACU_KaCK->addItem("A上天线B去负载", HPInfo::DEVICEID_KACKHPA);
        ui->toACU_KaCK->addItem("B上天线A去负载", HPInfo::DEVICEID_KACKHPB);
        ui->output_KaCK->addItem("加激励", 1);
        ui->output_KaCK->addItem("去激励", 2);
        ui->alc_KaCK->addItem("开环", 1);
        ui->alc_KaCK->addItem("闭环", 2);

        d_power_KaCKA = createFreqContol(HPInfo::DEVICEID_KACKHPA, HPInfo::MODEID, 1, "TransmPower");
        d_power_KaCKA->setVisible(true);
        ui->horizontalLayout_KaCK->addWidget(d_power_KaCKA);
        d_power_KaCKB = createFreqContol(HPInfo::DEVICEID_KACKHPB, HPInfo::MODEID, 1, "TransmPower");
        d_power_KaCKB->setVisible(false);
        ui->horizontalLayout_KaCK->addWidget(d_power_KaCKB);
        connect(ui->toACU_KaCK, static_cast<void (QComboBox::*)(int index)>(&QComboBox::currentIndexChanged), this, [=](int /*index*/) {
            if (ui->toACU_KaCK->currentData() == HPInfo::DEVICEID_KACKHPA)
            {
                d_power_KaCKA->setVisible(true);
                d_power_KaCKB->setVisible(false);
            }
            else
            {
                d_power_KaCKA->setVisible(false);
                d_power_KaCKB->setVisible(true);
            }
        });

        ui->toACU_KaSC->addItem("A上天线B去负载", HPInfo::DEVICEID_KASCHPA);
        ui->toACU_KaSC->addItem("B上天线A去负载", HPInfo::DEVICEID_KASCHPB);
        ui->output_KaSC->addItem("加激励", 1);
        ui->output_KaSC->addItem("去激励", 2);
        ui->alc_KaSC->addItem("开环", 1);
        ui->alc_KaSC->addItem("闭环", 2);
        d_power_KaSCA = createFreqContol(HPInfo::DEVICEID_KASCHPA, HPInfo::MODEID, 1, "TransmPower");
        d_power_KaSCA->setVisible(true);
        ui->horizontalLayout_KaSC->addWidget(d_power_KaSCA);
        d_power_KaSCB = createFreqContol(HPInfo::DEVICEID_KASCHPB, HPInfo::MODEID, 1, "TransmPower");
        d_power_KaSCB->setVisible(false);
        ui->horizontalLayout_KaSC->addWidget(d_power_KaSCB);
        connect(ui->toACU_KaSC, static_cast<void (QComboBox::*)(int index)>(&QComboBox::currentIndexChanged), this, [=](int /*index*/) {
            if (ui->toACU_KaSC->currentData() == HPInfo::DEVICEID_KACKHPA)
            {
                d_power_KaSCA->setVisible(true);
                d_power_KaSCB->setVisible(false);
            }
            else
            {
                d_power_KaSCA->setVisible(false);
                d_power_KaSCB->setVisible(true);
            }
        });
    }

    ui->alc_S->setCurrentIndex(1);
    ui->alc_KaCK->setCurrentIndex(1);
    ui->alc_KaSC->setCurrentIndex(1);

    ui->output_S->setCurrentIndex(1);
    ui->output_KaCK->setCurrentIndex(1);
    ui->output_KaSC->setCurrentIndex(1);

    connect(ui->outputSet_S, &QPushButton::clicked, this, [=]() {
        int inspire = ui->output_S->currentData().toInt();
        QByteArray data;
        QVariantMap params;

        if (inspire == 1)  //加激励    加激励只加主用功放的电  去激励就两个功放都去电
        {
            //功放开关设置:对应的功放;功放切换方式:手动;RF开关:开
            params["SwitcherSet"] = ui->toACU_S->currentIndex() + 1;  //功放开关设置
            params["SwitchMode"] = 2;                                 //功放切换方式
            params["RFSwitch"] = 1;                                   // RF开关
            HPCmd::Control(HPInfo::DEVICEID_SHQHCX, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
            if (data.size())
                emit signalsUnitDeviceJson(data);

            if (ui->toACU_S->currentData().toInt() == HPInfo::DEVICEID_SHPA)
            {
                //功放A加电,功放B去电
                data.clear();
                params.clear();
                params["ElectricControlor"] = 2;  //加去电控制
                HPCmd::Control(HPInfo::DEVICEID_SHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                if (data.size())
                    emit signalsUnitDeviceJson(data);

                data.clear();
                params.clear();
                params["ElectricControlor"] = 1;  //加去电控制
                HPCmd::Control(HPInfo::DEVICEID_SHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }
            else
            {
                //功放B加电,功放A去电
                data.clear();
                params.clear();
                params["ElectricControlor"] = 2;  //加去电控制
                HPCmd::Control(HPInfo::DEVICEID_SHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                if (data.size())
                    emit signalsUnitDeviceJson(data);

                data.clear();
                params.clear();
                params["ElectricControlor"] = 1;  //加去电控制
                HPCmd::Control(HPInfo::DEVICEID_SHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }

            ctrlCKBBESTX(true, HPTYPE::SCK);
        }
        if (inspire == 2)  //去激励  去激励两个功放都去电
        {
            int deviceID = HPInfo::getMasterSHP();
            if (deviceID != -1)
            {
                //功放开关设置:对应的功放;功放切换方式:手动;RF开关:关
                params["SwitcherSet"] = ui->toACU_S->currentIndex() + 1;  //功放开关设置
                params["SwitchMode"] = 2;                                 //功放切换方式
                params["RFSwitch"] = 2;                                   // RF开关
                HPCmd::Control(HPInfo::DEVICEID_SHQHCX, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                if (data.size())
                    emit signalsUnitDeviceJson(data);

                //功放AB都去电
                data.clear();
                params.clear();
                params["ElectricControlor"] = 1;  //加去电控制
                HPCmd::Control(HPInfo::DEVICEID_SHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                if (data.size())
                    emit signalsUnitDeviceJson(data);

                data.clear();
                params.clear();
                params["ElectricControlor"] = 1;  //加去电控制
                HPCmd::Control(HPInfo::DEVICEID_SHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }
            else
            {
                QMessageBox::warning(this, "提示", "获取主用S高功放设备ID失败", QString("确定"));
            }

            ctrlCKBBESTX(false, HPTYPE::SCK);
        }
    });
    connect(ui->alcSet_S, &QPushButton::clicked, this, [=]() {
        QByteArray data;
        QVariantMap params;

        int deviceID = HPInfo::getMasterSHP();
        if (deviceID != -1)
        {
            params["ALControl"] = ui->alc_S->currentData();  // ALC控制
            HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
            if (data.size())
                emit signalsUnitDeviceJson(data);
        }
        else
        {
            QMessageBox::warning(this, "提示", "获取主用S高功放设备ID失败", QString("确定"));
        }
    });
    connect(ui->powerSet_S, &QPushButton::clicked, this, [=]() {
        QByteArray data;
        QVariantMap params;

        int deviceID = HPInfo::getMasterSHP();
        if (deviceID != -1)
        {
            params["TransmPower"] = d_power_S->value();  //发射功率
            HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
            if (data.size())
                emit signalsUnitDeviceJson(data);
        }
        else
        {
            QMessageBox::warning(this, "提示", "获取主用S高功放设备ID失败", QString("确定"));
        }
    });

    if (m_currentProject == "4424") {}
    else
    {
        connect(ui->outputSet_KaCK, &QPushButton::clicked, this, [=]() {
            QByteArray data;
            QVariantMap params;

            int index = ui->toACU_KaCK->currentIndex() + 1;

            // A上天线,B去负载
            if (index == 1)
            {
                //功放A切为主机
                params["IsOnline"] = index;
                HPCmd::Control(HPInfo::DEVICEID_KACKHPA, HPInfo::MODEID, 16, params, HPCmd::ProcessControl, data, this);
                if (data.size())
                    emit signalsCmdDeviceJson(data);

                int RFOutput = ui->output_KaCK->currentData().toInt();
                if (RFOutput == 1)
                {
                    //攻放A射频输出允许,功放B射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 1;  //允许
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;  //禁止
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    ctrlCKBBESTX(true, HPTYPE::KaCK);
                }
                else
                {
                    //攻放A射频输出禁止,功放B射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    ctrlCKBBESTX(false, HPTYPE::KaCK);
                }
            }
            else  // B上天线,A去负载
            {
                //功放B切为主机
                params["IsOnline"] = index;
                HPCmd::Control(HPInfo::DEVICEID_KACKHPB, HPInfo::MODEID, 16, params, HPCmd::ProcessControl, data, this);
                if (data.size())
                    emit signalsCmdDeviceJson(data);

                int RFOutput = ui->output_KaCK->currentData().toInt();
                if (RFOutput == 1)
                {
                    //攻放B射频输出允许,功放A射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 1;  //允许
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;  //禁止
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    ctrlCKBBESTX(true, HPTYPE::KaCK);
                }
                else
                {
                    //攻放A射频输出禁止,功放B射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    ctrlCKBBESTX(false, HPTYPE::KaCK);
                }
            }
        });
        connect(ui->alcSet_KaCK, &QPushButton::clicked, this, [=]() {
            QByteArray data;
            QVariantMap params;

            int deviceID = HPInfo::getMasterKaCKHP();
            if (deviceID != -1)
            {
                params["ALControl"] = ui->alc_KaCK->currentData();  // ALC控制
                HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }
            else
            {
                QMessageBox::warning(this, "提示", "获取主用Ka测控功放设备ID失败", QString("确定"));
            }
        });
        connect(ui->powerSet_KaCK, &QPushButton::clicked, this, [=]() {
            QByteArray data;
            QVariantMap params;

            int deviceID = HPInfo::getMasterKaCKHP();
            if (deviceID != -1)
            {
                if (ui->toACU_KaCK->currentData() == HPInfo::DEVICEID_KACKHPA)
                {
                    params["TransmPower"] = d_power_KaCKA->value();  //发射功率
                }
                else
                {
                    params["TransmPower"] = d_power_KaCKB->value();  //发射功率
                }
                HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }
            else
            {
                QMessageBox::warning(this, "提示", "获取主用Ka测控功放设备ID失败", QString("确定"));
            }
        });

        connect(ui->outputSet_KaSC, &QPushButton::clicked, this, [=]() {
            QByteArray data;
            QVariantMap params;

            int index = ui->toACU_KaSC->currentIndex() + 1;

            // A上天线,B去负载
            if (index == 1)
            {
                params["IsOnline"] = index;
                HPCmd::Control(HPInfo::DEVICEID_KASCHPA, HPInfo::MODEID, 16, params, HPCmd::ProcessControl, data, this);
                if (data.size())
                    emit signalsCmdDeviceJson(data);

                int RFOutput = ui->output_KaSC->currentData().toInt();
                if (RFOutput == 1)
                {
                    //攻放A射频输出允许,功放B射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 1;  //允许
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;  //禁止
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);
                }
                else
                {
                    //攻放A射频输出禁止,功放B射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);
                }
            }
            else  // B上天线,A去负载
            {
                //功放B切为主机
                HPCmd::Control(HPInfo::DEVICEID_KASCHPB, HPInfo::MODEID, 16, params, HPCmd::ProcessControl, data, this);
                if (data.size())
                    emit signalsCmdDeviceJson(data);

                int RFOutput = ui->output_KaSC->currentData().toInt();
                if (RFOutput == 1)
                {
                    //攻放B射频输出允许,功放A射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 1;  //允许
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;  //禁止
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);
                }
                else
                {
                    //攻放A射频输出禁止,功放B射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);
                }
            }
        });
        connect(ui->alcSet_KaSC, &QPushButton::clicked, this, [=]() {
            QByteArray data;
            QVariantMap params;

            int deviceID = HPInfo::getMasterKaSCHP();
            if (deviceID != -1)
            {
                params["ALControl"] = ui->alc_KaSC->currentData();  // ALC控制
                HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }
            else
            {
                QMessageBox::warning(this, "提示", "获取主用Ka数传功放设备ID失败", QString("确定"));
            }
        });
        connect(ui->powerSet_KaSC, &QPushButton::clicked, this, [=]() {
            QByteArray data;
            QVariantMap params;

            int deviceID = HPInfo::getMasterKaSCHP();
            if (deviceID != -1)
            {
                if (ui->toACU_KaSC->currentData() == HPInfo::DEVICEID_KASCHPA)
                {
                    params["TransmPower"] = d_power_KaSCA->value();  //发射功率
                }
                else
                {
                    params["TransmPower"] = d_power_KaSCB->value();  //发射功率
                }
                HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data, this);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }
            else
            {
                QMessageBox::warning(this, "提示", "获取主用Ka测控功放设备ID失败", QString("确定"));
            }
        });
    }
}

HP::~HP()
{
    delete ui;
    if (d_power_S != nullptr)
    {
        delete d_power_S;
        d_power_S = nullptr;
    }
    if (d_power_KaCKA != nullptr)
    {
        delete d_power_KaCKA;
        d_power_KaCKA = nullptr;
    }
    if (d_power_KaCKB != nullptr)
    {
        delete d_power_KaCKB;
        d_power_KaCKB = nullptr;
    }
    if (d_power_KaSCA != nullptr)
    {
        delete d_power_KaSCA;
        d_power_KaSCA = nullptr;
    }
    if (d_power_KaSCB != nullptr)
    {
        delete d_power_KaSCB;
        d_power_KaSCB = nullptr;
    }
}

void HP::setType(HP::HPTYPE type)
{
    d_type = type;
    switch (type)
    {
    case SCK:
    {
        ui->widgetSHP->setVisible(true);
        ui->widgetKaCK->setVisible(false);
        ui->widgetKaSC->setVisible(false);
        break;
    }
    case KaCK:
    {
        ui->widgetSHP->setVisible(false);
        ui->widgetKaCK->setVisible(true);
        ui->widgetKaSC->setVisible(false);
        break;
    }
    case KaSC:
    {
        ui->widgetSHP->setVisible(false);
        ui->widgetKaCK->setVisible(false);
        ui->widgetKaSC->setVisible(true);
        break;
    }
    }
}

void HP::resetUIData()
{
    ui->toACU_S->setCurrentIndex(0);
    ui->output_S->setCurrentIndex(1);
    ui->alc_S->setCurrentIndex(1);
    ui->toACU_KaCK->setCurrentIndex(0);
    ui->output_KaCK->setCurrentIndex(1);
    ui->alc_KaCK->setCurrentIndex(1);
    ui->toACU_KaSC->setCurrentIndex(0);
    ui->output_KaSC->setCurrentIndex(1);
    ui->alc_KaSC->setCurrentIndex(1);

    d_power_S->setValue(31);
    d_power_KaCKA->setValue(31);
    d_power_KaCKB->setValue(31);
    d_power_KaSCA->setValue(30);
    d_power_KaSCB->setValue(30);
}

void HP::ctrlCKBBESTX(bool stx, HPTYPE type)
{
    QVariantMap params;
    QByteArray data;
    //射频输出禁止时,同时向基带AB发送功率上天线命令
    data.clear();
    params.clear();

    int value = 0;
    if (stx)
    {
        value = 1;  //发射
    }
    else
    {
        value = 0;  //不发射
    }

    //只有主机才发功率
    if (type == HPTYPE::SCK)  // S功放发S的  因为测控基带双模式第二台固定为Ka，第一台为S，所有不用判断哪个设备ID是什么模式，直接下就行
    {
        DeviceID ckDevIDA(HPInfo::DEVICEID_CKBBEA);
        DeviceID ckDevIDC(HPInfo::DEVICEID_CKBBEC);
        params["CurrPowStatus"] = value;  //当前功率发射

        if (deviceIsMaster(ckDevIDA))
        {
            // 4001
            HPCmd::Control(HPInfo::DEVICEID_CKBBEA, HPInfo::getModeID(HPInfo::DEVICEID_CKBBEA), 115, params, HPCmd::ProcessControl, data, this);
            if (data.size())
            {
                emit signalsCmdDeviceJson(data);
            }
        }
        if (deviceIsMaster(ckDevIDC))
        {
            // 4003
            HPCmd::Control(HPInfo::DEVICEID_CKBBEC, HPInfo::getModeID(HPInfo::DEVICEID_CKBBEC), 115, params, HPCmd::ProcessControl, data, this);
            if (data.size())
            {
                emit signalsCmdDeviceJson(data);
            }
        }
    }
    else if (HPTYPE::KaCK)  // Ka功放发Ka的
    {
        DeviceID ckDevIDB(HPInfo::DEVICEID_CKBBEB);
        DeviceID ckDevIDD(HPInfo::DEVICEID_CKBBED);
        params["CurrPowStatus"] = value;  //当前功率发射
        if (deviceIsMaster(ckDevIDB))
        {
            // 4002
            HPCmd::Control(HPInfo::DEVICEID_CKBBEB, HPInfo::getModeID(HPInfo::DEVICEID_CKBBEB), 115, params, HPCmd::ProcessControl, data, this);
            if (data.size())
            {
                emit signalsCmdDeviceJson(data);
            }
        }
        if (deviceIsMaster(ckDevIDD))
        {
            // 4004
            HPCmd::Control(HPInfo::DEVICEID_CKBBED, HPInfo::getModeID(HPInfo::DEVICEID_CKBBED), 115, params, HPCmd::ProcessControl, data, this);
            if (data.size())
            {
                emit signalsCmdDeviceJson(data);
            }
        }
    }
}

WWidget* HP::createFreqContol(const int deviceId, const int modeId, const int unitId, const QString& paramId)
{
    auto unit = GlobalData::getUnit(deviceId, modeId, unitId);

    for (auto paramAttr : unit.parameterSetList)
    {
        if (paramAttr.id == paramId)
        {
            QMap<QString, QVariant> enumMap;
            return ControlFactory::createSetControl(paramAttr, enumMap);
        }
    }
    return nullptr;
}

bool HP::deviceIsMaster(const DeviceID& deviceID)
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
