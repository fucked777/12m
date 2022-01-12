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
        if (GlobalData::getDeviceOnline(DEVICEID_SHQHCX))
        {
            ExtensionStatusReportMessage reportMessage = GlobalData::getExtenStatusReportData(DEVICEID_SHQHCX);
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
        if (GlobalData::getDeviceOnline(DEVICEID_KACKHPA))
        {
            ExtensionStatusReportMessage reportMessage = GlobalData::getExtenStatusReportData(DEVICEID_KACKHPA);
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
        if (GlobalData::getDeviceOnline(DEVICEID_KASCHPA))
        {
            ExtensionStatusReportMessage reportMessage = GlobalData::getExtenStatusReportData(DEVICEID_KASCHPA);
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
        if (GlobalData::getDeviceOnline(deviceID))
        {
            ExtensionStatusReportMessage reportMessage = GlobalData::getExtenStatusReportData(deviceID);
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

    connect(ui->outputSet_S, &QPushButton::clicked, this, [=]() {
        int inspire = ui->output_S->currentData().toInt();
        QByteArray data;
        QVariantMap params;

        if (inspire == 1)  //加激励
        {
            //功放开关设置:对应的功放;功放切换方式:手动;RF开关:开
            params["SwitcherSet"] = ui->toACU_S->currentIndex() + 1;  //功放开关设置
            params["SwitchMode"] = 2;                                 //功放切换方式
            params["RFSwitch"] = 1;                                   // RF开关
            HPCmd::Control(HPInfo::DEVICEID_SHQHCX, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
            if (data.size())
                emit signalsUnitDeviceJson(data);

            if (ui->toACU_S->currentData().toInt() == HPInfo::DEVICEID_SHPA)
            {
                //功放A加电,功放B去电
                data.clear();
                params.clear();
                params["ElectricControlor"] = 2;  //加去电控制
                HPCmd::Control(HPInfo::DEVICEID_SHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                if (data.size())
                    emit signalsUnitDeviceJson(data);

                data.clear();
                params.clear();
                params["ElectricControlor"] = 1;  //加去电控制
                HPCmd::Control(HPInfo::DEVICEID_SHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }
            else
            {
                //功放B加电,功放A去电
                data.clear();
                params.clear();
                params["ElectricControlor"] = 2;  //加去电控制
                HPCmd::Control(HPInfo::DEVICEID_SHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                if (data.size())
                    emit signalsUnitDeviceJson(data);

                data.clear();
                params.clear();
                params["ElectricControlor"] = 1;  //加去电控制
                HPCmd::Control(HPInfo::DEVICEID_SHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }

            ctrlCKBBESTX(true);
        }
        if (inspire == 2)  //去激励
        {
            int deviceID = HPInfo::getMasterSHP();
            if (deviceID != -1)
            {
                //功放开关设置:对应的功放;功放切换方式:手动;RF开关:关
                params["SwitcherSet"] = ui->toACU_S->currentIndex() + 1;  //功放开关设置
                params["SwitchMode"] = 2;                                 //功放切换方式
                params["RFSwitch"] = 2;                                   // RF开关
                HPCmd::Control(HPInfo::DEVICEID_SHQHCX, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                if (data.size())
                    emit signalsUnitDeviceJson(data);

                if (ui->toACU_S->currentData().toInt() == HPInfo::DEVICEID_SHPA)
                {
                    //功放A加电,功放B去电
                    data.clear();
                    params.clear();
                    params["ElectricControlor"] = 2;  //加去电控制
                    HPCmd::Control(HPInfo::DEVICEID_SHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["ElectricControlor"] = 1;  //加去电控制
                    HPCmd::Control(HPInfo::DEVICEID_SHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);
                }
                else
                {
                    //功放B加电,功放A去电
                    data.clear();
                    params.clear();
                    params["ElectricControlor"] = 2;  //加去电控制
                    HPCmd::Control(HPInfo::DEVICEID_SHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["ElectricControlor"] = 1;  //加去电控制
                    HPCmd::Control(HPInfo::DEVICEID_SHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);
                }
            }
            else
            {
                QMessageBox::warning(this, "提示", "获取主用S高功放设备ID失败", QString("确定"));
            }

            ctrlCKBBESTX(false);
        }
    });
    connect(ui->alcSet_S, &QPushButton::clicked, this, [=]() {
        QByteArray data;
        QVariantMap params;

        int deviceID = HPInfo::getMasterSHP();
        if (deviceID != -1)
        {
            params["ALControl"] = ui->alc_S->currentData();  // ALC控制
            HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
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
            HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
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
                HPCmd::Control(HPInfo::DEVICEID_KACKHPA, HPInfo::MODEID, 16, params, HPCmd::ProcessControl, data);
                if (data.size())
                    emit signalsCmdDeviceJson(data);

                int RFOutput = ui->output_KaCK->currentData().toInt();
                if (RFOutput == 1)
                {
                    //攻放A射频输出允许,功放B射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 1;  //允许
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;  //禁止
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    ctrlCKBBESTX(true);
                }
                else
                {
                    //攻放A射频输出禁止,功放B射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = RFOutput;
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = RFOutput;
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    ctrlCKBBESTX(false);
                }
            }
            else  // B上天线,A去负载
            {
                //功放B切为主机
                params["IsOnline"] = index;
                HPCmd::Control(HPInfo::DEVICEID_KACKHPB, HPInfo::MODEID, 16, params, HPCmd::ProcessControl, data);
                if (data.size())
                    emit signalsCmdDeviceJson(data);

                int RFOutput = ui->output_KaCK->currentData().toInt();
                if (RFOutput == 1)
                {
                    //攻放B射频输出允许,功放A射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 1;  //允许
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;  //禁止
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    ctrlCKBBESTX(true);
                }
                else
                {
                    //攻放A射频输出禁止,功放B射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = RFOutput;
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = RFOutput;
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    ctrlCKBBESTX(false);
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
                HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
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
                HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
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
            params["IsOnline"] = index;
            if (index == 1)
            {
                HPCmd::Control(HPInfo::DEVICEID_KASCHPA, HPInfo::MODEID, 16, params, HPCmd::ProcessControl, data);
                if (data.size())
                    emit signalsCmdDeviceJson(data);

                int RFOutput = ui->output_KaSC->currentData().toInt();
                if (RFOutput == 1)
                {
                    //攻放A射频输出允许,功放B射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 1;  //允许
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;  //禁止
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);
                }
                else
                {
                    //攻放A射频输出禁止,功放B射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = RFOutput;
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = RFOutput;
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);
                }
            }
            else  // B上天线,A去负载
            {
                //功放B切为主机
                HPCmd::Control(HPInfo::DEVICEID_KASCHPB, HPInfo::MODEID, 16, params, HPCmd::ProcessControl, data);
                if (data.size())
                    emit signalsCmdDeviceJson(data);

                int RFOutput = ui->output_KaSC->currentData().toInt();
                if (RFOutput == 1)
                {
                    //攻放B射频输出允许,功放A射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 1;  //允许
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;  //禁止
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);
                }
                else
                {
                    //攻放A射频输出禁止,功放B射频输出禁止
                    data.clear();
                    params.clear();
                    params["RFOutput"] = RFOutput;
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = RFOutput;
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
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
                HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
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
                HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
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

void HP::ctrlCKBBESTX(bool stx)
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

    params["CurrPowStatus"] = value;  //当前功率发射
    HPCmd::Control(HPInfo::DEVICEID_CKBBEA, HPInfo::getModeID(HPInfo::DEVICEID_CKBBEA), 115, params, HPCmd::ProcessControl, data);
    if (data.size())
    {
        emit signalsCmdDeviceJson(data);
    }

    data.clear();
    params.clear();
    params["CurrPowStatus"] = value;  //当前功率发射
    if (m_currentProject == "4424")
    {
        // 4424没有双模式，因此直接给B机发功率就行了
        HPCmd::Control(HPInfo::DEVICEID_CKBBEB, HPInfo::getModeID(HPInfo::DEVICEID_CKBBEB), 115, params, HPCmd::ProcessControl, data);
    }
    else
    {
        HPCmd::Control(HPInfo::DEVICEID_CKBBEC, HPInfo::getModeID(HPInfo::DEVICEID_CKBBEC), 115, params, HPCmd::ProcessControl, data);
        DeviceID deviceIDA2(0x4002);
        DeviceID deviceIDB2(0x4004);
        //判断双模式的情况，如果在线就一起发功率
        //如果是双模式，那么deviceIDA2就是A机的第二个软件    deviceIDB2就是B机的第二个软件都得发功率
        if (GlobalData::getDeviceOnline(deviceIDA2))
        {
            HPCmd::Control(HPInfo::DEVICEID_CKBBEB, HPInfo::getModeID(HPInfo::DEVICEID_CKBBEB), 115, params, HPCmd::ProcessControl, data);
        }

        if (GlobalData::getDeviceOnline(deviceIDB2))
        {
            HPCmd::Control(HPInfo::DEVICEID_CKBBED, HPInfo::getModeID(HPInfo::DEVICEID_CKBBED), 115, params, HPCmd::ProcessControl, data);
        }
    }

    if (data.size())
    {
        emit signalsCmdDeviceJson(data);
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
