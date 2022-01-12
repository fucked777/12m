#include "DataExchangeSystemWidget.h"
#include "ControlFactory.h"
#include "DevProtocolSerialize.h"
#include "DeviceProcessMessageSerialize.h"
#include "QFlowLayout.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "WNumber.h"
#include "ui_DataExchangeSystemWidget.h"
#include <QHostAddress>

DataExchangeSystemWidget::DataExchangeSystemWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DataExchangeSystemWidget)
{
    ui->setupUi(this);

    mMissionInfoModel = new MissionInfoModel(this);

    mCommStatusModel = new CommunicateStatusModel(this);
    mSoftUnitModel = new SoftUnitModel(this);
    mHostInfoMode = new HostInfoModel(this);
    mNetCardModel = new NetCardModel(this);
    mTcpStatusModel = new TcpStatusModel(this);
    mCenterDataControlModel = new CenterDataControlModel(this);
    mSoftUnitHostModel = new SoftUnitHostModel(this);
    mTaskRunCenterDataModel = new TaskRunCenterDataModel(this);
    mTaskRunBaseNumModel = new TaskRunBaseNumModel(this);

    startTimer(1000);
    ui->isOnline_2->setStyleSheet("font-size:30px;");

    //初始化qss
    QString fileName = "DeviceMonitor.qss";
    QssCommonHelper::setWidgetStyle(this, fileName);
}

DataExchangeSystemWidget::~DataExchangeSystemWidget() { delete ui; }

void DataExchangeSystemWidget::setModeCtrl(ModeCtrl modeCtrl)
{
    m_currentMode = modeCtrl;
    if (m_currentMode.desc.trimmed() == "")
    {
        ui->modeLabel_2->setText(QString("无模式"));
    }
    else
    {
        ui->modeLabel_2->setText(m_currentMode.desc);
    }
}

void DataExchangeSystemWidget::initWidget()
{
    if (m_currentMode.unitMap.isEmpty())
        return;

    // 只有一个单元
    auto unit = m_currentMode.unitMap.value(1);

    m_widgetDeal.setMode(m_currentMode);
    m_widgetDeal.setDeviceID(m_currentDeviceID);

    // 初始化状态参数控件
    initUnitStatusWidget(unit);

    auto unitEnumMap = unit.enumMap;
    for (auto queryResultAttr : unit.queryResultList)
    {
        auto recycleKey = queryResultAttr.id;
        auto targetType = unit.targetData;
        QList<ParameterAttribute> queryRecycleAttrList;
        for (auto key : targetType.keys())
        {
            auto targetData = targetType[key];
            if (targetData.recycleNum == recycleKey)
            {
                queryRecycleAttrList = unit.queryRecycleMap[key];
            }
        }

        if (queryRecycleAttrList.isEmpty())
            continue;

        auto tabName = queryResultAttr.desc;

        if (recycleKey == "MissionNum")
        {
            QStringList headers;
            headers << "任务ID"
                    << "天线"
                    << "控制中心"
                    << "弧段"
                    << "跟踪开始"
                    << "任务结束"
                    << "剩余时间/飞行时间"
                    << "运行状态"
                    << "引导方式"
                    << "上升段T0(起飞时间)";
            int index = -1;
            for (auto& attr : queryRecycleAttrList)
            {
                if (attr.id == "MissionSegment")
                {
                    index = 3;
                }
                else if (attr.id == "MissionRunStatus")
                {
                    index = 7;
                }
                else if (attr.id == "MissionGuideMode")
                {
                    index = 8;
                }
                else
                {
                    continue;
                }

                auto enumIter = unitEnumMap.find(attr.enumType);
                if (enumIter == unitEnumMap.end())
                    continue;

                DataExchangeWidgetDelegate* delegate = new DataExchangeWidgetDelegate(ui->missionInfoTableView);

                delegate->setEnumEntryList(enumIter->emumEntryList);
                ui->missionInfoTableView->setItemDelegateForColumn(index, delegate);
            }
            mMissionInfoModel->setHeaderData(headers);
            ui->missionInfoTableView->setModel(mMissionInfoModel);

            ui->missionInfoTableView->horizontalHeader()->setStretchLastSection(true);
            ui->missionInfoTableView->horizontalHeader()->setHighlightSections(false);
            ui->missionInfoTableView->verticalHeader()->setVisible(false);
            ui->missionInfoTableView->setFrameShape(QFrame::NoFrame);
            ui->missionInfoTableView->setSelectionMode(QAbstractItemView::SingleSelection);

            ui->missionInfoTableView->setColumnWidth(MissionInfoModel::ID, 100);
            ui->missionInfoTableView->setColumnWidth(MissionInfoModel::UAC, 100);
            ui->missionInfoTableView->setColumnWidth(MissionInfoModel::Center, 200);
            ui->missionInfoTableView->setColumnWidth(MissionInfoModel::Segment, 100);
            ui->missionInfoTableView->setColumnWidth(MissionInfoModel::StarTime, 200);
            ui->missionInfoTableView->setColumnWidth(MissionInfoModel::StopTime, 200);
            ui->missionInfoTableView->setColumnWidth(MissionInfoModel::RemainingTime, 150);
            ui->missionInfoTableView->setColumnWidth(MissionInfoModel::RunStatus, 100);
            ui->missionInfoTableView->setColumnWidth(MissionInfoModel::GuideMode, 150);
            ui->missionInfoTableView->setColumnWidth(MissionInfoModel::UpT0, 150);

            mTableNameMap.insert(ui->missionInfoTableView, 1);
        }
        else if (recycleKey == "CommStatusItemNum")
        {
            QStringList headers;
            auto tableView = new QTableView(ui->tabWidget);

            tableView->setModel(mCommStatusModel);

            initTable(unitEnumMap, queryRecycleAttrList, headers, tableView);

            mCommStatusModel->setHeaderData(headers);
            ui->tabWidget->addTab(tableView, tabName);
            mTableNameMap.insert(tableView, 2);
        }
        else if (recycleKey == "SoftUnitItemNum")
        {
            QStringList headers;
            auto tableView = new QTableView(ui->tabWidget);
            tableView->setModel(mSoftUnitModel);

            initTable(unitEnumMap, queryRecycleAttrList, headers, tableView);

            mSoftUnitModel->setHeaderData(headers);
            ui->tabWidget->addTab(tableView, tabName);
            mTableNameMap.insert(tableView, 3);
        }
        else if (recycleKey == "HostNum")
        {
            QStringList headers;
            auto tableView = new QTableView(ui->tabWidget);
            tableView->setModel(mHostInfoMode);

            initTable(unitEnumMap, queryRecycleAttrList, headers, tableView);

            mHostInfoMode->setHeaderData(headers);
            ui->tabWidget->addTab(tableView, tabName);
            mTableNameMap.insert(tableView, 4);
        }
        else if (recycleKey == "NetCardItemNum")
        {
            QStringList headers;
            auto tableView = new QTableView(ui->tabWidget);
            tableView->setModel(mNetCardModel);

            initTable(unitEnumMap, queryRecycleAttrList, headers, tableView);

            mNetCardModel->setHeaderData(headers);
            ui->tabWidget->addTab(tableView, tabName);
            mTableNameMap.insert(tableView, 5);
        }
        else if (recycleKey == "TcpStatusItemNum")
        {
            QStringList headers;
            auto tableView = new QTableView(ui->tabWidget);
            tableView->setModel(mTcpStatusModel);

            initTable(unitEnumMap, queryRecycleAttrList, headers, tableView);

            mTcpStatusModel->setHeaderData(headers);
            ui->tabWidget->addTab(tableView, tabName);
            mTableNameMap.insert(tableView, 6);
        }
        else if (recycleKey == "CenterDataControlItemNum")
        {
            QStringList headers;
            auto tableView = new QTableView(ui->tabWidget);
            tableView->setModel(mCenterDataControlModel);

            initTable(unitEnumMap, queryRecycleAttrList, headers, tableView);

            mCenterDataControlModel->setHeaderData(headers);
            ui->tabWidget->addTab(tableView, tabName);
            mTableNameMap.insert(tableView, 7);
        }
    }

    initOtherRecycleTabWidget(unit);
    ui->stackedWidget->setCurrentWidget(ui->paramPage_2);
}

void DataExchangeSystemWidget::initUnitStatusWidget(Unit& unit)
{
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    bool haveControlCommon = false;  // 如果没有公共参数就不显示公共参数的groupBox
    /* 下面是公共单元的参数
     * **********************************************************/
    for (auto groupKey : unit.statusGroupMap.keys())
    {
        QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
        auto commonAttrList = unit.statusGroupMap[groupKey];
        for (auto attr : commonAttrList)
        {
            if (attr.isShow)
            {
                QMap<QString, QVariant> enumMap;
                if (attr.displayFormat == DisplayFormat_Enum)
                {
                    m_widgetDeal.getEnumData(attr.enumType, unit, enumMap);
                }
                else if (attr.displayFormat == DisplayFormat_LED)
                {
                    m_widgetDeal.getEnumData(attr.enumType, unit, enumMap);
                }
                auto control = ControlFactory::createStatusControl(attr, enumMap);
                if (control)
                {
                    commonLayout->addWidget(control);
                    haveControlCommon = true;
                }
            }
        }
        vboxLayout->addItem(commonLayout);
    }

    ui->statusGroupBox->setLayout(vboxLayout);
}

void DataExchangeSystemWidget::initCmdWidget()
{
    if (!initCmdUI)
    {
        return;
    }
    initCmdUI = false;
    auto comboboxListLayout = new QHBoxLayout();
    comboboxListLayout->setSpacing(6);
    comboboxListLayout->setContentsMargins(6, 6, 6, 6);
    comboboxListLayout->addWidget(new QLabel("过程控制命令选择："));
    QComboBox* cmdListCombox = new QComboBox();
    connect(cmdListCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(cmdCurrentIndexChanged(int)));
    comboboxListLayout->addWidget(cmdListCombox);
    auto comboboxListspacerItem = new QSpacerItem(197, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    comboboxListLayout->addItem(comboboxListspacerItem);

    m_cmdscrollArea = new QScrollArea;

    QWidget* scrollContentWidget = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout;
    for (auto cmdID : m_currentMode.cmdMap.keys())
    {
        CmdAttribute cmdAttribute = m_currentMode.cmdMap.value(cmdID);

        if (!cmdAttribute.isShow)
        {
            continue;
        }

        //判断是否配置了布局
        auto haveLayout = false;
        haveLayout = cmdAttribute.unitLayoutMap.isEmpty() ? false : true;

        QGroupBox* currentWidget = new QGroupBox(cmdAttribute.desc);

        QVBoxLayout* currentLayout = new QVBoxLayout;

        QWidget* statusWidget = nullptr;
        QWidget* setWidget = nullptr;
        if (haveLayout)  //有分组
        {
            //添加状态面板
            statusWidget = initCmdStatusGroupWidget(cmdAttribute);
            if (statusWidget != nullptr)
            {
                currentLayout->addWidget(statusWidget);
            }

            //添加命令设置面板
            setWidget = initCmdSetGroupWidget(cmdAttribute);
            if (setWidget != nullptr)
            {
                currentLayout->addWidget(setWidget);
            }
        }
        else  //无分组
        {
            //添加状态面板
            statusWidget = initCmdStatusWidget(cmdAttribute);
            if (statusWidget != nullptr)
            {
                currentLayout->addWidget(statusWidget);
            }

            //添加命令设置面板
            setWidget = initCmdSetWidget(cmdAttribute);
            if (setWidget != nullptr)
            {
                currentLayout->addWidget(setWidget);
            }
        }

        auto btnLayout = new QHBoxLayout();
        btnLayout->setSpacing(6);
        btnLayout->setContentsMargins(0, 0, 0, 0);
        auto spacerItem = new QSpacerItem(197, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        btnLayout->addItem(spacerItem);

        QPushButton* cmdSettingCmdBtn = new QPushButton("命令设置");
        connect(cmdSettingCmdBtn, &QPushButton::clicked, this, &DataExchangeSystemWidget::packCmdParamSet);

        btnLayout->addWidget(cmdSettingCmdBtn);
        currentLayout->addLayout(btnLayout);

        currentWidget->setLayout(currentLayout);

        m_cmdIDWidgetMap[cmdID] = currentWidget;
        m_cmdWidgetIDMap[currentWidget] = cmdID;
        scrollLayout->addWidget(currentWidget);
        cmdListCombox->addItem(cmdAttribute.desc, cmdID);
    }

    scrollContentWidget->setLayout(scrollLayout);
    m_cmdscrollArea->setWidget(scrollContentWidget);
    m_cmdscrollArea->setWidgetResizable(true);

    QVBoxLayout* allVLayout = new QVBoxLayout;
    allVLayout->addLayout(comboboxListLayout);
    allVLayout->addWidget(m_cmdscrollArea);
    ui->cmdPageLayout->addLayout(allVLayout);
}

QWidget* DataExchangeSystemWidget::initCmdStatusWidget(CmdAttribute cmdAttribute)
{
    QGroupBox* groupBox = new QGroupBox(STATUS_TITLE);
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    bool haveControl = false;  // 如果没有控件需要显示就不创建界面

    for (auto groupKey : cmdAttribute.statusGroupMap.keys())
    {
        QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
        auto commonAttrList = cmdAttribute.statusGroupMap[groupKey];
        for (auto attr : commonAttrList)
        {
            if (attr.isShow)
            {
                QMap<QString, QVariant> enumMap;
                if (attr.displayFormat == DisplayFormat_Enum)
                {
                    m_widgetDeal.getCmdEnumData(attr.enumType, enumMap);
                }
                else if (attr.displayFormat == DisplayFormat_LED)
                {
                    m_widgetDeal.getCmdEnumData(attr.enumType, enumMap);
                }
                auto control = ControlFactory::createStatusControl(attr, enumMap);
                if (control)
                {
                    commonLayout->addWidget(control);
                    haveControl = true;
                }
            }
        }
        vboxLayout->addItem(commonLayout);
    }

    groupBox->setLayout(vboxLayout);

    //如果没有控件就删除界面，不创建
    if (!haveControl)
    {
        delete groupBox;
        groupBox = nullptr;
    }

    return groupBox;
}

QWidget* DataExchangeSystemWidget::initCmdSetWidget(CmdAttribute cmdAttribute)
{
    QGroupBox* groupBox = new QGroupBox(SET_TITLE);

    QVBoxLayout* vboxLayout = new QVBoxLayout();

    bool haveControl = false;  // 如果没有控件需要显示就不创建界面

    for (auto groupKey : cmdAttribute.settingGroupMap.keys())
    {
        QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
        auto commonAttrList = cmdAttribute.settingGroupMap[groupKey];

        for (auto attr : commonAttrList)
        {
            if (attr.isShow)
            {
                WWidget* control = nullptr;
                if (attr.displayFormat == DisplayFormat_Enum)
                {
                    QMap<int, QPair<QString, QVariant>> enumMap;
                    m_widgetDeal.getCmdEnumDataOrder(attr.enumType, enumMap);
                    control = ControlFactory::createSetControlOrder(attr, enumMap);
                }
                else if (attr.displayFormat == DisplayFormat_LED)
                {
                    QMap<int, QPair<QString, QVariant>> enumMap;
                    m_widgetDeal.getCmdEnumDataOrder(attr.enumType, enumMap);
                    control = ControlFactory::createSetControlOrder(attr, enumMap);
                }
                else
                {
                    QMap<QString, QVariant> enumMap;
                    control = ControlFactory::createSetControl(attr, enumMap);
                }

                if (control)
                {
                    commonLayout->addWidget(control);
                    haveControl = true;

                    void (WUNumber::*signal_valueChange)(const QVariant&) = &WUNumber::sig_valueChanged;
                    auto unumberControl = qobject_cast<WUNumber*>(control);
                    if (!unumberControl)
                        continue;

                    connect(unumberControl, signal_valueChange, this, [=](QVariant value) {
                        auto objName = control->objectName();
                        m_widgetDeal.dealCmdMulitiSetsWidgets(objName, value, cmdAttribute, m_cmdIDWidgetMap);
                    });
                }
            }
        }

        vboxLayout->addItem(commonLayout);
    }

    //创建动态界面
    auto hLayout = m_widgetDeal.initMultiWidgetSetsHLayout(cmdAttribute);

    if (hLayout != nullptr)
        vboxLayout->addItem(hLayout);

    groupBox->setLayout(vboxLayout);

    //如果没有控件就删除界面，不创建
    if (!haveControl)
    {
        delete groupBox;
        groupBox = nullptr;
    }

    return groupBox;
}

QWidget* DataExchangeSystemWidget::initCmdStatusGroupWidget(CmdAttribute cmdAttribute)
{
    QGroupBox* groupBox = new QGroupBox(STATUS_TITLE);
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    bool haveControl = false;  // 如果没有控件需要显示就不创建界面
    QGridLayout* commonGridLayout = new QGridLayout();
    for (auto groupKey : cmdAttribute.statusGroupMap.keys())
    {
        UnitLayout unitLayout;
        if (cmdAttribute.unitLayoutMap.contains(groupKey))
        {
            unitLayout = cmdAttribute.unitLayoutMap.value(groupKey);
        }
        QGroupBox* commomGroupBox = new QGroupBox(unitLayout.desc);
        QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
        auto commonAttrList = cmdAttribute.statusGroupMap[groupKey];
        for (auto attr : commonAttrList)
        {
            if (attr.isShow)
            {
                QMap<QString, QVariant> enumMap;
                if (attr.displayFormat == DisplayFormat_Enum)
                {
                    m_widgetDeal.getCmdEnumData(attr.enumType, enumMap);
                }
                else if (attr.displayFormat == DisplayFormat_LED)
                {
                    m_widgetDeal.getCmdEnumData(attr.enumType, enumMap);
                }
                auto control = ControlFactory::createStatusControl(attr, enumMap);
                if (control)
                {
                    commonLayout->addWidget(control);
                    haveControl = true;
                }
            }
        }
        commomGroupBox->setLayout(commonLayout);
        commonGridLayout->addWidget(commomGroupBox, unitLayout.row, unitLayout.col, unitLayout.rowspan, unitLayout.colspan);
    }
    vboxLayout->addItem(commonGridLayout);

    groupBox->setLayout(vboxLayout);

    //如果没有控件就删除界面，不创建
    if (!haveControl)
    {
        delete groupBox;
        groupBox = nullptr;
    }

    return groupBox;
}

QWidget* DataExchangeSystemWidget::initCmdSetGroupWidget(CmdAttribute cmdAttribute)
{
    QGroupBox* groupBox = new QGroupBox(SET_TITLE);
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    bool haveControl = false;  // 如果没有控件需要显示就不创建界面
    QGridLayout* commonGridLayout = new QGridLayout();
    for (auto groupKey : cmdAttribute.settingGroupMap.keys())
    {
        UnitLayout unitLayout;
        if (cmdAttribute.unitLayoutMap.contains(groupKey))
        {
            unitLayout = cmdAttribute.unitLayoutMap.value(groupKey);
        }
        QGroupBox* commomGroupBox = new QGroupBox(unitLayout.desc);
        QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
        auto commonAttrList = cmdAttribute.settingGroupMap[groupKey];
        for (auto attr : commonAttrList)
        {
            if (attr.isShow)
            {
                WWidget* control = nullptr;
                if (attr.displayFormat == DisplayFormat_Enum)
                {
                    QMap<int, QPair<QString, QVariant>> enumMap;
                    m_widgetDeal.getCmdEnumDataOrder(attr.enumType, enumMap);
                    control = ControlFactory::createSetControlOrder(attr, enumMap);
                }
                else if (attr.displayFormat == DisplayFormat_LED)
                {
                    QMap<int, QPair<QString, QVariant>> enumMap;
                    m_widgetDeal.getCmdEnumDataOrder(attr.enumType, enumMap);
                    control = ControlFactory::createSetControlOrder(attr, enumMap);
                }
                else
                {
                    QMap<QString, QVariant> enumMap;
                    control = ControlFactory::createSetControl(attr, enumMap);
                }

                if (control)
                {
                    commonLayout->addWidget(control);
                    haveControl = true;

                    void (WUNumber::*signal_valueChange)(const QVariant&) = &WUNumber::sig_valueChanged;
                    auto unumberControl = qobject_cast<WUNumber*>(control);
                    if (!unumberControl)
                        continue;

                    connect(unumberControl, signal_valueChange, this, [=](QVariant value) {
                        auto objName = control->objectName();
                        m_widgetDeal.dealCmdMulitiSetsWidgets(objName, value, cmdAttribute, m_cmdIDWidgetMap);
                    });
                }
            }
        }
        commomGroupBox->setLayout(commonLayout);
        commonGridLayout->addWidget(commomGroupBox, unitLayout.row, unitLayout.col, unitLayout.rowspan, unitLayout.colspan);
    }
    vboxLayout->addItem(commonGridLayout);

    //创建动态界面
    auto hLayout = m_widgetDeal.initMultiWidgetSetsHLayout(cmdAttribute);

    if (hLayout != nullptr)
        vboxLayout->addItem(hLayout);

    groupBox->setLayout(vboxLayout);

    //如果没有控件就删除界面，不创建
    if (!haveControl)
    {
        delete groupBox;
        groupBox = nullptr;
    }

    return groupBox;
}

void DataExchangeSystemWidget::initOtherRecycleTabWidget(Unit unit)
{
    for (auto recycleKey : unit.targetData.keys())
    {
        auto recycle = unit.targetData[recycleKey];
        auto tabName = recycle.desc;
        if (recycle.recycleNum == "Mission1CenterNum")  //任务参试中心送数状态
        {
            QStringList headers;
            auto tableView = new QTableView(ui->tabWidget);
            tableView->setModel(mTaskRunCenterDataModel);
            auto multiMap = unit.multiStatusMap[recycleKey];
            auto unitEnumMap = unit.enumMap;
            initTable(unitEnumMap, multiMap, headers, tableView);
            mTaskRunCenterDataModel->setHeaderData(headers);
            ui->tabWidget->addTab(tableView, tabName);
            mTableNameMap.insert(tableView, 8);
        }
        if (recycle.recycleNum == "Mission1BbeNum")  //任务参试基带数量
        {
            QStringList headers;
            auto tableView = new QTableView(ui->tabWidget);
            tableView->setModel(mTaskRunBaseNumModel);
            auto multiMap = unit.multiStatusMap[recycleKey];
            auto unitEnumMap = unit.enumMap;
            initTable(unitEnumMap, multiMap, headers, tableView);
            mTaskRunBaseNumModel->setHeaderData(headers);
            ui->tabWidget->addTab(tableView, tabName);
            mTableNameMap.insert(tableView, 9);
        }
        if (recycle.recycleNum == "SoftUnitItem1DetailItemNum")  //软件单元项目主机详细信息
        {
            QStringList headers;
            auto tableView = new QTableView(ui->tabWidget);
            tableView->setModel(mSoftUnitHostModel);
            auto multiMap = unit.multiStatusMap[recycleKey];
            auto unitEnumMap = unit.enumMap;
            initTable(unitEnumMap, multiMap, headers, tableView);
            mSoftUnitHostModel->setHeaderData(headers);
            ui->tabWidget->addTab(tableView, tabName);
            mTableNameMap.insert(tableView, 10);
        }
    }
}

void DataExchangeSystemWidget::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event)
    if (isOnline > -1)
    {
        isOnline--;
    }
    if (isOnline == 0)
    {
        ui->isOnline_2->setText(QString("设备离线"));
        ui->isOnline_2->setStyleSheet("color:red;font-size:30px;");
    }
}

void DataExchangeSystemWidget::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)
    m_isShow = true;
}

void DataExchangeSystemWidget::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event)
    m_isShow = false;
}

void DataExchangeSystemWidget::packCmdParamSet()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    QPushButton* settingBtn = qobject_cast<QPushButton*>(sender());
    if (settingBtn)
    {
        QGroupBox* currentWidget = qobject_cast<QGroupBox*>(settingBtn->parent());
        if (currentWidget && m_cmdWidgetIDMap.contains(currentWidget))
        {
            auto cmdID = m_cmdWidgetIDMap[currentWidget];
            int systemId = m_currentDeviceID.sysID;
            int deviceId = m_currentDeviceID.devID;
            int extionId = m_currentDeviceID.extenID;
            int modeId = m_currentMode.ID;

            CmdRequest cmdRequest;
            cmdRequest.m_systemNumb = systemId;
            cmdRequest.m_deviceNumb = deviceId;
            cmdRequest.m_extenNumb = extionId;

            cmdRequest.m_modeID = modeId;
            cmdRequest.m_cmdID = cmdID;

            QList<ParameterAttribute> cmdSetParamList = m_currentMode.cmdMap.value(cmdID).requestList;

            QMap<QString, QVariant> setParamData;
            if (!m_widgetDeal.getCmdSetParamMap(setParamData, currentWidget))
            {
                QMessageBox::warning(this, "提示", "参数值不在合法范围", QString("确定"));
                return;
            }

            if (!cmdSetParamList.isEmpty())
            {
                cmdRequest.m_paramdataMap = setParamData;
            }
            //留着有recycle标签的命令使用
            QMap<int, QList<ParameterAttribute>> setMultiParaRecycleMap = m_currentMode.cmdMap.value(cmdID).requestRecycle;

            QMap<int, QList<QPair<QString, QVariant>>> paraTargerMap;
            if (!m_widgetDeal.getCmdMultiParamMap(paraTargerMap, currentWidget))
            {
                QMessageBox::warning(this, "提示", "参数值不在合法范围", QString("确定"));
                return;
            }

            if (!setMultiParaRecycleMap.isEmpty())
            {
                cmdRequest.multiParamMap = paraTargerMap;
            }

            JsonWriter writer;
            writer& cmdRequest;
            RedisHelper::getInstance().setData("过程控制命令发送数据", writer.GetQString());

            emit signalsCmdDeviceJson(writer.GetString());
            qDebug() << writer.GetString();
        }
    }
}

void DataExchangeSystemWidget::setDeviceID(DeviceID deviceId) { m_currentDeviceID = deviceId; }

const DeviceID& DataExchangeSystemWidget::getDeviceID() const { return m_currentDeviceID; }

void DataExchangeSystemWidget::cmdCurrentIndexChanged(int index)
{
    Q_UNUSED(index);
    QComboBox* combobox = qobject_cast<QComboBox*>(sender());
    if (combobox)
    {
        auto cmdID = combobox->currentData().toInt();
        if (m_cmdIDWidgetMap.contains(cmdID))
        {
            if (m_cmdCurrGroupBox)
            {
                m_cmdCurrGroupBox->setStyleSheet(".QGroupBox,.MyGroupBox{font: 12pt;font-weight:bold; border:2px solid "
                                                 "#C0DCF2;border-radius:5px;margin-top:10px;margin-left:5px;}");  //还原上一个被选中的groupBox的样式表
            }
            QGroupBox* groupBox = qobject_cast<QGroupBox*>(m_cmdIDWidgetMap[cmdID]);
            if (groupBox)
            {
                m_cmdCurrGroupBox = groupBox;
                auto cmdscrollAreaBar = m_cmdscrollArea->verticalScrollBar();
                cmdscrollAreaBar->setValue(groupBox->y());
                groupBox->setStyleSheet(".QGroupBox,.MyGroupBox{font: 12pt;font-weight:bold; border:2px solid "
                                        "#000000;border-radius:5px;margin-top:10px;margin-left:5px;}");
            }
        }
    }
}

void DataExchangeSystemWidget::slotsExtenStatusReport(const ExtensionStatusReportMessage& statusReport)
{
    isOnline = 5;
    ui->isOnline_2->setText(QString("设备在线"));
    ui->isOnline_2->setStyleSheet("color:green;font-size:30px;");
    // 当前表格对应的recycleKey
    auto currentTable = qobject_cast<QTableView*>(ui->tabWidget->currentWidget());
    auto currentKey = mTableNameMap.value(currentTable);

    auto recycleReportMap = statusReport.unitReportMsgMap[1].multiTargetParamMap2;            // 只有一个单元
    auto recycleReportMap3 = statusReport.unitReportMsgMap[1].multiTargetParamMap3;           //动态数据里面再嵌套的动态数据
    auto dteTaskBZAndCenterBZMap = statusReport.unitReportMsgMap[1].dteTaskBZAndCenterBZMap;  //里面存储的是每个任务标识对应的中心标识

    // 更新状态参数
    updateStatusGroupBox(statusReport.unitReportMsgMap[1].paramMap);

    //更新任务信息
    auto missionReportMap = recycleReportMap.value(1);
    QList<MissionInfo> list;
    for (int i = 0; i < missionReportMap.size(); ++i)
    {
        auto valueMap = missionReportMap.at(i);
        MissionInfo missionInfo;
        missionInfo.id = valueMap.value("MissionID").toString().toUpper();
        missionInfo.uac = valueMap.value("MissionUAC").toString().toUpper();

        QStringList centerNameList;
        for (auto taskBZ : dteTaskBZAndCenterBZMap.keys())
        {
            if (taskBZ.toString().toUpper() == missionInfo.id)
            {
                auto centerBZList = dteTaskBZAndCenterBZMap[taskBZ];
                for (auto centerBZ : centerBZList)
                {
                    auto centerDataList = recycleReportMap.value(7);  // 每个中心的详细数据Map
                    for (int i = 0; i < centerDataList.size(); ++i)
                    {
                        auto centerMap = centerDataList.at(i);
                        // 参试的中心标识如果相等的话就取出这个中心标识对应的中心名称
                        if (centerMap.value("CenterDataControlItemId").toString() == centerBZ)
                        {
                            auto centerName = centerMap.value("CenterDataControlItemCode").toString();
                            centerNameList.append(centerName);
                        }
                    }
                }
            }
        }

        missionInfo.center = centerNameList.join(" ");
        missionInfo.segment = valueMap.value("MissionSegment").toUInt();

        quint32 startDays = valueMap.value("MissionStarTrackingJD").toUInt();
        quint32 startSecs = valueMap.value("MissionStarTrackingJS").toUInt() / 10000;
        QDateTime startDateTime = QDateTime(QDate(2000, 1, 1));
        startDateTime = startDateTime.addDays(startDays - 1);
        startDateTime = startDateTime.addSecs(startSecs);
        missionInfo.starTime = startDateTime.toString("yyyy-MM-dd hh:mm:ss");

        quint32 stopDays = valueMap.value("MissionMissionStopJD").toUInt();
        quint32 stopSecs = valueMap.value("MissionMissionStopJS").toUInt() / 10000;
        QDateTime stopDateTime = QDateTime(QDate(2000, 1, 1));
        stopDateTime = stopDateTime.addDays(stopDays - 1);
        stopDateTime = stopDateTime.addSecs(stopSecs);
        missionInfo.stopTime = stopDateTime.toString("yyyy-MM-dd hh:mm:ss");

        missionInfo.runStatus = valueMap.value("MissionRunStatus").toUInt();
        // 剩余时间/飞行时间：如果是任务准备，那么表示还有多久跟踪开始，其他就是到任务结束时间
        qint64 remainingDay = 0;
        qint64 remainingSecs = 0;
        if (missionInfo.runStatus == 1)
        {
            remainingDay = GlobalData::currentDateTime().daysTo(startDateTime);
            remainingSecs = GlobalData::currentDateTime().secsTo(startDateTime);
        }
        else
        {
            remainingDay = GlobalData::currentDateTime().daysTo(stopDateTime);
            remainingSecs = GlobalData::currentDateTime().secsTo(stopDateTime);
        }

        remainingDay = remainingDay > 0 ? remainingDay - 1 : remainingDay;

        QTime remainingTime(0, 0, 0);
        missionInfo.remainingTime = QString("%1天").arg(remainingDay) + remainingTime.addSecs(remainingSecs).toString("hh:mm:ss");
        if (missionInfo.remainingTime.indexOf("-") == 0)
        {
            missionInfo.remainingTime.replace(0, 1, "");
        }

        missionInfo.guideMode = valueMap.value("MissionGuideMode").toUInt();

        auto upT0 = valueMap.value("MissionT0").toUInt() / 10000;  //根据步进0.0001除10000得多少秒

        auto time = QTime::fromMSecsSinceStartOfDay(upT0 * 1000);  //*1000表示替换为毫秒

        missionInfo.upT0 = time.toString("HH:mm:ss");  //

        list.append(missionInfo);
    }
    //    ui->missionInfoTableView->resizeColumnsToContents();
    //    ui->missionInfoTableView->horizontalHeader()->setMinimumSectionSize(100);
    mMissionInfoModel->setData(list);

    if (recycleReportMap.contains(currentKey))
    {
        auto currentRecycleReportMap = recycleReportMap.value(currentKey);
        // 更新单个循环的表格
        if (currentKey == 2)
        {
            QList<CommunicateStatus> list;

            for (int i = 0; i < currentRecycleReportMap.size(); ++i)
            {
                auto valueMap = currentRecycleReportMap.at(i);
                CommunicateStatus commStat;
                commStat.portId = valueMap.value("CommStatusItemPortID").toInt();
                commStat.portCode = valueMap.value("CommStatusItemPortCode").toString();
                commStat.bId = valueMap.value("CommStatusItemBid").toInt();
                commStat.dataCode = valueMap.value("CommStatusItemDataCode").toString();
                commStat.mId = valueMap.value("CommStatusItemMid").toString().toUpper();
                commStat.uac = valueMap.value("CommStatusItemUAC").toString().toUpper();
                commStat.framesNum = valueMap.value("CommStatusItemFramesNum").toInt();
                commStat.dir = valueMap.value("CommStatusItemDir").toInt();

                list.append(commStat);
            }

            mCommStatusModel->setData(list);
        }
        else if (currentKey == 3)
        {
            QList<SoftUnit> list;

            for (int i = 0; i < currentRecycleReportMap.size(); ++i)
            {
                auto valueMap = currentRecycleReportMap.at(i);
                SoftUnit softUnit;
                softUnit.id = valueMap.value("SoftUnitItemID").toInt();
                softUnit.code = valueMap.value("SoftUnitItemCode").toString();
                softUnit.status = valueMap.value("SoftUnitItemStatus").toInt();
                softUnit.detailNum = valueMap.value("SoftUnitItem1DetailItemNum").toInt();
                softUnit.hostId = valueMap.value("SoftUnitItem1DetailItem1HostID").toInt();
                softUnit.detailStatus = valueMap.value("SoftUnitItem1DetailItem1Status").toInt();

                list.append(softUnit);
            }

            mSoftUnitModel->setData(list);
        }
        else if (currentKey == 4)
        {
            QList<HostInfo> list;

            for (int i = 0; i < currentRecycleReportMap.size(); ++i)
            {
                auto valueMap = currentRecycleReportMap.at(i);
                HostInfo hostInfo;
                hostInfo.IP = valueMap.value("HostID").toInt();
                hostInfo.name = valueMap.value("HostName").toString();
                hostInfo.status = valueMap.value("HostStatus").toInt();

                list.append(hostInfo);
            }

            mHostInfoMode->setData(list);
        }
        else if (currentKey == 5)
        {
            QList<NetCardInfo> list;

            for (int i = 0; i < currentRecycleReportMap.size(); ++i)
            {
                auto valueMap = currentRecycleReportMap.at(i);
                auto ipNum = valueMap.value("NetCardItemIP").toUInt();
                auto tempIP = qFromBigEndian<quint32>(ipNum);

                NetCardInfo netCard;
                netCard.IP = QHostAddress(tempIP).toString();
                netCard.hostId = valueMap.value("NetCardItemHostId").toInt();
                netCard.type = valueMap.value("NetCardItemType").toInt();
                netCard.status = valueMap.value("NetCardItemStatus").toInt();

                list.append(netCard);
            }

            mNetCardModel->setData(list);
        }
        else if (currentKey == 6)
        {
            QList<TcpStatus> list;

            for (int i = 0; i < currentRecycleReportMap.size(); ++i)
            {
                auto valueMap = currentRecycleReportMap.at(i);
                TcpStatus tcpStatus;
                tcpStatus.id = valueMap.value("TcpStatusItemHostID").toInt();
                tcpStatus.bbeIndex = valueMap.value("TcpStatusItemBbeIndex").toInt();
                tcpStatus.status = valueMap.value("TcpStatusItemStatus").toInt();

                list.append(tcpStatus);
            }

            mTcpStatusModel->setData(list);
        }
        else if (currentKey == 7)
        {
            QList<CenterDataControl> list;

            for (int i = 0; i < currentRecycleReportMap.size(); ++i)
            {
                auto valueMap = currentRecycleReportMap.at(i);
                CenterDataControl center;
                center.id = valueMap.value("CenterDataControlItemId").toString().toUpper();
                center.code = valueMap.value("CenterDataControlItemCode").toString();
                center.isUing = valueMap.value("CenterDataControlItemIsUsing").toInt();
                center.isSendTtcFirstRouter = valueMap.value("CenterDataControlItemIsSendTtcFirstRouter").toInt();
                center.isSendLinkFirstRouter = valueMap.value("CenterDataControlItemIsSendLinkFirstRouter").toInt();
                center.isSendTtcSecondRouter = valueMap.value("CenterDataControlItemIsSendTtcSecondRouter").toInt();
                center.isSendLinkSecondRouter = valueMap.value("CenterDataControlItemIsSendLinkSecondRouter").toInt();

                list.append(center);
            }

            mCenterDataControlModel->setData(list);
        }
    }
    else
    {
        // 更新单个循环的表格
        if (currentKey == 8)
        {
            QList<QMap<QString, QVariant>> currentRecycleReportMap;
            for (auto recycle : recycleReportMap3.keys())
            {
                if (!recycleReportMap3[recycle].contains(currentKey))  //直接取软件单元的嵌套循环数据，没有就返回
                    continue;

                currentRecycleReportMap = recycleReportMap3[1].value(currentKey);  //这里是任务参试中心送数状态
            }

            QList<TaskRunCenterData> list;

            for (int i = 0; i < currentRecycleReportMap.size(); ++i)
            {
                auto valueMap = currentRecycleReportMap.at(i);
                TaskRunCenterData taskRunCenterData;
                taskRunCenterData.taskRunCenterBZ = valueMap.value("Mission1Center1").toString();
                taskRunCenterData.taskRunCenterSSStatus = valueMap.value("Mission1Center1SendDataStatus").toInt();

                list.append(taskRunCenterData);
            }

            mTaskRunCenterDataModel->setData(list);
        }
        else if (currentKey == 9)
        {
            QList<QMap<QString, QVariant>> currentRecycleReportMap;
            for (auto recycle : recycleReportMap3.keys())
            {
                if (!recycleReportMap3[recycle].contains(currentKey))  //直接取软件单元的嵌套循环数据，没有就返回
                    continue;

                currentRecycleReportMap = recycleReportMap3[1].value(currentKey);  //这里是软件单元的数据
            }

            QList<TaskRunBaseNum> list;

            for (int i = 0; i < currentRecycleReportMap.size(); ++i)
            {
                auto valueMap = currentRecycleReportMap.at(i);
                TaskRunBaseNum taskRunBase;
                taskRunBase.taskRunBaseBZ = valueMap.value("Mission1Bbe1").toString();

                list.append(taskRunBase);
            }

            mTaskRunBaseNumModel->setData(list);
        }
        else if (currentKey == 10)
        {
            QList<QMap<QString, QVariant>> currentRecycleReportMap;
            for (auto recycle : recycleReportMap3.keys())
            {
                if (!recycleReportMap3[recycle].contains(currentKey))  //直接取软件单元的嵌套循环数据，没有就返回
                    continue;

                currentRecycleReportMap = recycleReportMap3[3].value(currentKey);  //这里是软件单元的数据
            }

            QList<SoftUnitHost> list;

            for (int i = 0; i < currentRecycleReportMap.size(); ++i)
            {
                auto valueMap = currentRecycleReportMap.at(i);
                SoftUnitHost softUnitHost;
                softUnitHost.runCode = valueMap.value("SoftUnitItem1DetailItem1HostID").toInt();
                softUnitHost.runStatus = valueMap.value("SoftUnitItem1DetailItem1Status").toInt();

                list.append(softUnitHost);
            }

            mSoftUnitHostModel->setData(list);
        }
    }
}

void DataExchangeSystemWidget::slotsControlCmdResponse(const ControlCmdResponse& cmdResponce)
{
    if (cmdResponce.deviceID == m_currentDeviceID)
    {
        auto result = DevProtocolEnumHelper::controlCmdResponseToDescStr(cmdResponce.responseResult);
        QMessageBox::warning(this, QString("提示"), QString(result), QString("确定"));
    }
}

void DataExchangeSystemWidget::slotsCmdResult(const CmdResult& cmdResponce)
{
    auto cmdID = cmdResponce.m_cmdID;
    if (m_cmdIDWidgetMap.contains(cmdID))
    {
        auto widget = m_cmdIDWidgetMap[cmdID];
        for (auto child : widget->children())
        {
            QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
            if (!groupBox)
            {
                continue;
            }
            //这里进行类型判断
            if (groupBox->title().trimmed() == STATUS_TITLE)
            {
                QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();
                for (auto& control : allPButtons)
                {
                    auto objName = control->objectName();
                    if (cmdResponce.m_paramdataMap.contains(objName))
                    {
                        auto value = cmdResponce.m_paramdataMap.value(objName);
                        WWidget* wWidget = qobject_cast<WWidget*>(control);
                        if (wWidget)
                        {
                            wWidget->setValue(value);
                        }
                    }
                }
            }
        }
    }
}

void DataExchangeSystemWidget::on_switchBtn_2_clicked()
{
    m_isParamSet = !m_isParamSet;
    if (m_isParamSet)
    {
        ui->stackedWidget->setCurrentWidget(ui->paramPage_2);
        ui->switchBtn_2->setText(QString("命令设置"));
    }
    else
    {
        initCmdWidget();
        ui->stackedWidget->setCurrentWidget(ui->cmdPage_2);
        ui->switchBtn_2->setText(QString("状态监视"));
    }
}

void DataExchangeSystemWidget::initTable(const QMap<QString, DevEnum>& enumMap, const QList<ParameterAttribute>& attrList, QStringList& headers,
                                         QTableView* tableView)
{
    for (int i = 0; i < attrList.size(); ++i)
    {
        const auto& attr = attrList.at(i);
        headers << attr.desc;

        auto enumIter = enumMap.find(attr.enumType);
        if (enumIter == enumMap.end())
            continue;

        DataExchangeWidgetDelegate* delegate = new DataExchangeWidgetDelegate(tableView);

        delegate->setEnumEntryList(enumIter->emumEntryList);
        tableView->setItemDelegateForColumn(i, delegate);
    }
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->horizontalHeader()->setHighlightSections(false);
    tableView->verticalHeader()->setVisible(false);
    tableView->setFrameShape(QFrame::NoFrame);
    tableView->setSelectionMode(QAbstractItemView::NoSelection);
}

void DataExchangeSystemWidget::updateStatusGroupBox(const QMap<QString, QVariant>& reportMap)
{
    QList<WWidget*> allPButtons = ui->statusGroupBox->findChildren<WWidget*>();
    for (auto& control : allPButtons)
    {
        auto objName = control->objectName();
        if (reportMap.contains(objName))
        {
            auto value = reportMap.value(objName);
            WWidget* wWidget = qobject_cast<WWidget*>(control);
            if (wWidget)
                wWidget->setValue(value);
        }
    }
}
