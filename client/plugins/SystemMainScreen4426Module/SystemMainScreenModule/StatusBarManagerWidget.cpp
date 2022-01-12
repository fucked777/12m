#include "StatusBarManagerWidget.h"
#include "ui_StatusBarManagerWidget.h"

#include "GlobalData.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "UserManagerMessageSerialize.h"
#include "RedisHelper.h"

static constexpr const char* greenQss = "QLabel {"
                                        "background-image: url(:/image/green16.png);"
                                        "background-position:center;"
                                        "background-repeat:no-repeat;"
                                        "border-style: none;"
                                        "}";

static constexpr const char* grayQss = "QLabel {"
                                       "background-image: url(:/image/gray16.png);"
                                       "background-position:center;"
                                       "background-repeat:no-repeat;"
                                       "border-style: none;"
                                       "}";

StatusBarManagerWidget::StatusBarManagerWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StatusBarManagerWidget)
{
    ui->setupUi(this);

    init();

    User currentuser;
    GlobalData::setCurrentUser(currentuser);
    ui->label_2->setHidden(true);
    ui->serverMasterLabel->setHidden(true);;
}

StatusBarManagerWidget::~StatusBarManagerWidget()
{
    delete ui;
}

void StatusBarManagerWidget::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)
    mShow = true;
}

void StatusBarManagerWidget::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event)
    mShow = false;
}

void StatusBarManagerWidget::init()
{
    ui->offlineDeviceScrollWidget->setStyleSheet("QLabel {color: rgb(225, 50, 50);}");
    ui->subCtrlDeviceScrollWidget->setStyleSheet("QLabel {color: rgb(225, 50, 50);}");
}

void StatusBarManagerWidget::slotRefreshUI(const StatusBarData& statusBarData)
{
    if (!mShow)
    {
        return;
    }

    // 当前用户
    auto currentuser = GlobalData::getCurrentUser();
    QString identity;
    if ("0" == currentuser.identity)
        identity = QString("监视员");
    else if ("1" == currentuser.identity)
        identity = QString("操作员");
    else if ("2" == currentuser.identity)
        identity = QString("管理员");
    else
        identity = QString("未知身份");
    ui->currentUserLabel->setText(currentuser.name + "(" + identity + ")");

    auto& serverMasterText = statusBarData.serverMasterText;

    if (!SERVICEONLINE)
    {
        ui->serverOnlineLedLabel->setStyleSheet(grayQss);

        ui->offlineDeviceScrollWidget->setCurrentText("");
        ui->subCtrlDeviceScrollWidget->setCurrentText("");

        ui->serverMasterLabel->setText("");

        return;
    }
    else
    {
        ui->serverOnlineLedLabel->setStyleSheet(greenQss);
        ui->serverMasterLabel->setText(serverMasterText);
    }

    auto& offlineDeviceMap = statusBarData.deviceIDOffName;
    auto& subCtrlDeviceMap = statusBarData.deviceIDSubName;

    // 更新离线设备
    if (!offlineDeviceMap.isEmpty())
    {
        if (mCurrentOfflineIndex > offlineDeviceMap.size())
        {
            mCurrentOfflineIndex = 0;
        }
        mCurrentOfflineIndex++;
        int offFlag = 0;
        for (auto deviceID : offlineDeviceMap.keys())
        {
            offFlag++;
            if (mCurrentOfflineIndex == offFlag)
            {
                auto deviceName = offlineDeviceMap[deviceID];
                ui->offlineDeviceScrollWidget->setCurrentText(deviceName);
                auto str = deviceName + "离线";
                RedisHelper::getInstance().publistMusic(str);
                break;//每次只读取一个值  保持刷新速度
            }
        }
    }
    else
    {
        ui->offlineDeviceScrollWidget->setCurrentText("");
    }

    // 更新分控设备
    if (!subCtrlDeviceMap.isEmpty())
    {
        if (mCurrentSubCtrlIndex > subCtrlDeviceMap.size())
        {
            mCurrentSubCtrlIndex = 0;
        }
        mCurrentSubCtrlIndex++;
        int subFlag = 0;
        for (auto deviceID : subCtrlDeviceMap.keys())
        {
            subFlag++;
            if (mCurrentSubCtrlIndex == subFlag)
            {
                auto deviceName = subCtrlDeviceMap[deviceID];
                ui->subCtrlDeviceScrollWidget->setCurrentText(deviceName);
                auto str = deviceName + "分控";
                RedisHelper::getInstance().publistMusic(str);
                break; //每次只读取一个值  保持刷新速度
            }
        }
    }
    else
    {
        ui->subCtrlDeviceScrollWidget->setCurrentText("");
    }

    // 更新S和Ka校零变频器加去电状态
    statusBarData.SZeroCalibrationOn ? ui->sZeroCalibrationLabel->setStyleSheet(greenQss) : ui->sZeroCalibrationLabel->setStyleSheet(grayQss);
    statusBarData.KaZeroCalibrationOn ? ui->kaZeroCalibrationLabel->setStyleSheet(greenQss) : ui->kaZeroCalibrationLabel->setStyleSheet(grayQss);
}
