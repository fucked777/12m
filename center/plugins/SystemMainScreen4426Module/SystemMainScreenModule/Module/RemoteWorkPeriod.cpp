#include "RemoteWorkPeriod.h"
#include "CustomPacketMessageDefine.h"
#include "DeviceProcessMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "JsonHelper.h"
#include "MessagePublish.h"
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

    setWindowIcon(QIcon(":/image/task_process_ctrl.png"));
    setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
}

RemoteWorkPeriod::~RemoteWorkPeriod() { delete ui; }

void RemoteWorkPeriod::btnClicked()
{
    // 服务器在线状态检测
    SERVICEONLINECHECKNOPARENT();

    QString objName = sender()->objectName();

    int TaskIdent = objName.right(1).toInt();
    QComboBox* comBox = findChild<QComboBox*>(QString("comboBox_%1").arg(QString::number(TaskIdent)));
    int StartModeWrok = comBox->currentData().toInt();

    // 设备在线检测
    if (!deviceIsOnline())
    {
        return;
    }
    // 设备模式检测
    if (!deviceIsMode())
    {
        return;
    }
    // 设备本分控检测
    if (!deviceIsSelfControl())
    {
        return;
    }
    // 判断是否是主机
    auto isMaster = deviceIsMaster();
    if (!isMaster)
    {
        auto deviceName = GlobalData::getExtensionName(d_deviceId);
        QString errorMsg;
        errorMsg = QString("%1设备为备机，是否继续进行下发命令").arg(deviceName);
        SystemLogPublish::errorMsg(errorMsg);
        if (QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("是"), QString("否")) != 0)
        {
            return;
        }
    }
    //目标判断，判断当前下发的参数是哪个目标里的，是否小于基带当前目标数
    bool tNumCheck = targetNumCheck(d_deviceId, TaskIdent);
    if (!tNumCheck)
        return;
    auto statusReportMsg = GlobalData::getExtenStatusReportDataByObject(d_deviceId);
    int targetNum = statusReportMsg.unitReportMsgMap[1].paramMap["TargetNum"].toInt();

    UnitParamRequest cmdRequest;
    DeviceID id;
    id << d_deviceId;
    cmdRequest.m_systemNumb = id.sysID;
    cmdRequest.m_deviceNumb = id.devID;
    cmdRequest.m_extenNumb = id.extenID;
    cmdRequest.m_modeID = d_modeId;
    cmdRequest.m_unitID = 5;
    cmdRequest.m_validIdent = targetNum + 1;  //扩频有效标识为当前目标数+1

    QVariantMap params;
    params.insert("K2_StartModeWrok", StartModeWrok);
    params.insert("K2_RemoteMode", 1);  //遥控工作期必须在常发的时候才行，所以也需要下发
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

bool RemoteWorkPeriod::targetNumCheck(const int deviceId, const int curTargetNum)
{
    //  只有当前目标数小于公共单元里面的目标数才可以下发命令
    auto statusReportMsg = GlobalData::getExtenStatusReportDataByObject(deviceId);
    int targetNum = statusReportMsg.unitReportMsgMap[1].paramMap["TargetNum"].toInt();
    if (curTargetNum > targetNum)
    {
        QString errorMsg;
        errorMsg = QString("所控参数目标数小于当前目标数,命令下发失败");
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::warning(this, QString("提示"), errorMsg, QString("确定"));
        return false;
    }
    else
    {
        return true;
    }
}

bool RemoteWorkPeriod::deviceIsOnline()
{
    if (!GlobalData::getDeviceOnlineByObject(d_deviceId))
    {
        auto deviceName = GlobalData::getExtensionName(d_deviceId);
        QString errorMsg;
        errorMsg = QString("%1设备离线，命令下发失败").arg(deviceName);
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
        return false;
    }
    return true;
}

bool RemoteWorkPeriod::deviceIsMode()
{
    QString errorMsg;
    if (!SystemWorkModeHelper::modeIdIsValid(d_modeId))
    {
        errorMsg = QString("当前下发命令的模式无效，命令下发失败");
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
    }

    int curModeId = GlobalData::getOnlineDeviceModeId(d_deviceId);
    if (d_modeId != curModeId)
    {
        auto workMode = SystemWorkModeHelper::modeIDToSystemWorkMode(curModeId);
        auto deviceName = GlobalData::getExtensionName(d_deviceId);
        auto curModeName = SystemWorkModeHelper::systemWorkModeToDesc(workMode);

        errorMsg = QString("%1设备当前模式为%2，与需要下发的命令模式不符，请重新选择模式").arg(deviceName).arg(curModeName);
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
        return false;
    }
    return true;
}

bool RemoteWorkPeriod::deviceIsSelfControl()
{
    if (!GlobalData::getDeviceSelfControl(d_deviceId))
    {
        auto deviceName = GlobalData::getExtensionName(d_deviceId);
        QString errorMsg;
        errorMsg = QString("%1设备为分控状态，命令下发失败").arg(deviceName);
        SystemLogPublish::warningMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
        return false;
    }

    return true;
}

bool RemoteWorkPeriod::deviceIsMaster()
{
    auto value = GlobalData::getReportParamData(d_deviceId, 1, "OnLineMachine");
    // 测控基带  无效代表该设备没有主备机概念，或者该参数id错误，默认不存在参数错误的情况，所以无效就代表是主机
    if (!value.isValid())
    {
        return true;
    }

    // 测控基带1为主机  2为备机
    return value.toInt() != 1 ? false : true;
}
