#include "TaskProcessControlWidget.h"
#include "ui_TaskProcessControlWidget.h"

#include <QMessageBox>
#include <QTreeWidget>

#include "ExtendedConfig.h"
#include "HP.h"
#include "PlanRunMessageSerialize.h"
#include "QssCommonHelper.h"
#include "SatelliteManagementDefine.h"
#include "SubscriberHelper.h"
#include "SystemCaptureProcessControl.h"
#include "SystemLogMessageSerialize.h"

TaskProcessControlWidget::TaskProcessControlWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TaskProcessControlWidget)
{
    ui->setupUi(this);

    initUI();

    initSlot();

    this->setWindowIcon(QIcon(":/image/task_process_ctrl.png"));

    //初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
}

TaskProcessControlWidget::~TaskProcessControlWidget() { delete ui; }

void TaskProcessControlWidget::showEvent(QShowEvent*)
{
    // 4426用户要求第一次显示时自动更新数据，之后打开保持上一次选择的状态不变
    static bool isFirstShow = true;
    if (isFirstShow)
    {
        isFirstShow = false;
        slotRefreshData();
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

    // 设置隐藏
    ui->kpRB->setVisible(false);
    ui->dtRB->setVisible(false);
    ui->kpRB->setChecked(true);

    // 初始化高功放界面
    m_sHP = new HP;
    m_sHP->setType(HP::SCK);
    connect(m_sHP, &HP::signalsCmdDeviceJson, this, &TaskProcessControlWidget::signalsCmdDeviceJson);
    connect(m_sHP, &HP::signalsUnitDeviceJson, this, &TaskProcessControlWidget::signalsUnitDeviceJson);
    m_kaSCHP = new HP;
    m_kaSCHP->setType(HP::KaSC);
    connect(m_kaSCHP, &HP::signalsCmdDeviceJson, this, &TaskProcessControlWidget::signalsCmdDeviceJson);
    connect(m_kaSCHP, &HP::signalsUnitDeviceJson, this, &TaskProcessControlWidget::signalsUnitDeviceJson);
    m_kaCKHP = new HP;
    m_kaCKHP->setType(HP::KaCK);
    connect(m_kaCKHP, &HP::signalsCmdDeviceJson, this, &TaskProcessControlWidget::signalsCmdDeviceJson);
    connect(m_kaCKHP, &HP::signalsUnitDeviceJson, this, &TaskProcessControlWidget::signalsUnitDeviceJson);

    ui->scrollArea_2->setMinimumHeight(180);

    // 初始化系统捕获流程界面
    m_systemCaptureProcessControl = new SystemCaptureProcessControl(this);
    connect(m_systemCaptureProcessControl, &SystemCaptureProcessControl::signalsCmdDeviceJson, this, &TaskProcessControlWidget::signalsCmdDeviceJson);
    connect(m_systemCaptureProcessControl, &SystemCaptureProcessControl::signalsUnitDeviceJson, this,
            &TaskProcessControlWidget::signalsUnitDeviceJson);

    auto systemCaptureProcessControlGroupBox = new QGroupBox("系统捕获流程控制");
    systemCaptureProcessControlGroupBox->setLayout(new QHBoxLayout);
    systemCaptureProcessControlGroupBox->layout()->addWidget(m_systemCaptureProcessControl);

    ui->scrollAreaWidgetContents->layout()->addWidget(systemCaptureProcessControlGroupBox);

    m_systemCaptureProcessControl->setCurModeComBox(ui->ckWidget1->getWorkModeComboBox());
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
    connect(ui->scWidget1, &SCTaskWorkModeTaskCodeSelectWidget::signalWorkModeChanged, this, &TaskProcessControlWidget::slotWorkModeChanged);
    connect(ui->sdWidget, &SDTaskWorkModeTaskCodeSelectWidget::signalWorkModeChanged, this, &TaskProcessControlWidget::slotWorkModeChanged);

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
    connect(ui->oneKeyXLBtn, &QPushButton::clicked, this, &TaskProcessControlWidget::slotOneKeyXLBtnClick);

    // 快捷控制和手动控制界面切换
    connect(ui->controlTabWidget, &QTabWidget::currentChanged, this, &TaskProcessControlWidget::slotContrlTabChanged);
    // 测控tab切换
    connect(ui->ckTabWidget, &QTabWidget::currentChanged, this, &TaskProcessControlWidget::slotCKTabChanged);
    //系统日志
    connect(SubscriberHelper::getInstance(), &SubscriberHelper::signalSystemLogReadable, this, &TaskProcessControlWidget::slotReadSystemLog);
}

void TaskProcessControlWidget::changeSystemWorkWay()
{
    auto index = ui->controlTabWidget->currentIndex();
    if (index == 0)  // 快捷控制界面
    {
        SystemWorkMode ckWorkMode1 = NotDefine;
        ckWorkMode1 = ui->ckWidget1->getWorkMode();

        SystemWorkMode ckWorkMode2 = NotDefine;
        ckWorkMode2 = ui->ckWidget2->getWorkMode();

        auto scWorkMode = ui->scWidget1->getWorkMode();

        ui->systemWorkWay->clear();

        // 只有选择了测控模式
        if ((ckWorkMode1 != NotDefine || ckWorkMode2 != NotDefine) && (scWorkMode == NotDefine))
        {
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::RWFS), static_cast<int>(LinkType::RWFS));
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::LSYDJYXBH), static_cast<int>(LinkType::LSYDJYXBH));
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::PKXLBPQBH), static_cast<int>(LinkType::PKXLBPQBH));
            // ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::SPMNYWXBH), static_cast<int>(LinkType::SPMNYWXBH));
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::SPMNYYXBH), static_cast<int>(LinkType::SPMNYYXBH));
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::SZHBH), static_cast<int>(LinkType::SZHBH));
        }
        // 只选择了数传模式
        else if ((ckWorkMode1 == NotDefine && ckWorkMode2 == NotDefine) && (scWorkMode != NotDefine))
        {
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::RWFS), static_cast<int>(LinkType::RWFS));
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::ZPBH), static_cast<int>(LinkType::ZPBH));
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::SPMNYYXBH), static_cast<int>(LinkType::SPMNYYXBH));
        }
        // 测控数传都选择
        else if ((ckWorkMode1 != NotDefine || ckWorkMode2 != NotDefine) && (scWorkMode != NotDefine))
        {
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::RWFS), static_cast<int>(LinkType::RWFS));
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::SPMNYYXBH), static_cast<int>(LinkType::SPMNYYXBH));
        }
    }
    else if (index == 1)  // 手动控制界面
    {
        auto currentMode = ui->sdWidget->getWorkMode();

        ui->systemWorkWay->clear();

        // 选择的是测控模式
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(currentMode))
        {
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::RWFS), static_cast<int>(LinkType::RWFS));
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::LSYDJYXBH), static_cast<int>(LinkType::LSYDJYXBH));
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::PKXLBPQBH), static_cast<int>(LinkType::PKXLBPQBH));
            // ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::SPMNYWXBH),static_cast<int>( LinkType::SPMNYWXBH));
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::SPMNYYXBH), static_cast<int>(LinkType::SPMNYYXBH));
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::SZHBH), static_cast<int>(LinkType::SZHBH));
        }
        // 选择的是数传模式
        else if (SystemWorkModeHelper::isDataTransmissionWorkMode(currentMode))
        {
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::RWFS), static_cast<int>(LinkType::RWFS));
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::ZPBH), static_cast<int>(LinkType::ZPBH));
            ui->systemWorkWay->addItem(PlanRunHelper::getLinkTypeDesc(LinkType::SPMNYYXBH), static_cast<int>(LinkType::SPMNYYXBH));
        }
    }
}

void TaskProcessControlWidget::hpTableReare()
{
    ui->hpTabWidget->clear();
    m_sHP->setVisible(false);
    m_kaCKHP->setVisible(false);
    m_kaSCHP->setVisible(false);

    int index = ui->controlTabWidget->currentIndex();
    if (index == 0)  // 快捷控制界面
    {
        auto ckWorkMode1 = ui->ckWidget1->getWorkMode();
        auto ckWorkMode2 = ui->ckWidget2->getWorkMode();

        auto scWorkMode = ui->scWidget1->getWorkMode();

        if ((ckWorkMode1 != NotDefine && ckWorkMode1 != KaKuo2) || (ckWorkMode2 != NotDefine && ckWorkMode2 != KaKuo2))
        {
            ui->hpTabWidget->addTab(m_sHP, "S功放");
        }
        if (ckWorkMode1 == KaKuo2 || ckWorkMode2 == KaKuo2)
        {
            ui->hpTabWidget->addTab(m_kaCKHP, "Ka测控功放");
        }

        if (scWorkMode == KaDS)
        {
            ui->hpTabWidget->addTab(m_kaSCHP, "Ka数传功放");
        }
    }
    else  // 手动控制界面
    {
        auto workMode = ui->sdWidget->getWorkMode();
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
        {
            if (workMode != KaKuo2)
            {
                ui->hpTabWidget->addTab(m_sHP, "S功放");
            }
            else
            {
                ui->hpTabWidget->addTab(m_kaCKHP, "Ka测控功放");
            }
        }
        else if (workMode == KaDS)
        {
            ui->hpTabWidget->addTab(m_kaSCHP, "Ka数传功放");
        }
    }
}

void TaskProcessControlWidget::manualFunction(ManualType type)
{
    ManualMessage manualMsg;
    manualMsg.manualType = ManualType(type);
    manualMsg.linkMasterSlave = (MasterSlave)mMasterSlaveLinkBtnGroup.checkedId();  // 使用主用链路还是备用链路

    int index = ui->controlTabWidget->currentIndex();
    if (index == 0)  // 快捷控制界面
    {
        // 测控第一个Tabl
        if (ui->ckWidget1->getWorkMode() != NotDefine)
        {
            auto linkLine = ui->ckWidget1->getLinkLine();
            linkLine.linkType = (LinkType)ui->systemWorkWay->currentData().toInt();  // 链路类型

            // 主用扩频还是数传（一体化+XXX中使用）
            if (ui->kpRB->isVisible() && ui->dtRB->isVisible())
            {
                if (ui->kpRB->isChecked())
                {
                    linkLine.masterType = MasterType::YTH_KP;
                }
                if (ui->dtRB->isChecked())
                {
                    linkLine.masterType = MasterType::YTH_DT;
                }
            }

            manualMsg.appendLine(linkLine);
        }

        // 测控第二个Tabl
        if (ui->ckWidget2->getWorkMode() != NotDefine)
        {
            auto linkLine = ui->ckWidget2->getLinkLine();
            linkLine.linkType = (LinkType)ui->systemWorkWay->currentData().toInt();  // 链路类型

            // 主用扩频还是数传（一体化+XXX中使用）
            if (ui->kpRB->isVisible() && ui->dtRB->isVisible())
            {
                if (ui->kpRB->isChecked())
                {
                    linkLine.masterType = MasterType::YTH_KP;
                }
                if (ui->dtRB->isChecked())
                {
                    linkLine.masterType = MasterType::YTH_DT;
                }
            }

            manualMsg.appendLine(linkLine);
        }

        // 判断数传是否可用
        if (ui->scWidget1->getWorkMode() != NotDefine)
        {
            auto linkLine = ui->scWidget1->getLinkLine();
            linkLine.linkType = (LinkType)ui->systemWorkWay->currentData().toInt();  // 链路类型

            manualMsg.appendLine(linkLine);
        }
    }
    else if (index == 1)  // 手动控制界面
    {
        manualMsg.isManualContrl = true;

        auto linkLine = ui->sdWidget->getLinkLink();
        if (linkLine.targetMap.isEmpty())
        {
            return;
        }

        linkLine.linkType = (LinkType)ui->systemWorkWay->currentData().toInt();  // 链路类型

        // 主用扩频还是数传（一体化+XXX中使用）
        if (ui->kpRB->isVisible() && ui->dtRB->isVisible())
        {
            if (ui->kpRB->isChecked())
            {
                linkLine.masterType = MasterType::YTH_KP;
            }
            if (ui->dtRB->isChecked())
            {
                linkLine.masterType = MasterType::YTH_DT;
            }
        }
        ConfigMacroData configMacroData;
        ui->sdWidget->getConfigMacroData(configMacroData);

        manualMsg.configMacroData = configMacroData;
        manualMsg.appendLine(linkLine);
    }

    QString json;
    json << manualMsg;
    qDebug().noquote() << json;

    emit signalManualFunction(json);
}

bool TaskProcessControlWidget::checkMistake()
{
    int index = ui->controlTabWidget->currentIndex();
    if (index == 0)  // 快捷控制界面
    {
        auto ckWorkMode = ui->ckWidget1->getWorkMode();
        auto ckWorkMode2 = ui->ckWidget2->getWorkMode();
        auto scWorkMode = ui->scWidget1->getWorkMode();
        if (ckWorkMode == NotDefine && ckWorkMode2 == NotDefine && scWorkMode == NotDefine)
        {
            QMessageBox::information(this, "提示", "请至少选择一个工作模式", "确定");
            return false;
        }

        if (ckWorkMode != NotDefine && !ui->ckWidget1->isSelectedTaskCode())
        {
            QMessageBox::information(this, "提示", "请选择测控Tab1任务代号", "确定");
            return false;
        }

        if (ckWorkMode2 != NotDefine && !ui->ckWidget2->isSelectedTaskCode())
        {
            QMessageBox::information(this, "提示", "请选择测控Tab2任务代号", "确定");
            return false;
        }

        if (scWorkMode != NotDefine && !ui->scWidget1->isSelectedTaskCode())
        {
            QMessageBox::information(this, "提示", "请选择数传任务代号", "确定");
            return false;
        }
    }
    else  //手动控制界面
    {
        if (ui->sdWidget->getWorkMode() == NotDefine)
        {
            QMessageBox::information(this, "提示", "请至少选择一个工作模式", "确定");
            return false;
        }
        else if (!ui->sdWidget->isSelectedTaskCode())
        {
            QMessageBox::information(this, "提示", "请选择任务代号", "确定");
            return false;
        }
    }

    return true;
}

void TaskProcessControlWidget::slotRefreshData()
{
    SatelliteManagementDataList satelliteList;
    GlobalData::getSatelliteManagementData(satelliteList);

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

    ui->scWidget1->setWorkModeTaskCodeMap(workModeTaskcodeMap);
    ui->scWidget1->setTaskCodeWorkModeDpMap(taskCodeWorkModeDpMap);

    ui->sdWidget->setWorkModeTaskCodeMap(workModeTaskcodeMap);
    ui->sdWidget->setTaskCodeWorkModeDpMap(taskCodeWorkModeDpMap);

    // 触发信号槽，重新设置一下当前数据
    auto index = ui->controlTabWidget->currentIndex();
    if (index == 0)  // 快捷控制界面
    {
        ui->ckWidget1->setWorkMode(ui->ckWidget1->getWorkMode());
        ui->ckWidget2->setWorkMode(ui->ckWidget2->getWorkMode());

        ui->scWidget1->setWorkMode(ui->scWidget1->getWorkMode());
    }
    else if (index == 1)  // 手动控制界面
    {
        ui->sdWidget->setWorkMode(ui->sdWidget->getWorkMode());
    }
}

void TaskProcessControlWidget::slotWorkModeChanged(SystemWorkMode workMode)
{
    // 当测控Tab1工作模式改变，手动控制界面的工作模式也修改为对应的工作模式
    auto widget = qobject_cast<QWidget*>(sender());
    if (widget == ui->ckWidget1)
    {
        ui->sdWidget->setWorkMode(workMode);
    }

    // 去改变系统工作方式
    changeSystemWorkWay();

    // 是否显示扩频和数传单选按钮
    slotIsShowRadioBtn();

    // 显示高功放
    hpTableReare();

    // 切换当前工作模式对应的系统捕获流程控制界面
    if (SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
    {
        m_systemCaptureProcessControl->setShowWidget(workMode);
    }
}

void TaskProcessControlWidget::slotSystemWorkWayChanged() { slotIsShowRadioBtn(); }

void TaskProcessControlWidget::slotIsShowRadioBtn()
{
    auto systemWorkWay = (LinkType)ui->systemWorkWay->currentData().toUInt();  // 系统工作方式
    auto workMode = NotDefine;
    int index = ui->controlTabWidget->currentIndex();
    if (index == 0)
    {
        if (ui->ckTabWidget->currentIndex() == 0)
        {
            workMode = ui->ckWidget1->getWorkMode();
        }
        else
        {
            workMode = ui->ckWidget2->getWorkMode();
        }
    }
    else if (index == 1)
    {
        workMode = ui->sdWidget->getWorkMode();
    }

    // 不是测控模式直接返回
    if (!SystemWorkModeHelper::isMeasureContrlWorkMode(workMode))
    {
        return;
    }

    auto curProject = ExtendedConfig::curProjectID();
    if (curProject == "4424")
    {
        if ((systemWorkWay == LinkType::SZHBH || systemWorkWay == LinkType::PKXLBPQBH || systemWorkWay == LinkType::SPMNYYXBH) &&
            (workMode == SYTHSMJK2GZB || workMode == SYTHSMJK2BGZB || workMode == SYTHWXSK2 || workMode == SYTHGMLSK2 || workMode == SKT))
        {
            ui->kpRB->setVisible(true);
            ui->dtRB->setVisible(true);
        }
        else
        {
            ui->kpRB->setVisible(false);
            ui->dtRB->setVisible(false);
        }
    }
    else if (curProject == "4426")
    {
        ui->kpRB->setVisible(false);
        ui->dtRB->setVisible(false);
    }
}

void TaskProcessControlWidget::slotContrlTabChanged(int index)
{
    // 去改变系统工作方式
    changeSystemWorkWay();

    // 快捷控制界面
    if (index == 0)
    {
        if (ui->ckTabWidget->currentIndex() == 0)
        {
            m_systemCaptureProcessControl->setCurModeComBox(ui->ckWidget1->getWorkModeComboBox());
        }
        else
        {
            m_systemCaptureProcessControl->setCurModeComBox(ui->ckWidget2->getWorkModeComboBox());
        }
    }
    else  // 手动控制界面
    {
        m_systemCaptureProcessControl->setCurModeComBox(ui->sdWidget->getWorkModeComboBox());
    }
}

void TaskProcessControlWidget::slotCKTabChanged(int index)
{
    if (index == 0)
    {
        m_systemCaptureProcessControl->setCurModeComBox(ui->ckWidget1->getWorkModeComboBox());
    }
    else
    {
        m_systemCaptureProcessControl->setCurModeComBox(ui->ckWidget2->getWorkModeComboBox());
    }
}

void TaskProcessControlWidget::slotResourceReleaseBtnClick()
{
    // 资源释放
    if (!checkMistake())  //做一些判断
    {
        return;
    }
    manualFunction(ManualType::ResourceRelease);
}

void TaskProcessControlWidget::slotSetParamMacroBtnClick()
{
    // 下发参数宏
    if (!checkMistake())  //做一些判断
    {
        return;
    }
    manualFunction(ManualType::ParamMacro);
}

void TaskProcessControlWidget::slotLinkSetBtnClick()
{
    // 下发配置宏
    if (!checkMistake())  //做一些判断
    {
        return;
    }
    if (0 == QMessageBox::information(this, "提示！", "是否下发链路配置", "确定", "取消"))
    {
        manualFunction(ManualType::ConfigMacro);
    }
}

void TaskProcessControlWidget::slotOneKeyXLBtnClick()
{
    // 一键校零
    if (!checkMistake())  //做一些判断
    {
        return;
    }
    manualFunction(ManualType::OnekeyXL);
}

void TaskProcessControlWidget::slotReadSystemLog(const SystemLogData& data)
{
    //只接收来自这两个模块的数据
    if (data.module != "AutoTask4424Module" && data.module != "AutoTask4426Module")
    {
        return;
    }

    auto contains = data.context;

    if (data.level == LogLevel::Warning)
    {
        ui->ResultOutInfo->setStyleSheet("color: rgb(229, 153, 0);");
    }
    else if (data.level == LogLevel::Error)
    {
        ui->ResultOutInfo->setStyleSheet("color: rgb(231, 35, 35);");
    }
    else
    {
        ui->ResultOutInfo->setStyleSheet("color: rgb(0, 170, 0);");
    }
    ui->ResultOutInfo->setText(contains);
}
