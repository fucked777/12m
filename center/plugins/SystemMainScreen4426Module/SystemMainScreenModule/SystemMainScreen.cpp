#include "SystemMainScreen.h"
#include "ui_SystemMainScreen.h"

#include "AntennaControlWidget4424.h"
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
        m_deviceDataLoadThread->stopRunning();
    delete ui;
}

void SystemMainScreen::slotsControlCmdResponse(const ControlCmdResponse& cmdResponce)
{
    auto result = DevProtocolEnumHelper::controlCmdResponseToDescStr(cmdResponce.responseResult);
    QMessageBox::warning(this, QString("提示"), QString(result), QString("确定"));
}

void SystemMainScreen::init()
{
    m_deviceDataLoadThread = new DeviceDataLoadThread();
    m_deviceDataLoadThread->start();
    ui->antennaCtrlBtn->setToolTip("天线设置");
    ui->taskProcessCtrlBtn->setToolTip("任务流程控制");
    auto* widget = new SystemParamWidgetContainer(this);
    connect(widget, &SystemParamWidgetContainer::signalsCmdDeviceJson, this, &SystemMainScreen::signalsCmdDeviceJson);
    connect(widget, &SystemParamWidgetContainer::signalsUnitDeviceJson, this, &SystemMainScreen::signalsUnitDeviceJson);
    ui->widget->layout()->addWidget(widget);

    auto statusBarManagerWidget = new StatusBarManagerWidget(this);
    statusBarManagerWidget->setMaximumHeight(30);
    ui->horizontalLayout_2->addWidget(statusBarManagerWidget);

    m_CurProject = ExtendedConfig::curProjectID();
    if (m_CurProject == "4424")
    {
        mAntennaControlWidget4424 = new AntennaControlWidget4424(this);
        mAntennaControlWidget4424->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
        mAntennaControlWidget4424->setWindowTitle("天线设置");
        connect(mAntennaControlWidget4424, &AntennaControlWidget4424::signalsCmdDeviceJson, this, &SystemMainScreen::signalsCmdDeviceJson);
        connect(mAntennaControlWidget4424, &AntennaControlWidget4424::signalsUnitDeviceJson, this, &SystemMainScreen::signalsUnitDeviceJson);
        mAntennaControlWidget4424->hide();
    }
    else
    {
        mAntennaControlWidget4426 = new AntennaControlWidget4426(this);
        mAntennaControlWidget4426->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
        mAntennaControlWidget4426->setWindowTitle("天线设置");
        connect(mAntennaControlWidget4426, &AntennaControlWidget4426::signalsCmdDeviceJson, this, &SystemMainScreen::signalsCmdDeviceJson);
        connect(mAntennaControlWidget4426, &AntennaControlWidget4426::signalsUnitDeviceJson, this, &SystemMainScreen::signalsUnitDeviceJson);
        connect(mAntennaControlWidget4426, &AntennaControlWidget4426::signalManualFunction, this, &SystemMainScreen::signalManualFunction);
        connect(this, &SystemMainScreen::signalsDeviceCMDResult, mAntennaControlWidget4426, &AntennaControlWidget4426::slotCmdResult);
        connect(this, &SystemMainScreen::signalsCMDResponceJson, mAntennaControlWidget4426, &AntennaControlWidget4426::slotCmdResponse);
        mAntennaControlWidget4426->hide();
    }

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

    if (m_CurProject == "4424")
    {
        connect(ui->antennaCtrlBtn, &QPushButton::clicked, this, [=]() {
            mAntennaControlWidget4424->setGeometry(width() - mAntennaControlWidget4424->width(), 50, mAntennaControlWidget4424->width(), height());
            mAntennaControlWidget4424->show();
        });
    }
    else
    {
        connect(ui->antennaCtrlBtn, &QPushButton::clicked, this, [=]() {
            mAntennaControlWidget4426->setGeometry(width() - mAntennaControlWidget4426->width(), 50, mAntennaControlWidget4426->width(), height());
            mAntennaControlWidget4426->show();
        });
    }

    connect(ui->taskProcessCtrlBtn, &QPushButton::clicked, this, [=]() {
        mTaskProcessControlWidget->setGeometry(width() - mTaskProcessControlWidget->width(), 50, mTaskProcessControlWidget->width(), height());
        mTaskProcessControlWidget->show();
    });
}
