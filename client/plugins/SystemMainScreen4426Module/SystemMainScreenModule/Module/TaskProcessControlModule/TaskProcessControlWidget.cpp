#include "TaskProcessControlWidget.h"
#include "ui_TaskProcessControlWidget.h"

#include <QMessageBox>
#include <QTreeWidget>

#include "DeviceProcessMessageSerialize.h"
#include "ExtendedConfig.h"
#include "HP.h"
#include "PlanRunMessageSerialize.h"
#include "QssCommonHelper.h"
#include "SatelliteManagementDefine.h"
#include "StatusBarManagerThread.h"
#include "SubscriberHelper.h"
#include "SystemCaptureProcessControl.h"
#include "SystemLogMessageSerialize.h"

TaskProcessControlWidget::TaskProcessControlWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TaskProcessControlWidget)
{
    ui->setupUi(this);
    qRegisterMetaType<CurrentRunningTaskPlanData>("CurrentRunningTaskPlanData");
    qRegisterMetaType<CurrentRunningTaskPlanData>("const CurrentRunningTaskPlanData&");
    qRegisterMetaType<CurrentRunningTaskPlanData>("CurrentRunningTaskPlanData&");

    initUI();

    initSlot();

    this->setWindowIcon(QIcon(":/image/task_process_ctrl.png"));

    //初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
}

TaskProcessControlWidget::~TaskProcessControlWidget()
{
    delete ui;
}

void TaskProcessControlWidget::showEvent(QShowEvent*)
{
    // 4426用户要求第一次显示时自动更新数据，之后打开保持上一次选择的状态不变
    static bool isFirstShow = true;
    if (isFirstShow)
    {
        isFirstShow = false;
        slotRefreshData();
    }

    if (m_timerID == -1)
    {
        m_timerID = startTimer(1000);
    }
    //把当前正在运行中的任务的信息刷新一遍
    refreshRunningTaskPlanData();
}

void TaskProcessControlWidget::timerEvent(QTimerEvent* event)
{
    auto currentCKDeviceIDInt = m_systemCaptureProcessControl->getCurrentDeviceId();
    auto currentModeID = m_systemCaptureProcessControl->getCurrentModeID();
    DeviceID ckDeviceID(currentCKDeviceIDInt);

    int needSwitchModeID = GlobalData::getOnlineDeviceModeId(ckDeviceID);

    if (currentModeID != needSwitchModeID)
    {
        auto workMode = SystemWorkModeHelper::modeIDToSystemWorkMode(needSwitchModeID);
        // 切换当前工作模式对应的系统捕获流程控制界面
        m_systemCaptureProcessControl->setShowWidget(workMode);
    }

    //测试界面  自己想要哪种模式的系统捕获流程界面，就在这设置
    // m_systemCaptureProcessControl->setShowWidget(STTC);
}

void TaskProcessControlWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    if (m_timerID != -1)
    {
        killTimer(m_timerID);
        m_timerID = -1;
    }
}

void TaskProcessControlWidget::initUI()
{
    // 隐藏设备组合号下的全部内容
    ui->groupBox_3->setHidden(true);

    // 添加链路主备选择
    mMasterSlaveLinkBtnGroup.addButton(ui->masterRadioButton, static_cast<int>(MasterSlave::Master));
    mMasterSlaveLinkBtnGroup.addButton(ui->slaveRadioButton, static_cast<int>(MasterSlave::Slave));
    ui->masterRadioButton->setChecked(true);

    // 初始化高功放界面
    // S功放
    m_sHP = new HP;
    m_sHP->setType(HP::SCK);
    connect(m_sHP, &HP::signalsCmdDeviceJson, this, &TaskProcessControlWidget::signalsCmdDeviceJson);
    connect(m_sHP, &HP::signalsUnitDeviceJson, this, &TaskProcessControlWidget::signalsUnitDeviceJson);

    // ka测控功放
    m_kaCKHP = new HP;
    m_kaCKHP->setType(HP::KaCK);
    connect(m_kaCKHP, &HP::signalsCmdDeviceJson, this, &TaskProcessControlWidget::signalsCmdDeviceJson);
    connect(m_kaCKHP, &HP::signalsUnitDeviceJson, this, &TaskProcessControlWidget::signalsUnitDeviceJson);

    //低速数传功放
    m_kaSCHP = new HP;
    m_kaSCHP->setType(HP::KaSC);
    // 获取Ka低速链路
    ui->dshpatabWidgetPage->layout()->addWidget(m_kaSCHP);

    connect(m_kaSCHP, &HP::signalsCmdDeviceJson, this, &TaskProcessControlWidget::signalsCmdDeviceJson);
    connect(m_kaSCHP, &HP::signalsUnitDeviceJson, this, &TaskProcessControlWidget::signalsUnitDeviceJson);

    //存储转发界面
    ui->dtwidget->layout()->setContentsMargins(0, 0, 0, 0);

    connect(ui->dtwidget, &CCZFTaskPlanWidget::signalsCmdDeviceJson, this, &TaskProcessControlWidget::signalsCmdDeviceJson);
    connect(ui->dtwidget, &CCZFTaskPlanWidget::signalsUnitDeviceJson, this, &TaskProcessControlWidget::signalsUnitDeviceJson);

    // 初始化系统捕获流程界面
    m_systemCaptureProcessControl = new SystemCaptureProcessControl(this);
    ui->systemCaptureProcessControlGroupBox->layout()->addWidget(m_systemCaptureProcessControl);

    connect(m_systemCaptureProcessControl, &SystemCaptureProcessControl::signalsCmdDeviceJson, this, &TaskProcessControlWidget::signalsCmdDeviceJson);
    connect(m_systemCaptureProcessControl, &SystemCaptureProcessControl::signalsUnitDeviceJson, this,
            &TaskProcessControlWidget::signalsUnitDeviceJson);

    m_systemCaptureProcessControl->setXMLSlot();

    // TODO 数传的时候单独使用
    ui->scsystemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::RWFS), static_cast<int>(LinkType::RWFS));
    ui->scsystemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::ZPBH), static_cast<int>(LinkType::ZPBH));
    //    ui->scsystemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::SPMNYYXBH), static_cast<int>(LinkType::SPMNYYXBH));
    ui->scsystemWorkWay->addItem("Ka低速射频模拟源", static_cast<int>(LinkType::SPMNYYXBH) + 100);
    ui->scsystemWorkWay->addItem("X低速射频模拟源", static_cast<int>(LinkType::SPMNYYXBH) + 101);
    ui->scsystemWorkWay->addItem("Ka高速射频模拟源", static_cast<int>(LinkType::SPMNYYXBH) + 102);
}

void TaskProcessControlWidget::initSlot()
{
    // 刷新按钮点击
    connect(ui->refreshBtn, &QPushButton::clicked, this, &TaskProcessControlWidget::slotRefreshData);

    //初始化
    ui->ckWidget1->setWorkModeType(0);
    ui->ckWidget1->initUI();
    ui->ckWidget2->setWorkModeType(1);
    ui->ckWidget2->initUI();

    // 工作模式改变
    connect(ui->ckWidget1, &CKTaskWorkModeTaskCodeSelectWidget::signalWorkModeChanged, this, &TaskProcessControlWidget::slotWorkModeChanged);
    connect(ui->ckWidget2, &CKTaskWorkModeTaskCodeSelectWidget::signalWorkModeChanged, this, &TaskProcessControlWidget::slotWorkModeChanged);
    // connect(ui->scWidget1, &SCTaskWorkModeTaskCodeSelectWidget::signalWorkModeChanged, this, &TaskProcessControlWidget::slotWorkModeChanged);

    ui->kadsWidget->setWorkMode(SystemWorkMode::KaDS);
    ui->xdsWidget->setWorkMode(SystemWorkMode::XDS);
    ui->kagsWidget->setWorkMode(SystemWorkMode::KaGS);

    // 系统工作方式改变
    connect(ui->systemWorkWay, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &TaskProcessControlWidget::slotSystemWorkWayChanged);

    // 资源释放
    connect(ui->resourceReleaseBtn, &QPushButton::clicked, this, &TaskProcessControlWidget::slotResourceReleaseBtnClick);
    // 下发参数宏
    connect(ui->setParamMacroBtn, &QPushButton::clicked, this, &TaskProcessControlWidget::slotSetParamMacroBtnClick);
    // 链路配置
    connect(ui->linkSetBtn, &QPushButton::clicked, this, &TaskProcessControlWidget::slotLinkSetBtnClick);
    // 一键校零
    connect(ui->sOneKeyXLBtn, &QPushButton::clicked, this, &TaskProcessControlWidget::slotSOneKeyXLBtnClick);
    connect(ui->kaOneKeyXLBtn, &QPushButton::clicked, this, &TaskProcessControlWidget::slotKaOneKeyXLBtnClick);

    //系统日志
    connect(SubscriberHelper::getInstance(), &SubscriberHelper::signalSystemLogReadable, this, &TaskProcessControlWidget::slotReadSystemLog);

    // 测控链路设置按钮
    connect(ui->setting, &QPushButton::clicked, this, &TaskProcessControlWidget::slotsCkSettingBtnClick);

    // 数传链路设置按钮
    connect(ui->scsetting, &QPushButton::clicked, this, &TaskProcessControlWidget::slotsDtSettingBtnClick);

    //获取自动化运行任务信息
    connect(SubscriberHelper::getInstance(), &SubscriberHelper::signalAutoRunTaskStepStatusReadable, this,
            &TaskProcessControlWidget::slotReadAutoRunTaskStepStatus);
}

void TaskProcessControlWidget::changeSystemWorkWay()
{
    SystemWorkMode ckWorkMode1 = NotDefine;
    ckWorkMode1 = ui->ckWidget1->getWorkMode();

    SystemWorkMode ckWorkMode2 = NotDefine;
    ckWorkMode2 = ui->ckWidget2->getWorkMode();

    ui->systemWorkWay->clear();

    // 只有选择了测控模式
    if ((ckWorkMode1 != NotDefine || ckWorkMode2 != NotDefine))
    {
        ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::RWFS), static_cast<int>(LinkType::RWFS));
        ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::LSYDJYXBH), static_cast<int>(LinkType::LSYDJYXBH));
        ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::PKXLBPQBH), static_cast<int>(LinkType::PKXLBPQBH));
        // ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::SPMNYWXBH), static_cast<int>(LinkType::SPMNYWXBH));
        ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::SPMNYYXBH), static_cast<int>(LinkType::SPMNYYXBH));
        ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::SZHBH), static_cast<int>(LinkType::SZHBH));
    }
}

void TaskProcessControlWidget::hpTableReare()
{
    ui->hpTabWidget->clear();
    m_sHP->setVisible(false);
    m_kaCKHP->setVisible(false);

    auto ckWorkMode1 = ui->ckWidget1->getWorkMode();
    auto ckWorkMode2 = ui->ckWidget2->getWorkMode();

    if ((ckWorkMode1 != NotDefine && ckWorkMode1 != KaKuo2) || (ckWorkMode2 != NotDefine && ckWorkMode2 != KaKuo2))
    {
        ui->hpTabWidget->addTab(m_sHP, "S功放");
    }
    if (ckWorkMode1 == KaKuo2 || ckWorkMode2 == KaKuo2)
    {
        ui->hpTabWidget->addTab(m_kaCKHP, "Ka测控功放");
    }
}
void TaskProcessControlWidget::manualFunction(ManualType type, LinkType linkType, UiLinkType uilinkType)
{
    if (uilinkType == UiLinkType::CKLink)
    {
        if (linkType != LinkType::RWFS)
        {
            linkType = (LinkType)ui->systemWorkWay->currentData().toInt();
        }
    }

    if (uilinkType == UiLinkType::DTLink)
    {
        if (linkType != LinkType::RWFS)
        {
            linkType = (LinkType)ui->scsystemWorkWay->currentData().toInt();
        }
    }

    ManualMessage manualMsg;
    manualMsg.manualType = ManualType(type);
    manualMsg.linkMasterSlave = (MasterSlave)mMasterSlaveLinkBtnGroup.checkedId();  // 使用主用链路还是备用链路

    if (uilinkType == UiLinkType::CKLink || uilinkType == UiLinkType::CK_DTLink)
    {
        // 测控第一个Tabl
        if (ui->ckWidget1->getWorkMode() != NotDefine)
        {
            auto linkLine = ui->ckWidget1->getLinkLine();
            linkLine.linkType = linkType;

            manualMsg.appendLine(linkLine);
        }

        // 测控第二个Tabl
        if (ui->ckWidget2->getWorkMode() != NotDefine)
        {
            auto linkLine = ui->ckWidget2->getLinkLine();
            linkLine.linkType = linkType;

            manualMsg.appendLine(linkLine);
        }
    }
    if (uilinkType == UiLinkType::DTLink || uilinkType == UiLinkType::CK_DTLink)
    {
        // 获取Ka低速链路
        if (ui->kadsWidget->isSelectedTaskCode())
        {
            auto linkLine = ui->kadsWidget->getLinkLine();
            linkLine.linkType = linkType;

            manualMsg.appendLine(linkLine);
        }
        // 获取Ka低速链路
        if (ui->xdsWidget->isSelectedTaskCode())
        {
            auto linkLine = ui->xdsWidget->getLinkLine();
            linkLine.linkType = linkType;

            manualMsg.appendLine(linkLine);
        }

        // 获取Ka高速链路
        if (ui->kagsWidget->isSelectedTaskCode())
        {
            auto linkLine = ui->kagsWidget->getLinkLine();
            linkLine.linkType = linkType;

            manualMsg.appendLine(linkLine);
        }
    }

    //处理主跟目标代号和标识
    dealMainTrackTargetInfo(manualMsg);

    QString json;
    json << manualMsg;
    qDebug().noquote() << json;

    emit signalManualFunction(json);
}

void TaskProcessControlWidget::manualXLFunction(bool sOrKa)
{
    ManualMessage manualMsg;
    manualMsg.manualType = ManualType::OnekeyXL;
    manualMsg.linkMasterSlave = (MasterSlave)mMasterSlaveLinkBtnGroup.checkedId();  // 使用主用链路还是备用链路

    // 测控第一个Tabl
    if (ui->ckWidget1->getWorkMode() != NotDefine)
    {
        auto linkLine = ui->ckWidget1->getLinkLine();
        linkLine.linkType = LinkType::RWFS;

        manualMsg.appendLine(linkLine);

        if (sOrKa)
        {
            manualMsg.xlMode = linkLine.workMode;
        }
    }

    // 测控第二个Tabl
    if (ui->ckWidget2->getWorkMode() != NotDefine)
    {
        auto linkLine = ui->ckWidget2->getLinkLine();
        linkLine.linkType = LinkType::RWFS;

        manualMsg.appendLine(linkLine);

        if (!sOrKa)
        {
            manualMsg.xlMode = linkLine.workMode;
        }
    }

    //处理主跟目标代号和标识
    dealMainTrackTargetInfo(manualMsg);

    QString json;
    json << manualMsg;
    qDebug().noquote() << json;

    emit signalManualFunction(json);
}

void TaskProcessControlWidget::manualDtFunction(ManualType type, LinkType linkType, TaskProcessControlWidget::UiLinkType uilinkType, int _type)
{
    ManualMessage manualMsg;
    manualMsg.manualType = ManualType(type);
    manualMsg.linkMasterSlave = (MasterSlave)mMasterSlaveLinkBtnGroup.checkedId();  // 使用主用链路还是备用链路

    if (uilinkType == UiLinkType::DTLink || uilinkType == UiLinkType::CK_DTLink)
    {
        // 获取Ka低速链路
        if (ui->kadsWidget->isSelectedTaskCode() && _type == (int)LinkType::SPMNYYXBH + 100)
        {
            auto linkLine = ui->kadsWidget->getLinkLine();
            linkLine.linkType = linkType;

            manualMsg.appendLine(linkLine);
        }
        // 获取x低速链路
        if (ui->xdsWidget->isSelectedTaskCode() && _type == (int)LinkType::SPMNYYXBH + 101)
        {
            auto linkLine = ui->xdsWidget->getLinkLine();
            linkLine.linkType = linkType;

            manualMsg.appendLine(linkLine);
        }

        // 获取Ka高速链路
        if (ui->kagsWidget->isSelectedTaskCode() && _type == (int)LinkType::SPMNYYXBH + 102)
        {
            auto linkLine = ui->kagsWidget->getLinkLine();
            linkLine.linkType = linkType;

            manualMsg.appendLine(linkLine);
        }
    }

    //处理主跟目标代号和标识
    dealMainTrackTargetInfo(manualMsg);

    QString json;
    json << manualMsg;
    qDebug().noquote() << json;

    emit signalManualFunction(json);
}

bool TaskProcessControlWidget::checkMistake()
{
    auto ckWorkMode = ui->ckWidget1->getWorkMode();
    auto ckWorkMode2 = ui->ckWidget2->getWorkMode();

    if (ckWorkMode != NotDefine && !ui->ckWidget1->isSelectedTaskCode())
    {
        QMessageBox::information(this, "提示", "请选择S测控的任务代号", "确定");
        return false;
    }

    if (ckWorkMode2 != NotDefine && !ui->ckWidget2->isSelectedTaskCode())
    {
        QMessageBox::information(this, "提示", "请选择Ka测控的任务代号", "确定");
        return false;
    }
    auto haveKaDs = ui->kadsWidget->isSelectedTaskCode();
    auto haveXDs = ui->xdsWidget->isSelectedTaskCode();
    auto haveKags = ui->kagsWidget->isSelectedTaskCode();
    if (!haveKaDs && !haveXDs && !haveKags && ckWorkMode == NotDefine && ckWorkMode2 == NotDefine)
    {
        QMessageBox::information(this, "提示", "请至少选择一个工作模式", "确定");
        return false;
    }

    //这个是判断S扩二+Ka扩二时主跟目标的任务代号是否一致
    if (ckWorkMode == Skuo2 && ckWorkMode2 == KaKuo2)
    {
        auto sLink = ui->ckWidget1->getLinkLine();
        auto kaLink = ui->ckWidget2->getLinkLine();

        if (sLink.targetMap[sLink.masterTargetNo].taskCode != kaLink.targetMap[kaLink.masterTargetNo].taskCode)
        {
            QString msg = "当前所选模式为S扩二+Ka扩二，但主跟目标的任务代号不一致，是否继续下发？";
            if (QMessageBox::information(this, "提示", msg, QString("确定"), QString("取消")))
            {
                return false;
            }
        }
    }

    //下面是判断离线和分控设备
    QString offNameText;
    QString subNameText;
    if (!m_offDeviceNameList.isEmpty())
    {
        auto offDeviceNameText = m_offDeviceNameList.join(",");
        offNameText = "当前离线设备有：" + offDeviceNameText + "。" + "\n";
    }

    if (!m_subDeviceNameList.isEmpty())
    {
        auto subDeviceNameText = m_subDeviceNameList.join(",");
        subNameText = "当前分控设备有：" + subDeviceNameText + +"。" + "\n";
    }

    if (!m_offDeviceNameList.isEmpty() || !m_subDeviceNameList.isEmpty())
    {
        if (QMessageBox::information(this, "提示", offNameText + subNameText + "需要继续下发？", QString("确定"), QString("取消")))
        {
            return false;
        }
    }

    return true;
}

bool TaskProcessControlWidget::checkXLMistake(bool sOrKa)
{
    auto ckWorkMode = ui->ckWidget1->getWorkMode();
    auto ckWorkMode2 = ui->ckWidget2->getWorkMode();
    if (sOrKa)
    {
        if (ckWorkMode == NotDefine || !ui->ckWidget1->isSelectedTaskCode())
        {
            QMessageBox::information(this, "提示", "请选择S测控的任务代号", "确定");
            return false;
        }
    }
    else
    {
        if (ckWorkMode2 == NotDefine || !ui->ckWidget2->isSelectedTaskCode())
        {
            QMessageBox::information(this, "提示", "请选择Ka测控的任务代号", "确定");
            return false;
        }
    }

    //下面是判断离线和分控设备
    QString offNameText;
    QString subNameText;
    if (!m_offDeviceNameList.isEmpty())
    {
        auto offDeviceNameText = m_offDeviceNameList.join(",");
        offNameText = "当前离线设备有：" + offDeviceNameText + "。" + "\n";
    }

    if (!m_subDeviceNameList.isEmpty())
    {
        auto subDeviceNameText = m_subDeviceNameList.join(",");
        subNameText = "当前分控设备有：" + subDeviceNameText + +"。" + "\n";
    }

    if (!m_offDeviceNameList.isEmpty() || !m_subDeviceNameList.isEmpty())
    {
        if (QMessageBox::information(this, "提示", offNameText + subNameText + "需要继续下发？", QString("确定"), QString("取消")))
        {
            return false;
        }
    }

    return true;
}

void TaskProcessControlWidget::dealMainTrackTargetInfo(ManualMessage& manualMsg)
{
    bool isContainCk = manualMsg.isContainsCkMode();

    SystemWorkMode useWorkMode;
    //测控
    if (isContainCk)
    {
        for (const auto& link : manualMsg.linkLineMap)
        {
            if (SystemWorkModeHelper::isMeasureContrlWorkMode(link.workMode))  // 测控模式都使用同一个任务代号 随便一个就行了
            {
                useWorkMode = link.workMode;
                break;
            }
        }
    }
    else  //数传  Ka高低速优先  X低速放最后
    {
        for (const auto& link : manualMsg.linkLineMap)
        {
            if (manualMsg.linkLineMap.contains(KaDS))
            {
                useWorkMode = KaDS;
                break;
            }
            if (manualMsg.linkLineMap.contains(KaGS))
            {
                useWorkMode = KaGS;
                break;
            }
            if (manualMsg.linkLineMap.contains(XDS))
            {
                useWorkMode = XDS;
                break;
            }
        }
    }

    auto linkData = manualMsg.linkLineMap[useWorkMode];
    auto taskcode = linkData.targetMap[linkData.masterTargetNo].taskCode;
    auto taskBz = m_taskCodeSateliteMap[taskcode].m_satelliteIdentification;
    manualMsg.masterTaskCode = taskcode;
    manualMsg.masterTaskBz = taskBz;
}

void TaskProcessControlWidget::resetUIData()
{
    /**********************重置测控界面*********************/
    //重置系统工作方式
    ui->systemWorkWay->setCurrentIndex(0);
    //重置S和Ka测控功放
    if (m_sHP)
    {
        m_sHP->resetUIData();
    }
    if (m_kaCKHP)
    {
        m_kaCKHP->resetUIData();
    }
    //重置系统捕获流程控制界面  注：只重置测控基带选择参数，其它不管，其它参数如果重置会下发命令所以不动它
    m_systemCaptureProcessControl->resetUIData();
    /**********************重置数传界面*********************/
    //重置系统工作方式
    ui->scsystemWorkWay->setCurrentIndex(0);
    //重置数传功放
    if (m_kaSCHP)
    {
        m_kaSCHP->resetUIData();
    }
    //重置存储转发单元和命令界面参数
    ui->dtwidget->resetUIData();
}

void TaskProcessControlWidget::refreshRunningTaskPlanData()
{
    ui->ckWidget1->refreshRunningTaskPlanData(m_runningTaskPlanDataMsg);
    ui->ckWidget2->refreshRunningTaskPlanData(m_runningTaskPlanDataMsg);
    ui->kadsWidget->refreshRunningTaskPlanData(m_runningTaskPlanDataMsg);
    ui->xdsWidget->refreshRunningTaskPlanData(m_runningTaskPlanDataMsg);
    ui->kagsWidget->refreshRunningTaskPlanData(m_runningTaskPlanDataMsg);
}

void TaskProcessControlWidget::slotCmdResponse(const QByteArray& data)
{
    QByteArray bak = data;

    ControlCmdResponse controlcmdResponse;

    bak >> controlcmdResponse;

    auto deviceID = controlcmdResponse.deviceID;

    //存储转发的命令才会进入到这里面
    if (deviceID.sysID == 8 && deviceID.devID == 1 && controlcmdResponse.cmdId == 1)
    {
        auto result = DevProtocolEnumHelper::controlCmdResponseToDescStr(controlcmdResponse.responseResult);
        QMessageBox::information(this, QString("提示"), QString(result), QString("确定"));
    }
}

void TaskProcessControlWidget::slotRefreshData()
{
    auto satelliteList = GlobalData::getSatelliteManagementDataByObject();

    if (!satelliteList.size())
    {
        QMessageBox::information(this, "提示", "获取卫星数据失败", "确定");
    }

    QMap<SystemWorkMode, QList<QString>> workModeTaskcodeMap;        // 工作模式对应的任务代号的列表
    QMap<QString, QMap<SystemWorkMode, int>> taskCodeWorkModeDpMap;  // QMap<任务代号, QMap<工作模式, 点频数>>

    //满足界面憨憨需求 wjy
    QMap<SystemWorkMode, QList<QString>> nokak2workModeTaskcodeMap;        // 工作模式对应的任务代号的列表
    QMap<QString, QMap<SystemWorkMode, int>> nokak2taskCodeWorkModeDpMap;  // QMap<任务代号, QMap<工作模式, 点频数>>

    QMap<SystemWorkMode, QList<QString>> onlykak2workModeTaskcodeMap;        // 工作模式对应的任务代号的列表
    QMap<QString, QMap<SystemWorkMode, int>> onlykak2taskCodeWorkModeDpMap;  // QMap<任务代号, QMap<工作模式, 点频数>>

    for (auto item : satelliteList)
    {
        m_taskCodeSateliteMap[item.m_satelliteCode] = item;
        auto map = item.getworkMode();
        for (auto workModeId : map.keys())
        {
            SystemWorkMode workMode = (SystemWorkMode)workModeId;

            workModeTaskcodeMap[workMode].append(map[workModeId]);
            taskCodeWorkModeDpMap[item.m_satelliteCode][workMode] = item.getDpNumByWorkMode(workMode);

            if (workMode == SystemWorkMode::KaKuo2)
            {
                onlykak2workModeTaskcodeMap[workMode].append(map[workModeId]);
                onlykak2taskCodeWorkModeDpMap[item.m_satelliteCode][workMode] = item.getDpNumByWorkMode(workMode);
            }
            else
            {
                nokak2workModeTaskcodeMap[workMode].append(map[workModeId]);
                nokak2taskCodeWorkModeDpMap[item.m_satelliteCode][workMode] = item.getDpNumByWorkMode(workMode);
            }
        }
    }

    // 重新设置工作模式、任务代号、点频信息

    ui->ckWidget1->setWorkModeTaskCodeMap(nokak2workModeTaskcodeMap);
    ui->ckWidget1->setTaskCodeWorkModeDpMap(nokak2taskCodeWorkModeDpMap);

    ui->ckWidget2->setWorkModeTaskCodeMap(onlykak2workModeTaskcodeMap);
    ui->ckWidget2->setTaskCodeWorkModeDpMap(onlykak2taskCodeWorkModeDpMap);

    ui->ckWidget1->setWorkMode(ui->ckWidget1->getWorkMode());
    ui->ckWidget2->setWorkMode(ui->ckWidget2->getWorkMode());

    ui->kadsWidget->setWorkModeTaskCodeMap(workModeTaskcodeMap);
    ui->kadsWidget->setTaskCodeWorkModeDpMap(taskCodeWorkModeDpMap);

    ui->xdsWidget->setWorkModeTaskCodeMap(workModeTaskcodeMap);
    ui->xdsWidget->setTaskCodeWorkModeDpMap(taskCodeWorkModeDpMap);

    ui->kagsWidget->setWorkModeTaskCodeMap(workModeTaskcodeMap);
    ui->kagsWidget->setTaskCodeWorkModeDpMap(taskCodeWorkModeDpMap);

    ui->kadsWidget->setWorkMode(SystemWorkMode::KaDS);
    ui->xdsWidget->setWorkMode(SystemWorkMode::XDS);
    ui->kagsWidget->setWorkMode(SystemWorkMode::KaGS);
    ui->dtwidget->refreUIData();
}

void TaskProcessControlWidget::slotWorkModeChanged(SystemWorkMode workMode)
{
    // 去改变系统工作方式
    changeSystemWorkWay();

    // 是否显示扩频和数传单选按钮
    slotIsShowRadioBtn();

    // 显示高功放
    hpTableReare();
}

void TaskProcessControlWidget::slotSystemWorkWayChanged() { slotIsShowRadioBtn(); }

void TaskProcessControlWidget::slotIsShowRadioBtn()
{
    auto workMode = NotDefine;

    if (ui->ckTabWidget->currentIndex() == 0)
    {
        workMode = ui->ckWidget1->getWorkMode();
    }
    else
    {
        workMode = ui->ckWidget2->getWorkMode();
    }

    // 不是测控模式直接返回
    if (!SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
    {
        return;
    }

    auto curProject = ExtendedConfig::curProjectID();
}

void TaskProcessControlWidget::slotResourceReleaseBtnClick()
{
    if (QMessageBox::information(this, "提示", "是否进行资源释放？", QString("确定"), QString("取消")))
    {
        return;
    }
    ui->ResultOutInfo->clear();
    ManualMessage manualMsg;
    manualMsg.manualType = ManualType::ResourceRelease;
    manualMsg.linkMasterSlave = (MasterSlave)mMasterSlaveLinkBtnGroup.checkedId();

    QString json;
    json << manualMsg;

    emit signalManualFunction(json);

    //四个按钮的样式清空
    ui->linkSetBtn->setStyleSheet("");
    ui->setParamMacroBtn->setStyleSheet("");
    ui->sOneKeyXLBtn->setStyleSheet("");
    ui->kaOneKeyXLBtn->setStyleSheet("");
}

void TaskProcessControlWidget::slotSetParamMacroBtnClick()
{
    ui->ResultOutInfo->clear();
    // 下发参数宏
    if (!checkMistake())  //做一些判断
    {
        return;
    }
    manualFunction(ManualType::ParamMacro, LinkType::RWFS, UiLinkType::CK_DTLink);

    ui->setParamMacroBtn->setStyleSheet("background-color:rgb(0,170,0);color:rgb(255,0,0);");

    //重置部分界面信息
    resetUIData();
}

void TaskProcessControlWidget::slotLinkSetBtnClick()
{
    ui->ResultOutInfo->clear();
    // 下发配置宏
    if (!checkMistake())  //做一些判断
    {
        return;
    }
    manualFunction(ManualType::ConfigMacro, LinkType::RWFS, UiLinkType::CK_DTLink);

    ui->linkSetBtn->setStyleSheet("background-color:rgb(0,170,0);color:rgb(255,0,0);");
}

void TaskProcessControlWidget::slotSOneKeyXLBtnClick()
{
    ui->ResultOutInfo->clear();
    // 一键S校零
    if (!checkXLMistake(true))  //做一些判断
    {
        return;
    }
    manualXLFunction(true);

    ui->sOneKeyXLBtn->setStyleSheet("background-color:rgb(0,170,0);color:rgb(255,0,0);");
}
void TaskProcessControlWidget::slotKaOneKeyXLBtnClick()
{
    ui->ResultOutInfo->clear();
    // 一键Ka校零
    if (!checkXLMistake(false))  //做一些判断
    {
        return;
    }
    manualXLFunction(false);

    ui->kaOneKeyXLBtn->setStyleSheet("background-color:rgb(0,170,0);color:rgb(255,0,0);");
}

void TaskProcessControlWidget::slotReadSystemLog(const SystemLogData& data)
{
    //只接收关键数据
    auto contains = data.context;

    if (data.level == LogLevel::SpecificTips)
    {
        ui->ResultOutInfo->setStyleSheet("color: rgb(0, 170, 0);");
        ui->ResultOutInfo->setText(contains);
    }
}

void TaskProcessControlWidget::slotsCkSettingBtnClick()
{
    // 下发参数宏
    if (!checkMistake())  //做一些判断
    {
        return;
    }
    manualFunction(ManualType::ConfigMacroAndParamMacro, (LinkType)ui->systemWorkWay->currentData().toInt(), UiLinkType::CKLink);
}

void TaskProcessControlWidget::slotsDtSettingBtnClick()
{
    // 下发参数宏
    if (!checkMistake())  //做一些判断
    {
        return;
    }

    if ((LinkType)ui->scsystemWorkWay->currentData().toInt() == LinkType::ZPBH ||
        (LinkType)ui->scsystemWorkWay->currentData().toInt() == LinkType::RWFS)
    {
        manualFunction(ManualType::ConfigMacroAndParamMacro, (LinkType)ui->scsystemWorkWay->currentData().toInt(), UiLinkType::DTLink);
    }
    else
    {
        manualDtFunction(ManualType::ConfigMacroAndParamMacro, LinkType::SPMNYYXBH, UiLinkType::DTLink, ui->scsystemWorkWay->currentData().toInt());
    }
}

void TaskProcessControlWidget::slotRefreshUI(const StatusBarData& statusBarData)
{
    auto& offlineDeviceMap = statusBarData.deviceIDOffName;
    auto& subCtrlDeviceMap = statusBarData.deviceIDSubName;

    if (m_offDeviceNameMap.size() != offlineDeviceMap.size())
    {
        m_offDeviceNameMap.clear();
        m_offDeviceNameList.clear();
        m_offDeviceNameMap = offlineDeviceMap;

        for (auto deviceID : m_offDeviceNameMap.keys())
        {
            //伪码服务器 测试计算机 联试应答机终端/扩跳不显示
            if ((deviceID.sysID == 6 && deviceID.devID == 2) ||
                (deviceID.sysID == 6 && deviceID.devID == 0 && (deviceID.extenID == 1 || deviceID.extenID == 2)) ||
                ((deviceID.sysID == 4 && deviceID.devID == 5)))
            {
                continue;
            }
            else
            {
                m_offDeviceNameList.append(m_offDeviceNameMap[deviceID]);
            }
        }
    }

    if (m_subDeviceNameMap.size() != subCtrlDeviceMap.size())
    {
        m_subDeviceNameMap.clear();
        m_subDeviceNameList.clear();
        m_subDeviceNameMap = subCtrlDeviceMap;

        for (auto deviceID : m_subDeviceNameMap.keys())
        {
            //伪码服务器 测试计算机 联试应答机终端/扩跳不显示
            if ((deviceID.sysID == 6 && deviceID.devID == 2) ||
                (deviceID.sysID == 6 && deviceID.devID == 0 && (deviceID.extenID == 1 || deviceID.extenID == 2)) ||
                ((deviceID.sysID == 4 && deviceID.devID == 5)))
            {
                continue;
            }
            else
            {
                m_subDeviceNameList.append(m_subDeviceNameMap[deviceID]);
            }
        }
    }
}

void TaskProcessControlWidget::slotReadAutoRunTaskStepStatus(const CurrentRunningTaskPlanData& data) { m_runningTaskPlanDataMsg = data.msg; }
