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
    ui->toACU_S->addItem("A?????????B?????????", HPInfo::DEVICEID_SHPA);
    ui->toACU_S->addItem("B?????????A?????????", HPInfo::DEVICEID_SHPB);
    ui->output_S->addItem("?????????", 1);
    ui->output_S->addItem("?????????", 2);
    ui->alc_S->addItem("??????", 1);
    ui->alc_S->addItem("??????", 2);
    d_power_S = createFreqContol(HPInfo::DEVICEID_SHPA, HPInfo::MODEID, 1, "TransmPower");
    ui->horizontalLayout_S->addWidget(d_power_S);

    if (m_currentProject == "4424") {}
    else
    {
        ui->toACU_KaCK->addItem("A?????????B?????????", HPInfo::DEVICEID_KACKHPA);
        ui->toACU_KaCK->addItem("B?????????A?????????", HPInfo::DEVICEID_KACKHPB);
        ui->output_KaCK->addItem("?????????", 1);
        ui->output_KaCK->addItem("?????????", 2);
        ui->alc_KaCK->addItem("??????", 1);
        ui->alc_KaCK->addItem("??????", 2);

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

        ui->toACU_KaSC->addItem("A?????????B?????????", HPInfo::DEVICEID_KASCHPA);
        ui->toACU_KaSC->addItem("B?????????A?????????", HPInfo::DEVICEID_KASCHPB);
        ui->output_KaSC->addItem("?????????", 1);
        ui->output_KaSC->addItem("?????????", 2);
        ui->alc_KaSC->addItem("??????", 1);
        ui->alc_KaSC->addItem("??????", 2);
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

        if (inspire == 1)  //?????????
        {
            //??????????????????:???????????????;??????????????????:??????;RF??????:???
            params["SwitcherSet"] = ui->toACU_S->currentIndex() + 1;  //??????????????????
            params["SwitchMode"] = 2;                                 //??????????????????
            params["RFSwitch"] = 1;                                   // RF??????
            HPCmd::Control(HPInfo::DEVICEID_SHQHCX, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
            if (data.size())
                emit signalsUnitDeviceJson(data);

            if (ui->toACU_S->currentData().toInt() == HPInfo::DEVICEID_SHPA)
            {
                //??????A??????,??????B??????
                data.clear();
                params.clear();
                params["ElectricControlor"] = 2;  //???????????????
                HPCmd::Control(HPInfo::DEVICEID_SHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                if (data.size())
                    emit signalsUnitDeviceJson(data);

                data.clear();
                params.clear();
                params["ElectricControlor"] = 1;  //???????????????
                HPCmd::Control(HPInfo::DEVICEID_SHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }
            else
            {
                //??????B??????,??????A??????
                data.clear();
                params.clear();
                params["ElectricControlor"] = 2;  //???????????????
                HPCmd::Control(HPInfo::DEVICEID_SHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                if (data.size())
                    emit signalsUnitDeviceJson(data);

                data.clear();
                params.clear();
                params["ElectricControlor"] = 1;  //???????????????
                HPCmd::Control(HPInfo::DEVICEID_SHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }

            ctrlCKBBESTX(true);
        }
        if (inspire == 2)  //?????????
        {
            int deviceID = HPInfo::getMasterSHP();
            if (deviceID != -1)
            {
                //??????????????????:???????????????;??????????????????:??????;RF??????:???
                params["SwitcherSet"] = ui->toACU_S->currentIndex() + 1;  //??????????????????
                params["SwitchMode"] = 2;                                 //??????????????????
                params["RFSwitch"] = 2;                                   // RF??????
                HPCmd::Control(HPInfo::DEVICEID_SHQHCX, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                if (data.size())
                    emit signalsUnitDeviceJson(data);

                if (ui->toACU_S->currentData().toInt() == HPInfo::DEVICEID_SHPA)
                {
                    //??????A??????,??????B??????
                    data.clear();
                    params.clear();
                    params["ElectricControlor"] = 2;  //???????????????
                    HPCmd::Control(HPInfo::DEVICEID_SHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["ElectricControlor"] = 1;  //???????????????
                    HPCmd::Control(HPInfo::DEVICEID_SHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);
                }
                else
                {
                    //??????B??????,??????A??????
                    data.clear();
                    params.clear();
                    params["ElectricControlor"] = 2;  //???????????????
                    HPCmd::Control(HPInfo::DEVICEID_SHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["ElectricControlor"] = 1;  //???????????????
                    HPCmd::Control(HPInfo::DEVICEID_SHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);
                }
            }
            else
            {
                QMessageBox::warning(this, "??????", "????????????S???????????????ID??????", QString("??????"));
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
            params["ALControl"] = ui->alc_S->currentData();  // ALC??????
            HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
            if (data.size())
                emit signalsUnitDeviceJson(data);
        }
        else
        {
            QMessageBox::warning(this, "??????", "????????????S???????????????ID??????", QString("??????"));
        }
    });
    connect(ui->powerSet_S, &QPushButton::clicked, this, [=]() {
        QByteArray data;
        QVariantMap params;

        int deviceID = HPInfo::getMasterSHP();
        if (deviceID != -1)
        {
            params["TransmPower"] = d_power_S->value();  //????????????
            HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
            if (data.size())
                emit signalsUnitDeviceJson(data);
        }
        else
        {
            QMessageBox::warning(this, "??????", "????????????S???????????????ID??????", QString("??????"));
        }
    });

    if (m_currentProject == "4424") {}
    else
    {
        connect(ui->outputSet_KaCK, &QPushButton::clicked, this, [=]() {
            QByteArray data;
            QVariantMap params;

            int index = ui->toACU_KaCK->currentIndex() + 1;

            // A?????????,B?????????
            if (index == 1)
            {
                //??????A????????????
                params["IsOnline"] = index;
                HPCmd::Control(HPInfo::DEVICEID_KACKHPA, HPInfo::MODEID, 16, params, HPCmd::ProcessControl, data);
                if (data.size())
                    emit signalsCmdDeviceJson(data);

                int RFOutput = ui->output_KaCK->currentData().toInt();
                if (RFOutput == 1)
                {
                    //??????A??????????????????,??????B??????????????????
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 1;  //??????
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;  //??????
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    ctrlCKBBESTX(true);
                }
                else
                {
                    //??????A??????????????????,??????B??????????????????
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
            else  // B?????????,A?????????
            {
                //??????B????????????
                params["IsOnline"] = index;
                HPCmd::Control(HPInfo::DEVICEID_KACKHPB, HPInfo::MODEID, 16, params, HPCmd::ProcessControl, data);
                if (data.size())
                    emit signalsCmdDeviceJson(data);

                int RFOutput = ui->output_KaCK->currentData().toInt();
                if (RFOutput == 1)
                {
                    //??????B??????????????????,??????A??????????????????
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 1;  //??????
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;  //??????
                    HPCmd::Control(HPInfo::DEVICEID_KACKHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    ctrlCKBBESTX(true);
                }
                else
                {
                    //??????A??????????????????,??????B??????????????????
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
                params["ALControl"] = ui->alc_KaCK->currentData();  // ALC??????
                HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }
            else
            {
                QMessageBox::warning(this, "??????", "????????????Ka??????????????????ID??????", QString("??????"));
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
                    params["TransmPower"] = d_power_KaCKA->value();  //????????????
                }
                else
                {
                    params["TransmPower"] = d_power_KaCKB->value();  //????????????
                }
                HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }
            else
            {
                QMessageBox::warning(this, "??????", "????????????Ka??????????????????ID??????", QString("??????"));
            }
        });

        connect(ui->outputSet_KaSC, &QPushButton::clicked, this, [=]() {
            QByteArray data;
            QVariantMap params;

            int index = ui->toACU_KaSC->currentIndex() + 1;

            // A?????????,B?????????
            params["IsOnline"] = index;
            if (index == 1)
            {
                HPCmd::Control(HPInfo::DEVICEID_KASCHPA, HPInfo::MODEID, 16, params, HPCmd::ProcessControl, data);
                if (data.size())
                    emit signalsCmdDeviceJson(data);

                int RFOutput = ui->output_KaSC->currentData().toInt();
                if (RFOutput == 1)
                {
                    //??????A??????????????????,??????B??????????????????
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 1;  //??????
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;  //??????
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);
                }
                else
                {
                    //??????A??????????????????,??????B??????????????????
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
            else  // B?????????,A?????????
            {
                //??????B????????????
                HPCmd::Control(HPInfo::DEVICEID_KASCHPB, HPInfo::MODEID, 16, params, HPCmd::ProcessControl, data);
                if (data.size())
                    emit signalsCmdDeviceJson(data);

                int RFOutput = ui->output_KaSC->currentData().toInt();
                if (RFOutput == 1)
                {
                    //??????B??????????????????,??????A??????????????????
                    data.clear();
                    params.clear();
                    params["RFOutput"] = 1;  //??????
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPB, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);

                    data.clear();
                    params.clear();
                    params["RFOutput"] = 2;  //??????
                    HPCmd::Control(HPInfo::DEVICEID_KASCHPA, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                    if (data.size())
                        emit signalsUnitDeviceJson(data);
                }
                else
                {
                    //??????A??????????????????,??????B??????????????????
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
                params["ALControl"] = ui->alc_KaSC->currentData();  // ALC??????
                HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }
            else
            {
                QMessageBox::warning(this, "??????", "????????????Ka??????????????????ID??????", QString("??????"));
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
                    params["TransmPower"] = d_power_KaSCA->value();  //????????????
                }
                else
                {
                    params["TransmPower"] = d_power_KaSCB->value();  //????????????
                }
                HPCmd::Control(deviceID, HPInfo::MODEID, 0x1, params, HPCmd::UnitParameter, data);
                if (data.size())
                    emit signalsUnitDeviceJson(data);
            }
            else
            {
                QMessageBox::warning(this, "??????", "????????????Ka??????????????????ID??????", QString("??????"));
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
    //?????????????????????,???????????????AB???????????????????????????
    data.clear();
    params.clear();

    int value = 0;
    if (stx)
    {
        value = 1;  //??????
    }
    else
    {
        value = 0;  //?????????
    }

    params["CurrPowStatus"] = value;  //??????????????????
    HPCmd::Control(HPInfo::DEVICEID_CKBBEA, HPInfo::getModeID(HPInfo::DEVICEID_CKBBEA), 115, params, HPCmd::ProcessControl, data);
    if (data.size())
    {
        emit signalsCmdDeviceJson(data);
    }

    data.clear();
    params.clear();
    params["CurrPowStatus"] = value;  //??????????????????
    if (m_currentProject == "4424")
    {
        // 4424?????????????????????????????????B?????????????????????
        HPCmd::Control(HPInfo::DEVICEID_CKBBEB, HPInfo::getModeID(HPInfo::DEVICEID_CKBBEB), 115, params, HPCmd::ProcessControl, data);
    }
    else
    {
        HPCmd::Control(HPInfo::DEVICEID_CKBBEC, HPInfo::getModeID(HPInfo::DEVICEID_CKBBEC), 115, params, HPCmd::ProcessControl, data);
        DeviceID deviceIDA2(0x4002);
        DeviceID deviceIDB2(0x4004);
        //?????????????????????????????????????????????????????????
        //???????????????????????????deviceIDA2??????A?????????????????????    deviceIDB2??????B????????????????????????????????????
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
