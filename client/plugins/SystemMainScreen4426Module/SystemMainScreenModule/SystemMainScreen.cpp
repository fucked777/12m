#include "SystemMainScreen.h"
#include "ui_SystemMainScreen.h"

#include "AntennaControlWidget4426.h"
#include "DevProtocolSerialize.h"
#include "DeviceProcessMessageDefine.h"
#include "ExtendedConfig.h"
#include "StatusBarManagerWidget.h"
#include "SystemParamWidgetContainer.h"
#include "TaskProcessControlWidget.h"
#include <QLabel>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>

SystemMainScreen::SystemMainScreen(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SystemMainScreen)
{
    ui->setupUi(this);
    init();
    setStyleSheet("QPalette {"
                  "background:#EAF7FF;"
                  "}"
                  "*{"
                  "outline:0px;"
                  "color:#386487;"
                  "}");
}

SystemMainScreen::~SystemMainScreen()
{
    if (m_deviceDataLoadThread)
    {
        m_deviceDataLoadThread->stopRunning();
        m_deviceDataLoadThread->wait();
        m_deviceDataLoadThread->quit();
        delete m_statusBarThread;
        m_statusBarThread = nullptr;
    }
    if (m_statusBarThread)
    {
        m_statusBarThread->stopRunning();
        m_statusBarThread->wait();
        m_statusBarThread->quit();
        delete m_statusBarThread;
        m_statusBarThread = nullptr;
    }
    delete ui;
}

void SystemMainScreen::slotsControlCmdResponse(const ControlCmdResponse& cmdResponce)
{
    auto result = DevProtocolEnumHelper::controlCmdResponseToDescStr(cmdResponce.responseResult);
    QMessageBox::warning(this, QString("提示"), QString(result), QString("确定"));
}
void SystemMainScreen::statusBarRefreshUI(const StatusBarData& data)
{
    mTaskProcessControlWidget->slotRefreshUI(data);
    m_statusBarManagerWidget->slotRefreshUI(data);
}

void SystemMainScreen::init()
{
    m_statusBarThread = new StatusBarManagerThread;
    connect(m_statusBarThread, &StatusBarManagerThread::signalRefreshUI, this, &SystemMainScreen::statusBarRefreshUI);
    m_statusBarThread->start();

    m_deviceDataLoadThread = new DeviceDataLoadThread;
    m_deviceDataLoadThread->start();

    ui->antennaCtrlBtn->setToolTip("天线设置");
    ui->taskProcessCtrlBtn->setToolTip("任务流程控制");
    auto* widget = new SystemParamWidgetContainer(this);
    connect(widget, &SystemParamWidgetContainer::signalsCmdDeviceJson, this, &SystemMainScreen::signalsCmdDeviceJson);
    connect(widget, &SystemParamWidgetContainer::signalsUnitDeviceJson, this, &SystemMainScreen::signalsUnitDeviceJson);
    ui->widget->layout()->addWidget(widget);

    m_statusBarManagerWidget = new StatusBarManagerWidget(this);
    m_statusBarManagerWidget->setMaximumHeight(30);
    ui->horizontalLayout_2->addWidget(m_statusBarManagerWidget);


    mAntennaControlWidget4426 = new AntennaControlWidget4426(this);
    mAntennaControlWidget4426->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    mAntennaControlWidget4426->setWindowTitle("天线设置");
    connect(mAntennaControlWidget4426, &AntennaControlWidget4426::signalsCmdDeviceJson, this, &SystemMainScreen::signalsCmdDeviceJson);
    connect(mAntennaControlWidget4426, &AntennaControlWidget4426::signalsUnitDeviceJson, this, &SystemMainScreen::signalsUnitDeviceJson);
    connect(mAntennaControlWidget4426, &AntennaControlWidget4426::signalManualFunction, this, &SystemMainScreen::signalManualFunction);
    connect(this, &SystemMainScreen::signalsDeviceCMDResult, mAntennaControlWidget4426, &AntennaControlWidget4426::slotCmdResult);
    connect(this, &SystemMainScreen::signalsCMDResponceJson, mAntennaControlWidget4426, &AntennaControlWidget4426::slotCmdResponse);
    mAntennaControlWidget4426->hide();

    mTaskProcessControlWidget = new TaskProcessControlWidget(this);
    mTaskProcessControlWidget->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    mTaskProcessControlWidget->setWindowTitle("任务流程控制");
    mTaskProcessControlWidget->hide();

    connect(mTaskProcessControlWidget, &TaskProcessControlWidget::signalManualFunction, this, &SystemMainScreen::signalManualFunction);
    connect(mTaskProcessControlWidget, &TaskProcessControlWidget::signalsCmdDeviceJson, this, &SystemMainScreen::signalsCmdDeviceJson);
    connect(mTaskProcessControlWidget, &TaskProcessControlWidget::signalsUnitDeviceJson, this, &SystemMainScreen::signalsUnitDeviceJson);
    connect(this, &SystemMainScreen::signalsCMDResponceJson, mTaskProcessControlWidget, &TaskProcessControlWidget::slotCmdResponse);

    ui->antennaCtrlBtn->setStyleSheet("QPushButton {"
                                      "min-height:24px;"
                                      "min-width:24px;"
                                      "border-style:none;"
                                      "background-image: url(:/image/antenna_ctrl.png);"
                                      "}"
                                      "QPushButton:hover{"
                                      "min-height:24px;"
                                      "min-width:24px;"
                                      "border-style:none;"
                                      "background-image:  url(:/image/antenna_ctrl_hover.png);"
                                      "}"
                                      "QPushButton:pressed{"
                                      "min-height:24px;"
                                      "min-width:24px;"
                                      "border-style:none;"
                                      "background-image:  url(:/image/antenna_ctrl.png);"
                                      "}");
    ui->taskProcessCtrlBtn->setStyleSheet("QPushButton {"
                                          "min-height:24px;"
                                          "min-width:24px;"
                                          "border-style:none;"
                                          "background-image: url(:/image/task_process_ctrl.png);"
                                          "}"
                                          "QPushButton:hover{"
                                          "min-height:24px;"
                                          "min-width:24px;"
                                          "border-style:none;"
                                          "background-image: url(:/image/task_process_ctrl_hover.png);"
                                          "}"
                                          "QPushButton:pressed{"
                                          "min-height:24px;"
                                          "min-width:24px;"
                                          "border-style:none;"
                                          "background-image: url(:/image/task_process_ctrl.png);"
                                          "}");


    connect(ui->antennaCtrlBtn, &QPushButton::clicked, this, [=]() {
        mAntennaControlWidget4426->setGeometry(width() - mAntennaControlWidget4426->width(), 50, mAntennaControlWidget4426->width(), height());
        mAntennaControlWidget4426->show();
    });

    connect(ui->taskProcessCtrlBtn, &QPushButton::clicked, this, [=]() {
        mTaskProcessControlWidget->setGeometry(width() - mTaskProcessControlWidget->width(), 50, mTaskProcessControlWidget->width(), height());
        mTaskProcessControlWidget->show();
    });
}
