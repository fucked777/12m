#include "AntennaControlWidget4426.h"
#include "AutorunPolicyMessageSerialize.h"
#include "CustomPacketMessageSerialize.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "JsonHelper.h"
#include "ProtocolXmlTypeDefine.h"
#include "RedisHelper.h"
#include "SatelliteManagementSerialize.h"
#include "ServiceCheck.h"
#include "SubscriberHelper.h"
#include "ui_AntennaControlWidget4426.h"
#include <QAbstractItemView>
#include <QButtonGroup>
#include <QCompleter>
#include <QFontDatabase>
#include <QMessageBox>
#include <QStandardItem>
#include <QStandardItemModel>

#define ArrowTopCode    0xe6a5
#define ArrowLeftCode   0xe6a6
#define ArrowRightCode  0xe6a7
#define ArrowBottomCode 0xe6a8

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
}

AntennaControlWidget4426::~AntennaControlWidget4426() { delete ui; }

void AntennaControlWidget4426::setCKPlanData(QVariantMap& params)
{
    auto workMode = std::get<0>(m_currentCKPlanData);
    auto taskCode = std::get<1>(m_currentCKPlanData);
    auto point = std::get<2>(m_currentCKPlanData);
    if (m_taskCodeSateliteMap.contains(taskCode))
    {
        auto satelite = m_taskCodeSateliteMap[taskCode];
        SystemOrientation orientation;
        satelite.getSReceivPolar(workMode, point, orientation);
        auto taskIdent = satelite.m_satelliteIdentification;
        params["TaskIdent"] = taskIdent;  //任务标识

        if (orientation == SystemOrientation::Unkonwn || orientation == SystemOrientation::LRCircumflex)
        {
            orientation = SystemOrientation::LCircumflex;
        }
        params["SEmissPolar"] = (int)orientation;     // S接收极化
        params["KaYCEmissPolar"] = (int)orientation;  // Ka遥测接收极化
        params["KaSCEmissPolar"] = (int)orientation;  // Ka数传接收极化
    }
    else
    {
        params["TaskIdent"] = "12A7";  //任务标识
        params["SEmissPolar"] = 1;     //接收极化
        params["KaYCEmissPolar"] = 1;  // Ka遥测接收极化
        params["KaSCEmissPolar"] = 1;  // Ka数传接收极化
    }
}

void AntennaControlWidget4426::setGSOrDSPlanData(QVariantMap& params)
{
    auto taskGSCode = std::get<1>(m_currentGSPlanData);
    auto taskDSCode = std::get<1>(m_currentDSPlanData);
    if (!taskGSCode.isEmpty() && !taskDSCode.isEmpty())
    {
        //在高速和低速都存在时，高速和低速都是放在一个卫星里的，只有这一种情况，所以只用一个任务代号就能拿到卫星
        auto satelite = m_taskCodeSateliteMap[taskGSCode];
        auto taskIdent = satelite.m_satelliteIdentification;
        SystemOrientation orientation;
        params["TaskIdent"] = taskIdent;  //任务标识
        //高速和低速存在时需要判断跟踪方式来选择旋向
        auto trackWay = satelite.m_trackingMode;
        if (trackWay == _4426_KaGSDT || trackWay == _4426_STTC_KaGSDT || trackWay == _4426_STTC_KaCK_KaGSDT || trackWay == _4426_SK2_KaGSDT ||
            trackWay == _4426_SK2_KaCK_KaGSDT)
        {
            auto workMode = std::get<0>(m_currentGSPlanData);
            auto point = std::get<2>(m_currentGSPlanData);
            satelite.getSReceivPolar(workMode, point, orientation);
            if (orientation == SystemOrientation::Unkonwn || orientation == SystemOrientation::LRCircumflex)
            {
                orientation = SystemOrientation::LCircumflex;
            }
            params["SEmissPolar"] = (int)orientation;     // S接收极化
            params["KaSCEmissPolar"] = (int)orientation;  // Ka数传接收极化
            params["KaYCEmissPolar"] = (int)orientation;  // Ka遥测接收极化
        }
        else
        {
            auto workMode = std::get<0>(m_currentDSPlanData);
            auto point = std::get<2>(m_currentDSPlanData);
            satelite.getSReceivPolar(workMode, point, orientation);
            if (orientation == SystemOrientation::Unkonwn || orientation == SystemOrientation::LRCircumflex)
            {
                orientation = SystemOrientation::LCircumflex;
            }
            params["SEmissPolar"] = (int)orientation;     // S接收极化
            params["KaSCEmissPolar"] = (int)orientation;  // Ka数传接收极化
            params["KaYCEmissPolar"] = (int)orientation;  // Ka遥测接收极化
        }
    }
    else if (!taskGSCode.isEmpty())
    {
        auto satelite = m_taskCodeSateliteMap[taskGSCode];
        auto taskIdent = satelite.m_satelliteIdentification;
        SystemOrientation orientation;
        params["TaskIdent"] = taskIdent;  //任务标识
        auto workMode = std::get<0>(m_currentGSPlanData);
        auto point = std::get<2>(m_currentGSPlanData);
        satelite.getSReceivPolar(workMode, point, orientation);
        if (orientation == SystemOrientation::Unkonwn || orientation == SystemOrientation::LRCircumflex)
        {
            orientation = SystemOrientation::LCircumflex;
        }
        params["SEmissPolar"] = (int)orientation;     // S接收极化
        params["KaSCEmissPolar"] = (int)orientation;  // Ka数传接收极化
        params["KaYCEmissPolar"] = (int)orientation;  // Ka遥测接收极化
    }
    else if (!taskDSCode.isEmpty())
    {
        auto satelite = m_taskCodeSateliteMap[taskDSCode];
        auto taskIdent = satelite.m_satelliteIdentification;
        SystemOrientation orientation;
        params["TaskIdent"] = taskIdent;  //任务标识
        auto workMode = std::get<0>(m_currentDSPlanData);
        auto point = std::get<2>(m_currentDSPlanData);
        satelite.getSReceivPolar(workMode, point, orientation);
        if (orientation == SystemOrientation::Unkonwn || orientation == SystemOrientation::LRCircumflex)
        {
            orientation = SystemOrientation::LCircumflex;
        }
        params["SEmissPolar"] = (int)orientation;     // S接收极化
        params["KaSCEmissPolar"] = (int)orientation;  // Ka数传接收极化
        params["KaYCEmissPolar"] = (int)orientation;  // Ka遥测接收极化
    }
    else
    {
        params["TaskIdent"] = "12A7";  //任务标识
        params["SEmissPolar"] = 1;     // S接收极化
        params["KaSCEmissPolar"] = 1;  // Ka数传接收极化
        params["KaYCEmissPolar"] = 1;  // Ka遥测接收极化
    }
}

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
        // 去掉小数后多于的0
        QRegExp rx(R"((\.){0,1}0+$)");
        QString DateArrival = params.value("DateArrival").toString();
        QString ArrivalTime = params.value("ArrivalTime").toString();
        ui->inStationTimeLab->setText(QString("%1 %2").arg(DateArrival).arg(ArrivalTime));
        QVariant ApprPosit = params.value("ApprPosit");
        auto ApprPositString = QString("%1").arg(ApprPosit.toDouble(), 0, 'f', 6).replace(rx, "");
        ui->inStationAzimuthEdit->setText(ApprPositString);
        QVariant PitchPitch = params.value("PitchPitch");
        auto PitchPitchString = QString("%1").arg(PitchPitch.toDouble(), 0, 'f', 6).replace(rx, "");
        ui->inStationPitchEdit->setText(PitchPitchString);

        QString OverDate = params.value("OverDate").toString();
        QString OvershTime = params.value("OvershTime").toString();
        ui->overTopTimeLab->setText(QString("%1 %2").arg(OverDate).arg(OvershTime));
        QVariant OutBDirect = params.value("OutBDirect");
        auto OutBDirectString = QString("%1").arg(OutBDirect.toDouble(), 0, 'f', 6).replace(rx, "");
        ui->overTopAzimuthEdit->setText(OutBDirectString);
        QVariant OutBPitch = params.value("OutBPitch");
        auto OutBPitchString = QString("%1").arg(OutBPitch.toDouble(), 0, 'f', 6).replace(rx, "");
        ui->overTopPitchEdit->setText(OutBPitchString);

        QString DateDepart = params.value("DateDepart").toString();
        QString OutBTime = params.value("OutBTime").toString();
        ui->outStationTimeLab->setText(QString("%1 %2").arg(DateDepart).arg(OutBTime));
        QVariant OvershAzim = params.value("OvershAzim");
        auto OvershAzimString = QString("%1").arg(OvershAzim.toDouble(), 0, 'f', 6).replace(rx, "");
        ui->outStationAzimuthEdit->setText(OvershAzimString);
        QVariant OvershPitch = params.value("OvershPitch");
        auto OvershPitchString = QString("%1").arg(OvershPitch.toDouble(), 0, 'f', 6).replace(rx, "");
        ui->outStationPitchEdit->setText(OvershPitchString);
        break;
    }
    default: break;
    }
}

void AntennaControlWidget4426::initUI()
{
    connect(ui->acuRunModeCheckBox, &QCheckBox::clicked, [this](bool checked) {
        ui->manualRadioBtn->setEnabled(checked);
        ui->autoRadiobtn->setEnabled(checked);
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
    startTimer(1000);
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
    ui->acuAzimuthSpeedEdit->setRange(0.0, 10.0);
    ui->acuAzimuthSpeedEdit->setToolTip(QString("当前输入范围为0~10"));
    ui->acuPitchSpeedEdit->setRange(0.0, 8.0);
    ui->acuPitchSpeedEdit->setToolTip(QString("当前输入范围为0~8"));

    ui->inStationAzimuthEdit->setEnabled(false);
    ui->inStationPitchEdit->setEnabled(false);
    ui->overTopAzimuthEdit->setEnabled(false);
    ui->overTopPitchEdit->setEnabled(false);
    ui->outStationAzimuthEdit->setEnabled(false);
    ui->outStationPitchEdit->setEnabled(false);

    ui->acuAzimuthSpeedEdit->setValue(1.0);
    ui->acuPitchSpeedEdit->setValue(1.0);

    // 1:人工 2:自动
    auto btnGroup = new QButtonGroup(this);
    btnGroup->addButton(ui->manualRadioBtn, 1);
    btnGroup->addButton(ui->autoRadiobtn, 2);
    ui->manualRadioBtn->setChecked(true);
    ui->autoRadiobtn->setChecked(false);
    connect(btnGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &AntennaControlWidget4426::acuRunMode);

    ui->predictionComBox->addItem("历元时间预报", 1);
    ui->predictionComBox->addItem("指定时间段预报", 2);

    ui->gdybStartDateTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
    ui->gdybEndDateTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");

    //开始时间比当前快5分钟  结束时间比当前快65分钟
    auto currentStartDateTime = GlobalData::currentDateTime().addSecs(5 * 60);
    auto currentEndDateTime = GlobalData::currentDateTime().addSecs(65 * 60);

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
    QString ckPlanData;
    QString dsscPlanData;
    QString gsscPlanData;
    RedisHelper::getInstance().getData("CKWorkTaskCodePoint", ckPlanData);
    RedisHelper::getInstance().getData("DSSCWorkTaskCodePoint", dsscPlanData);
    RedisHelper::getInstance().getData("GSSCWorkTaskCodePoint", gsscPlanData);
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

/* 待机 */
void AntennaControlWidget4426::standby() { cmdSetting(2); }
/* 指向 */
void AntennaControlWidget4426::point()
{
    QVariantMap params;
    params.insert("DirectMode", 3);
    params.insert("DesigAzimu", ui->acuAzimuthLab->text().toDouble());
    params.insert("DesigElevat", ui->acuPitchLab->text().toDouble());
    params.insert("DesigAzimVel", ui->acuAzimuthSpeedEdit->value());
    params.insert("SpePitAngVel", ui->acuPitchSpeedEdit->value());

    cmdSetting(3, params);
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

    cmdSetting(3, params);
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

    cmdSetting(3, params);
}

/* 数字引导 */
void AntennaControlWidget4426::numberLead() { cmdSetting(4); }
/* 程序引导 */
void AntennaControlWidget4426::programLead() { cmdSetting(5); }
/* 收藏不插锁 */
void AntennaControlWidget4426::collection()
{
    QVariantMap params;
    params.insert("LockInPlaceNot", 2);

    cmdSetting(9, params);
}
/* 拔锁 */
void AntennaControlWidget4426::unlock() { cmdSetting(10); }

/* 任务开始 */
void AntennaControlWidget4426::taskStart()
{
    QVariantMap params;
    params.insert("TaskIdent", ui->comboBox_2->currentText());

    cmdSetting(15, params);
}

/* s跟踪 */
void AntennaControlWidget4426::sTrack1() { cmdSetting(8); }

// void AntennaControlWidget4426::kaMeasureTrack1() {} /* ka测控跟踪 */

/* ka数传跟踪 */
void AntennaControlWidget4426::kaDataTransTrack() { cmdSetting(7); }

void AntennaControlWidget4426::kaYCTrack() { cmdSetting(6); }

/* acu运行方式 */
void AntennaControlWidget4426::acuRunMode()
{
    QVariantMap params;
    params.insert("OperatMode", 1);
    params.insert("OperatMode", 2);

    cmdSetting(39, params);
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

    cmdSetting(27, params);
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

bool AntennaControlWidget4426::isOnline()
{
    if (!ServiceCheck::serviceOnline())
    {
        ui->statusLabel->setText("当前监控服务器离线");
        QColor color(Qt::red);
        ui->statusLabel->setStyleSheet(QString(".QLabel{color: rgb(%1, %2, %3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        return false;
    }
    //判断哪台ACU在线 因为ACU不可能两台同时在线 判断一个就可以了
    auto onlineA = GlobalData::getDeviceOnline(0x1011);
    int deviceIDInt = onlineA ? 0x1011 : 0x1012;
    auto isOnline = GlobalData::getDeviceOnline(deviceIDInt);
    if (!isOnline)
    {
        ui->statusLabel->setText("当前设备离线");
        QColor color(Qt::red);
        ui->statusLabel->setStyleSheet(QString(".QLabel{color: rgb(%1, %2, %3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        return false;
    }

    QColor color(Qt::black);
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

    cmdSetting(3, params);
}

void AntennaControlWidget4426::comboxRedal(QComboBox* combox)
{
    combox->clear();
    QStandardItemModel* model = new QStandardItemModel(this);  //源数据
    QList<QStandardItem*> row;
    QStringList list;
    SatelliteManagementDataList dataList;
    GlobalData::getSatelliteManagementData(dataList);

    if (!dataList.size())
    {
        QMessageBox::information(this, "提示", "获取卫星数据失败", "确定");
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

    cmdSetting(41, params);
}

void AntennaControlWidget4426::solt()
{
    comboxRedal(ui->comboBox_1);
    comboxRedal(ui->comboBox_2);
    comboxRedal(ui->gdybTaskIdentCbx);

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
    AutorunPolicyData autoRunPolicyMap;
    GlobalData::getAutorunPolicyData(autoRunPolicyMap);
    //任务代号自动从当前任务获取,这个先不管,后面会提供一个接口
    //注意：实际下发参数的是任务代号对应的任务标识 所以需要从卫星里根据当前任务代号再获取标识
    QVariantMap params;
    getCurrentPlanData();     //获取当前任务计划数据
    setCKPlanData(params);    //测控都一样，随便取一个测控的数据就行
    params["EquipNumb"] = 0;  //设备组合号从对应的配置宏获取  暂时为0
    params["FreqBand"] = 1;   // S频段
    params["PhaseCtrl"] = 1;  //校相控制  开始校相
    // params["SEmissPolar"] = ;//S接收极化 从当前任务对应的任务代号里找极化  Ka数传和Ka遥测极化不管
    params["PhaseCorrMeth"] = autoRunPolicyMap.policy["PCaM"];             //校相方式
    params["FollowCheck"] = autoRunPolicyMap.policy["S_FollowCheck"];      // S的跟踪检查
    params["CrossLowLimit"] = autoRunPolicyMap.policy["S_CrossLowLimit"];  // S的交叉耦合下限值（分母）
    params["StandSensit"] = autoRunPolicyMap.policy["S_StandSensit"];      // S的定向灵敏度标准值
    params["DirectToler"] = autoRunPolicyMap.policy["S_DirectToler"];      // S的定向灵敏度允差

    cmdSetting(34, params);
}

void AntennaControlWidget4426::slotOneKeyXXKaYc()
{
    AutorunPolicyData autoRunPolicyMap;
    GlobalData::getAutorunPolicyData(autoRunPolicyMap);
    //任务代号自动从当前任务获取,这个先不管,后面会提供一个接口
    //注意：实际下发参数的是任务代号对应的任务标识 所以需要从卫星里根据当前任务代号再获取标识
    QVariantMap params;
    //设备组合号也是从当前任务那获取 待定
    getCurrentPlanData();     //获取当前任务计划数据
    setCKPlanData(params);    //测控都一样，随便取一个测控的数据就行
    params["EquipNumb"] = 0;  //设备组合号
    params["FreqBand"] = 2;   // Ka遥测
    params["PhaseCtrl"] = 1;  //校相控制  开始校相
    // params["KaYCEmissPolar"] = ;//Ka遥测接收极化 从当前任务对应的任务代号里找极化  Ka数传和S频段不管
    params["PhaseCorrMeth"] = autoRunPolicyMap.policy["PCaM"];              //校相方式
    params["FollowCheck"] = autoRunPolicyMap.policy["KA_FollowCheck"];      // Ka遥测的跟踪检查
    params["CrossLowLimit"] = autoRunPolicyMap.policy["KA_CrossLowLimit"];  // Ka遥测的交叉耦合下限值（分母）
    params["StandSensit"] = autoRunPolicyMap.policy["KA_StandSensit"];      // Ka遥测的定向灵敏度标准值
    params["DirectToler"] = autoRunPolicyMap.policy["KA_DirectToler"];      // Ka遥测的定向灵敏度允差

    cmdSetting(34, params);
}

void AntennaControlWidget4426::slotOneKeyXXKaSc()
{
    AutorunPolicyData autoRunPolicyMap;
    GlobalData::getAutorunPolicyData(autoRunPolicyMap);
    //任务代号自动从当前任务获取,这个先不管,后面会提供一个接口
    //注意：实际下发参数的是任务代号对应的任务标识 所以需要从卫星里根据当前任务代号再获取标识
    QVariantMap params;
    getCurrentPlanData();
    setGSOrDSPlanData(params);  //获取高低速数据，其中旋向需要根据跟踪方式来判断
    params["EquipNumb"] = 0;    //设备组合号
    params["FreqBand"] = 3;     // Ka数传
    params["PhaseCtrl"] = 1;    //校相控制  开始校相
    // params["KaSCEmissPolar"] = ;//Ka数传接收极化 从当前任务对应的任务代号里找极化  S频段和Ka遥测不管
    params["PhaseCorrMeth"] = autoRunPolicyMap.policy["PCaM"];                //校相方式
    params["FollowCheck"] = autoRunPolicyMap.policy["KADT_FollowCheck"];      // Ka数传的跟踪检查
    params["CrossLowLimit"] = autoRunPolicyMap.policy["KADT_CrossLowLimit"];  // Ka数传的交叉耦合下限值（分母）
    params["StandSensit"] = autoRunPolicyMap.policy["KADT_StandSensit"];      // Ka数传的定向灵敏度标准值
    params["DirectToler"] = autoRunPolicyMap.policy["KADT_DirectToler"];      // Ka数传的定向灵敏度允差

    cmdSetting(34, params);
}

void AntennaControlWidget4426::cmdSetting(int cmdID, QMap<QString, QVariant> paramMap)
{
    if (!isOnline())
    {
        return;
    }

    //判断哪台ACU在线 因为ACU不可能两台同时在线 判断一个就可以了
    auto onlineA = GlobalData::getDeviceOnline(0x1011);

    int deviceIDInt = onlineA ? 0x1011 : 0x1012;
    DeviceID deviceID(deviceIDInt);

    CmdRequest cmdRequest;
    cmdRequest.m_systemNumb = deviceID.sysID;
    cmdRequest.m_deviceNumb = deviceID.devID;
    cmdRequest.m_extenNumb = deviceID.extenID;
    cmdRequest.m_modeID = 65535;
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
    auto statusReportMsg = GlobalData::getExtenStatusReportData(0x1011);
    double AZ = statusReportMsg.unitReportMsgMap[1].paramMap["CurrentAZ"].toDouble();
    double PA = statusReportMsg.unitReportMsgMap[1].paramMap["CurrentPA"].toDouble();

    ui->acuAzimuthLab->setText(QString::number(AZ, 'g', 5));
    ui->acuPitchLab->setText(QString::number(PA, 'g', 5));

    auto sCKLock = statusReportMsg.unitReportMsgMap[2].paramMap["STrackLockInst"].toInt();
    auto kaCKLock = statusReportMsg.unitReportMsgMap[2].paramMap["XKaTrackLockInst"].toInt();
    auto kaSCLock = statusReportMsg.unitReportMsgMap[2].paramMap["XKaSCTrackLockInst"].toInt();

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
}

void AntennaControlWidget4426::showEvent(QShowEvent* /*event*/) { solt(); }

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
