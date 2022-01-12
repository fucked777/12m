#include "LocalPlanDialog.h"
#include "DataTransmissionCenterSerialize.h"
#include "GlobalData.h"
#include "SatelliteManagementSerialize.h"
#include "SystemWorkMode.h"
#include "TaskPlanMessageSerialize.h"
#include "XComboBox.h"
#include "ui_LocalPlanDialog.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QPalette>
#include <array>
/* 行分组 */
struct LPDRowGroup
{
    QComboBox* workmode_cbx{ nullptr };
    QComboBox* pointfrequency_cbx{ nullptr };
    QComboBox* yc{ nullptr };
    QComboBox* yk{ nullptr };
    QComboBox* sxhy{ nullptr };
    QComboBox* xxcj{ nullptr };
    QCheckBox* checkBox_yc{ nullptr };
    QCheckBox* checkBox_yk{ nullptr };
    QCheckBox* checkBox_cg{ nullptr };
    QCheckBox* checkBox_sc{ nullptr };
};

/* 初始每行的数据结构 */
#define ROW_GROUP_INIT(groupList, num)                                                                                                               \
    LPDRowGroup group##num;                                                                                                                          \
    group##num.workmode_cbx = ui->workmode_cbx##num;                                                                                                 \
    group##num.pointfrequency_cbx = ui->pointfrequency_cbx##num;                                                                                     \
    group##num.yc = ui->yc_##num;                                                                                                                    \
    group##num.yk = ui->yk_##num;                                                                                                                    \
    group##num.sxhy = ui->sxhy_##num;                                                                                                                \
    group##num.xxcj = ui->xxcj_##num;                                                                                                                \
    group##num.checkBox_yc = ui->checkBox_yc_##num;                                                                                                  \
    group##num.checkBox_yk = ui->checkBox_yk_##num;                                                                                                  \
    group##num.checkBox_cg = ui->checkBox_cg_##num;                                                                                                  \
    group##num.checkBox_sc = ui->checkBox_sc_##num;                                                                                                  \
    (m_impl->groupList)[num - 1] = group##num;

/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
/***********************************************************************************************************************/
class LocalPlanDialogImpl
{
public:
    static constexpr int rowNum = 8;
    QButtonGroup* taskType{ nullptr };
    QButtonGroup* openRemoteControl{ nullptr };
    QButtonGroup* closeRemoteControl{ nullptr };
    QButtonGroup* openUpSignal{ nullptr };
    QButtonGroup* closeUpSignal{ nullptr };
    DataTransmissionCenterMap dataCenterMap; /* 数传中心数据 */
    SatelliteManagementDataMap satelliteMap;
    SatelliteManagementData curSatellite;
    WidgetMode widgetMode{ WidgetMode::New };  // 是新建、编辑、查看状态
    int curTarget{ 1 };
    int targetChangeFlag = 0;
    QList<int> currWorkMode;  //当前的所有模式

    std::array<LPDRowGroup, rowNum> rowGroupList; /* 下方的8行数据每行一组 */

public:
    void refreshSatelliteData(const SatelliteManagementData& info, int currentWorkMode);
    void clearUI();

    static void codeGroupEnable(LPDRowGroup& group, bool enable);
    static void codeGroupInitItem(QComboBox* cb);
    template<int num>
    void workmodeCbxCurrentIndexChanged();
    template<int num>
    void pointFrequencyCbxCurrentIndexChanged();
    static void initDisableControl(LocalPlanDialogImpl* m_impl);
    static void codeGroupInit(LocalPlanDialogImpl* m_impl);
};
void LocalPlanDialogImpl::refreshSatelliteData(const SatelliteManagementData& info, int currentWorkModeID)
{
    /* currentWorkModeID 这个就是 测控 数传  测控+数传的ButtonGroupID */
    auto curWorkModeMap = info.getworkMode();
    QList<int> tempWorkMode;
    switch (currentWorkModeID)
    {
    case int(TaskPlanType::DataTrans): /* 数传 */
    {
        for (auto iter = curWorkModeMap.begin(); iter != curWorkModeMap.end(); ++iter)
        {
            auto key = iter.key();
            if (SystemWorkModeHelper::isDataTransmissionWorkMode(SystemWorkMode(key)))
            {
                tempWorkMode << key;
            }
        }
        break;
    }
    case int(TaskPlanType::MeasureCtrlDataTrasn): /* 数传+测控 */
    {
        for (auto iter = curWorkModeMap.begin(); iter != curWorkModeMap.end(); ++iter)
        {
            tempWorkMode << iter.key();
        }
        break;
    }
    case int(TaskPlanType::MeasureControl): /* 测控 */
    {
        for (auto iter = curWorkModeMap.begin(); iter != curWorkModeMap.end(); ++iter)
        {
            auto key = iter.key();
            if (SystemWorkModeHelper::isMeasureContrlWorkMode(SystemWorkMode(key)))
            {
                tempWorkMode << key;
            }
        }
        break;
    }
    }

    for (auto& item : rowGroupList)
    {
        auto& workModeCombobox = item.workmode_cbx;
        workModeCombobox->clear();
        workModeCombobox->addItem(QString("空"), 0);
        for (auto& item : tempWorkMode)
        {
            workModeCombobox->addItem(SystemWorkModeHelper::systemWorkModeToDesc(SystemWorkMode(item)), item);
        }
    }

    currWorkMode = tempWorkMode;
}

void LocalPlanDialogImpl::clearUI()
{
    for (auto& item : rowGroupList)
    {
        auto& workModeCombobox = item.workmode_cbx;
        workModeCombobox->clear();
        workModeCombobox->addItem(QString("空"), 0);
    }
}
void LocalPlanDialogImpl::codeGroupEnable(LPDRowGroup& group, bool enable)
{
    group.yk->setEnabled(enable);
    group.yc->setEnabled(enable);
    group.sxhy->setEnabled(enable);
    group.xxcj->setEnabled(enable);
    if (!enable)
    {
        group.yk->setCurrentIndex(0);
        group.yc->setCurrentIndex(0);
        group.sxhy->setCurrentIndex(0);
        group.xxcj->setCurrentIndex(0);
    }
}
/* 码组初始化 */
void LocalPlanDialogImpl::codeGroupInitItem(QComboBox* cb)
{
    cb->clear();
    cb->addItem("1", 1);
    cb->addItem("2", 2);
    cb->addItem("3", 3);
    cb->addItem("4", 4);
    cb->addItem("5", 5);
    cb->addItem("6", 6);
    cb->addItem("7", 7);
    cb->addItem("8", 8);
}
void LocalPlanDialogImpl::codeGroupInit(LocalPlanDialogImpl* m_impl)
{
    for (auto& item : m_impl->rowGroupList)
    {
        codeGroupInitItem(item.yk);
        codeGroupInitItem(item.yc);
        codeGroupInitItem(item.sxhy);
        codeGroupInitItem(item.xxcj);
    }
}
void LocalPlanDialogImpl::initDisableControl(LocalPlanDialogImpl* m_impl)
{
    for (auto& item : m_impl->rowGroupList)
    {
        // item.workmode_cbx->setEnabled(false);
        item.pointfrequency_cbx->setEnabled(false);
        item.yc->setEnabled(false);
        item.yk->setEnabled(false);
        item.sxhy->setEnabled(false);
        item.xxcj->setEnabled(false);
        item.checkBox_yc->setEnabled(false);
        item.checkBox_yk->setEnabled(false);
        item.checkBox_cg->setEnabled(false);
        item.checkBox_sc->setEnabled(false);
    }
}
static void taskTypeEnable(LPDRowGroup& group, bool enable)
{
    group.checkBox_yc->setEnabled(enable);
    group.checkBox_yk->setEnabled(enable);
    group.checkBox_sc->setEnabled(enable);
    group.checkBox_cg->setEnabled(enable);
    if (!enable)
    {
        group.checkBox_yc->setChecked(false);
        group.checkBox_yk->setChecked(false);
        group.checkBox_sc->setChecked(false);
        group.checkBox_cg->setChecked(false);
    }
}

/* 工作模式改变 */
template<int num>
void LocalPlanDialogImpl::workmodeCbxCurrentIndexChanged()
{
    static_assert(num >= 1 && num <= LocalPlanDialogImpl::rowNum, "The range is 1-8");

    auto& group = rowGroupList[num - 1];
    auto& pointfrequency_cbx = group.pointfrequency_cbx;
    auto workmode = (SystemWorkMode)group.workmode_cbx->currentData().toInt();

    pointfrequency_cbx->clear();
    pointfrequency_cbx->setEnabled(true);

    /* 点频数map */
    auto dpNumMap = curSatellite.getDpNumMap();
    auto dpFind = dpNumMap.find(workmode);
    if (dpFind == dpNumMap.end())
    {
        pointfrequency_cbx->addItem("空", -1);
    }
    else
    {
        auto dpNum = dpFind.value();
        for (int i = 1; i <= dpNum; ++i)
        {
            pointfrequency_cbx->addItem(QString("点频%1").arg(i), i);
        }
    }

    // 判断链路中是否含有S扩二或者一体化上面级的模式
    bool flag = false;
    for (auto item : rowGroupList)
    {
        auto workMode = (SystemWorkMode)item.workmode_cbx->currentData().toInt();
        if (workMode == SystemWorkMode::Skuo2 || workMode == SystemWorkMode::SYTHSMJ)
        {
            flag = true;
            break;
        }
    }
}
// 这个是点频改变
template<int num>
void LocalPlanDialogImpl::pointFrequencyCbxCurrentIndexChanged()
{
    static_assert(num >= 1 && num <= LocalPlanDialogImpl::rowNum, "The range is 1-8");

    auto& group = rowGroupList[num - 1];
    auto& pointfrequency_cbx = group.pointfrequency_cbx;

    auto workmode = (SystemWorkMode)group.workmode_cbx->currentData().toInt();
    auto dpNum = pointfrequency_cbx->currentData().toInt();
    if (dpNum <= 0)
    {
        taskTypeEnable(group, false);
        codeGroupEnable(group, false);
        return;
    }

    /*
     * 已知工作模式
     * 数传不存在码组 不存在任务类型
     * sttc不存在码组
     */

    /* STTC是特殊的 */
    if (SystemWorkModeHelper::isDataTransmissionWorkMode(SystemWorkMode(workmode)) || workmode == SystemWorkMode::STTC)
    {
        codeGroupEnable(group, false);
    }
    else if (SystemWorkModeHelper::isMeasureContrlWorkMode(SystemWorkMode(workmode)))
    {
        /* 任务类型点频会去更改enable */
        codeGroupEnable(group, true);
    }
    else
    {
        taskTypeEnable(group, false);
        codeGroupEnable(group, false);
        return;
    }

    // Ka低速有上行
    if (SystemWorkModeHelper::isDataTransmissionWorkMode(SystemWorkMode(workmode)) && SystemWorkMode(workmode) != KaDS)
    {
        return;
    }
    /* 模式和点频号码 */

    auto& tempData = curSatellite.m_workModeParamMap[workmode].m_dpInfoMap[dpNum];

    /* 这里的值是1是有 2是无 */
    bool yc = false;
    bool cg = false;
    bool yk = false;
    bool sc = false;

    // 对应的码组选的第几组
    int ycmz = 1;
    int ykmz = 1;
    int sclmz = 1;
    int xclmz = 1;

    for (auto item : tempData.m_itemMap.keys())
    {
        if (item.endsWith("_YaoC"))
        {
            yc = tempData.m_itemMap.value(item).toInt() == 1 ? true : false;
        }
        else if (item.endsWith("_CGui"))
        {
            cg = tempData.m_itemMap.value(item).toInt() == 1 ? true : false;
        }
        else if (item.endsWith("_YaoK"))
        {
            yk = tempData.m_itemMap.value(item).toInt() == 1 ? true : false;
        }
        else if (item.endsWith("_SC"))
        {
            sc = tempData.m_itemMap.value(item).toInt() == 1 ? true : false;
        }

        else if (item.endsWith("YaoC_MZ"))
        {
            ycmz = tempData.m_itemMap.value(item).toInt();
        }
        else if (item.endsWith("YaoK_MZ"))
        {
            ykmz = tempData.m_itemMap.value(item).toInt();
        }
        else if (item.endsWith("SXCL_MZ"))
        {
            sclmz = tempData.m_itemMap.value(item).toInt();
        }
        else if (item.endsWith("XXCL_MZ"))
        {
            xclmz = tempData.m_itemMap.value(item).toInt();
        }
    }

    group.checkBox_yc->setEnabled(yc);
    group.checkBox_cg->setEnabled(cg);
    group.checkBox_yk->setEnabled(yk);
    group.checkBox_sc->setEnabled(sc);

    group.checkBox_yc->setChecked(yc);
    group.checkBox_cg->setChecked(cg);
    group.checkBox_yk->setChecked(yk);
    group.checkBox_sc->setChecked(sc);

    group.yk->setCurrentText(QString::number(ycmz));
    group.yc->setCurrentText(QString::number(ykmz));
    group.sxhy->setCurrentText(QString::number(sclmz));
    group.xxcj->setCurrentText(QString::number(xclmz));
}

LocalPlanDialog::LocalPlanDialog(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LocalPlanDialog)
    , m_impl(new LocalPlanDialogImpl)
{
    ui->setupUi(this);

    m_impl->taskType = new QButtonGroup(this);
    m_impl->taskType->addButton(ui->target_test_task, static_cast<int>(TaskPlanType::MeasureControl));
    m_impl->taskType->addButton(ui->target_data_task, static_cast<int>(TaskPlanType::DataTrans));
    m_impl->taskType->addButton(ui->target_data_test_task, static_cast<int>(TaskPlanType::MeasureCtrlDataTrasn));

    m_impl->openRemoteControl = new QButtonGroup(this);
    m_impl->openRemoteControl->addButton(ui->rbtn_open_rmctr_zero, 0);
    m_impl->openRemoteControl->addButton(ui->rbtn_open_rmctr_f, 1);
    m_impl->openRemoteControl->addButton(ui->rbtn_open_rmctr_time, 2);

    m_impl->closeRemoteControl = new QButtonGroup(this);
    m_impl->closeRemoteControl->addButton(ui->rbtn_close_rmctr_zero, 0);
    m_impl->closeRemoteControl->addButton(ui->rbtn_close_rmctr_f, 1);
    m_impl->closeRemoteControl->addButton(ui->rbtn_close_rmctr_time, 2);

    m_impl->openUpSignal = new QButtonGroup(this);
    m_impl->openUpSignal->addButton(ui->rbtn_open_high_zero, 0);
    m_impl->openUpSignal->addButton(ui->rbtn_open_high_f, 1);
    m_impl->openUpSignal->addButton(ui->rbtn_open_high_time, 2);

    m_impl->closeUpSignal = new QButtonGroup(this);
    m_impl->closeUpSignal->addButton(ui->rbtn_close_high_zero, 0);
    m_impl->closeUpSignal->addButton(ui->rbtn_close_high_f, 1);
    m_impl->closeUpSignal->addButton(ui->rbtn_close_high_time, 2);

    ui->rbtn_close_rmctr_zero->setEnabled(false);
    ui->rbtn_close_rmctr_f->setEnabled(false);
    ui->rbtn_close_rmctr_time->setEnabled(false);
    ui->target_time_end_remotecontrol->setEnabled(false);

    auto month = GlobalData::currentDate().month();
    auto day = GlobalData::currentDate().day();
    auto str = QString::number(month) + QString::number(day);
    ui->target_digital_trnscircle_number->setValue(str.toInt());

    ui->rbtn_close_high_zero->setEnabled(false);
    ui->rbtn_close_high_f->setEnabled(false);
    ui->rbtn_close_high_time->setEnabled(false);
    ui->target_time_stdn_sigh->setEnabled(false);
    /* 工作模式那里有8组将这些控件放到list里面 */
    ROW_GROUP_INIT(rowGroupList, 1)
    ROW_GROUP_INIT(rowGroupList, 2)
    ROW_GROUP_INIT(rowGroupList, 3)
    ROW_GROUP_INIT(rowGroupList, 4)
    ROW_GROUP_INIT(rowGroupList, 5)
    ROW_GROUP_INIT(rowGroupList, 6)
    ROW_GROUP_INIT(rowGroupList, 7)
    ROW_GROUP_INIT(rowGroupList, 8)
    /* 码组初始化 */
    LocalPlanDialogImpl::codeGroupInit(m_impl);
    /* 初始化禁用工作体制工作点频 */
    LocalPlanDialogImpl::initDisableControl(m_impl);

    // 初始化数传优先级
    ui->target_priority->addItem(TaskPlanHelper::dataTransPriorityToString(DataTransPriority::Urgent), static_cast<int>(DataTransPriority::Urgent));
    ui->target_priority->addItem(TaskPlanHelper::dataTransPriorityToString(DataTransPriority::Priority),
                                 static_cast<int>(DataTransPriority::Priority));
    ui->target_priority->addItem(TaskPlanHelper::dataTransPriorityToString(DataTransPriority::Order), static_cast<int>(DataTransPriority::Order));

    // 初始化数传模式
    ui->target_data_mode->addItem(TaskPlanHelper::dataTransModeToString(DataTransMode::RealTime), static_cast<int>(DataTransMode::RealTime));
    ui->target_data_mode->addItem(TaskPlanHelper::dataTransModeToString(DataTransMode::Afterwards), static_cast<int>(DataTransMode::Afterwards));
    ui->target_data_mode->addItem(TaskPlanHelper::dataTransModeToString(DataTransMode::OnlyRecv), static_cast<int>(DataTransMode::OnlyRecv));

    ui->target_center_protocol->addItem("PDXP", static_cast<int>(CenterProtocol::PDXP));
    ui->target_center_protocol->addItem("FEP", static_cast<int>(CenterProtocol::FEP));
    ui->target_center_protocol->addItem("HDR", static_cast<int>(CenterProtocol::HDR));

    setWidgetMode(WidgetMode::New);

    connect(m_impl->taskType, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &LocalPlanDialog::taskTypeChange);
    //    taskTypeChange(int(TaskPlanType::MeasureControl));

    connect(m_impl->openRemoteControl, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this,
            &LocalPlanDialog::openRemoteControlChanged);
    connect(m_impl->openUpSignal, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this,
            &LocalPlanDialog::openUpSignalChanged);
    connect(ui->target_task_code, &QComboBox::currentTextChanged, this, &LocalPlanDialog::targetTaskCodeChanged);

    // 以跟踪开始时间、跟踪结束时间为基准，准备开始时间=跟踪开始时间-5min，任务开始时间=跟踪开始时间-1min
    // 任务结束时间=跟踪结束时间+1min
    // 若未下达数传开始和结束时间，则数传开始时间默认为跟踪开始时间，数传结束时间默认为跟踪结束时间（即直接关联），否则按实际下达时间关联
    connect(ui->target_time_start_track, &QDateTimeEdit::dateTimeChanged, this, [=](const QDateTime& dateTime) {
        ui->target_time_ready->setDateTime(dateTime.addSecs(-5 * 60));  // 任务准备时间
        ui->target_time_start->setDateTime(dateTime.addSecs(-1 * 60));  // 任务开始时间

        ui->target_time_start_mac->setDateTime(dateTime);  // 数传开始时间
    });
    connect(ui->target_time_end_track, &QDateTimeEdit::dateTimeChanged, this, [=](const QDateTime& dateTime) {
        ui->target_time_end->setDateTime(dateTime.addSecs(1 * 60));  // 任务结束时间
        ui->target_time_end_mac->setDateTime(dateTime);              // 数传结束时间
    });

    ui->target_time_start_track->setStyleSheet("background-color:rgb(0,170,255);color:rgb(255,255,255);");
    ui->target_time_end_track->setStyleSheet("background-color:rgb(0,170,255);color:rgb(255,255,255);");
    //替换XCombobox
    m_target_center = new XComboBox();
    ui->target_center->layout()->addWidget(m_target_center);
}

LocalPlanDialog::~LocalPlanDialog()
{
    delete ui;
    delete m_impl;
}
void LocalPlanDialog::taskTypeChange(int id)
{
    auto taskType = m_impl->taskType->button(id);
    if (taskType != nullptr)
    {
        taskType->setChecked(true);
    }
    /* 这个ID是ButtonGroup的ID
     * 1.任务类型改变首先将控件的启用与禁用先改了
     * 2.重新刷新界面数据
     *
     * 数传只有上行
     * 测控两个都有
     */
    ui->target_task_code->clear();
    SatelliteManagementDataMap tempMap;
    switch (id)
    {
    case int(TaskPlanType::DataTrans): /* 数传 */
    {
        /* 任务类型改变首先将控件的启用与禁用先改了任务类型改变首先将控件的启用与禁用先改了 */
        if (m_impl->widgetMode != WidgetMode::View)
        {
            ui->target_data_mode->setEnabled(true);
            ui->target_priority->setEnabled(true);
            ui->target_center->setEnabled(true);
            ui->remote_on->setEnabled(false);
            ui->remote_off->setEnabled(false);
            ui->up_on->setEnabled(true);
            ui->up_off->setEnabled(true);
            ui->target_time_start_mac->setEnabled(true);
            ui->target_time_end_mac->setEnabled(true);
            /* 这里将遥控的重置一下 */
            ui->rbtn_open_rmctr_zero->setChecked(true);
        }

        for (const auto& satellite : m_impl->satelliteMap)
        {
            for (auto iter = satellite.m_workModeParamMap.begin(); iter != satellite.m_workModeParamMap.end(); ++iter)
            {
                if (SystemWorkModeHelper::isDataTransmissionWorkMode(SystemWorkMode(iter.key())))
                {
                    tempMap.insert(satellite.m_satelliteCode, satellite);
                    ui->target_task_code->addItem(satellite.m_satelliteCode, satellite.m_satelliteCode);
                    break;
                }
                continue;
            }
        }
        break;
    }
    case int(TaskPlanType::MeasureCtrlDataTrasn): /* 数传+测控 */
    {
        if (m_impl->widgetMode != WidgetMode::View)
        {
            ui->target_data_mode->setEnabled(true);
            ui->target_priority->setEnabled(true);
            ui->target_center->setEnabled(true);
            ui->remote_on->setEnabled(true);
            ui->remote_off->setEnabled(true);
            ui->up_on->setEnabled(true);
            ui->up_off->setEnabled(true);
            ui->target_time_start_mac->setEnabled(true);
            ui->target_time_end_mac->setEnabled(true);
        }

        for (const auto& satellite : m_impl->satelliteMap)
        {
            tempMap.insert(satellite.m_satelliteCode, satellite);
            ui->target_task_code->addItem(satellite.m_satelliteCode, satellite.m_satelliteCode);
        }
        break;
    }
    case int(TaskPlanType::MeasureControl): /* 测控 */
    {
        if (m_impl->widgetMode != WidgetMode::View)
        {
            ui->target_data_mode->setEnabled(false);
            ui->target_priority->setEnabled(false);
            ui->target_center->setEnabled(false);
            ui->remote_on->setEnabled(true);
            ui->remote_off->setEnabled(true);
            ui->up_on->setEnabled(true);
            ui->up_off->setEnabled(true);
            ui->target_time_start_mac->setEnabled(false);
            ui->target_time_end_mac->setEnabled(false);
        }

        for (const auto& satellite : m_impl->satelliteMap)
        {
            for (auto iter = satellite.m_workModeParamMap.begin(); iter != satellite.m_workModeParamMap.end(); ++iter)
            {
                if (SystemWorkModeHelper::isMeasureContrlWorkMode(SystemWorkMode(iter.key())))
                {
                    tempMap.insert(satellite.m_satelliteCode, satellite);
                    ui->target_task_code->addItem(satellite.m_satelliteCode, satellite.m_satelliteCode);
                    break;
                }
                continue;
            }
        }
        break;
    }
    }

    if (tempMap.isEmpty())
    {
        m_impl->clearUI();
    }
    else
    {
        m_impl->refreshSatelliteData(tempMap.first(), id);
        if (m_impl->targetChangeFlag == 0)  //为1代表是最上面的目标数量更改引起的  就不做默认刷新
        {
            refreshDefaultWorkMode(tempMap.first());
        }
        m_target_center->ClearSelect();
        auto centerList = tempMap.first().m_digitalCenterID.split("+");
        for (auto item : centerList)
        {
            m_target_center->SetItemChecked(item, true);
        }
    }

    m_impl->targetChangeFlag = 0;
}

void LocalPlanDialog::openRemoteControlChanged(int id)
{
    auto closeRemoteControl = m_impl->closeRemoteControl->button(id);
    if (closeRemoteControl != nullptr)
    {
        closeRemoteControl->setChecked(true);
    }

    switch (id)
    {
    case 0:
        ui->target_time_start_remotecontrol->setEnabled(false);
        ui->target_time_end_remotecontrol->setEnabled(false);
        break;
    case 1:
        ui->target_time_start_remotecontrol->setEnabled(false);
        ui->target_time_end_remotecontrol->setEnabled(false);
        break;
    case 2:
        ui->target_time_start_remotecontrol->setEnabled(true);
        ui->target_time_end_remotecontrol->setEnabled(true);
        break;
    default: break;
    }
}

void LocalPlanDialog::openUpSignalChanged(int id)
{
    auto closeUpSignal = m_impl->closeUpSignal->button(id);
    if (closeUpSignal != nullptr)
    {
        closeUpSignal->setChecked(true);
    }

    switch (id)
    {
    case 0:
        ui->target_time_open_sigh->setEnabled(false);
        ui->target_time_stdn_sigh->setEnabled(false);
        break;
    case 1:
        ui->target_time_open_sigh->setEnabled(false);
        ui->target_time_stdn_sigh->setEnabled(false);
        break;
    case 2:
        ui->target_time_open_sigh->setEnabled(true);
        ui->target_time_stdn_sigh->setEnabled(true);
        break;
    default: break;
    }
}

void LocalPlanDialog::setCurTarget(int target) { m_impl->curTarget = target; }
int LocalPlanDialog::curTarget() { return m_impl->curTarget; }
QString LocalPlanDialog::curTaskCode() { return ui->target_task_code->currentText(); }

bool LocalPlanDialog::checkRelation(QString& errmsg)
{
    int flag = 0;
    for (auto& item : m_impl->rowGroupList)
    {
        auto yk_fxk = item.checkBox_yk;
        auto cg_fxk = item.checkBox_cg;

        auto workModeCombox = item.workmode_cbx;
        auto workmode = (SystemWorkMode)workModeCombox->currentData().toInt();
        //只有多测控时的发上行需要判断下
        if (SystemWorkModeHelper::isMeasureContrlWorkMode(workmode))
        {
            if (yk_fxk->isChecked() || cg_fxk->isChecked())
            {
                flag++;
            }
        }
    }

    if (flag > 1)
    {
        errmsg = QString("多测控模式存在时，只有一个测控模式可选遥控和测轨！");

        return false;
    }

    return true;
}

void LocalPlanDialog::refreshDefaultWorkMode(const SatelliteManagementData& info)
{
    //这里是展示所有模式
    auto num = m_impl->currWorkMode.size();
    int flag = 0;
    auto dpNumMap = info.getDpNumMap();
    //这里的意思是如果8个下拉框都为空的话再刷新界面值 这里在查看和编辑界面的时候会影响到,只有新增的时候才会用到这个
    if (m_impl->widgetMode == WidgetMode::New)
    {
        for (auto& item : m_impl->rowGroupList)
        {
            //设置模式下拉框
            auto workModeCombox = item.workmode_cbx;
            auto pointCombox = item.pointfrequency_cbx;
            auto mode = m_impl->currWorkMode[flag++];
            workModeCombox->setCurrentText(SystemWorkModeHelper::systemWorkModeToDesc(SystemWorkMode(mode)));
            //设置该模式对应的默认点频
            auto defaultPoint = info.getDefaultDpNumByWorkMode((SystemWorkMode)mode);
            auto workmode = (SystemWorkMode)workModeCombox->currentData().toInt();
            auto dpFind = dpNumMap.find(workmode);
            pointCombox->clear();
            if (dpFind != dpNumMap.end())
            {
                auto dpNum = dpFind.value();
                for (int i = 1; i <= dpNum; ++i)
                {
                    pointCombox->addItem(QString("点频%1").arg(i), i);
                }
            }
            pointCombox->setCurrentText(QString("点频%1").arg(defaultPoint));
            if (num == flag)
                break;
        }
    }
}

void LocalPlanDialog::setWidgetMode(WidgetMode widgetMode)
{
    m_impl->widgetMode = widgetMode;
    if (m_impl->widgetMode == WidgetMode::New)
    {
        auto curDate = GlobalData::currentDateTime();

        auto taskReadyTime = curDate.addSecs(60);
        auto taskStartTime = curDate.addSecs(170);
        auto trackStartTime = taskReadyTime.addSecs(5 * 60);
        auto trackEndTime = trackStartTime.addSecs(3 * 60);
        auto dataTransStartTime = trackStartTime;
        auto dataTransEndTime = trackEndTime;
        auto taskEndTime = trackEndTime.addSecs(1 * 60);

        ui->target_time_ready->setDateTime(taskReadyTime);                 // 任务准备开始时间
        ui->target_time_start->setDateTime(taskStartTime);                 // 任务开始时间
        ui->target_time_start_track->setDateTime(trackStartTime);          // 跟踪起始时间
        ui->target_time_end_track->setDateTime(trackEndTime);              // 跟踪结束时间
        ui->target_time_start_mac->setDateTime(dataTransStartTime);        // 数传开始时间
        ui->target_time_end_mac->setDateTime(dataTransEndTime);            // 数传结束时间
        ui->target_time_end->setDateTime(taskEndTime);                     // 任务结束时间
        ui->target_time_open_sigh->setDateTime(trackStartTime);            // 开上行时间
        ui->target_time_stdn_sigh->setDateTime(trackEndTime);              // 关上行时间
        ui->target_time_start_remotecontrol->setDateTime(trackStartTime);  // 开遥控时间
        ui->target_time_end_remotecontrol->setDateTime(trackEndTime);      // 关遥控时间
    }
}
void LocalPlanDialog::setSCCenterData(const DataTransmissionCenterMap& arg)
{
    m_impl->dataCenterMap = arg;
    m_target_center->clear();
    for (auto& item : m_impl->dataCenterMap)
    {
        m_target_center->AddItem(item.centerName, false);
    }
}
void LocalPlanDialog::setControlEnable()
{
    ui->target_time_ready->setEnabled(false);
    ui->target_time_start->setEnabled(false);
    ui->target_time_end->setEnabled(false);
    ui->target_time_start_track->setEnabled(false);
    ui->target_time_end_track->setEnabled(false);
    ui->target_time_start_mac->setEnabled(false);
    ui->target_time_end_mac->setEnabled(false);
    ui->remote_on->setEnabled(false);
    ui->remote_off->setEnabled(false);
    ui->up_on->setEnabled(false);
    ui->up_off->setEnabled(false);

    ui->target_test_task->setEnabled(false);
    ui->target_data_task->setEnabled(false);
    ui->target_data_test_task->setEnabled(false);
    ui->target_task_code->setEnabled(false);
    ui->target_data_mode->setEnabled(false);
    ui->target_digital_trnscircle_number->setEnabled(false);
    ui->target_priority->setEnabled(false);
    ui->target_center_protocol->setEnabled(false);
    ui->target_center->setEnabled(false);

    ui->groupBox->setEnabled(false);
    ui->groupBox_2->setEnabled(false);
    ui->groupBox_3->setEnabled(false);
    ui->groupBox_4->setEnabled(false);
}
void LocalPlanDialog::setSatelliteMap(const SatelliteManagementDataMap& satelliteMap)
{
    m_impl->satelliteMap = satelliteMap;
    taskTypeChange(int(TaskPlanType::MeasureControl));
}

bool LocalPlanDialog::dtEffective()
{
    /* 0是测控 其他的都可以有数传 */
    return m_impl->taskType->checkedId() != static_cast<int>(TaskPlanType::MeasureControl);
}
Optional<DeviceWorkTaskTarget> LocalPlanDialog::getDeviceWorkTaskTarget()
{
    using ResType = Optional<DeviceWorkTaskTarget>;
    DeviceWorkTaskTarget target;

    auto curDateTime = GlobalData::currentDateTime();
    auto readyStartTime = ui->target_time_ready->dateTime();
    auto taskStartTime = ui->target_time_start->dateTime();
    auto taskEndTime = ui->target_time_end->dateTime();
    auto trackStartTime = ui->target_time_start_track->dateTime();
    auto trackEndTime = ui->target_time_end_track->dateTime();
    auto remoteCtrlStartTime = ui->target_time_start_remotecontrol->dateTime();
    auto remoteCtrlEndTime = ui->target_time_end_remotecontrol->dateTime();
    auto upStreamStartTime = ui->target_time_open_sigh->dateTime();
    auto upStreamEndTime = ui->target_time_stdn_sigh->dateTime();

    target.m_task_code = ui->target_task_code->currentText();            // 任务代号
    target.m_circle_no = ui->target_digital_trnscircle_number->value();  // 圈号
    target.m_task_ready_start_time = readyStartTime;                     // 任务准备开始时间
    target.m_task_start_time = taskStartTime;                            // 任务开始时间
    target.m_track_start_time = trackStartTime;                          // 跟踪起始时间
    target.m_track_end_time = trackEndTime;                              // 跟踪结束时间
    target.m_task_end_time = taskEndTime;                                // 任务结束时间
    target.m_planType = TaskPlanType(m_impl->taskType->checkedId());

    // 遥控开始时间
    if (ui->rbtn_open_rmctr_zero->isChecked() == true)
    {
        target.m_remote_start_time = TimeZero;
    }
    else if (ui->rbtn_open_rmctr_f->isChecked() == true)
    {
        target.m_remote_start_time = TimeF;
    }
    else if (ui->rbtn_open_rmctr_time->isChecked() == true)
    {
        target.m_remote_start_time = Utility::dateTimeToStr(remoteCtrlStartTime);
    }
    // 遥控结束时间
    if (ui->rbtn_close_rmctr_zero->isChecked() == true)
    {
        target.m_remote_control_end_time = TimeZero;
    }
    else if (ui->rbtn_close_rmctr_f->isChecked() == true)
    {
        target.m_remote_control_end_time = TimeF;
    }
    else if (ui->rbtn_close_rmctr_time->isChecked() == true)
    {
        target.m_remote_control_end_time = Utility::dateTimeToStr(remoteCtrlEndTime);
    }

    // 开上行信号时间
    if (ui->rbtn_open_high_zero->isChecked() == true)
    {
        target.m_on_uplink_signal_time = TimeZero;
    }
    else if (ui->rbtn_open_high_f->isChecked() == true)
    {
        target.m_on_uplink_signal_time = TimeF;
    }
    else if (ui->rbtn_open_high_time->isChecked() == true)
    {
        target.m_on_uplink_signal_time = Utility::dateTimeToStr(upStreamStartTime);
    }

    // 关上行信号时间
    if (ui->rbtn_close_high_zero->isChecked() == true)
    {
        target.m_turn_off_line_signal_time = TimeZero;
    }
    else if (ui->rbtn_close_high_f->isChecked() == true)
    {
        target.m_turn_off_line_signal_time = TimeF;
    }
    else if (ui->rbtn_close_high_time->isChecked() == true)
    {
        target.m_turn_off_line_signal_time = Utility::dateTimeToStr(upStreamEndTime);
    }

    //
    target.m_master_control_center = "";  // 主控中心
    target.m_backup_center = "";          // 备份中心
    //
    int linkNumber = 0;  //链路数量
    for (auto& item : m_impl->rowGroupList)
    {
        auto& workModeCombobox = item.workmode_cbx;
        auto dpNum = item.pointfrequency_cbx->currentData().toInt();
        if ((dpNum <= 0) || (workModeCombobox->count() <= 0))
        {
            continue;
        }

        linkNumber++;
        DeviceWorkTaskTargetLink link;
        link.working_point_frequency = dpNum;                                          //点频
        link.m_work_system = SystemWorkMode(workModeCombobox->currentData().toInt());  //工作体制
        link.m_baseband_number = "--";                                                 //基带编号

        link.m_codeGroup_yc = item.yc->currentText();      // 遥测码组
        link.m_codeGroup_yk = item.yk->currentText();      // 遥控码组
        link.m_codeGroup_sxcj = item.sxhy->currentText();  // 上行测距
        link.m_codeGroup_xxcj = item.xxcj->currentText();  // 下行测距

        link.m_taskType_yc = item.checkBox_yc->isChecked();  // 任务类型 遥测
        link.m_taskType_yk = item.checkBox_yk->isChecked();  // 任务类型 遥控
        link.m_taskType_cg = item.checkBox_cg->isChecked();  // 任务类型 测轨
        link.m_taskType_sc = item.checkBox_sc->isChecked();  // 任务类型 数传

        target.m_linkMap[linkNumber] = link;
    }
    target.m_link_number = linkNumber;

    // 判断链路是否有效
    if (target.m_linkMap.isEmpty())
    {
        return ResType(ErrorCode::InvalidData, "请至少选择一个工作计划与点频", target);
    }

    if (curDateTime > readyStartTime)
    {
        return ResType(ErrorCode::InvalidData, "准备开始时间不能早于当前时间", target);
    }
    if (taskStartTime < readyStartTime)
    {
        return ResType(ErrorCode::InvalidData, "任务开始时间不能早于任务准备开始时间", target);
    }
    if (taskEndTime < taskStartTime)
    {
        return ResType(ErrorCode::InvalidData, "任务结束时间不能早于任务开始时间", target);
    }

    if (trackStartTime < taskStartTime || trackStartTime > taskEndTime)
    {
        return ResType(ErrorCode::InvalidData, "跟踪开始时间不在任务开始时间和任务结束时间之间", target);
    }
    if (trackEndTime < taskStartTime || trackEndTime > taskEndTime)
    {
        return ResType(ErrorCode::InvalidData, "跟踪结束时间不在任务开始时间和任务结束时间之间", target);
    }
    if (trackEndTime < trackStartTime)
    {
        return ResType(ErrorCode::InvalidData, "跟踪结束时间不能早于跟踪开始时间", target);
    }

    // 开遥控、关遥控时间要在跟踪时间内
    // 有测控计划
    if (ui->target_test_task->isChecked() || ui->target_data_test_task->isChecked())
    {
        auto remoteCtrlStartTime = ui->target_time_start_remotecontrol->dateTime();
        auto remoteCtrlEndTime = ui->target_time_end_remotecontrol->dateTime();
        // 勾选了开遥控时间
        if (ui->rbtn_open_rmctr_time->isChecked())
        {
            if (remoteCtrlStartTime < trackStartTime || remoteCtrlStartTime > trackEndTime)
            {
                return ResType(ErrorCode::InvalidData, "开遥控时间不在跟踪开始时间和跟踪结束时间之间", target);
            }
        }
        // 勾选了关遥控时间
        if (ui->rbtn_close_rmctr_time->isChecked())
        {
            if (remoteCtrlEndTime < trackStartTime || remoteCtrlEndTime > trackEndTime)
            {
                return ResType(ErrorCode::InvalidData, "关遥控时间不在跟踪开始时间和跟踪结束时间之间", target);
            }
        }

        // 开遥控时间/关遥控时间都勾选
        if (ui->rbtn_open_rmctr_time->isChecked() && ui->rbtn_close_rmctr_time->isChecked())
        {
            if (remoteCtrlEndTime < remoteCtrlStartTime)
            {
                return ResType(ErrorCode::InvalidData, "关遥控时间不能早于开遥控时间", target);
            }
        }

        auto upStreamStartTime = ui->target_time_open_sigh->dateTime();
        auto upStreamEndTime = ui->target_time_stdn_sigh->dateTime();
        // 勾选了开上行时间
        if (ui->rbtn_open_high_time->isChecked())
        {
            if (upStreamStartTime < trackStartTime || upStreamStartTime > trackEndTime)
            {
                return ResType(ErrorCode::InvalidData, "开上行时间不在跟踪开始时间和跟踪结束时间之间", target);
            }
        }

        if (ui->rbtn_close_high_time->isChecked())
        {
            if (upStreamEndTime < trackStartTime || upStreamEndTime > trackEndTime)
            {
                return ResType(ErrorCode::InvalidData, "关上行时间不在跟踪开始时间和跟踪结束时间之间", target);
            }
        }

        if (ui->rbtn_open_high_time->isChecked() && ui->rbtn_close_high_time->isChecked())
        {
            if (upStreamEndTime < upStreamStartTime)
            {
                return ResType(ErrorCode::InvalidData, "关上行时间不能早于开上行时间", target);
            }
        }
        if (!m_target_center->GetSelItemsText().size())
        {
            return ResType(ErrorCode::InvalidData, "数传中心为空，请去数传中心下添加数据后\n再进行任务计划的添加", target);
        }
    }

    return ResType(target);
}

void LocalPlanDialog::setDeviceWorkTaskTarget(const DeviceWorkTaskTarget& deviceWorkTarget)
{
    m_impl->targetChangeFlag = 1;
    taskTypeChange(static_cast<int>(deviceWorkTarget.m_planType));  // 计划类型 （必须先设置这个，通过不同的计划类型显示不同的任务代号）

    ui->target_task_code->setCurrentText(deviceWorkTarget.m_task_code);             //任务代号
    ui->target_digital_trnscircle_number->setValue(deviceWorkTarget.m_circle_no);   //圈号
    ui->target_time_ready->setDateTime(deviceWorkTarget.m_task_ready_start_time);   //任务准备开始时间
    ui->target_time_start->setDateTime(deviceWorkTarget.m_task_start_time);         //任务开始时间
    ui->target_time_end->setDateTime(deviceWorkTarget.m_task_end_time);             //任务结束时间
    ui->target_time_start_track->setDateTime(deviceWorkTarget.m_track_start_time);  //跟踪起始时间
    ui->target_time_end_track->setDateTime(deviceWorkTarget.m_track_end_time);      //跟踪结束时间

    // 遥控开始时间
    if (deviceWorkTarget.m_remote_start_time == TimeZero)
    {
        ui->rbtn_open_rmctr_zero->setChecked(true);
        ui->target_time_start_remotecontrol->setEnabled(false);
    }
    else if (deviceWorkTarget.m_remote_start_time == TimeF)
    {
        ui->rbtn_open_rmctr_f->setChecked(true);
        ui->target_time_start_remotecontrol->setEnabled(false);
    }
    else
    {
        ui->rbtn_open_rmctr_time->setChecked(true);
        ui->target_time_start_remotecontrol->setEnabled(true);
        ui->target_time_start_remotecontrol->setDateTime(Utility::dateTimeFromStr(deviceWorkTarget.m_remote_start_time));  // 遥控开始时间
    }
    // 遥控结束时间
    if (deviceWorkTarget.m_remote_control_end_time == TimeZero)
    {
        ui->rbtn_close_rmctr_zero->setChecked(true);
        ui->target_time_end_remotecontrol->setEnabled(false);
    }
    else if (deviceWorkTarget.m_remote_control_end_time == TimeF)
    {
        ui->rbtn_close_rmctr_f->setChecked(true);
        ui->target_time_end_remotecontrol->setEnabled(false);
    }
    else
    {
        ui->rbtn_close_rmctr_time->setChecked(true);
        ui->target_time_end_remotecontrol->setEnabled(true);
        ui->target_time_end_remotecontrol->setDateTime(Utility::dateTimeFromStr(deviceWorkTarget.m_remote_control_end_time));  // 遥控结束时间
    }

    // 开上行信号时间
    if (deviceWorkTarget.m_on_uplink_signal_time == TimeZero)
    {
        ui->rbtn_open_high_zero->setChecked(true);
        ui->target_time_open_sigh->setEnabled(false);
    }
    else if (deviceWorkTarget.m_on_uplink_signal_time == TimeF)
    {
        ui->rbtn_open_high_f->setChecked(true);
        ui->target_time_open_sigh->setEnabled(false);
    }
    else
    {
        ui->rbtn_open_high_time->setChecked(true);
        ui->target_time_open_sigh->setEnabled(true);
        ui->target_time_open_sigh->setDateTime(Utility::dateTimeFromStr(deviceWorkTarget.m_on_uplink_signal_time));  // 开上行信号时间
    }

    // 关上行信号时间
    if (deviceWorkTarget.m_turn_off_line_signal_time == TimeZero)
    {
        ui->rbtn_close_high_zero->setChecked(true);
        ui->target_time_stdn_sigh->setEnabled(false);
    }
    else if (deviceWorkTarget.m_turn_off_line_signal_time == TimeF)
    {
        ui->rbtn_close_high_f->setChecked(true);
        ui->target_time_stdn_sigh->setEnabled(false);
    }
    else
    {
        ui->rbtn_close_high_time->setChecked(true);
        ui->target_time_stdn_sigh->setEnabled(true);
        ui->target_time_stdn_sigh->setDateTime(Utility::dateTimeFromStr(deviceWorkTarget.m_turn_off_line_signal_time));  // 关上行信号时间
    }
    //先把所有值刷新为空 下面再刷新前面保存的值  避免一些bug
    for (auto& item : m_impl->rowGroupList)
    {
        //设置模式下拉框
        auto workModeCombox = item.workmode_cbx;
        workModeCombox->setCurrentText("空");
    }
    // 设置链路
    for (auto iter = deviceWorkTarget.m_linkMap.begin(); iter != deviceWorkTarget.m_linkMap.end(); ++iter)
    {
        int index = iter.key() - 1;
        auto link = iter.value();
        if (index < 0 || index >= static_cast<int>(m_impl->rowGroupList.size()))
            continue;
        auto& item = m_impl->rowGroupList[index];

        item.workmode_cbx->setCurrentText(SystemWorkModeHelper::systemWorkModeToDesc(link.m_work_system));  // 工作体制
        item.pointfrequency_cbx->setCurrentText(QString("点频%1").arg(link.working_point_frequency));       // 点频
        item.yc->setCurrentText(link.m_codeGroup_yc);                                                       // 遥测码组
        item.yk->setCurrentText(link.m_codeGroup_yk);                                                       // 遥控码组
        item.sxhy->setCurrentText(link.m_codeGroup_sxcj);                                                   // 上行测距
        item.xxcj->setCurrentText(link.m_codeGroup_xxcj);                                                   // 下行测距

        item.checkBox_yc->setChecked(link.m_taskType_yc);  // 任务类型 遥测
        item.checkBox_yk->setChecked(link.m_taskType_yk);  // 任务类型 遥控
        item.checkBox_cg->setChecked(link.m_taskType_cg);  // 任务类型 测轨
        item.checkBox_sc->setChecked(link.m_taskType_sc);  // 任务类型 数传
    }
}
Optional<DataTranWorkTask> LocalPlanDialog::getDataTranWorkTask()
{
    using ResType = Optional<DataTranWorkTask>;
    DataTranWorkTask dataTransWorkTask;

    auto taskStartTime = ui->target_time_start->dateTime();
    auto taskEndTime = ui->target_time_end->dateTime();

    auto dataTransStartTime = ui->target_time_start_mac->dateTime();
    auto dataTransEndTime = ui->target_time_end_mac->dateTime();

    dataTransWorkTask.m_priority = DataTransPriority(ui->target_priority->currentData().toInt());  // 优先级
    dataTransWorkTask.m_ring_num = ui->target_digital_trnscircle_number->value();                  // 圈号
    dataTransWorkTask.m_task_code = ui->target_task_code->currentData().toString();
    dataTransWorkTask.m_task_status = TaskPlanStatus::NoStart;
    dataTransWorkTask.m_source = TaskPlanSource::Local;

    dataTransWorkTask.m_equipment_no = QString();
    dataTransWorkTask.m_working_mode = DataTransMode(ui->target_data_mode->currentData().toInt());  // 数传模式
    dataTransWorkTask.m_create_userid = "";
    dataTransWorkTask.m_task_end_time = dataTransEndTime;
    dataTransWorkTask.m_task_total_num = 1;
    dataTransWorkTask.m_task_start_time = dataTransStartTime;
    dataTransWorkTask.m_transport_time = dataTransWorkTask.m_task_start_time;
    dataTransWorkTask.m_task_commit_time = GlobalData::currentDateTime();

    /* 数传中心 */
    dataTransWorkTask.m_center_protocol = CenterProtocol(ui->target_center_protocol->currentData().toInt());  // 中心协议
    dataTransWorkTask.m_datatrans_center = m_target_center->GetSelItemsText().join("+");                      // 数传中心
    if (dataTransStartTime < taskStartTime || dataTransStartTime > taskEndTime)
    {
        return ResType(ErrorCode::InvalidData, "数传开始时间不在计划开始时间和计划结束时间之间", dataTransWorkTask);
    }
    if (dataTransEndTime < taskStartTime || dataTransEndTime > taskEndTime)
    {
        return ResType(ErrorCode::InvalidData, "数传结束时间不在计划开始时间和计划结束时间之间", dataTransWorkTask);
    }
    if (dataTransEndTime < dataTransStartTime)
    {
        return ResType(ErrorCode::InvalidData, "数传结束时间不能早于数传开始时间", dataTransWorkTask);
    }

    // 开上行、关上行时间要在数传时间内
    // 有数传计划
    if (ui->target_data_task->isChecked() || ui->target_data_test_task->isChecked())
    {
        auto upStreamStartTime = ui->target_time_open_sigh->dateTime();
        auto upStreamEndTime = ui->target_time_stdn_sigh->dateTime();
        // 勾选了开上行时间
        if (ui->rbtn_open_high_time->isChecked())
        {
            if (upStreamStartTime < dataTransStartTime || upStreamStartTime > dataTransEndTime)
            {
                return ResType(ErrorCode::InvalidData, "开上行时间不在数传开始时间和数传结束时间之间", dataTransWorkTask);
            }
        }

        if (ui->rbtn_close_high_time->isChecked())
        {
            if (upStreamEndTime < dataTransStartTime || upStreamEndTime > dataTransEndTime)
            {
                return ResType(ErrorCode::InvalidData, "关上行时间不在数传开始时间和数传结束时间之间", dataTransWorkTask);
            }
        }

        if (ui->rbtn_open_high_time->isChecked() && ui->rbtn_close_high_time->isChecked())
        {
            if (upStreamEndTime < upStreamStartTime)
            {
                return ResType(ErrorCode::InvalidData, "关上行时间不能早于开上行时间", dataTransWorkTask);
            }
        }

        if (!m_target_center->GetSelItemsText().size())
        {
            return ResType(ErrorCode::InvalidData, "数传中心为空，请去数传中心下添加数据后\n再进行任务计划的添加", dataTransWorkTask);
        }
    }

    return ResType(dataTransWorkTask);
}

void LocalPlanDialog::setDataTranWorkTask(const DataTranWorkTask& dataTranWorkTask)
{
    ui->target_time_start_mac->setDateTime(dataTranWorkTask.m_task_start_time);  // 数传开始时间
    ui->target_time_end_mac->setDateTime(dataTranWorkTask.m_task_end_time);      // 数传结束时间

    ui->target_priority->setCurrentText(QString::number(int(dataTranWorkTask.m_priority)));                        // 优先级
    ui->target_digital_trnscircle_number->setValue(dataTranWorkTask.m_ring_num);                                   // 圈号
    ui->target_data_mode->setCurrentText(TaskPlanHelper::dataTransModeToString(dataTranWorkTask.m_working_mode));  // 数传模式
    auto scList = dataTranWorkTask.m_datatrans_center.split("+");
    m_target_center->ClearSelect();
    for (auto& item : scList)
    {
        m_target_center->SetItemChecked(item, true);
    }
    ui->target_center_protocol->setCurrentText(TaskPlanHelper::centerProtocolToDesc(dataTranWorkTask.m_center_protocol));  // 中心协议
}

// void LocalPlanDialog::on_rbtn_open_rmctr_time_toggled(bool checked)
//{
//    ui->target_time_start_remotecontrol->setEnabled(checked);  //时间可编辑
//    ui->rbtn_close_rmctr_time->setChecked(checked);
//}

// void LocalPlanDialog::on_rbtn_open_high_time_toggled(bool checked)
//{
//    ui->target_time_open_sigh->setEnabled(checked);  //时间可编辑
//    ui->rbtn_close_high_time->setChecked(checked);
//}

// void LocalPlanDialog::on_rbtn_close_rmctr_time_toggled(bool checked)
//{
//    ui->target_time_end_remotecontrol->setEnabled(checked);  //时间可编辑
//    ui->rbtn_open_rmctr_time->setChecked(checked);
//}

// void LocalPlanDialog::on_rbtn_close_high_time_toggled(bool checked)
//{
//    ui->target_time_stdn_sigh->setEnabled(checked);  //时间可编辑
//    ui->rbtn_open_high_time->setChecked(checked);
//}

void LocalPlanDialog::targetTaskCodeChanged(const QString& /*arg1*/)
{
    auto satelliteID = ui->target_task_code->currentData().toString();
    auto find = m_impl->satelliteMap.find(satelliteID);
    if (find == m_impl->satelliteMap.end())
    {
        m_impl->curSatellite = SatelliteManagementData();
        return;
    }

    m_impl->curSatellite = find.value();
    m_impl->refreshSatelliteData(m_impl->curSatellite, m_impl->taskType->checkedId());
    refreshDefaultWorkMode(m_impl->curSatellite);
    m_target_center->ClearSelect();
    auto centerList = m_impl->curSatellite.m_digitalCenterID.split("+");
    for (auto item : centerList)
    {
        m_target_center->SetItemChecked(item, true);
    }
    emit signalCurTargetTaskCodeChanged();
}

void LocalPlanDialog::on_workmode_cbx1_currentIndexChanged(int) { m_impl->workmodeCbxCurrentIndexChanged<1>(); }
void LocalPlanDialog::on_workmode_cbx2_currentIndexChanged(int) { m_impl->workmodeCbxCurrentIndexChanged<2>(); }
void LocalPlanDialog::on_workmode_cbx3_currentIndexChanged(int) { m_impl->workmodeCbxCurrentIndexChanged<3>(); }
void LocalPlanDialog::on_workmode_cbx4_currentIndexChanged(int) { m_impl->workmodeCbxCurrentIndexChanged<4>(); }
void LocalPlanDialog::on_workmode_cbx5_currentIndexChanged(int) { m_impl->workmodeCbxCurrentIndexChanged<5>(); }
void LocalPlanDialog::on_workmode_cbx6_currentIndexChanged(int) { m_impl->workmodeCbxCurrentIndexChanged<6>(); }
void LocalPlanDialog::on_workmode_cbx7_currentIndexChanged(int) { m_impl->workmodeCbxCurrentIndexChanged<7>(); }
void LocalPlanDialog::on_workmode_cbx8_currentIndexChanged(int) { m_impl->workmodeCbxCurrentIndexChanged<8>(); }

void LocalPlanDialog::on_pointfrequency_cbx1_currentIndexChanged(int) { m_impl->pointFrequencyCbxCurrentIndexChanged<1>(); }
void LocalPlanDialog::on_pointfrequency_cbx2_currentIndexChanged(int) { m_impl->pointFrequencyCbxCurrentIndexChanged<2>(); }
void LocalPlanDialog::on_pointfrequency_cbx3_currentIndexChanged(int) { m_impl->pointFrequencyCbxCurrentIndexChanged<3>(); }
void LocalPlanDialog::on_pointfrequency_cbx4_currentIndexChanged(int) { m_impl->pointFrequencyCbxCurrentIndexChanged<4>(); }
void LocalPlanDialog::on_pointfrequency_cbx5_currentIndexChanged(int) { m_impl->pointFrequencyCbxCurrentIndexChanged<5>(); }
void LocalPlanDialog::on_pointfrequency_cbx6_currentIndexChanged(int) { m_impl->pointFrequencyCbxCurrentIndexChanged<6>(); }
void LocalPlanDialog::on_pointfrequency_cbx7_currentIndexChanged(int) { m_impl->pointFrequencyCbxCurrentIndexChanged<7>(); }
void LocalPlanDialog::on_pointfrequency_cbx8_currentIndexChanged(int) { m_impl->pointFrequencyCbxCurrentIndexChanged<8>(); }
/*****************************************************************************************************************/
/*****************************************************************************************************************/
/*****************************************************************************************************************/
/*****************************************************************************************************************/
