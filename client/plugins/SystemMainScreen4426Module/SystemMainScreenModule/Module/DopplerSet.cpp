#include "DopplerSet.h"
#include "CustomPacketMessageDefine.h"
#include "DeviceProcessMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "JsonHelper.h"
#include "MessagePublish.h"
#include "ProtocolXmlTypeDefine.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "ui_DopplerSet.h"
#include <QMessageBox>

DopplerSet::DopplerSet(QWidget* parent, const int deviceId, const int modeId)
    : QDialog(parent)
    , ui(new Ui::DopplerSet)
    , d_deviceId(deviceId)
    , d_modeId(modeId)
{
    ui->setupUi(this);
    init();
}

DopplerSet::~DopplerSet() { delete ui; }

void DopplerSet::btnSetClicked()
{
    CmdRequest cmdRequest;
    DeviceID id;
    id << d_deviceId;
    cmdRequest.m_systemNumb = id.sysID;
    cmdRequest.m_deviceNumb = id.devID;
    cmdRequest.m_extenNumb = id.extenID;
    cmdRequest.m_modeID = d_modeId;
    cmdRequest.m_cmdID = 32;

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

    QVariantMap params;
    params.insert("ChanNumb", ui->SExtendBand_ChannelNum->currentData());
    params.insert("ReceUnit", ui->SExtendBand_ReceiveUnit->currentData());
    params.insert("PresFreq", ui->SExtendBand_SetFreq->value());
    params.insert("CaptRange", ui->SExtendBand_CatchRange->value());
    cmdRequest.m_paramdataMap = params;

    JsonWriter writer;
    writer& cmdRequest;
    RedisHelper::getInstance().setData("过程控制命令发送数据", writer.GetQString());
    qDebug() << writer.GetString();
    emit signalsCmdDeviceJson(writer.GetString());
    this->close();
}

void DopplerSet::init()
{
    ui->SExtendBand_ChannelNum->addItem("通道1", 1);
    ui->SExtendBand_ChannelNum->addItem("通道2", 2);
    ui->SExtendBand_ChannelNum->addItem("通道3", 3);
    ui->SExtendBand_ChannelNum->addItem("通道4", 4);

    ui->SExtendBand_ReceiveUnit->addItem("TTC主接收机", 1);
    ui->SExtendBand_ReceiveUnit->addItem("TTC引导接收机", 2);
    // ui->SExtendBand_ReceiveUnit->addItem("保留", 3);
    ui->SExtendBand_ReceiveUnit->addItem("全部接收机", 4);

    this->setWindowFlags(windowFlags() | Qt::WindowCloseButtonHint);
    setWindowTitle("多普勒预置");

    connect(ui->pushButton_set, &QPushButton::clicked, this, &DopplerSet::btnSetClicked);
}

bool DopplerSet::subControlCheck(const int deviceId, const int unitId, const QString& paramId, int subControlVal)
{
    auto statusReportMsg = GlobalData::getExtenStatusReportData(deviceId);

    QVariant paramValue;
    paramValue = statusReportMsg.unitReportMsgMap[unitId].paramMap[paramId];

    if (!paramValue.isValid() || paramValue.toInt() == subControlVal)
    {
        QMessageBox::warning(nullptr, QString("提示"), QString("当前设备为分控状态,命令下发失败"), QString("确定"));
        return false;
    }
    else
    {
        return true;
    }
}

bool DopplerSet::workModeCheck(int srcDeviceId, const int curWorkMode)
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

bool DopplerSet::serverCheck()
{
    if (!SERVICEONLINE)
    {
        QString errorMsg;
        errorMsg = QString("当前监控服务器离线");
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::information(nullptr, QString("提示"), errorMsg, QString("确定"));
        return false;
    }
    else
    {
        return true;
    }
}
