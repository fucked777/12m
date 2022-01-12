#include "AntennaControlWidget4426.h"
#include "AutorunPolicyMessageSerialize.h"
#include "CustomPacketMessageSerialize.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "JsonHelper.h"
#include "MessagePublish.h"
#include "ProtocolXmlTypeDefine.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "SatelliteManagementSerialize.h"
#include "ServiceCheck.h"
#include "SubscriberHelper.h"
#include "ui_AntennaControlWidget4426.h"
#include <QAbstractItemView>
#include <QCompleter>
#include <QFontDatabase>
#include <QMessageBox>
#include <QStandardItem>
#include <QStandardItemModel>

#define ArrowTopCode    0xe6a5
#define ArrowLeftCode   0xe6a6
#define ArrowRightCode  0xe6a7
#define ArrowBottomCode 0xe6a8

#define JWC_GROUP_BASEINIT(name)                                                                                                                     \
    ui->name##BaseChooseCombx->addItem("无", 0);                                                                                                     \
    ui->name##BaseChooseCombx->addItem("跟踪基带A", 1);                                                                                              \
    ui->name##BaseChooseCombx->addItem("跟踪基带B", 2);                                                                                              \
    ui->name##BaseChooseCombx->addItem("跟踪基带C", 3);                                                                                              \
    ui->name##BaseChooseCombx->addItem("跟踪基带D", 4);

AntennaControlWidget4426::AntennaControlWidget4426(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AntennaControlWidget4426)
{
    ui->setupUi(this);
    initArrow();
    initUI();
    // 隐藏轨道预报
    // ui->groupBox_7->hide();
    this->setWindowIcon(QIcon(":/image/antenna_ctrl.png"));

    //初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
}

AntennaControlWidget4426::~AntennaControlWidget4426() { delete ui; }

void AntennaControlWidget4426::slotCmdResult(const QByteArray& data)
{
    QByteArray bak = data;
    CmdResult result;
    bak >> result;
    QVariantMap params = result.m_paramdataMap;
    switch (result.m_cmdID)
    {
    case 41:
    {
        //任务时间查询
        dealWithTaskQueryCmdResult(result.m_paramdataMap);
        break;
    }
    case 34:
    {
        //一键校相结果解析
        dealWithOneKeyXXCmdResult(result.m_paramdataMap);
        break;
    }
    default: break;
    }
}

void AntennaControlWidget4426::slotCmdResponse(const QByteArray& data)
{
    QByteArray bak = data;

    ControlCmdResponse controlcmdResponse;

    bak >> controlcmdResponse;

    auto deviceID = controlcmdResponse.deviceID;
    auto cmdID = controlcmdResponse.cmdId;

    //跟踪基带的角误差命令和视频切换单元的单元命令以及部分ACU的命令才会进入到这里面
    if ((deviceID.sysID == 4 && deviceID.devID == 3) ||
        (deviceID.sysID == 1 && (cmdID == 4 || cmdID == 5 || cmdID == 9 || cmdID == 10 || cmdID == 15 || cmdID == 8 || cmdID == 7 || cmdID == 6 ||
                                 cmdID == 39 || cmdID == 27 || cmdID == 34)))
    {
        auto result = DevProtocolEnumHelper::controlCmdResponseToDescStr(controlcmdResponse.responseResult);
        QMessageBox::information(this, QString("提示"), QString(result), QString("确定"));
    }
}

void AntennaControlWidget4426::initUI()
{
    connect(ui->acuRunModeCheckBox, &QCheckBox::clicked, [this](bool checked) {
        ui->manualRadioBtn->setEnabled(checked);
        ui->autoRadiobtn->setEnabled(checked);
        if (checked == true)
        {
            acuRunMode();
        }
    });
    ui->manualRadioBtn->setEnabled(false);
    ui->autoRadiobtn->setEnabled(false);

    connect(ui->setupBtn, &QPushButton::clicked, [=]() {
        m_currentAZ = ui->acuAzimuthEdit->value();
        m_currentPA = ui->acuPitchEdit->value();
        m_AZspeed = ui->acuAzimuthSpeedEdit->value();
        m_PAspeed = ui->acuPitchSpeedEdit->value();
        setupDirectImpl(m_currentAZ, m_currentPA, m_AZspeed, m_PAspeed);
    });
    connect(ui->standbyBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::standby);
    connect(ui->pointBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::point);       // 3.按指定角度
    connect(ui->tower, &QPushButton::clicked, this, &AntennaControlWidget4426::tower);          // 1.对塔
    connect(ui->waitPoint, &QPushButton::clicked, this, &AntennaControlWidget4426::waitPoint);  // 2. 转等待点
    connect(ui->numberLeadBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::numberLead);
    connect(ui->programLeadBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::programLead);
    connect(ui->collectionBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::collection);
    connect(ui->unlockBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::unlock);

    connect(ui->taskStartBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::taskStart);  //任务开始
    connect(ui->sTrackBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::sTrack1);
    // connect(ui->kaMeasureTrackBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::kaMeasureTrack1);
    connect(ui->kaDataTransTrackBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::kaDataTransTrack);
    connect(ui->kaYCBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::kaYCTrack);

    //    connect(ui->taskTimeQueryBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::taskTimeQuery);
    connect(ui->gdybSetBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::gdybSet);

    connect(ui->arrowTopBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::arrowTop);
    connect(ui->arrowBottomBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::arrowBottom);
    connect(ui->arrowRightBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::arrowRight);
    connect(ui->arrowLeftBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::arrowLeft);
    connect(ui->zero, &QPushButton::clicked, this, &AntennaControlWidget4426::arrowZero);

    connect(ui->searchBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::search);  //查询
    connect(ui->soltButton, &QPushButton::clicked, this, &AntennaControlWidget4426::solt);   // genxin

    connect(ui->oneKeyXXSBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::slotOneKeyXXS);        //一键校相S频段
    connect(ui->oneKeyXXKaYcBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::slotOneKeyXXKaYc);  //一键校相Ka遥测
    connect(ui->oneKeyXXKaScBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::slotOneKeyXXKaSc);  //一键校相Ka数传

    ui->acuAzimuthEdit->setRange(0, 359.99);
    ui->acuAzimuthEdit->setToolTip(QString("当前输入范围为0~359.99"));
    ui->acuPitchEdit->setRange(0, 90);
    ui->acuPitchEdit->setToolTip(QString("当前输入范围为0~90"));
    //  ui->acuAzimuthSpeedEdit->setRange(0.0, 10.0);
    ui->acuAzimuthSpeedEdit->setToolTip(QString("当前输入范围为1~3"));
    //  ui->acuPitchSpeedEdit->setRange(0.0, 8.0);
    ui->acuPitchSpeedEdit->setToolTip(QString("当前输入范围为1~3"));

    ui->inStationAzimuthEdit->setEnabled(false);
    ui->inStationPitchEdit->setEnabled(false);
    ui->overTopAzimuthEdit->setEnabled(false);
    ui->overTopPitchEdit->setEnabled(false);
    ui->outStationAzimuthEdit->setEnabled(false);
    ui->outStationPitchEdit->setEnabled(false);

    ui->acuAzimuthSpeedEdit->setValue(1.0);
    ui->acuPitchSpeedEdit->setValue(1.0);

    // 1:人工 2:自动
    m_autoManualBtn = new QButtonGroup(this);
    m_autoManualBtn->addButton(ui->manualRadioBtn, 1);
    m_autoManualBtn->addButton(ui->autoRadiobtn, 2);
    ui->manualRadioBtn->setChecked(true);
    ui->autoRadiobtn->setChecked(false);
    connect(m_autoManualBtn, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &AntennaControlWidget4426::acuRunMode);

    ui->predictionComBox->addItem("历元时间预报", 1);
    ui->predictionComBox->addItem("指定时间段预报", 2);

    ui->gdybStartDateTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
    ui->gdybEndDateTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");

    //开始时间为当前NTP时间  结束时间比当前快2000分钟
    auto currentStartDateTime = GlobalData::currentDateTime();
    auto currentEndDateTime = GlobalData::currentDateTime().addSecs(2000 * 60);

    ui->gdybStartDateTimeEdit->setDateTime(currentStartDateTime);
    ui->gdybEndDateTimeEdit->setDateTime(currentEndDateTime);

    //    ui->rwcxTaskCodeComboBox->clear();
    //    SatelliteManagementDataMap satelliteManagementDataMap;

    //    /* 获取卫星管理数据 */
    //    if (!GlobalData::getSatelliteManagementData(satelliteManagementDataMap))
    //    {
    //        return;
    //    }
    //    for (auto iter = satelliteManagementDataMap.begin(); iter != satelliteManagementDataMap.end(); ++iter)
    //    {
    //        auto key = iter.key();
    //        ui->rwcxTaskCodeComboBox->addItem(iter->m_satelliteCode, key);
    //    }

    ui->label->setStyleSheet("QLabel{font:13pt;}");
    ui->label_2->setStyleSheet("QLabel{font:13pt;}");
    ui->acuAzimuthLab->setStyleSheet("QLabel{font:13pt;}");
    ui->acuPitchLab->setStyleSheet("QLabel{font:13pt;}");

    /***************************角误差命令********************************/
    /*基带初始化*/
    JWC_GROUP_BASEINIT(a);
    JWC_GROUP_BASEINIT(b);
    JWC_GROUP_BASEINIT(c);

    connect(ui->jwcSetBtn, &QPushButton::clicked, this, &AntennaControlWidget4426::slotJWCCmdSet);
}

void AntennaControlWidget4426::initArrow()
{
    int fontId = QFontDatabase::addApplicationFont(":/image/iconfont.ttf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    QFont font;
    //    font.setFamily(fontFamilies.at(0));
    font.setPixelSize(ui->arrowBottomBtn->height());

    //    ui->arrowTopBtn->setFont(font);
    //    ui->arrowTopBtn->setText(QChar(ArrowTopCode));

    //    ui->arrowRightBtn->setFont(font);
    //    ui->arrowRightBtn->setText(QChar(ArrowRightCode));

    //    ui->arrowBottomBtn->setFont(font);
    //    ui->arrowBottomBtn->setText(QChar(ArrowBottomCode));

    //    ui->arrowLeftBtn->setFont(font);
    //    ui->arrowLeftBtn->setText(QChar(ArrowLeftCode));

    ui->arrowTopBtn->setIcon(QIcon(":/image/arrow_above.png"));
    ui->arrowBottomBtn->setIcon(QIcon(":/image/arrow_bottom.png"));
    ui->arrowRightBtn->setIcon(QIcon(":/image/arrow_forward.png"));
    ui->arrowLeftBtn->setIcon(QIcon(":/image/arrow_back.png"));
}

void AntennaControlWidget4426::getCurrentPlanData()
{
    //初始化
    m_currentCKPlanData = std::make_tuple(NotDefine, "", -1);
    m_currentDSPlanData = std::make_tuple(NotDefine, "", -1);
    m_currentGSPlanData = std::make_tuple(NotDefine, "", -1);

    m_manualMsg = GlobalData::getTaskRunningInfoByObject();
    QString ckPlanData;
    QString dsscPlanData;
    QString gsscPlanData;

    for (const auto& link : m_manualMsg.linkLineMap)
    {
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(link.workMode))  // 测控模式都使用同一个任务代号
        {
            auto taskcode = link.targetMap[link.masterTargetNo].taskCode;
            auto point = link.targetMap[link.masterTargetNo].pointFreqNo;
            ckPlanData = QString(QString::number(link.workMode) + "|" + taskcode + "|" + QString::number(point));
        }
        else  //数传需要区分高低速  从而找到不同的旋向
        {
            if (link.workMode == KaGS)  //是否是高速数传
            {
                auto taskcode = link.targetMap[link.masterTargetNo].taskCode;
                auto point = link.targetMap[link.masterTargetNo].pointFreqNo;
                gsscPlanData = QString(QString::number(link.workMode) + "|" + taskcode + "|" + QString::number(point));
            }
            else if (link.workMode == KaDS)  //不是高速就是低速
            {
                auto taskcode = link.targetMap[link.masterTargetNo].taskCode;
                auto point = link.targetMap[link.masterTargetNo].pointFreqNo;
                dsscPlanData = QString(QString::number(link.workMode) + "|" + taskcode + "|" + QString::number(point));
            }
        }
    }

    if (!ckPlanData.isEmpty())
    {
        auto taskCodePointList = ckPlanData.split("|");
        if (taskCodePointList.size() == 3)
        {
            std::get<0>(m_currentCKPlanData) = (SystemWorkMode)taskCodePointList[0].toInt();
            std::get<1>(m_currentCKPlanData) = taskCodePointList[1];
            std::get<2>(m_currentCKPlanData) = taskCodePointList[2].toInt();
        }
    }

    if (!dsscPlanData.isEmpty())
    {
        auto taskCodePointList = dsscPlanData.split("|");
        if (taskCodePointList.size() == 3)
        {
            std::get<0>(m_currentDSPlanData) = (SystemWorkMode)taskCodePointList[0].toInt();
            std::get<1>(m_currentDSPlanData) = taskCodePointList[1];
            std::get<2>(m_currentDSPlanData) = taskCodePointList[2].toInt();
        }
    }

    if (!gsscPlanData.isEmpty())
    {
        auto taskCodePointList = gsscPlanData.split("|");
        if (taskCodePointList.size() == 3)
        {
            std::get<0>(m_currentGSPlanData) = (SystemWorkMode)taskCodePointList[0].toInt();
            std::get<1>(m_currentGSPlanData) = taskCodePointList[1];
            std::get<2>(m_currentGSPlanData) = taskCodePointList[2].toInt();
        }
    }
}

void AntennaControlWidget4426::dealWithTaskQueryCmdResult(QVariantMap params)
{
    // 去掉小数后多于的0
    QRegExp rx(R"((\.){0,1}0+$)");
    QString DateArrival = params.value("DateArrival").toString();  //进站日期
    QString ArrivalTime = params.value("ArrivalTime").toString();  //进站时间
    ui->inStationTimeLab->setText(QString("%1 %2").arg(DateArrival).arg(ArrivalTime));
    QVariant ApprPosit = params.value("ApprPosit");  //进站方位
    auto ApprPositString = QString("%1").arg(ApprPosit.toDouble(), 0, 'f', 6).replace(rx, "");
    ui->inStationAzimuthEdit->setText(ApprPositString);
    QVariant PitchPitch = params.value("PitchPitch");  //进站俯仰
    auto PitchPitchString = QString("%1").arg(PitchPitch.toDouble(), 0, 'f', 6).replace(rx, "");
    ui->inStationPitchEdit->setText(PitchPitchString);

    QString OverDate = params.value("OverDate").toString();      //过顶日期
    QString OvershTime = params.value("OvershTime").toString();  //过顶时间
    ui->overTopTimeLab->setText(QString("%1 %2").arg(OverDate).arg(OvershTime));
    QVariant OutBDirect = params.value("OvershAzim");  //出站方位
    auto OutBDirectString = QString("%1").arg(OutBDirect.toDouble(), 0, 'f', 6).replace(rx, "");
    ui->overTopAzimuthEdit->setText(OutBDirectString);
    QVariant OutBPitch = params.value("OvershPitch");  //出站俯仰
    auto OutBPitchString = QString("%1").arg(OutBPitch.toDouble(), 0, 'f', 6).replace(rx, "");
    ui->overTopPitchEdit->setText(OutBPitchString);

    QString DateDepart = params.value("DateDepart").toString();  //出站日期
    QString OutBTime = params.value("OutBTime").toString();      //出站时间
    ui->outStationTimeLab->setText(QString("%1 %2").arg(DateDepart).arg(OutBTime));
    QVariant OvershAzim = params.value("OutBDirect");  //过顶方位
    auto OvershAzimString = QString("%1").arg(OvershAzim.toDouble(), 0, 'f', 6).replace(rx, "");
    ui->outStationAzimuthEdit->setText(OvershAzimString);
    QVariant OvershPitch = params.value("OutBPitch");  //过顶俯仰
    auto OvershPitchString = QString("%1").arg(OvershPitch.toDouble(), 0, 'f', 6).replace(rx, "");
    ui->outStationPitchEdit->setText(OvershPitchString);
}

void AntennaControlWidget4426::dealWithOneKeyXXCmdResult(QVariantMap params) {}

/* 待机 */
void AntennaControlWidget4426::standby() { cmdSettingACU(2); }
/* 指向 */
void AntennaControlWidget4426::point()
{
    QVariantMap params;
    params.insert("DirectMode", 3);
    params.insert("DesigAzimu", ui->acuAzimuthLab->text().toDouble());
    params.insert("DesigElevat", ui->acuPitchLab->text().toDouble());
    params.insert("DesigAzimVel", ui->acuAzimuthSpeedEdit->value());
    params.insert("SpePitAngVel", ui->acuPitchSpeedEdit->value());

    cmdSettingACU(3, params);
}

/* 对塔 */
void AntennaControlWidget4426::tower()
{
    QVariantMap params;
    params.insert("DirectMode", 1);
    params.insert("DesigAzimu", ui->acuAzimuthEdit->value());
    params.insert("DesigElevat", ui->acuPitchEdit->value());
    params.insert("DesigAzimVel", ui->acuAzimuthSpeedEdit->value());
    params.insert("SpePitAngVel", ui->acuPitchSpeedEdit->value());

    cmdSettingACU(3, params);
}

/* 等待转点 */
void AntennaControlWidget4426::waitPoint()
{
    QVariantMap params;
    params.insert("DirectMode", 2);
    params.insert("DesigAzimu", ui->acuAzimuthEdit->value());
    params.insert("DesigElevat", ui->acuPitchEdit->value());
    params.insert("DesigAzimVel", ui->acuAzimuthSpeedEdit->value());
    params.insert("SpePitAngVel", ui->acuPitchSpeedEdit->value());

    cmdSettingACU(3, params);
}

/* 数字引导 */
void AntennaControlWidget4426::numberLead() { cmdSettingACU(4); }
/* 程序引导 */
void AntennaControlWidget4426::programLead() { cmdSettingACU(5); }
/* 收藏不插锁 */
void AntennaControlWidget4426::collection()
{
    QVariantMap params;
    params.insert("LockInPlaceNot", 2);

    cmdSettingACU(9, params);
}
/* 拔锁 */
void AntennaControlWidget4426::unlock() { cmdSettingACU(10); }

/* 任务开始 */
void AntennaControlWidget4426::taskStart()
{
    QVariantMap params;

    auto taskCode = ui->comboBox_2->currentText();
    auto taskIdentifi = m_taskCodeSateliteMap[taskCode].m_satelliteIdentification;
    params.insert("TaskIdent", taskIdentifi);

    cmdSettingACU(15, params);
}

/* s跟踪 */
void AntennaControlWidget4426::sTrack1() { cmdSettingACU(8); }

// void AntennaControlWidget4426::kaMeasureTrack1() {} /* ka测控跟踪 */

/* ka数传跟踪 */
void AntennaControlWidget4426::kaDataTransTrack() { cmdSettingACU(7); }

void AntennaControlWidget4426::kaYCTrack() { cmdSettingACU(6); }

/* acu运行方式 */
void AntennaControlWidget4426::acuRunMode()
{
    QVariantMap params;
    auto checkId = m_autoManualBtn->checkedId();
    params.insert("OperatMode", checkId);  // 1：人工 2：自动
    cmdSettingACU(39, params);
}

void AntennaControlWidget4426::taskTimeQuery() {} /* 任务时间查询 */

/* 轨道预报 */
void AntennaControlWidget4426::gdybSet()
{
    QMap<QString, QVariant> params;
    //界面上显示的是任务代号，但实际上需要下发的参数是任务标识
    //需要的任务代号时当前任务的任务代号，这个东西后面会提供一个接口直接获取，现在是直接从下拉框选择
    auto taskCode = ui->gdybTaskIdentCbx->currentText();
    auto taskIdentifi = m_taskCodeSateliteMap[taskCode].m_satelliteIdentification;
    //预报方式
    auto prediction = ui->predictionComBox->currentData().toInt();
    //开始日期时间
    auto startDate = ui->gdybStartDateTimeEdit->date().toString(DATE_DISPLAY_FORMAT);
    auto startTime = ui->gdybStartDateTimeEdit->time().toString(TIME_DISPLAY_FORMAT);
    //结束日期时间
    auto endDate = ui->gdybEndDateTimeEdit->date().toString(DATE_DISPLAY_FORMAT);
    auto endTime = ui->gdybEndDateTimeEdit->time().toString(TIME_DISPLAY_FORMAT);

    params["TaskIdent"] = taskIdentifi;
    params["PredMethod"] = prediction;
    params["DesStartDate"] = startDate;
    params["DesStartTime"] = startTime;
    params["DesEndDate"] = endDate;
    params["DesEndTime"] = endTime;

    cmdSettingACU(27, params);
}

void AntennaControlWidget4426::arrowTop()
{
    double az = ui->acuAzimuthLab->text().toDouble();
    double pa = ui->acuPitchLab->text().toDouble();
    double azSpeed = ui->acuAzimuthSpeedEdit->value();
    double paSpeed = ui->acuPitchSpeedEdit->value();

    pa += (0.06 * ui->stepSpinBox->value());  //步进是1，范围是1-100,所以要乘以0.06
    if (pa > 90)
    {
        pa = 90;
    }
    setupDirectImpl(az, pa, azSpeed, paSpeed);
}
void AntennaControlWidget4426::arrowBottom()
{
    double az = ui->acuAzimuthLab->text().toDouble();
    double pa = ui->acuPitchLab->text().toDouble();
    double azSpeed = ui->acuAzimuthSpeedEdit->value();
    double paSpeed = ui->acuPitchSpeedEdit->value();

    pa -= (0.06 * ui->stepSpinBox->value());  //步进是1，范围是1-100,所以要乘以0.06
    if (pa < 0)
    {
        pa = 0.0;
    }
    setupDirectImpl(az, pa, azSpeed, paSpeed);
}
void AntennaControlWidget4426::arrowRight()
{
    double az = ui->acuAzimuthLab->text().toDouble();
    double pa = ui->acuPitchLab->text().toDouble();
    double azSpeed = ui->acuAzimuthSpeedEdit->value();
    double paSpeed = ui->acuPitchSpeedEdit->value();

    az += (0.06 * ui->stepSpinBox->value());  //步进是1，范围是1-100,所以要乘以0.06
    if (az > 359.99)
    {
        az = 359.99;
    }
    setupDirectImpl(az, pa, azSpeed, paSpeed);
}
void AntennaControlWidget4426::arrowLeft()
{
    double az = ui->acuAzimuthLab->text().toDouble();
    double pa = ui->acuPitchLab->text().toDouble();
    double azSpeed = ui->acuAzimuthSpeedEdit->value();
    double paSpeed = ui->acuPitchSpeedEdit->value();

    az -= (0.06 * ui->stepSpinBox->value());  //步进是1，范围是1-100,所以要乘以0.06
    if (az < 0)
    {
        az = 0;
    }
    setupDirectImpl(az, pa, azSpeed, paSpeed);

    // setupDirectImpl(3, value, m_currentPA);
}
void AntennaControlWidget4426::arrowZero()
{
    double az = ui->acuAzimuthLab->text().toDouble();
    double pa = ui->acuPitchLab->text().toDouble();
    double azSpeed = ui->acuAzimuthSpeedEdit->value();
    double paSpeed = ui->acuPitchSpeedEdit->value();
    az = 0.0;
    pa = 0.0;
    setupDirectImpl(az, pa, azSpeed, paSpeed);
}

bool AntennaControlWidget4426::isOnlineACU()
{
    QString errorMsg;
    if (!ServiceCheck::serviceOnline())
    {
        errorMsg = QString("当前监控服务器离线");
        SystemLogPublish::errorMsg(errorMsg);
        ui->statusLabel->setText(errorMsg);
        QColor color(Qt::red);
        ui->statusLabel->setStyleSheet(QString(".QLabel{color: rgb(%1, %2, %3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        return false;
    }
    DeviceID acuAID(1, 0, 17);
    DeviceID acuBID(1, 0, 18);

    auto isOnlneA = GlobalData::getDeviceOnline(acuAID);
    auto isOnlneB = GlobalData::getDeviceOnline(acuBID);

    DeviceID useACUID;
    if (!isOnlneA && !isOnlneB)
    {
        QString errorMsg = QString("ACU_A和ACU_B都处于离线状态");
        SystemLogPublish::warningMsg(errorMsg);
        ui->statusLabel->setText(errorMsg);
        QColor color(Qt::red);
        ui->statusLabel->setStyleSheet(QString(".QLabel{color: rgb(%1, %2, %3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
        return false;
    }
    //判断ACU是否在线
    if (isOnlneA)
    {
        auto acuExMapA = GlobalData::getExtenStatusReportData(acuAID);
        if (acuExMapA.unitReportMsgMap[1].paramMap["OnLineMachine"] == 1)  //判断是否是主机
        {
            useACUID = acuAID;
        }
    }

    if (isOnlneB)
    {
        auto acuExMapB = GlobalData::getExtenStatusReportData(acuBID);
        if (acuExMapB.unitReportMsgMap[1].paramMap["OnLineMachine"] == 1)  //判断是否是主机
        {
            useACUID = acuBID;
        }
    }

    if (!useACUID.isValid())
    {
        QString errorMsg = QString("可使用的ACU都处于备机状态，下发命令失败！");
        SystemLogPublish::warningMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
        return false;
    }

    m_useACUDeviceID = useACUID;

    auto isControl = GlobalData::getDeviceSelfControl(m_useACUDeviceID, 2);
    if (!isControl)
    {
        errorMsg = QString("ACU设备为分控");
        SystemLogPublish::warningMsg(errorMsg);
        ui->statusLabel->setText(errorMsg);
        QColor color(229, 153, 0);
        ui->statusLabel->setStyleSheet(QString(".QLabel{color: rgb(%1, %2, %3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        return false;
    }

    QColor color(0, 170, 0);
    ui->statusLabel->setText("命令已下发");
    ui->statusLabel->setStyleSheet(QString(".QLabel{color: rgb(%1, %2, %3);}").arg(color.red()).arg(color.green()).arg(color.blue()));

    return true;
}

bool AntennaControlWidget4426::isOnlineGZJD()
{
    QString errorMsg;
    if (!ServiceCheck::serviceOnline())
    {
        errorMsg = QString("当前监控服务器离线");
        SystemLogPublish::errorMsg(errorMsg);
        ui->statusLabel->setText(errorMsg);
        QColor color(Qt::red);
        ui->statusLabel->setStyleSheet(QString(".QLabel{color: rgb(%1, %2, %3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        return false;
    }
    auto onlineA = GlobalData::getDeviceOnline(0x4201);
    auto onlineB = GlobalData::getDeviceOnline(0x4202);
    auto onlineC = GlobalData::getDeviceOnline(0x4203);
    auto onlineD = GlobalData::getDeviceOnline(0x4204);
    //四台都不在线才返回false,有一台在线就下发命令
    if (!onlineA && !onlineB && !onlineC && !onlineD)
    {
        errorMsg = QString("跟踪基带四台设备都处于离线状态");
        SystemLogPublish::errorMsg(errorMsg);
        ui->statusLabel->setText(errorMsg);
        QColor color(Qt::red);
        ui->statusLabel->setStyleSheet(QString(".QLabel{color: rgb(%1, %2, %3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        return false;
    }

    QColor color(0, 170, 0);
    ui->statusLabel->setText("命令已下发");
    ui->statusLabel->setStyleSheet(QString(".QLabel{color: rgb(%1, %2, %3);}").arg(color.red()).arg(color.green()).arg(color.blue()));

    return true;
}

void AntennaControlWidget4426::setupDirectImpl(double az, double pa, double azSpeed, double paSpeed)
{
    QVariantMap params;
    params.insert("DirectMode", 3);
    params.insert("DesigAzimu", az);         //方位角
    params.insert("DesigElevat", pa);        //俯仰角
    params.insert("DesigAzimVel", azSpeed);  //方位限速
    params.insert("SpePitAngVel", paSpeed);  //俯仰限速

    cmdSettingACU(3, params);
}

void AntennaControlWidget4426::comboxRedal(QComboBox* combox)
{
    combox->clear();
    QStandardItemModel* model = new QStandardItemModel(this);  //源数据
    QList<QStandardItem*> row;
    QStringList list;
    auto dataList = GlobalData::getSatelliteManagementDataByObject();

    if (!dataList.size())
    {
        QString errorMsg;
        errorMsg = QString("获取卫星数据失败");
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::information(this, "提示", errorMsg, "确定");
    }

    for (auto item : dataList)
    {
        mWorkModeDescMap[item.m_satelliteCode] = item.getWorkModeDesc();
        m_currentWorkMode[item.m_satelliteCode] = item.m_workMode;
        list << item.m_satelliteCode;
        m_taskCodeSateliteMap[item.m_satelliteCode] = item;
    }
    for (QString name : list)
    {
        if (!name.isEmpty())
        {
            row.clear();
            QStandardItem* item = new QStandardItem(name);
            row << item;
            model->appendRow(row);  //添加一行，2列数据
        }
    }
    combox->setModel(model);  // QComboBox设置数据源
    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(combox);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterKeyColumn(-1);  //-1表示匹配所有列

    // QCompleter* completer = new QCompleter(combox);
    //    completer->setCaseSensitivity(Qt::CaseInsensitive);
    //    completer->setModel(proxy);
    //    completer->setCompletionColumn(combox->modelColumn());
    //    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    //    completer->setMaxVisibleItems(10);            //下拉最大高度
    // completer->setFilterMode(Qt::MatchContains);  //设置匹配规则
    //    completer->popup()->setStyleSheet(
    //        "QListView{font:75 12pt \"微软雅黑\";subcontrol-origin: padding;subcontrol-position: top right;width: 20px;}"); //设置弹出的补全列表样式

    // combox->setCompleter(completer);  //设置输入自动补全
    combox->setStyleSheet("QComboBox{combobox-popup: 0;}");
    combox->setMaxVisibleItems(10);  //下拉最大高度
}

void AntennaControlWidget4426::search()
{
    QVariantMap params;
    params.insert("TaskIdent", ui->comboBox_1->currentText());

    cmdSettingACU(41, params);
}

void AntennaControlWidget4426::solt()
{
    m_taskCodeSateliteMap.clear();
    comboxRedal(ui->comboBox_1);
    comboxRedal(ui->comboBox_2);
    comboxRedal(ui->gdybTaskIdentCbx);

    //开始时间为当前时间  结束时间比当前快60分钟 轨道预报的时间
    auto currentStartDateTime = GlobalData::currentDateTime();
    auto currentEndDateTime = GlobalData::currentDateTime().addSecs(2000 * 60);

    ui->gdybStartDateTimeEdit->setDateTime(currentStartDateTime);
    ui->gdybEndDateTimeEdit->setDateTime(currentEndDateTime);

    //每次界面打开时初始化一次方位和俯仰的值，便于调试
    auto acuAzimuthInitValue = ui->acuAzimuthLab->text().toDouble();
    ui->acuAzimuthEdit->setValue(acuAzimuthInitValue);

    auto acuPitcInitValue = ui->acuPitchLab->text().toDouble();
    ui->acuPitchEdit->setValue(acuPitcInitValue);

    //获取当前任务代号并设置到该界面各个任务代号下拉框上
    getCurrentPlanData();
    //因为各个模式的任务代号都是取的主跟的任务代号，而主跟任务代号都一样，所以哪个测控或数传的任务代号不为空，直接设置就行
    if (!std::get<1>(m_currentCKPlanData).isEmpty())  //判断下任务代号为不为空
    {
        auto taskCode = std::get<1>(m_currentCKPlanData);
        ui->comboBox_1->setCurrentText(taskCode);
        ui->comboBox_2->setCurrentText(taskCode);
        ui->gdybTaskIdentCbx->setCurrentText(taskCode);
    }
    else if (!std::get<1>(m_currentGSPlanData).isEmpty())  //判断下任务代号为不为空
    {
        auto taskCode = std::get<1>(m_currentGSPlanData);
        ui->comboBox_1->setCurrentText(taskCode);
        ui->comboBox_2->setCurrentText(taskCode);
        ui->gdybTaskIdentCbx->setCurrentText(taskCode);
    }
    else if (!std::get<1>(m_currentDSPlanData).isEmpty())  //判断下任务代号为不为空
    {
        auto taskCode = std::get<1>(m_currentDSPlanData);
        ui->comboBox_1->setCurrentText(taskCode);
        ui->comboBox_2->setCurrentText(taskCode);
        ui->gdybTaskIdentCbx->setCurrentText(taskCode);
    }
}

void AntennaControlWidget4426::slotOneKeyXXS()
{
    if (!isOnlineACU())
    {
        return;
    }

    /* 获取当前任务计划数据 */
    getCurrentPlanData();

    if (m_manualMsg.linkLineMap.isEmpty())
    {
        QMessageBox::warning(this, "提示", "未获取到当前的任务信息无法校相", "确认");
        return;
    }
    /* 当前是S的 这边要处理任务信息只保留S */
    auto bak = m_manualMsg;
    QMap<SystemWorkMode, LinkLine> linkLineMap;
    for (auto iter = bak.linkLineMap.begin(); iter != bak.linkLineMap.end(); ++iter)
    {
        auto workMode = iter.key();
        if (SystemWorkModeHelper::systemWorkModeToSystemBand(workMode) == SBand)
        {
            linkLineMap.insert(workMode, iter.value());
        }
    }

    if (linkLineMap.isEmpty() || linkLineMap.first().targetMap.isEmpty())
    {
        QMessageBox::warning(this, "提示", "当前任务无S频段信息,不能校相", "确认");
        return;
    }
    bak.xxMode = ACUPhaseCalibrationFreqBand::S;
    bak.manualType = ManualType::OnekeyXX;

    QString json;
    json << bak;
    emit signalManualFunction(json);

    QMessageBox::information(this, "提示", "命令已下发", "确定");
}

void AntennaControlWidget4426::slotOneKeyXXKaYc()
{
    if (!isOnlineACU())
    {
        return;
    }
    /* 获取当前任务计划数据 */
    getCurrentPlanData();

    if (m_manualMsg.linkLineMap.isEmpty())
    {
        QMessageBox::warning(this, "提示", "未获取到当前的任务信息无法校相", "确认");
        return;
    }
    /* 当前是Ka遥测的 这边要处理任务信息只保留Ka遥测 */
    auto bak = m_manualMsg;
    QMap<SystemWorkMode, LinkLine> linkLineMap;
    for (auto iter = bak.linkLineMap.begin(); iter != bak.linkLineMap.end(); ++iter)
    {
        auto workMode = iter.key();
        if (workMode == KaKuo2)
        {
            linkLineMap.insert(workMode, iter.value());
        }
    }

    if (linkLineMap.isEmpty() || linkLineMap.first().targetMap.isEmpty())
    {
        QMessageBox::warning(this, "提示", "当前任务无Ka遥测信息,不能校相", "确认");
        return;
    }
    bak.xxMode = ACUPhaseCalibrationFreqBand::KaYC;
    bak.manualType = ManualType::OnekeyXX;

    QString json;
    json << bak;
    emit signalManualFunction(json);

    QMessageBox::information(this, "提示", "命令已下发", "确定");
}

void AntennaControlWidget4426::slotOneKeyXXKaSc()
{
    if (!isOnlineACU())
    {
        return;
    }
    /* 获取当前任务计划数据 */
    getCurrentPlanData();

    if (m_manualMsg.linkLineMap.isEmpty())
    {
        QMessageBox::warning(this, "提示", "未获取到当前的任务信息无法校相", "确认");
        return;
    }
    /* 当前是Ka数传的 这边要处理任务信息只保留Ka数传 */
    auto bak = m_manualMsg;
    QMap<SystemWorkMode, LinkLine> linkLineMap;
    for (auto iter = bak.linkLineMap.begin(); iter != bak.linkLineMap.end(); ++iter)
    {
        auto workMode = iter.key();
        if (SystemWorkModeHelper::isDataTransmissionWorkMode(workMode) && workMode != XDS)
        {
            linkLineMap.insert(workMode, iter.value());
        }
    }

    if (linkLineMap.isEmpty() || linkLineMap.first().targetMap.isEmpty())
    {
        QMessageBox::warning(this, "提示", "当前任务无Ka数传信息,不能校相", "确认");
        return;
    }
    bak.xxMode = ACUPhaseCalibrationFreqBand::KaSC;
    bak.manualType = ManualType::OnekeyXX;

    QString json;
    json << bak;
    emit signalManualFunction(json);

    QMessageBox::information(this, "提示", "命令已下发", "确定");
}

void AntennaControlWidget4426::slotJWCCmdSet()
{
    /*跟踪基带角误差命令和视频切换单元的单元下发枚举不一样，这里做些处理，key是跟踪基带角误差命令的值，value是视频切换单元的值*/
    QMap<int, int> ckJWCUnitValueMap;  //测控
    ckJWCUnitValueMap[0] = 10;
    ckJWCUnitValueMap[1] = 1;
    ckJWCUnitValueMap[2] = 3;
    ckJWCUnitValueMap[3] = 5;
    ckJWCUnitValueMap[4] = 7;

    QMap<int, int> scJWCUnitValueMap;  //数传
    scJWCUnitValueMap[0] = 10;
    scJWCUnitValueMap[1] = 2;
    scJWCUnitValueMap[2] = 4;
    scJWCUnitValueMap[3] = 6;
    scJWCUnitValueMap[4] = 8;
    QVariantMap params;
    auto aBaseChoose = ui->aBaseChooseCombx->currentData().toInt();  // S测控
    auto bBaseChoose = ui->bBaseChooseCombx->currentData().toInt();  // Ka遥测
    auto cBaseChoose = ui->cBaseChooseCombx->currentData().toInt();  // Ka数传

    QVariant aPDSwitch = 0;  //这里做个操作，S测控选择的话默认频段选择S频段
    QVariant bPDSwitch = 0;  //这里做个操作，Ka遥测选择的话默认频段选择Ka频段
    QVariant cPDSwitch = 0;  //这里做个操作，Ka数传选择的话默认频段选择Ka频段

    QVariant aModeChoose = 0;  //这里做个操作，S测控选择的话默认频段选择测控
    QVariant bModeChoose = 0;  //这里做个操作，Ka遥测选择的话默认频段选择测控
    QVariant cModeChoose = 0;  //这里做个操作，Ka数传选择的话默认频段选择数传

    if (aBaseChoose != 0)
    {
        aPDSwitch = 1;
        aModeChoose = 1;
    }
    if (bBaseChoose != 0)
    {
        bPDSwitch = 3;
        bModeChoose = 1;
    }
    if (cBaseChoose != 0)
    {
        cPDSwitch = 3;
        cModeChoose = 2;
    }

    params["SF_BANDCHECK"] = aBaseChoose;
    params["KAF_BANDCHECK"] = bBaseChoose;
    params["KADTF_BANDCHECK"] = cBaseChoose;

    params["SF_FRAGCHECK"] = aPDSwitch;
    params["KAF_FRAGCHECK"] = bPDSwitch;
    params["KADTF_FRAGCHECK"] = cPDSwitch;

    params["JWC_BANDSIGNAL1"] = aModeChoose;
    params["JWC_BANDSIGNAL2"] = bModeChoose;
    params["JWC_BANDSIGNAL3"] = cModeChoose;

    //给四台跟踪基带下发命令
    cmdSettingGZJD(109, 0x4201, params);
    cmdSettingGZJD(109, 0x4202, params);
    cmdSettingGZJD(109, 0x4203, params);
    cmdSettingGZJD(109, 0x4204, params);

    //给视频切换单元下发命令
    auto aBaseUnitValue = ckJWCUnitValueMap[aBaseChoose];
    auto bBaseUnitValue = ckJWCUnitValueMap[bBaseChoose];
    auto cBaseUnitValue = scJWCUnitValueMap[cBaseChoose];
    params.clear();
    params["SBandSelectOut"] = aBaseUnitValue;
    params["XBandSelectOut"] = bBaseUnitValue;
    params["KaBandSelectOut"] = cBaseUnitValue;

    unitSetting(1, 0x4301, params);
}

void AntennaControlWidget4426::cmdSettingGZJD(int cmdID, int deviceIDInt, QMap<QString, QVariant> paramMap)
{
    if (!isOnlineGZJD())
    {
        return;
    }

    DeviceID deviceID(deviceIDInt);
    auto modeID = GlobalData::getOnlineDeviceModeId(deviceID);

    CmdRequest cmdRequest;
    cmdRequest.m_systemNumb = deviceID.sysID;
    cmdRequest.m_deviceNumb = deviceID.devID;
    cmdRequest.m_extenNumb = deviceID.extenID;
    cmdRequest.m_modeID = modeID;
    cmdRequest.m_cmdID = cmdID;
    cmdRequest.m_paramdataMap = paramMap;

    JsonWriter writer;
    writer& cmdRequest;
    RedisHelper::getInstance().setData("过程控制命令发送数据", writer.GetQString());
    qDebug() << writer.GetString();
    emit signalsCmdDeviceJson(writer.GetString());
}

void AntennaControlWidget4426::unitSetting(int unitID, int deviceIDInt, QMap<QString, QVariant> paramMap)
{
    QString errorMsg;
    if (!ServiceCheck::serviceOnline())
    {
        errorMsg = QString("当前服务器离线");
        SystemLogPublish::errorMsg(errorMsg);
        ui->statusLabel->setText(errorMsg);
        QColor color(Qt::red);
        ui->statusLabel->setStyleSheet(QString(".QLabel{color: rgb(%1, %2, %3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        return;
    }

    DeviceID deviceID(deviceIDInt);
    auto modeID = GlobalData::getOnlineDeviceModeId(deviceID);

    if (!GlobalData::getDeviceOnline(deviceID))
    {
        auto deviceName = GlobalData::getExtensionName(deviceID.sysID, deviceID.devID, deviceID.extenID);
        errorMsg = QString("%1设备离线").arg(deviceName);
        SystemLogPublish::errorMsg(errorMsg);
        ui->statusLabel->setText(errorMsg);
        QColor color(Qt::red);
        ui->statusLabel->setStyleSheet(QString(".QLabel{color: rgb(%1, %2, %3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        return;
    }

    UnitParamRequest unitParamRequest;

    unitParamRequest.m_systemNumb = deviceID.sysID;
    unitParamRequest.m_deviceNumb = deviceID.devID;
    unitParamRequest.m_extenNumb = deviceID.extenID;
    unitParamRequest.m_modeID = modeID;
    unitParamRequest.m_unitID = unitID;
    unitParamRequest.m_paramdataMap = paramMap;

    JsonWriter writer;
    writer& unitParamRequest;
    RedisHelper::getInstance().setData("单元发送数据", writer.GetQString());
    qDebug() << writer.GetString();
    emit signalsUnitDeviceJson(writer.GetString());
}

void AntennaControlWidget4426::cmdSettingACU(int cmdID, QMap<QString, QVariant> paramMap, int modeID)
{
    if (!isOnlineACU())
    {
        return;
    }

    CmdRequest cmdRequest;
    cmdRequest.m_systemNumb = m_useACUDeviceID.sysID;
    cmdRequest.m_deviceNumb = m_useACUDeviceID.devID;
    cmdRequest.m_extenNumb = m_useACUDeviceID.extenID;
    cmdRequest.m_modeID = modeID;
    cmdRequest.m_cmdID = cmdID;
    cmdRequest.m_paramdataMap = paramMap;

    JsonWriter writer;
    writer& cmdRequest;
    RedisHelper::getInstance().setData("过程控制命令发送数据", writer.GetQString());
    qDebug() << writer.GetString();
    emit signalsCmdDeviceJson(writer.GetString());
}

void AntennaControlWidget4426::timerEvent(QTimerEvent* /*event*/)
{
    auto statusReportMsg = GlobalData::getExtenStatusReportDataByObject(0x1011);
    double AZ = statusReportMsg.unitReportMsgMap[1].paramMap["CurrentAZ"].toDouble();
    double PA = statusReportMsg.unitReportMsgMap[1].paramMap["CurrentPA"].toDouble();

    ui->acuAzimuthLab->setText(QString::number(AZ, 'g', 5));
    ui->acuPitchLab->setText(QString::number(PA, 'g', 5));

    //处理校相是否能控
    auto sCKLock = statusReportMsg.unitReportMsgMap[2].paramMap["STrackLockInst"].toInt();
    auto kaCKLock = statusReportMsg.unitReportMsgMap[2].paramMap["XKaTrackLockInst"].toInt();
    auto kaSCLock = statusReportMsg.unitReportMsgMap[2].paramMap["XKaSCTrackLockInst"].toInt();

    //程引和数引是否可控
    auto szYD = statusReportMsg.unitReportMsgMap[2].paramMap["ValidArgumData"].toInt();
    auto cxYD = statusReportMsg.unitReportMsgMap[2].paramMap["ValidLeadData"].toInt();

    if (sCKLock == 1)
    {
        ui->oneKeyXXSBtn->setEnabled(true);
    }
    else
    {
        ui->oneKeyXXSBtn->setEnabled(false);
    }

    if (kaCKLock == 1)
    {
        ui->oneKeyXXKaYcBtn->setEnabled(true);
    }
    else
    {
        ui->oneKeyXXKaYcBtn->setEnabled(false);
    }

    if (kaSCLock == 1)
    {
        ui->oneKeyXXKaScBtn->setEnabled(true);
    }
    else
    {
        ui->oneKeyXXKaScBtn->setEnabled(false);
    }

    if (szYD == 1)
    {
        ui->numberLeadBtn->setEnabled(true);
    }
    else
    {
        ui->numberLeadBtn->setEnabled(false);
    }

    if (cxYD == 1)
    {
        ui->programLeadBtn->setEnabled(true);
    }
    else
    {
        ui->programLeadBtn->setEnabled(false);
    }
}

void AntennaControlWidget4426::showEvent(QShowEvent* /*event*/)
{
    if (m_timerID == -1)
    {
        m_timerID = startTimer(1000);
    }
    solt();
}

void AntennaControlWidget4426::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    if (m_timerID != -1)
    {
        killTimer(m_timerID);
        m_timerID = -1;
    }
}

// bool AntennaControlWidget4426::serverCheck()
//{
//    if (!SERVICEONLINE)
//    {
//        QMessageBox::information(nullptr, QString("提示"), QString("当前服务器离线"), QString("确定"));
//        return false;
//    }
//    else
//    {
//        return true;
//    }
//}
