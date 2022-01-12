#include "CCZFTaskPlanWidget.h"
#include "CustomPacketMessageSerialize.h"
#include "DataTransmissionCenterSerialize.h"
#include "DeviceProcessMessageSerialize.h"
#include "MessagePublish.h"
#include "ProtocolXmlTypeDefine.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "XComboBox.h"
#include "ui_CCZFTaskPlanWidget.h"
#include <QMessageBox>

const int cczfDeviceIDA = 0x8101;
const int cczfDeviceIDB = 0x8102;

CCZFTaskPlanWidget::CCZFTaskPlanWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CCZFTaskPlanWidget)
{
    ui->setupUi(this);

    initUI();
    initSlot();
}

CCZFTaskPlanWidget::~CCZFTaskPlanWidget() { delete ui; }

void CCZFTaskPlanWidget::resetUIData()
{
    ui->jdsbNameCombx->setCurrentIndex(0);
    ui->jdtdhCombx->setCurrentIndex(0);
    ui->modeBZCombx->setCurrentIndex(0);
    ui->iqOutWayCombx->setCurrentIndex(0);
    ui->taskCodeCombx->setCurrentIndex(0);
    ui->yxjCombx->setCurrentIndex(0);
    ui->cscProtocolCombx->setCurrentIndex(0);
    ui->localNumCombx->setCurrentIndex(0);
    ui->dataProtocolCombx->setCurrentIndex(0);
    ui->cczfSBChooseCombx->setCurrentIndex(0);

    //信息类别默认是1
    ui->informationEdit->setText("1");
    // I通道号初始值是312 Q通道号是311
    ui->iChannelEdit->setText("312");
    ui->qChannelEdit->setText("311");
    //传输速率 初始值为2000000
    ui->sendSDEdit->setText("2000000");
    //任务圈次默认10
    ui->taskqcEdit->setText("10");
    //跟踪接收计划号默认1 圈次内编号也是1
    ui->trackjhEdit->setText("1");
    ui->qcnNumEdit->setText("1");
    //控制标志默认为启动
    ui->controlBZCombx->setCurrentIndex(1);
}

void CCZFTaskPlanWidget::showEvent(QShowEvent* event) { refreUIData(); }

void CCZFTaskPlanWidget::initUI()
{
    /***********************************存储单元***********************************/
    //存储转发设备选择
    ui->cczfSBChooseCombx->addItem("数据存储转发设备A", cczfDeviceIDA);
    ui->cczfSBChooseCombx->addItem("数据存储转发设备B", cczfDeviceIDB);
    //基带设备名称
    ui->jdsbNameCombx->addItem("高速基带A", "HDD101");
    ui->jdsbNameCombx->addItem("高速基带B", "HDD102");
    ui->jdsbNameCombx->addItem("低速基带A", "LDD101");
    ui->jdsbNameCombx->addItem("低速基带B", "LDD102");

    //基带通道号
    ui->jdtdhCombx->addItem("通道1", 1);
    ui->jdtdhCombx->addItem("通道2", 2);

    //模式标识
    ui->modeBZCombx->addItem("TS", "TS");
    ui->modeBZCombx->addItem("OP", "OP");

    // IQ输出方式
    ui->iqOutWayCombx->addItem("合路", 1);
    ui->iqOutWayCombx->addItem("分路", 2);

    /***********************************数据传输计划命令***********************************/
    //控制标志
    ui->controlBZCombx->addItem("停止", 1);
    ui->controlBZCombx->addItem("启动", 2);

    //优先级
    ui->yxjCombx->addItem("1", 1);
    ui->yxjCombx->addItem("2", 2);
    ui->yxjCombx->addItem("3", 3);

    //传输层协议
    ui->cscProtocolCombx->addItem("TCP", 1);
    ui->cscProtocolCombx->addItem("UDP点播", 2);
    ui->cscProtocolCombx->addItem("UDP组播", 3);

    //时间
    auto currentStartTime = GlobalData::currentDateTime().addSecs(20);
    auto currentEndTime = GlobalData::currentDateTime().addSecs(48 * 60 * 60);
    auto currentSHTime = GlobalData::currentDateTime().addSecs(20);

    ui->taskStartDate->setDate(currentStartTime.date());
    ui->taskStartTime->setTime(currentStartTime.time());
    ui->taskEndDate->setDate(currentEndTime.date());
    ui->taskEndTime->setTime(currentEndTime.time());
    ui->shSendDate->setDate(currentSHTime.date());
    ui->shSendTime->setTime(currentSHTime.time());

    //本机通道号
    ui->localNumCombx->addItem("通道1", 1);
    ui->localNumCombx->addItem("通道2", 2);
    ui->localNumCombx->addItem("通道3", 3);
    ui->localNumCombx->addItem("通道4", 4);

    //数据协议
    ui->dataProtocolCombx->addItem("PDXP", 1);
    ui->dataProtocolCombx->addItem("FEP", 2);
    ui->dataProtocolCombx->addItem("HDR", 3);

    ui->objectBZEdit->setEnabled(false);

    m_target_center = new XComboBox();

    ui->horizontalLayout_3->addWidget(m_target_center);

    //信息类别默认是1
    ui->informationEdit->setText("1");
    // I通道号初始值是312 Q通道号是311
    ui->iChannelEdit->setText("312");
    ui->qChannelEdit->setText("311");
    //传输速率 初始值为2000000
    ui->sendSDEdit->setText("2000000");
    //任务圈次默认10
    ui->taskqcEdit->setText("10");
    //跟踪接收计划号默认1 圈次内编号也是1
    ui->trackjhEdit->setText("1");
    ui->qcnNumEdit->setText("1");
    //控制标志默认为启动
    ui->controlBZCombx->setCurrentIndex(1);

    //    ui->jdtdhCombx->setEnabled(false);
    //    ui->localNumCombx->setEnabled(false);

    //这里是把要用到的参数名称背景颜色全部置为绿色
    setSettingPanelColor(ui->label_4, QColor(0, 170, 0));
    setSettingPanelColor(ui->label_16, QColor(0, 170, 0));
    setSettingPanelColor(ui->label_18, QColor(0, 170, 0));
    setSettingPanelColor(ui->label_29, QColor(0, 170, 0));
    setSettingPanelColor(ui->label_21, QColor(0, 170, 0));
    setSettingPanelColor(ui->label_24, QColor(0, 170, 0));
    setSettingPanelColor(ui->label_32, QColor(0, 170, 0));
    setSettingPanelColor(ui->label_33, QColor(0, 170, 0));
    setSettingPanelColor(ui->label_25, QColor(0, 170, 0));
    setSettingPanelColor(ui->label_26, QColor(0, 170, 0));
    setSettingPanelColor(ui->label_34, QColor(0, 170, 0));
}

void CCZFTaskPlanWidget::initSlot()
{
    connect(ui->taskCodeCombx, &QComboBox::currentTextChanged, this, &CCZFTaskPlanWidget::slotSatellitCodeChange);
    connect(ui->oneKeySSSHSetBtn, &QPushButton::clicked, this, &CCZFTaskPlanWidget::slotOnkeySSSHSetting);
    connect(ui->oneKeySHSetBtn, &QPushButton::clicked, this, &CCZFTaskPlanWidget::slotOnkeySHSetting);
    connect(ui->oneKeyJSSetBtn, &QPushButton::clicked, this, &CCZFTaskPlanWidget::slotOnkeyJSSetting);
    connect(ui->timeRefreshBtn, &QPushButton::clicked, this, &CCZFTaskPlanWidget::slotRefreshTime);
    connect(ui->jdsbNameCombx, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CCZFTaskPlanWidget::slotJDNameRelation);
}

void CCZFTaskPlanWidget::slotSatellitCodeChange(const QString& arg1)
{
    Q_UNUSED(arg1);
    auto taskCode = ui->taskCodeCombx->currentText();
    if (m_taskCodeSateliteMap.contains(taskCode))
    {
        ui->objectBZEdit->setText(m_taskCodeSateliteMap[taskCode].m_satelliteIdentification);
    }
}

void CCZFTaskPlanWidget::slotRefreshTime()
{
    //刷新时间
    auto currentStartTime = GlobalData::currentDateTime().addSecs(20);
    auto currentEndTime = GlobalData::currentDateTime().addSecs(48 * 60 * 60);
    auto currentSHTime = GlobalData::currentDateTime().addSecs(20);

    ui->taskStartDate->setDate(currentStartTime.date());
    ui->taskStartTime->setTime(currentStartTime.time());
    ui->taskEndDate->setDate(currentEndTime.date());
    ui->taskEndTime->setTime(currentEndTime.time());
    ui->shSendDate->setDate(currentSHTime.date());
    ui->shSendTime->setTime(currentSHTime.time());
}

void CCZFTaskPlanWidget::slotJDNameRelation(int index) { ui->localNumCombx->setCurrentIndex(index); }

void CCZFTaskPlanWidget::refreUIData()
{
    //更新卫星任务代号数据
    QStringList list;
    auto dataList = GlobalData::getSatelliteManagementDataByObject();

    ui->taskCodeCombx->clear();
    if (!dataList.size())
    {
        QString errorMsg;
        errorMsg = QString("获取卫星数据失败");
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::information(this, "提示", errorMsg, "确定");
    }

    for (auto item : dataList)
    {
        list << item.m_satelliteCode;
        m_taskCodeSateliteMap[item.m_satelliteCode] = item;
    }

    for (QString name : list)
    {
        if (!name.isEmpty())
        {
            ui->taskCodeCombx->addItem(name, name);
        }
    }
    //刷新时间
    auto currentStartTime = GlobalData::currentDateTime().addSecs(20);
    auto currentEndTime = GlobalData::currentDateTime().addSecs(48 * 60 * 60);
    auto currentSHTime = GlobalData::currentDateTime().addSecs(20);

    ui->taskStartDate->setDate(currentStartTime.date());
    ui->taskStartTime->setTime(currentStartTime.time());
    ui->taskEndDate->setDate(currentEndTime.date());
    ui->taskEndTime->setTime(currentEndTime.time());
    ui->shSendDate->setDate(currentSHTime.date());
    ui->shSendTime->setTime(currentSHTime.time());

    //更新数传中心数据
    m_target_center->clear();
    QString json;
    if (!RedisHelper::getInstance().getData("DataTransmissionCenter", json))
    {
        qWarning() << json;
        QString errorMsg;
        errorMsg = QString("获取数传中心数据失败");
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::information(this, "提示", errorMsg, "确定");
        return;
    }
    DataTransmissionCenterMap dataTransmissionCenterDataMap;
    json >> dataTransmissionCenterDataMap;

    for (auto item : dataTransmissionCenterDataMap.values())
    {
        m_target_center->AddItem(item.centerName, false);
    }
}

void CCZFTaskPlanWidget::slotOnkeySSSHSetting()
{
    int workWay = 1;
    slotUnitAndCmdSetting(workWay);
}

void CCZFTaskPlanWidget::slotOnkeySHSetting()
{
    int workWay = 2;
    slotUnitAndCmdSetting(workWay);
}

void CCZFTaskPlanWidget::slotOnkeyJSSetting()
{
    int workWay = 3;
    slotUnitAndCmdSetting(workWay);
}

void CCZFTaskPlanWidget::slotUnitAndCmdSetting(int workWay)
{
    QString errorMsg;
    //检查设备是否在线 服务器是否在线
    if (!checkDeviceStatus(errorMsg))
    {
        QMessageBox::warning(this, QString("提示"), errorMsg, QString("确定"));
        return;
    }

    //检查关系,如开始时间是否晚于结束时间
    if (!checkRelation(errorMsg))
    {
        QMessageBox::warning(this, QString("提示"), errorMsg, QString("确定"));
        return;
    }

    QMap<QString, QVariant> params;
    params["BaseDev"] = ui->jdsbNameCombx->currentData().toString();           //基带设备名称
    params["BaseChan"] = ui->jdtdhCombx->currentData().toInt();                //基带通道号
    params["FileSize"] = 500;                                                  //单个文件大小 默认500MB
    params["DateFormat"] = "B";                                                //日期类型 默认北京时
    params["Date"] = GlobalData::currentDate().toString(DATE_DISPLAY_FORMAT);  //日期  当前日期
    params["Time"] = GlobalData::currentTime().toString(TIME_DISPLAY_FORMAT);  //时间  当前时间
    params["InformCat"] = ui->informationEdit->text();                         //信息类别
    params["TaskCircle"] = ui->taskqcEdit->text();                             //任务圈次
    params["ObjectIden"] = ui->objectBZEdit->text();                           //对象标识
    params["PattIdent"] = ui->modeBZCombx->currentData().toString();           //模式标识
    params["DpNum"] = 1;                                                       //点频 默认1
    params["XuanXiang"] = 1;                                                   //旋向 默认1
    params["IQOutPut"] = ui->iqOutWayCombx->currentData().toInt();             // IQ输出方式
    params["IChannelNum"] = ui->iChannelEdit->text();                          // I通道号
    params["QChannelNum"] = ui->qChannelEdit->text();                          // Q通道号

    auto deviceIDInt = ui->cczfSBChooseCombx->currentData().toInt();
    auto unitID = ui->localNumCombx->currentData().toInt() + 1;
    DeviceID deviceID(deviceIDInt);

    UnitParamRequest unitParamRequest;

    unitParamRequest.m_systemNumb = deviceID.sysID;
    unitParamRequest.m_deviceNumb = deviceID.devID;
    unitParamRequest.m_extenNumb = deviceID.extenID;
    unitParamRequest.m_modeID = 65535;
    unitParamRequest.m_unitID = unitID;
    unitParamRequest.m_paramdataMap = params;

    JsonWriter writer;
    writer& unitParamRequest;
    RedisHelper::getInstance().setData("单元发送数据", writer.GetQString());

    emit signalsUnitDeviceJson(writer.GetString());

    //数传中心数据单独处理
    auto centerName = m_target_center->GetSelItemsText().join("+");
    if (!m_target_center->GetSelItemsText().size())
    {
        QMessageBox::information(this, QString("提示"), QString("数传中心为空，请选择需使用的数传中心"), QString("确定"));
        return;
    }

    QMap<QString, QVariant> paramCmdMap;

    QDateTime startDateTime;
    QDateTime endDateTime;
    QDateTime shDateTime;

    startDateTime.setDate(ui->taskStartDate->date());
    startDateTime.setTime(ui->taskStartTime->time());
    endDateTime.setDate(ui->taskEndDate->date());
    endDateTime.setTime(ui->taskEndTime->time());
    shDateTime.setDate(ui->shSendDate->date());
    shDateTime.setTime(ui->shSendTime->time());

    paramCmdMap["CtrlSign"] = ui->controlBZCombx->currentData().toInt();          //控制标志
    paramCmdMap["OperatMode"] = workWay;                                          //工作方式
    paramCmdMap["TaskCode"] = ui->taskCodeCombx->currentText();                   //任务代号
    paramCmdMap["TaskMID"] = ui->objectBZEdit->text();                            //任务标识
    paramCmdMap["TraceNum"] = ui->trackjhEdit->text();                            //跟踪接收计划号
    paramCmdMap["DataTran"] = centerName;                                         //数据传输中心
    paramCmdMap["Priority"] = ui->yxjCombx->currentData().toInt();                //优先级
    paramCmdMap["Agrenment"] = ui->cscProtocolCombx->currentData().toInt();       //传输层协议
    paramCmdMap["StartTime"] = startDateTime.toString(DATETIME_DISPLAY_FORMAT3);  //任务开始时间
    paramCmdMap["TEndTime"] = endDateTime.toString(DATETIME_DISPLAY_FORMAT3);     //任务结束时间
    paramCmdMap["PostTime"] = shDateTime.toString(DATETIME_DISPLAY_FORMAT3);      //事后传输时间
    paramCmdMap["TranSpeed"] = ui->sendSDEdit->text();                            //传输速率
    paramCmdMap["CircleNumb"] = ui->qcnNumEdit->text();                           //圈次内编号
    paramCmdMap["LocalNumb"] = ui->localNumCombx->currentData().toInt();          //本机通道号
    paramCmdMap["DataProtocal"] = ui->dataProtocolCombx->currentData().toInt();   //数据协议

    CmdRequest cmdRequest;
    cmdRequest.m_systemNumb = deviceID.sysID;
    cmdRequest.m_deviceNumb = deviceID.devID;
    cmdRequest.m_extenNumb = deviceID.extenID;
    cmdRequest.m_modeID = 65535;
    cmdRequest.m_cmdID = 1;
    cmdRequest.m_paramdataMap = paramCmdMap;

    JsonWriter writerCmd;
    writerCmd& cmdRequest;
    RedisHelper::getInstance().setData("过程控制命令发送数据", writerCmd.GetQString());

    emit signalsCmdDeviceJson(writerCmd.GetString());
}

bool CCZFTaskPlanWidget::checkDeviceStatus(QString& errorMsg)
{
    //检查设备是否离线
    if (!ServiceCheck::serviceOnline())
    {
        errorMsg = QString("当前监控服务器离线");
        SystemLogPublish::errorMsg(errorMsg);
        return false;
    }

    auto deviceID = ui->cczfSBChooseCombx->currentData().toInt();
    //先判断设备是否在线
    if (!GlobalData::getDeviceOnline(deviceID))
    {
        errorMsg = QString("当前设备离线,命令下发失败");
        SystemLogPublish::errorMsg(errorMsg);
        return false;
    }
    //在判断设备是否是本分控
    bool isRemoteCtrl = GlobalData::getDeviceSelfControl(deviceID);
    if (!isRemoteCtrl)
    {
        errorMsg = QString("当前设备为分控状态,命令下发失败");
        SystemLogPublish::errorMsg(errorMsg);
        return false;
    }

    return true;
}

bool CCZFTaskPlanWidget::checkRelation(QString& errorMsg)
{
    QDateTime startDateTime;
    QDateTime endDateTime;

    startDateTime.setDate(ui->taskStartDate->date());
    startDateTime.setTime(ui->taskStartTime->time());
    endDateTime.setDate(ui->taskEndDate->date());
    endDateTime.setTime(ui->taskEndTime->time());

    int relationTime = startDateTime.toTime_t() - endDateTime.toTime_t();

    if (relationTime > 0)
    {
        errorMsg = QString("任务开始时间不能晚于任务结束时间");
        SystemLogPublish::errorMsg(errorMsg);
        return false;
    }

    return true;
}

void CCZFTaskPlanWidget::setSettingPanelColor(QWidget* control, QColor color)
{
    if (control != nullptr)
    {
        if (color.isValid())
        {
            control->setStyleSheet(
                QString(".QLabel {background-color: rgb(%1, %2, %3);color:rgb(255,255,255);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        }
    }
}
