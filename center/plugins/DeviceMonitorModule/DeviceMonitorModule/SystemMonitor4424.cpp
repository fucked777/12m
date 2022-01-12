#include "SystemMonitor4424.h"
#include "ui_SystemMonitor4424.h"

#include <QComboBox>
#include <QListWidgetItem>

#include "CustomPacketMessageDefine.h"
#include "DataExchangeSystemWidget.h"
#include "DeviceMonitor.h"
#include "GlobalData.h"
#include "QssCommonHelper.h"

ParamRelationManager SystemMonitor4424::m_paraRelation;

SystemMonitor4424::SystemMonitor4424(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SystemMonitor4424)
{
    ui->setupUi(this);

    connect(ui->deviceList, &QListWidget::clicked, this, &SystemMonitor4424::deviceChange);
}

SystemMonitor4424::~SystemMonitor4424() { delete ui; }

void SystemMonitor4424::setSystem(System currentSystem) { m_currentSystem = currentSystem; }

void SystemMonitor4424::initSystemWidget()
{
    //初始化qss
    QString fileName = "SystemMonitor.qss";
    QssCommonHelper::setWidgetStyle(this, fileName);

    bool flag = false;
    for (auto devID : m_currentSystem.deviceMap.keys())
    {
        auto device = m_currentSystem.deviceMap.value(devID);
        for (auto extenID : device.extensionMap.keys())
        {
            auto extension = device.extensionMap.value(extenID);
            //这里知道分机信息就可以创建界面
            DeviceID deviceID;
            deviceID.sysID = m_currentSystem.ID;
            deviceID.devID = devID;
            deviceID.extenID = extenID;

            if (extension.modeCtrlMap.size() > 0)
            {
                m_deviceMode[deviceID] = extension.modeCtrlMap.first().ID;
            }
            else
            {
                m_deviceMode[deviceID] = -1;
            }

            if (!flag)
            {
                m_currentDeviceID = deviceID;  //只记录当前分系统第一个设备的deviceID
                flag = true;
            }

            m_deviceNameMap[deviceID] = extension.desc;
        }
    }
    initDeviceList();
    initDeviceStackWidget();

    if (m_currentDeviceID.sysID == 4)
    {
        ui->comboBox->addItem("标准", 1);
        ui->comboBox->addItem("S扩二", 4);
        ui->comboBox->addItem("Ka扩二", 5);
        ui->comboBox->addItem("一体化（上面级）", 10);
        ui->comboBox->addItem("一体化（卫星）", 11);
        ui->comboBox->addItem("一体化（上行高码率）", 12);
        ui->comboBox->addItem("一体化（上面+扩二不共载波）", 13);
        ui->comboBox->addItem("一体化（上面+扩二共载波）", 15);
        ui->comboBox->addItem("一体化（卫星+扩二）", 17);
        ui->comboBox->addItem("一体化（上行高码率+扩二）", 19);
        ui->comboBox->addItem("扩跳", 21);
        ui->comboBox->addItem("标准TTC(跟踪)", 1);
        ui->comboBox->addItem("扩频TTC(跟踪)", 2);
        ui->comboBox->addItem("测控数传一体化跟踪基带(跟踪)", 3);
        ui->comboBox->addItem("数传跟踪基带(跟踪)", 4);
        ui->comboBox->addItem("标准TTC+数传跟踪基带(跟踪)", 5);
        ui->comboBox->addItem("扩频TTC+数传跟踪基带(跟踪)", 6);
        ui->comboBox->addItem("测控数传一体化+数传跟踪基带(跟踪)", 7);
        ui->comboBox->addItem("测控数传一体化+扩频跟踪基带（跟踪）", 8);
        ui->comboBox->addItem("扩跳频跟踪基带(跟踪)", 9);
        ui->comboBox->addItem("标准+低速", "A");
        ui->comboBox->addItem("扩频+低速", "B");
        ui->comboBox->addItem("低速跟踪基带", "C");
    }
    else if (m_currentDeviceID.sysID == 6)
    {
        ui->comboBox->addItem("标准TTC", 1);
        ui->comboBox->addItem("扩频TTC", 2);
        ui->comboBox->addItem("一体化上面级", 3);
        ui->comboBox->addItem("一体化卫星", 4);
        ui->comboBox->addItem("一体化上面级+扩频共载波", 5);
        ui->comboBox->addItem("一体化上面级+扩频不共载波", 6);
        ui->comboBox->addItem("一体化卫星+扩二", 7);
        ui->comboBox->addItem("一体化上行高码率", 8);
    }
    else
    {
        ui->comboBox->hide();
    }
    connect(ui->comboBox, &QComboBox::currentTextChanged, this, &SystemMonitor4424::modeChanged);
    //    ui->comboBox->hide();
}

void SystemMonitor4424::setDeviceID(const int deviceID)
{
    int deviceId = deviceID;
    m_currentDeviceID << deviceId;

    if (!m_deviceWidgetMaps.contains(m_currentDeviceID))
    {
        createOtherExtentWidget(m_currentDeviceID);  //当点击到其他设备的分机时才创建界面
    }

    if (m_deviceWidgetMaps.contains(m_currentDeviceID) && m_deviceMode.contains(m_currentDeviceID))
    {
        int modeID = m_deviceMode[m_currentDeviceID];
        ui->deviceStackWidget->setCurrentWidget(m_deviceWidgetMaps[m_currentDeviceID][modeID]);
    }

    for (int i = 0; i < ui->deviceList->count(); i++)
    {
        QVariant variant = ui->deviceList->item(i)->data(Qt::UserRole + 1);
        if (variant.toInt() == deviceID)
        {
            ui->deviceList->setCurrentRow(i);
        }
    }
}

int SystemMonitor4424::getDeviceID() const
{
    int deviceId;
    m_currentDeviceID >> deviceId;
    return deviceId;
}

void SystemMonitor4424::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event)

    auto statusReportMsg = GlobalData::getExtenStatusReportData(m_currentDeviceID);
    if (statusReportMsg.modeId == -1)
    {
        return;
    }

    auto reportModeId = statusReportMsg.modeId;
    if (m_currentDeviceID.sysID == 8 && m_currentDeviceID.devID == 0)
    {
        auto currentWidget = qobject_cast<DataExchangeSystemWidget*>(ui->deviceStackWidget->currentWidget());
        if (currentWidget != nullptr)
        {
            currentWidget->slotsExtenStatusReport(statusReportMsg);
        }
    }
    else
    {
        auto currentWidget = qobject_cast<DeviceMonitor*>(ui->deviceStackWidget->currentWidget());
        if (currentWidget)
        {
            //当前界面的模式
            auto currentDeviceModeID = currentWidget->currentModeID();
            //进行模式切换
            if (reportModeId != currentDeviceModeID)
            {
                if (m_deviceWidgetMaps.contains(m_currentDeviceID) && m_deviceMode.contains(m_currentDeviceID))
                {
                    if (m_deviceWidgetMaps[m_currentDeviceID].contains(reportModeId))
                    {
                        //对模式进行赋值操作
                        m_deviceMode[m_currentDeviceID] = reportModeId;
                        ui->deviceStackWidget->setCurrentWidget(m_deviceWidgetMaps[m_currentDeviceID][reportModeId]);
                        //模式切换完成后就进行数据更新
                        currentWidget = qobject_cast<DeviceMonitor*>(ui->deviceStackWidget->currentWidget());
                        if (currentWidget)
                        {
                            currentWidget->slotsExtenStatusReport(statusReportMsg);
                        }
                    }
                    else if (m_currentSystem.deviceMap[m_currentDeviceID.devID].extensionMap[m_currentDeviceID.extenID].modeCtrlMap.contains(
                                 reportModeId))
                    {
                        m_deviceMode[m_currentDeviceID] = reportModeId;
                        auto modeCtrl = m_currentSystem.deviceMap[m_currentDeviceID.devID].extensionMap[m_currentDeviceID.extenID].modeCtrlMap.value(
                            reportModeId);
                        createDeviceWidget(modeCtrl);
                        ui->deviceStackWidget->setCurrentWidget(m_deviceWidgetMaps[m_currentDeviceID][reportModeId]);
                        currentWidget = qobject_cast<DeviceMonitor*>(ui->deviceStackWidget->currentWidget());
                        if (currentWidget)
                        {
                            currentWidget->slotsExtenStatusReport(statusReportMsg);
                        }
                    }
                }
            }
            else
            {
                //更新界面 如果和选中模式相同就更新界面
                currentWidget->slotsExtenStatusReport(statusReportMsg);
            }
        }
    }
}

/* 计时器的开关 */
void SystemMonitor4424::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (m_timerID == -1)
    {
        m_timerID = startTimer(1000);
    }
}
void SystemMonitor4424::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    if (m_timerID != -1)
    {
        killTimer(m_timerID);
        m_timerID = -1;
    }
}

void SystemMonitor4424::initDeviceList()
{
    for (auto key : m_deviceNameMap.keys())
    {
        QListWidgetItem* item = new QListWidgetItem;
        item->setText(m_deviceNameMap[key]);
        int deviceID = 0;
        key >> deviceID;
        item->setData(Qt::UserRole + 1, deviceID);
        ui->deviceList->addItem(item);
    }
}

void SystemMonitor4424::initDeviceStackWidget()
{
    for (auto devID : m_currentSystem.deviceMap.keys())
    {
        auto device = m_currentSystem.deviceMap.value(devID);
        for (auto extenID : device.extensionMap.keys())
        {
            auto extension = device.extensionMap.value(extenID);
            //这里知道分机信息就可以创建界面
            DeviceID deviceID;
            deviceID.sysID = m_currentSystem.ID;
            deviceID.devID = devID;
            deviceID.extenID = extenID;

            for (auto modeID : extension.modeCtrlMap.keys())
            {
                auto modeCtrl = extension.modeCtrlMap.value(modeID);

                // 创建数据储存与交互表格
                if (deviceID.sysID == 8 && deviceID.devID == 0 && deviceID.extenID == 1)
                {
                    auto dataExchangeWidget = new DataExchangeSystemWidget();
                    dataExchangeWidget->setDeviceID(deviceID);
                    dataExchangeWidget->setModeCtrl(modeCtrl);
                    dataExchangeWidget->initWidget();

                    connect(dataExchangeWidget, &DataExchangeSystemWidget::signalsCmdDeviceJson, this, &SystemMonitor4424::signalsCmdDeviceJson);

                    ui->deviceStackWidget->addWidget(dataExchangeWidget);
                    m_deviceWidgetMaps[deviceID][modeID] = dataExchangeWidget;
                }
                else
                {
                    DeviceMonitor* deviceMonitor = new DeviceMonitor();

                    //设置设备ID
                    deviceMonitor->setDeviceID(deviceID);
                    deviceMonitor->setModeCtrl(modeCtrl);
                    deviceMonitor->initWidget();

                    connect(deviceMonitor, &DeviceMonitor::signalsUnitDeviceJson, this, &SystemMonitor4424::signalsUnitDeviceJson);

                    connect(deviceMonitor, &DeviceMonitor::signalsCmdDeviceJson, this, &SystemMonitor4424::signalsCmdDeviceJson);

                    connect(deviceMonitor, &DeviceMonitor::signalsParaMacroDeviceJson, this, &SystemMonitor4424::signalsParaMacroDeviceJson);

                    //设置设备ID
                    // deviceMonitor->setDeviceID(deviceID);

                    ui->deviceStackWidget->addWidget(deviceMonitor);
                    m_deviceWidgetMaps[deviceID][modeID] = deviceMonitor;
                    //添加单元关联关系
                    m_paraRelation.addUnit(deviceMonitor);
                    //添加过程控制命令关联关系
                    connect(deviceMonitor, &DeviceMonitor::signalInitCmdFinished, &m_paraRelation, &ParamRelationManager::addCmd);

                    if ((m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0) ||
                        (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 2) ||
                        (m_currentDeviceID.sysID == 6 && m_currentDeviceID.devID == 0))
                        return;  // 这里是只创建测控基带，跟踪基带，联试应答机三个设备的第一个模式界面
                }
            }
            return;  //这里是只创建第一个分机的界面，让运行的时候更流畅
        }
    }
}

void SystemMonitor4424::createOtherExtentWidget(DeviceID otherDeviceID)
{
    for (auto devID : m_currentSystem.deviceMap.keys())
    {
        auto device = m_currentSystem.deviceMap.value(devID);
        for (auto extenID : device.extensionMap.keys())
        {
            auto extension = device.extensionMap.value(extenID);
            //这里知道分机信息就可以创建界面
            DeviceID deviceID;
            deviceID.sysID = m_currentSystem.ID;
            deviceID.devID = devID;
            deviceID.extenID = extenID;

            if (deviceID.sysID == otherDeviceID.sysID && deviceID.devID == otherDeviceID.devID && deviceID.extenID == otherDeviceID.extenID)
            {
                for (auto modeID : extension.modeCtrlMap.keys())
                {
                    auto modeCtrl = extension.modeCtrlMap.value(modeID);

                    if (deviceID.sysID == 8 && deviceID.devID == 0 && deviceID.extenID == 2)
                    {
                        auto dataExchangeWidget = new DataExchangeSystemWidget();

                        dataExchangeWidget->setDeviceID(deviceID);
                        dataExchangeWidget->setModeCtrl(modeCtrl);
                        dataExchangeWidget->initWidget();

                        connect(dataExchangeWidget, &DataExchangeSystemWidget::signalsCmdDeviceJson, this, &SystemMonitor4424::signalsCmdDeviceJson);

                        ui->deviceStackWidget->addWidget(dataExchangeWidget);
                        m_deviceWidgetMaps[deviceID][modeID] = dataExchangeWidget;
                    }
                    else
                    {
                        DeviceMonitor* deviceMonitor = new DeviceMonitor();

                        //设置设备ID
                        deviceMonitor->setDeviceID(deviceID);
                        deviceMonitor->setModeCtrl(modeCtrl);
                        deviceMonitor->initWidget();

                        connect(deviceMonitor, &DeviceMonitor::signalsUnitDeviceJson, this, &SystemMonitor4424::signalsUnitDeviceJson);

                        connect(deviceMonitor, &DeviceMonitor::signalsCmdDeviceJson, this, &SystemMonitor4424::signalsCmdDeviceJson);

                        connect(deviceMonitor, &DeviceMonitor::signalsParaMacroDeviceJson, this, &SystemMonitor4424::signalsParaMacroDeviceJson);

                        //设置设备ID
                        // deviceMonitor->setDeviceID(deviceID);

                        ui->deviceStackWidget->addWidget(deviceMonitor);
                        m_deviceWidgetMaps[deviceID][modeID] = deviceMonitor;

                        //添加单元关联关系
                        m_paraRelation.addUnit(deviceMonitor);
                        //添加过程控制命令关联关系
                        connect(deviceMonitor, &DeviceMonitor::signalInitCmdFinished, &m_paraRelation, &ParamRelationManager::addCmd);

                        if ((m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0) ||
                            (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 2) ||
                            (m_currentDeviceID.sysID == 6 && m_currentDeviceID.devID == 0))
                            return;  // 这里是只创建测控基带，跟踪基带，联试应答机三个设备的第一个模式界面
                    }
                }
            }
        }
    }
}

void SystemMonitor4424::createDeviceWidget(ModeCtrl modeCtrl)
{
    DeviceMonitor* deviceMonitor = new DeviceMonitor();

    //设置设备ID
    deviceMonitor->setDeviceID(m_currentDeviceID);
    deviceMonitor->setModeCtrl(modeCtrl);
    deviceMonitor->initWidget();

    connect(deviceMonitor, &DeviceMonitor::signalsUnitDeviceJson, this, &SystemMonitor4424::signalsUnitDeviceJson);

    connect(deviceMonitor, &DeviceMonitor::signalsCmdDeviceJson, this, &SystemMonitor4424::signalsCmdDeviceJson);

    connect(deviceMonitor, &DeviceMonitor::signalsParaMacroDeviceJson, this, &SystemMonitor4424::signalsParaMacroDeviceJson);

    //设置设备ID
    // deviceMonitor->setDeviceID(deviceID);

    ui->deviceStackWidget->addWidget(deviceMonitor);
    m_deviceWidgetMaps[m_currentDeviceID][modeCtrl.ID] = deviceMonitor;

    //添加单元关联关系
    m_paraRelation.addUnit(deviceMonitor);
    //添加过程控制命令关联关系
    connect(deviceMonitor, &DeviceMonitor::signalInitCmdFinished, &m_paraRelation, &ParamRelationManager::addCmd);
}

void SystemMonitor4424::deviceChange(const QModelIndex& index)
{
    int deviceID = index.data(Qt::UserRole + 1).toInt();

    m_currentDeviceID << deviceID;

    if (!m_deviceWidgetMaps.contains(m_currentDeviceID))
    {
        createOtherExtentWidget(m_currentDeviceID);  //当点击到其他设备的分机时才创建界面
    }

    if (m_deviceWidgetMaps.contains(m_currentDeviceID) && m_deviceMode.contains(m_currentDeviceID))
    {
        int modeID = m_deviceMode[m_currentDeviceID];
        ui->deviceStackWidget->setCurrentWidget(m_deviceWidgetMaps[m_currentDeviceID][modeID]);
    }
}

void SystemMonitor4424::modeChanged(const QString& text)
{
    auto data = ui->comboBox->currentData().toString();  //选中的模式
    auto modeID = data.toInt(0, 16);
    if ((m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0) || (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 2) ||
        (m_currentDeviceID.sysID == 6 && m_currentDeviceID.devID == 0))
    {
        if (m_deviceWidgetMaps[m_currentDeviceID].contains(modeID))
        {
            auto widget = m_deviceWidgetMaps[m_currentDeviceID][modeID];
            if (widget)
            {
                m_deviceMode[m_currentDeviceID] = modeID;
                ui->deviceStackWidget->setCurrentWidget(widget);
            }
        }
        else if (m_currentSystem.deviceMap[m_currentDeviceID.devID].extensionMap[m_currentDeviceID.extenID].modeCtrlMap.contains(modeID))
        {
            m_deviceMode[m_currentDeviceID] = modeID;
            auto modeCtrl = m_currentSystem.deviceMap[m_currentDeviceID.devID].extensionMap[m_currentDeviceID.extenID].modeCtrlMap.value(modeID);
            createDeviceWidget(modeCtrl);
            auto widget = m_deviceWidgetMaps[m_currentDeviceID][modeID];
            if (widget)
            {
                m_deviceMode[m_currentDeviceID] = modeID;
                ui->deviceStackWidget->setCurrentWidget(widget);
            }
        }
    }
}

void SystemMonitor4424::slotsControlCmdResponseJson(ControlCmdResponse cmdRespnce)
{
    if (cmdRespnce.deviceID == m_currentDeviceID)
    {
        if (cmdRespnce.deviceID.sysID == 8 && cmdRespnce.deviceID.devID == 0)
        {
            auto currentWidget = qobject_cast<DataExchangeSystemWidget*>(ui->deviceStackWidget->currentWidget());
            if (currentWidget)
            {
                currentWidget->slotsControlCmdResponse(cmdRespnce);
            }
        }
        else
        {
            auto currentWidget = qobject_cast<DeviceMonitor*>(ui->deviceStackWidget->currentWidget());
            if (currentWidget)
            {
                currentWidget->slotsControlCmdResponse(cmdRespnce);
            }
        }
    }
}

// void SystemMonitor::slotsStatusReportJson(const StatusReportingRequest& statusReport)
//{
//    auto sysID = statusReport.m_systemNumb;
//    auto devID = statusReport.m_deviceNumb;
//    auto extentID = statusReport.m_extenNumb;
//    auto modeID = statusReport.m_modeID;

//    if (m_currentDeviceID.devID == devID && m_currentDeviceID.sysID == sysID && m_currentDeviceID.extenID == extentID)
//    {
//        if (sysID == 8 && devID == 0)
//        {
//            auto currentWidget = qobject_cast<DataExchangeSystemWidget*>(ui->deviceStackWidget->currentWidget());
//            if (currentWidget != nullptr)
//            {
//                // currentWidget3->slotsExtenStatusReport(statusReport);
//            }
//        }
//        else
//        {
//            auto currentWidget = qobject_cast<DeviceMonitor*>(ui->deviceStackWidget->currentWidget());
//            if (currentWidget)
//            {
//                //当前界面的模式
//                auto currentDeviceModeID = currentWidget->currentModeID();
//                //进行模式切换
//                if (modeID != currentDeviceModeID)
//                {
//                    if (m_deviceWidgetMaps.contains(m_currentDeviceID) && m_deviceMode.contains(m_currentDeviceID))
//                    {
//                        if (m_deviceWidgetMaps[m_currentDeviceID].contains(modeID))
//                        {
//                            //对模式进行赋值操作
//                            m_deviceMode[m_currentDeviceID] = modeID;
//                            ui->deviceStackWidget->setCurrentWidget(m_deviceWidgetMaps[m_currentDeviceID][modeID]);
//                            //模式切换完成后就进行数据更新
//                            currentWidget = qobject_cast<DeviceMonitor*>(ui->deviceStackWidget->currentWidget());
//                            if (currentWidget)
//                            {
//                                currentWidget->slotsExtenStatusReport(statusReport);
//                            }
//                        }
//                    }
//                    else if
//                    (m_currentSystem.deviceMap[m_currentDeviceID.devID].extensionMap[m_currentDeviceID.extenID].modeCtrlMap.contains(modeID))
//                    {
//                        m_deviceMode[m_currentDeviceID] = modeID;
//                        auto modeCtrl =
//                            m_currentSystem.deviceMap[m_currentDeviceID.devID].extensionMap[m_currentDeviceID.extenID].modeCtrlMap.value(modeID);
//                        createDeviceWidget(modeCtrl);
//                        ui->deviceStackWidget->setCurrentWidget(m_deviceWidgetMaps[m_currentDeviceID][modeID]);
//                        currentWidget = qobject_cast<DeviceMonitor*>(ui->deviceStackWidget->currentWidget());
//                        if (currentWidget)
//                        {
//                            currentWidget->slotsExtenStatusReport(statusReport);
//                        }
//                    }
//                }
//                else
//                {
//                    //更新界面 如果和选中模式相同就更新界面
//                    currentWidget->slotsExtenStatusReport(statusReport);
//                }
//            }
//        }
//    }
//}

void SystemMonitor4424::slotsCmdResultJson(const CmdResult& cmdReport)
{
    auto sysID = cmdReport.m_systemNumb;
    auto devID = cmdReport.m_deviceNumb;
    auto extentID = cmdReport.m_extenNumb;

    if (m_currentDeviceID.devID == devID && m_currentDeviceID.sysID == sysID && m_currentDeviceID.extenID == extentID)
    {
        if (cmdReport.m_systemNumb == 8 && cmdReport.m_deviceNumb == 0)
        {
            auto currentWidget = qobject_cast<DataExchangeSystemWidget*>(ui->deviceStackWidget->currentWidget());
            if (currentWidget != nullptr)
            {
                currentWidget->slotsCmdResult(cmdReport);
            }
        }
        else
        {
            auto currentWidget = qobject_cast<DeviceMonitor*>(ui->deviceStackWidget->currentWidget());
            if (currentWidget != nullptr)
            {
                currentWidget->slotsCmdResult(cmdReport);
            }
        }
    }
}
