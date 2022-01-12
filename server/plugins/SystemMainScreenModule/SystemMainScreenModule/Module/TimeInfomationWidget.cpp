#include "TimeInfomationWidget.h"
#include "ui_TimeInfomationWidget.h"

#include "GlobalData.h"
#include "GlobalDefine.h"
#include "MessagePublish.h"
#include "TimeMessageDefine.h"

#include <QButtonGroup>
#include <QMessageBox>

TimeInfomationWidget::TimeInfomationWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TimeInfomationWidget)
{
    ui->setupUi(this);

    init();

    setStyleSheet("QLabel[flag='nameLabel']  {"
                  "color: rgb(9, 0, 255);"
                  "font: 75 22pt 方正楷体;"
                  "}");

    connect(mBtnGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &TimeInfomationWidget::slotTimeTypeBtnClicked);

    mTimerID = startTimer(1000);
}

TimeInfomationWidget::~TimeInfomationWidget()
{
    if(mTimerID != -1)
    {
        killTimer(mTimerID);
        mTimerID = -1;
    }
    delete ui;
}

void TimeInfomationWidget::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event)

    // 获取时间信息
    auto timeData = GlobalData::getTimeData();
    ui->BCodeTimeLabel->setText(timeData.bCodeTime.toString(DATETIME_DISPLAY_FORMAT2));

    ui->ntpTimeLabel->setText(timeData.ntpTime.toString(DATETIME_DISPLAY_FORMAT2));
    ui->ntpTimeLabel->setToolTip(QString("NTP服务器：%1").arg(timeData.ntpServerAddr.toString()));

    ui->systemTimeLabel->setText(timeData.serverSystemTime.toString(DATETIME_DISPLAY_FORMAT2));

    // 更新使用时间类型
    auto usedTimeType = (int)timeData.usedTimeType;
    if (!mIsChanging && usedTimeType != mBtnGroup->checkedId())
    {
        auto btn = mBtnGroup->button(usedTimeType);
        if (btn != nullptr)
        {
            btn->setChecked(true);
        }
    }
}

void TimeInfomationWidget::init()
{
    ui->BCodeTimeLabel->setProperty("flag", "nameLabel");
    ui->ntpTimeLabel->setProperty("flag", "nameLabel");
    ui->systemTimeLabel->setProperty("flag", "nameLabel");

    ui->BCodeTimeNameLabel->setProperty("flag", "nameLabel");
    ui->ntpTimeNameLabel->setProperty("flag", "nameLabel");
    ui->systemTimeNameLabel->setProperty("flag", "nameLabel");

    mBtnGroup = new QButtonGroup();
    mBtnGroup->addButton(ui->BCodeRadioBtn, (int)TimeType::BCodeTime);
    mBtnGroup->addButton(ui->ntpRadioBtn, (int)TimeType::NTPTime);
    mBtnGroup->addButton(ui->systemRadioBtn, (int)TimeType::SystemTime);

    // 通过配置获取使用默认使用的时间类型
    auto timeConfigInfo = GlobalData::getTimeConfigInfo();
    GlobalData::setUsedTimeType((TimeType)timeConfigInfo.usedTimeType);
    GlobalData::setUsedTimeTypeRedis((TimeType)timeConfigInfo.usedTimeType);
    if(timeConfigInfo.usedTimeType == TimeType::SystemTime)
    {
        ui->systemRadioBtn->setChecked(true);
    }
    else if(timeConfigInfo.usedTimeType == TimeType::BCodeTime)
    {
        ui->BCodeRadioBtn->setChecked(true);
    }
    else
    {
        ui->ntpRadioBtn->setChecked(true);
    }
}

void TimeInfomationWidget::slotTimeTypeBtnClicked(int id)
{
    // 获取之前的使用时间类型，和原来一样不进行改变
    auto usedTimeType = GlobalData::getUsedTimeType();
    if (usedTimeType == TimeType(id))
    {
        return;
    }

    if(!GlobalData::masterSlave())
    {
        if(usedTimeType == TimeType::SystemTime)
        {
            ui->systemRadioBtn->setChecked(true);
        }
        else if(usedTimeType == TimeType::BCodeTime)
        {
            ui->BCodeRadioBtn->setChecked(true);
        }
        else
        {
            ui->ntpRadioBtn->setChecked(true);
        }
        return;
    }
    mIsChanging = true;
    auto timeTypeName = TimeDataHelper::getTimeTypeString((TimeType)id);
    auto result = QMessageBox::information(this, "提示", QString("您确定要切换为%1吗?").arg(timeTypeName), QString("确定"), QString("取消"));
    if (result != 0)
    {
        mIsChanging = false;
        if(usedTimeType == TimeType::SystemTime)
        {
            ui->systemRadioBtn->setChecked(true);
        }
        else if(usedTimeType == TimeType::BCodeTime)
        {
            ui->BCodeRadioBtn->setChecked(true);
        }
        else
        {
            ui->ntpRadioBtn->setChecked(true);
        }
        return;
    }

    // 设置新的时间类型
    GlobalData::setUsedTimeType(TimeType(id));
    GlobalData::setUsedTimeTypeRedis(TimeType(id));

    TimeConfigInfo info = GlobalData::getTimeConfigInfo();
    info.usedTimeType = TimeType(id);
    GlobalData::setTimeConfigInfo(info);
    // 触发时间配置改变信号，保存新的配置到配置文件
    emit signalTimeConfigInfoChanged(info);

    SystemLogPublish::specificTipsMsg(QString("时间类型切换为%1").arg(timeTypeName));
    mIsChanging = false;
}
