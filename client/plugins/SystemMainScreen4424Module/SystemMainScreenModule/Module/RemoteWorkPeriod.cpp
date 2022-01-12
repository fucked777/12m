#include "RemoteWorkPeriod.h"
#include "CustomPacketMessageDefine.h"
#include "DeviceProcessMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "JsonHelper.h"
#include "ProtocolXmlTypeDefine.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "ui_RemoteWorkPeriod.h"
#include <QMessageBox>

RemoteWorkPeriod::RemoteWorkPeriod(QWidget* parent, const int deviceId, const int modeId)
    : QDialog(parent)
    , ui(new Ui::RemoteWorkPeriod)
    , d_deviceId(deviceId)
    , d_modeId(modeId)
{
    ui->setupUi(this);
    init();
}

RemoteWorkPeriod::~RemoteWorkPeriod() { delete ui; }

void RemoteWorkPeriod::btnClicked()
{
    QString objName = sender()->objectName();

    int TaskIdent = objName.right(1).toInt();
    QComboBox* comBox = findChild<QComboBox*>(QString("comboBox_%1").arg(QString::number(TaskIdent)));
    int StartModeWrok = comBox->currentData().toInt();

    UnitParamRequest cmdRequest;
    DeviceID id;
    id << d_deviceId;
    cmdRequest.m_systemNumb = id.sysID;
    cmdRequest.m_deviceNumb = id.devID;
    cmdRequest.m_extenNumb = id.extenID;
    cmdRequest.m_modeID = d_modeId;
    cmdRequest.m_unitID = 5;
    cmdRequest.m_validIdent = 5;

    //判断服务器通断,再判断基带模式是否正确,最后就是分控判断
    bool serverCk = serverCheck();
    if (!serverCk)
        return;
    bool modeCheck = workModeCheck(d_deviceId, d_modeId);
    if (!modeCheck)
        return;
    bool subCtrlCheck = subControlCheck(d_deviceId, 1);
    if (!subCtrlCheck)
        return;
    bool tNumCheck = targetNumCheck(d_deviceId, TaskIdent);
    if (!tNumCheck)
        return;

    QVariantMap params;
    params.insert("K2_StartModeWrok", StartModeWrok);
    params.insert("K2_RemoteMode", 1);
    cmdRequest.m_multiTargetParamMap[TaskIdent].m_paramdataMap = params;

    JsonWriter writer;
    writer& cmdRequest;
    RedisHelper::getInstance().setData("单元发送数据", writer.GetQString());
    qDebug() << writer.GetString();
    emit signalsUnitDeviceJson(writer.GetString());
}

void RemoteWorkPeriod::init()
{
    setWindowTitle("遥控工作期");
    this->setWindowFlags(windowFlags() | Qt::WindowCloseButtonHint);
    ui->comboBox_1->addItem("开始", 2);
    ui->comboBox_1->addItem("结束", 1);
    ui->comboBox_2->addItem("开始", 2);
    ui->comboBox_2->addItem("结束", 1);
    ui->comboBox_3->addItem("开始", 2);
    ui->comboBox_3->addItem("结束", 1);
    ui->comboBox_4->addItem("开始", 2);
    ui->comboBox_4->addItem("结束", 1);
    connect(ui->pushButton_1, &QPushButton::clicked, this, &RemoteWorkPeriod::btnClicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &RemoteWorkPeriod::btnClicked);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &RemoteWorkPeriod::btnClicked);
    connect(ui->pushButton_4, &QPushButton::clicked, this, &RemoteWorkPeriod::btnClicked);
}

bool RemoteWorkPeriod::subControlCheck(const int deviceId, const int unitId, const QString& paramId, int subControlVal)
{
    bool isRemoteCtrl = GlobalData::getDeviceSelfControl(deviceId);

    if (!isRemoteCtrl)
    {
        QMessageBox::warning(nullptr, QString("提示"), QString("当前设备为分控状态,命令下发失败"), QString("确定"));
        return false;
    }
    else
    {
        return true;
    }
}

bool RemoteWorkPeriod::workModeCheck(int srcDeviceId, const int curWorkMode)
{
    DeviceID id;
    id << srcDeviceId;
    int modeId = GlobalData::getOnlineDeviceModeId(id);

    if (curWorkMode == modeId)
    {
        return true;
    }
    else
    {
        QMessageBox::warning(nullptr, QString("提示"), QString("设备当前系统模式与所选模式不一致,命令下发失败"), QString("确定"));
        return false;
    }
}

bool RemoteWorkPeriod::serverCheck()
{
    if (!SERVICEONLINE)
    {
        QMessageBox::information(nullptr, QString("提示"), QString("当前监控服务器离线"), QString("确定"));
        return false;
    }
    else
    {
        return true;
    }
}

bool RemoteWorkPeriod::targetNumCheck(const int deviceId, const int curTargetNum)
{
    //  只有当前目标数小于公共单元里面的目标数才可以下发命令
    auto statusReportMsg = GlobalData::getExtenStatusReportData(deviceId);
    int targetNum = statusReportMsg.unitReportMsgMap[1].paramMap["TargetNum"].toInt();
    if (curTargetNum > targetNum)
    {
        QMessageBox::warning(this, QString("提示"), QString("所控参数目标数小于当前目标数,命令下发失败"), QString("确定"));
        return false;
    }
    else
    {
        return true;
    }
}
