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

    if (m_statusBarThread == nullptr)
    {
        m_statusBarThread = new StatusBarManagerThread;
        connect(m_statusBarThread, &StatusBarManagerThread::signalRefreshUI, this, &StatusBarManagerWidget::slotRefreshUI);
        m_statusBarThread->start();
    }
    ui->serverMasterLabel->setHidden(true);;
}

StatusBarManagerWidget::~StatusBarManagerWidget()
{
    if (m_statusBarThread)
    {
        m_statusBarThread->stopRunning();
        m_statusBarThread->quit();
        m_statusBarThread->wait();
        delete m_statusBarThread;
        m_statusBarThread = nullptr;
    }

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

    auto& offlineDeviceList = statusBarData.offlineDeviceList;
    auto& subCtrlDeviceList = statusBarData.subCtrlDeviceList;

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

    // 更新离线设备
    if (!offlineDeviceList.isEmpty())
    {
        if (mCurrentOfflineIndex >= offlineDeviceList.size())
        {
            mCurrentOfflineIndex = 0;
        }
        ui->offlineDeviceScrollWidget->setCurrentText(offlineDeviceList.at(mCurrentOfflineIndex));
        auto str = offlineDeviceList.at(mCurrentOfflineIndex) + "离线";
        RedisHelper::getInstance().publistMusic(str);
        ++mCurrentOfflineIndex;
    }
    else
    {
        ui->offlineDeviceScrollWidget->setCurrentText("");
    }

    // 更新分控设备
    if (!subCtrlDeviceList.isEmpty())
    {
        if (mCurrentSubCtrlIndex >= subCtrlDeviceList.size())
        {
            mCurrentSubCtrlIndex = 0;
        }

        ui->subCtrlDeviceScrollWidget->setCurrentText(subCtrlDeviceList.at(mCurrentSubCtrlIndex));
        auto str = subCtrlDeviceList.at(mCurrentSubCtrlIndex) + "分控";
        RedisHelper::getInstance().publistMusic(str);
        ++mCurrentSubCtrlIndex;
    }
    else
    {
        ui->subCtrlDeviceScrollWidget->setCurrentText("");
    }

    // 更新S和Ka校零变频器加去电状态
    statusBarData.SZeroCalibrationOn ? ui->sZeroCalibrationLabel->setStyleSheet(greenQss) : ui->sZeroCalibrationLabel->setStyleSheet(grayQss);
    statusBarData.KaZeroCalibrationOn ? ui->kaZeroCalibrationLabel->setStyleSheet(greenQss) : ui->kaZeroCalibrationLabel->setStyleSheet(grayQss);
}
