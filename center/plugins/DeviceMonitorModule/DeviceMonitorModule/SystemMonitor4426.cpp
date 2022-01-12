#include "SystemMonitor4426.h"
#include "ui_SystemMonitor4426.h"

#include <QComboBox>
#include <QListWidgetItem>

#include "CustomPacketMessageDefine.h"
#include "DataExchangeSystemWidget.h"
#include "DeviceMonitor.h"
#include "GlobalData.h"
#include "QssCommonHelper.h"

ParamRelationManager SystemMonitor4426::m_paraRelation;

SystemMonitor4426::SystemMonitor4426(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SystemMonitor4426)
{
    ui->setupUi(this);
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &SystemMonitor4426::deviceChange);
}

SystemMonitor4426::~SystemMonitor4426() { delete ui; }

void SystemMonitor4426::setSystem(System currentSystem) { m_currentSystem = currentSystem; }

void SystemMonitor4426::initSystemWidget()
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

            m_relationNameIsShowDevice[extension.relationDevice][deviceID] = extension.isShow;

            if (extension.modeCtrlMap.size() > 0)
            {
                m_deviceMode[deviceID] = extension.modeCtrlMap.first().ID;
                for (auto modeID : extension.modeCtrlMap.keys())
                {
                    auto modeCtrl = extension.modeCtrlMap[modeID];
                    m_deviceModeNameMap[deviceID][extension.desc][modeID] = modeCtrl.desc;
                }
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

            // m_deviceNameMap[deviceID] = extension.desc;
        }
    }
    initDeviceList();
    initDeviceStackWidget();

    //这个4426界面用不着测控的下拉框来换界面了，因为它创建的时候全部模式就有了，只需要点击不同的树节点就会有界面创建
    //前提是把服务器关掉，或者测控基带不在线也行，因为在线的话就会把在线的工作模式树节点打开，不在线的工作模式树节点隐藏
    if (m_currentDeviceID.sysID == 4)
    {
        ui->comboBox->addItem("标准", 1);
        ui->comboBox->addItem("S扩二", 4);
        ui->comboBox->addItem("Ka扩二", 5);
        ui->comboBox->addItem("扩跳", 21);
        ui->comboBox->addItem("标准TTC(跟踪)", 1);
        ui->comboBox->addItem("扩频TTC(跟踪)", 2);
        ui->comboBox->addItem("测控数传一体化跟踪基带(跟踪)", 3);
        ui->comboBox->addItem("数传跟踪基带(跟踪)", 4);
        ui->comboBox->addItem("标准TTC+数传跟踪基带(跟踪)", 5);
        ui->comboBox->addItem("扩频TTC+数传跟踪基带(跟踪)", 6);
        ui->comboBox->addItem("扩跳频跟踪基带(跟踪)", 9);
        ui->comboBox->addItem("标准+低速", "A");
        ui->comboBox->addItem("扩频+低速", "B");
        ui->comboBox->addItem("低速跟踪基带", "C");
    }
    else if (m_currentDeviceID.sysID == 6)
    {
        ui->comboBox->addItem("标准TTC", 1);
        ui->comboBox->addItem("扩频TTC", 2);
    }
    else
    {
        ui->comboBox->hide();
    }
    connect(ui->comboBox, &QComboBox::currentTextChanged, this, &SystemMonitor4426::modeChanged);
    ui->comboBox->hide();
}

void SystemMonitor4426::setDeviceID(const int deviceID)
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
        //这里必须用for循环来获得deviceIDS，然后通过item里的data的设备ID来和m_currentDeviceID进行比较，因为m_deviceModeTreeItem里第一个key比如0x4001
        //它后面的map里面存的是4个0x4001的树节点以及4个0x4002的树节点，所以必须拿树节点的设备ID来进行比较
        bool flag = false;
        if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0)
        {
            for (auto deviceIDS : m_deviceModeTreeItem.keys())
            {
                auto modeTreeMap = m_deviceModeTreeItem[deviceIDS];
                for (auto item : modeTreeMap.keys())
                {
                    auto deviceIDInt = item->data(0, Qt::UserRole + 1).toInt();
                    DeviceID deviceItemID(deviceIDInt);
                    auto modeIDOther = modeTreeMap[item];
                    if (deviceItemID == m_currentDeviceID && modeID == modeIDOther)
                    {
                        ui->treeWidget->setCurrentItem(item);
                        flag = true;
                        break;
                    }
                    flag = false;
                }
                if (flag)
                    break;
            }
        }
        else
        {
            for (auto deviceIDS : m_deviceTreeItem.keys())
            {
                auto item = m_deviceTreeItem[deviceIDS];
                auto deviceIDInt = item->data(0, Qt::UserRole + 1).toInt();
                DeviceID deviceItemID(deviceIDInt);
                if (deviceItemID == m_currentDeviceID)
                {
                    ui->treeWidget->setCurrentItem(item);
                    break;
                }
            }
        }

        ui->deviceStackWidget->setCurrentWidget(m_deviceWidgetMaps[m_currentDeviceID][modeID]);
    }
}

int SystemMonitor4426::getDeviceID() const
{
    int deviceId;
    m_currentDeviceID >> deviceId;
    return deviceId;
}

void SystemMonitor4426::modeChanged(const QString& text)
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
            createDeviceWidget(m_currentDeviceID, modeCtrl);
            auto widget = m_deviceWidgetMaps[m_currentDeviceID][modeID];
            if (widget)
            {
                m_deviceMode[m_currentDeviceID] = modeID;
                ui->deviceStackWidget->setCurrentWidget(widget);
            }
        }
    }
}

void SystemMonitor4426::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event)

    if (m_currentDeviceID.sysID == 8 && m_currentDeviceID.devID == 0)
    {
        auto statusReportMsg = GlobalData::getExtenStatusReportData(m_currentDeviceID);
        if (statusReportMsg.modeId == -1)
        {
            return;
        }
        auto currentWidget = qobject_cast<DataExchangeSystemWidget*>(ui->deviceStackWidget->currentWidget());
        if (currentWidget != nullptr)
        {
            currentWidget->slotsExtenStatusReport(statusReportMsg);
        }
    }
    else if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0)
    {
        DeviceID secondDevoceID;
        secondDevoceID = m_currentDeviceID;
        bool ckFlag1 = false;
        bool ckFlag2 = false;
        if (m_currentDeviceID.extenID == 1)
        {
            secondDevoceID.extenID = 2;
        }
        else if (m_currentDeviceID.extenID == 2)
        {
            secondDevoceID.extenID = 1;
        }
        else if (m_currentDeviceID.extenID == 3)
        {
            secondDevoceID.extenID = 4;
        }
        else if (m_currentDeviceID.extenID == 4)
        {
            secondDevoceID.extenID = 3;
        }
        //表示第一个测控基带软件在线   注：这里的第一第二个软件表示的是同一个测控基带可以启双模式，但不知道到底是单模式还是双模式，都要判断
        if (GlobalData::getDeviceOnline(m_currentDeviceID))
        {
            ckFlag1 = true;
        }
        //表示第二个测控基带软件在线
        if (GlobalData::getDeviceOnline(secondDevoceID))
        {
            ckFlag2 = true;
        }

        ExtensionStatusReportMessage statusReportMsg1;
        ExtensionStatusReportMessage statusReportMsg2;
        if (ckFlag1)
        {
            statusReportMsg1 = GlobalData::getExtenStatusReportData(m_currentDeviceID);
        }

        if (ckFlag2)
        {
            statusReportMsg2 = GlobalData::getExtenStatusReportData(secondDevoceID);
        }

        if ((ckFlag1 && statusReportMsg1.modeId == -1) && (ckFlag2 && statusReportMsg2.modeId == -1))
        {
            return;
        }

        QMap<QTreeWidgetItem*, int> modeItemMap;
        if (m_currentDeviceID.extenID == 1 || m_currentDeviceID.extenID == 2)
        {
            DeviceID ckDevice(0x4001);
            //只要是测控基带分机1或者2的  就只取m_deviceModeTreeItem里的4001树节点结构,因为4002这个大树节点结构早就被隐藏了，取它没用
            modeItemMap = m_deviceModeTreeItem[ckDevice];
        }
        else if (m_currentDeviceID.extenID == 3 || m_currentDeviceID.extenID == 4)
        {
            DeviceID ckDevice(0x4003);
            //只要是测控基带分机3或者4的  就只取m_deviceModeTreeItem里的4003树节点结构,因为4004这个大树节点结构早就被隐藏了，取它没用
            modeItemMap = m_deviceModeTreeItem[ckDevice];
        }
        //这里的意思是如果一台测控基带两个软件都不在线那么久不刷新数据了   方便离线调试
        if (!ckFlag1 && !ckFlag2)
        {
            return;
        }

        for (auto item : modeItemMap.keys())
        {
            auto deviceIDInt = item->data(0, Qt::UserRole + 1).toInt();
            DeviceID deviceItemID(deviceIDInt);
            auto modeID = modeItemMap[item];
            if (deviceItemID == m_currentDeviceID && ckFlag1)  //对第一个软件做判断
            {
                auto reportModeId = statusReportMsg1.modeId;
                if (modeID == reportModeId)
                {
                    item->setHidden(false);
                    if (m_deviceWidgetMaps.contains(deviceItemID) && m_deviceWidgetMaps[deviceItemID].contains(modeID))
                    {
                        m_deviceMode[deviceItemID] = modeID;
                        auto widget = m_deviceWidgetMaps[deviceItemID][modeID];
                        auto currentWidget = qobject_cast<DeviceMonitor*>(widget);
                        currentWidget->slotsExtenStatusReport(statusReportMsg1);
                        if (!ckFlag1 || !ckFlag2)  //表明单模式时再显示当前模式界面
                        {
                            ui->deviceStackWidget->setCurrentWidget(widget);
                        }
                    }
                    else
                    {
                        m_deviceMode[deviceItemID] = modeID;
                        auto modeCtrl = m_currentSystem.deviceMap[deviceItemID.devID].extensionMap[deviceItemID.extenID].modeCtrlMap[modeID];
                        createDeviceWidget(deviceItemID, modeCtrl);
                        auto widget = m_deviceWidgetMaps[deviceItemID][modeID];
                        auto currentWidget = qobject_cast<DeviceMonitor*>(widget);
                        currentWidget->slotsExtenStatusReport(statusReportMsg1);
                    }
                }
                else
                {
                    item->setHidden(true);
                }
            }
            else if (deviceItemID == secondDevoceID && ckFlag2)  //对第二个软件做判断
            {
                auto reportModeId = statusReportMsg2.modeId;
                if (modeID == reportModeId)
                {
                    item->setHidden(false);
                    if (m_deviceWidgetMaps.contains(deviceItemID) && m_deviceWidgetMaps[deviceItemID].contains(modeID))
                    {
                        m_deviceMode[deviceItemID] = modeID;
                        auto widget = m_deviceWidgetMaps[deviceItemID][modeID];
                        auto currentWidget = qobject_cast<DeviceMonitor*>(widget);
                        currentWidget->slotsExtenStatusReport(statusReportMsg2);
                        if (!ckFlag1 || !ckFlag2)  //表明单模式时再显示当前模式界面
                        {
                            ui->deviceStackWidget->setCurrentWidget(widget);
                        }
                    }
                    else
                    {
                        m_deviceMode[deviceItemID] = modeID;
                        auto modeCtrl = m_currentSystem.deviceMap[deviceItemID.devID].extensionMap[deviceItemID.extenID].modeCtrlMap[modeID];
                        createDeviceWidget(deviceItemID, modeCtrl);
                        auto widget = m_deviceWidgetMaps[deviceItemID][modeID];
                        auto currentWidget = qobject_cast<DeviceMonitor*>(widget);
                        currentWidget->slotsExtenStatusReport(statusReportMsg2);
                    }
                }
                else
                {
                    item->setHidden(true);
                }
            }
            else
            {
                item->setHidden(true);
            }
        }
    }
    else
    {
        auto statusReportMsg = GlobalData::getExtenStatusReportData(m_currentDeviceID);
        if (statusReportMsg.modeId == -1)
        {
            return;
        }
        auto reportModeId = statusReportMsg.modeId;
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
                        createDeviceWidget(m_currentDeviceID, modeCtrl);
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
void SystemMonitor4426::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (m_timerID == -1)
    {
        m_timerID = startTimer(1000);
    }
}
void SystemMonitor4426::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    if (m_timerID != -1)
    {
        killTimer(m_timerID);
        m_timerID = -1;
    }
}

void SystemMonitor4426::initDeviceList()
{
    ui->treeWidget->setHeaderHidden(true);

    //先把倒三角隐藏掉，有模式再打开
    ui->treeWidget->setRootIsDecorated(false);
    for (auto key : m_deviceModeNameMap.keys())
    {
        auto deviceNameModeName = m_deviceModeNameMap[key];
        for (auto deviceName : deviceNameModeName.keys())
        {
            QTreeWidgetItem* topItem = new QTreeWidgetItem;
            if (deviceName.endsWith("-1"))
            {
                auto name = deviceName;
                name.replace("-1", "");
                topItem->setText(0, name);
            }
            else
            {
                topItem->setText(0, deviceName);
            }
            ui->treeWidget->addTopLevelItem(topItem);
            auto modeNameMap = deviceNameModeName[deviceName];
            int deviceID = 0;
            key >> deviceID;
            topItem->setData(0, Qt::UserRole + 1, deviceID);
            m_deviceTreeItem[deviceID] = topItem;
            //下面是把需要隐藏的分机隐藏掉
            for (auto relationName : m_relationNameIsShowDevice.keys())
            {
                auto deviceIDIsShow = m_relationNameIsShowDevice[relationName];
                if (deviceIDIsShow.contains(deviceID))
                {
                    bool isShow = deviceIDIsShow[deviceID];
                    // isShow为true就显示，false隐藏
                    topItem->setHidden(!isShow);
                }
            }
            //先把第一台测控基带的树节点加上去
            for (auto modeID : modeNameMap.keys())
            {
                auto modeName = modeNameMap[modeID];
                //只有测控基带有双模式存在
                if (modeID != gNoModeType && key.sysID == 4 && key.devID == 0)
                {
                    QTreeWidgetItem* childItem = new QTreeWidgetItem;
                    childItem->setText(0, modeName);
                    topItem->addChild(childItem);
                    m_deviceModeTreeItem[deviceID][childItem] = modeID;
                    childItem->setData(0, Qt::UserRole + 1, deviceID);
                    ui->treeWidget->setRootIsDecorated(true);
                }
            }
            //再把关联设备名称相等的但设备ID不同的第二台测控基带树节点加到第一台去
            for (auto modeID : modeNameMap.keys())
            {
                auto modeName = modeNameMap[modeID];
                //只有测控基带有双模式存在
                if (modeID != gNoModeType && key.sysID == 4 && key.devID == 0)
                {
                    for (auto relationName : m_relationNameIsShowDevice.keys())
                    {
                        auto deviceIDIsShow = m_relationNameIsShowDevice[relationName];
                        if (deviceIDIsShow.contains(key))  //先判断是不是同一个关联设备
                        {
                            for (auto ckDeviceID : deviceIDIsShow.keys())
                            {
                                //如果设备ID不等，说明是第二台测控基带，那么把第二台测控基带的树节点也加入到第一台里
                                if (ckDeviceID == key)
                                {
                                    continue;
                                }
                                else
                                {
                                    int ckDeviceIDInt = 0;
                                    ckDeviceID >> ckDeviceIDInt;
                                    QTreeWidgetItem* otherchildItem = new QTreeWidgetItem;
                                    otherchildItem->setText(0, modeName);
                                    topItem->addChild(otherchildItem);
                                    //这里也用deviceID来当key的原因是将一个测控基带的所有树节点全部放在一起，数据上报的时候才好处理
                                    m_deviceModeTreeItem[deviceID][otherchildItem] = modeID;
                                    otherchildItem->setData(0, Qt::UserRole + 1, ckDeviceIDInt);
                                    ui->treeWidget->setRootIsDecorated(true);
                                    //第二个基带的树节点创建好后先全部隐藏，有数据上报的时候再将其中对应模式的树节点显示出来
                                    otherchildItem->setHidden(true);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //   ui->treeWidget->expandAll();
}

void SystemMonitor4426::initDeviceStackWidget()
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

                    connect(dataExchangeWidget, &DataExchangeSystemWidget::signalsCmdDeviceJson, this, &SystemMonitor4426::signalsCmdDeviceJson);

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

                    connect(deviceMonitor, &DeviceMonitor::signalsUnitDeviceJson, this, &SystemMonitor4426::signalsUnitDeviceJson);

                    connect(deviceMonitor, &DeviceMonitor::signalsCmdDeviceJson, this, &SystemMonitor4426::signalsCmdDeviceJson);

                    connect(deviceMonitor, &DeviceMonitor::signalsParaMacroDeviceJson, this, &SystemMonitor4426::signalsParaMacroDeviceJson);

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

void SystemMonitor4426::createOtherExtentWidget(DeviceID otherDeviceID)
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

                        connect(dataExchangeWidget, &DataExchangeSystemWidget::signalsCmdDeviceJson, this, &SystemMonitor4426::signalsCmdDeviceJson);

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

                        connect(deviceMonitor, &DeviceMonitor::signalsUnitDeviceJson, this, &SystemMonitor4426::signalsUnitDeviceJson);

                        connect(deviceMonitor, &DeviceMonitor::signalsCmdDeviceJson, this, &SystemMonitor4426::signalsCmdDeviceJson);

                        connect(deviceMonitor, &DeviceMonitor::signalsParaMacroDeviceJson, this, &SystemMonitor4426::signalsParaMacroDeviceJson);

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

void SystemMonitor4426::createDeviceWidget(DeviceID deviceID, ModeCtrl modeCtrl)
{
    DeviceMonitor* deviceMonitor = new DeviceMonitor();

    //设置设备ID
    deviceMonitor->setDeviceID(deviceID);
    deviceMonitor->setModeCtrl(modeCtrl);
    deviceMonitor->initWidget();

    connect(deviceMonitor, &DeviceMonitor::signalsUnitDeviceJson, this, &SystemMonitor4426::signalsUnitDeviceJson);

    connect(deviceMonitor, &DeviceMonitor::signalsCmdDeviceJson, this, &SystemMonitor4426::signalsCmdDeviceJson);

    connect(deviceMonitor, &DeviceMonitor::signalsParaMacroDeviceJson, this, &SystemMonitor4426::signalsParaMacroDeviceJson);

    //设置设备ID
    // deviceMonitor->setDeviceID(deviceID);

    ui->deviceStackWidget->addWidget(deviceMonitor);
    m_deviceWidgetMaps[deviceID][modeCtrl.ID] = deviceMonitor;
    ui->deviceStackWidget->setCurrentWidget(deviceMonitor);
    //添加单元关联关系
    m_paraRelation.addUnit(deviceMonitor);
    //添加过程控制命令关联关系
    connect(deviceMonitor, &DeviceMonitor::signalInitCmdFinished, &m_paraRelation, &ParamRelationManager::addCmd);
}

void SystemMonitor4426::deviceChange(QTreeWidgetItem* item, int column)
{
    int deviceID = item->data(column, Qt::UserRole + 1).toInt();

    m_currentDeviceID << deviceID;

    if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0)
    {
        //这里的创建界面只适用于测控基带
        for (auto ckdeviceID : m_deviceModeTreeItem.keys())
        {
            if (ckdeviceID == m_currentDeviceID)
            {
                auto modeTreeMap = m_deviceModeTreeItem[ckdeviceID];
                if (modeTreeMap.contains(item))
                {
                    auto modeID = modeTreeMap[item];
                    if (m_deviceWidgetMaps.contains(m_currentDeviceID) && m_deviceWidgetMaps[m_currentDeviceID].contains(modeID))
                    {
                        ui->deviceStackWidget->setCurrentWidget(m_deviceWidgetMaps[m_currentDeviceID][modeID]);
                    }
                    else
                    {
                        auto modeCtrl =
                            m_currentSystem.deviceMap[m_currentDeviceID.devID].extensionMap[m_currentDeviceID.extenID].modeCtrlMap[modeID];
                        createDeviceWidget(m_currentDeviceID, modeCtrl);
                    }
                }
            }
            else
            {
                auto modeTreeMap = m_deviceModeTreeItem[ckdeviceID];
                if (modeTreeMap.contains(item))
                {
                    auto modeID = modeTreeMap[item];
                    if (m_deviceWidgetMaps.contains(m_currentDeviceID) && m_deviceWidgetMaps[m_currentDeviceID].contains(modeID))
                    {
                        ui->deviceStackWidget->setCurrentWidget(m_deviceWidgetMaps[m_currentDeviceID][modeID]);
                    }
                    else
                    {
                        auto modeCtrl =
                            m_currentSystem.deviceMap[m_currentDeviceID.devID].extensionMap[m_currentDeviceID.extenID].modeCtrlMap[modeID];
                        createDeviceWidget(m_currentDeviceID, modeCtrl);
                    }
                }
            }
        }
    }
    else
    {
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
}

void SystemMonitor4426::slotsControlCmdResponseJson(ControlCmdResponse cmdRespnce)
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

void SystemMonitor4426::slotsCmdResultJson(const CmdResult& cmdReport)
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
