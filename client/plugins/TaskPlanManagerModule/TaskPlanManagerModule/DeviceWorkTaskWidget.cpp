#include "DeviceWorkTaskWidget.h"
#include "DataTransmissionCenterSerialize.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "LocalPlanDialog.h"
#include "MessagePublish.h"
#include "PlanListWidget.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "SatelliteManagementSerialize.h"
#include "ServiceCheck.h"
#include "TaskPlanMessageSerialize.h"
#include "TaskPlanSql.h"
#include "ui_DeviceWorkTaskWidget.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>
#include <QTabBar>
#include <QTableWidget>
#include <QTextCodec>
#include <QToolButton>
#include <QWidget>
#include <array>
#include <tuple>

static auto constexpr gTaskPlanTotalTagrgetNum = 4;
struct IndexToTargetInfo
{
    /* 目标号=索引+1 */
    Optional<DeviceWorkTaskTarget> targetDevInfo; /* 目标的设备计划 */
    Optional<DataTranWorkTask> targetDTInfo;      /*目标的数传计划 */
    bool dtEffective;
    QString curTaskCode; /* 这里不能使用上面的数据,因为上面的数据可能是无效的 */

    IndexToTargetInfo()
        : targetDevInfo(Optional<DeviceWorkTaskTarget>::emptyOptional())
        , targetDTInfo(Optional<DataTranWorkTask>::emptyOptional())
        , dtEffective(false)
    {
    }
    void setDevInfo(const DeviceWorkTaskTarget& targetDevInfo_) { targetDevInfo = Optional<DeviceWorkTaskTarget>(targetDevInfo_); }
    void setDTInfo(const DataTranWorkTask& targetDTInfo_) { targetDTInfo = Optional<DataTranWorkTask>(targetDTInfo_); }
};
class DeviceWorkTaskWidgetImpl
{
public:
    IndexToTargetInfo defaultTargetInfoData; /* 默认的数据 */
    std::array<IndexToTargetInfo, gTaskPlanTotalTagrgetNum> indexToTarget;
    // QMap<int, IndexToTargetInfo> indexToTarget; /* 目标与目标数据的映射 */
    WidgetMode widgetMode;  // 窗口模式

    TaskPlanData taskPlanData;

    PlanListWidget* parent{ nullptr };

    DeviceWorkTaskWidgetImpl(WidgetMode widgetMode_)
        : widgetMode(widgetMode_)
    {
    }
};

DeviceWorkTaskWidget::DeviceWorkTaskWidget(WidgetMode widgetMode, PlanListWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DeviceWorkTaskWidget)
    , m_impl(new DeviceWorkTaskWidgetImpl(widgetMode))
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    m_impl->parent = parent;
    // 初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);  //只显示关闭按钮

    //加载当前卫星数据
    SatelliteManagementDataMap dataMap;
    GlobalData::getSatelliteManagementData(dataMap);

    ui->plan_school_zero->addItem("是", static_cast<int>(TaskCalibration::CaliBrationOnce));
    ui->plan_school_zero->addItem("否", static_cast<int>(TaskCalibration::NoCalibration));
    ui->plan_school_phase->addItem("是", static_cast<int>(TaskCalibration::CaliBrationOnce));
    ui->plan_school_phase->addItem("否", static_cast<int>(TaskCalibration::NoCalibration));
    /* 任务类型 */
    ui->plan_type->addItem("实战", "SZ");
    ui->plan_type->addItem("标校", "BJ");

    //加载当前数传中心数据
    DataTransmissionCenterMap dataCenterMap;
    GlobalData::getDataTransmissionCenterInfo(dataCenterMap);

    ui->targetWidget->setSCCenterData(dataCenterMap);
    ui->targetWidget->setSatelliteMap(dataMap);

    ui->targetTabBar->setSelectionBehaviorOnRemove(QTabBar::SelectLeftTab);
    ui->targetTabBar->setMinimumWidth(75);
    QStringList targetString;
    for (int i = 1; i <= gTaskPlanTotalTagrgetNum; ++i)
    {
        ui->task_target_total->addItem(QString::number(i), i);
    }
    ui->task_target_total->setCurrentIndex(0); /* 默认只有一个目标 */
    resetTableBarCount(ui->task_target_total->currentData().toInt());

    ui->targetWidget->setCurTarget(1);
    ui->targetWidget->setWidgetMode(m_impl->widgetMode);

    initAllTargetData();

    auto curTK = ExtendedConfig::curTKID();
    ui->plan_device_id->addItem(curTK, curTK);

    switch (m_impl->widgetMode)
    {
        //因为是查看，所以不能进行编辑
    case View:
    {
        ui->plan_time->setEnabled(false);
        ui->task_target_total->setEnabled(false);
        ui->plan_sn->setEnabled(false);
        ui->task_main_target->setEnabled(false);
        ui->plan_device_id->setEnabled(false);
        ui->plan_school_zero->setEnabled(false);
        ui->plan_employer->setEnabled(false);
        ui->plan_school_phase->setEnabled(false);
        ui->targetWidget->setControlEnable();

        break;
    }
    case New:
    {
        ui->plan_sn->setEnabled(false);
        ui->plan_sn->setText("服务器自动生成");

        ui->plan_time->setEnabled(false);
        ui->plan_time->setText("当前时间");
        break;
    }
    case Edit:
    {
        break;
    }
    }

    fullMainTarget();
    connect(ui->targetTabBar, &QTabBar::currentChanged, this, &DeviceWorkTaskWidget::targetChange);
    connect(ui->targetWidget, &LocalPlanDialog::signalCurTargetTaskCodeChanged, this, &DeviceWorkTaskWidget::fullMainTarget);

    connect(ui->task_target_total, &QComboBox::currentTextChanged, this, &DeviceWorkTaskWidget::taskTargetTotalChanged);
    connect(ui->ok, &QPushButton::clicked, this, &DeviceWorkTaskWidget::ok);
    connect(ui->cancel, &QPushButton::clicked, this, &DeviceWorkTaskWidget::reject);
    connect(parent, &PlanListWidget::sg_addItemACK, this, &DeviceWorkTaskWidget::addACK);
    connect(parent, &PlanListWidget::sg_updateItemACK, this, &DeviceWorkTaskWidget::updateItemACK);
    ui->targetTabBar->setDrawBase(false);  // 隐藏下方多余显示条
    if (widgetMode == WidgetMode::New)
    {
        ui->ok->setText("新增");
    }
    if (widgetMode == WidgetMode::View)
    {
        ui->ok->hide();
    }
    if (widgetMode == WidgetMode::Edit)
    {
        ui->ok->setText("编辑");
    }
}

DeviceWorkTaskWidget::~DeviceWorkTaskWidget()
{
    delete ui;
    delete m_impl;
}
void DeviceWorkTaskWidget::targetChange(int index)
{
    /* 切换目标 */
    /* 先保存当前目标数据 */
    if (m_impl->widgetMode != WidgetMode::View)
    {
        saveCurTargetData();
    }
    /* 切换目标 */
    ui->targetWidget->setCurTarget(index + 1);
    refreshCurTargetData();
}

void DeviceWorkTaskWidget::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    event->accept();
}

// PlanWorkTaskRespond DeviceWorkTaskWidget::getCurValue() { return m_impl->respond; }

void DeviceWorkTaskWidget::setWidgetData(const DeviceWorkTask& deviceWorkTask, const DataTranWorkTaskList& dataTranWorkTaskList)
{
    if (m_impl->widgetMode == WidgetMode::View)
    {
        ui->ok->hide();
    }
    /*
     * 20210623
     * 增加编辑功能
     * 这里直接阻塞所有能触发信号槽的控件
     * 然后手动进行数据的填充和切换
     */

    /* 数据转换 */
    auto indexToTargetBak = m_impl->indexToTarget;
    for (int i = 0; i < gTaskPlanTotalTagrgetNum; ++i)
    {
        auto targetNum = i + 1; /* 目标编号 */
        auto& targetInfo = indexToTargetBak.at(i);

        // 设置设备工作计划
        if (deviceWorkTask.m_totalTargetMap.contains(targetNum))
        {
            auto& value = deviceWorkTask.m_totalTargetMap.value(targetNum);
            targetInfo.curTaskCode = value.m_task_code;
            targetInfo.setDevInfo(value);
        }

        // 设置数传工作计划
        const DataTranWorkTask* dataTranWorkTask = nullptr;
        // 查找该目标对应的数传计划
        for (auto& dataTask : dataTranWorkTaskList)
        {
            if (dataTask.m_belongTarget == targetNum)
            {
                dataTranWorkTask = &dataTask;
                break;
            }
        }
        targetInfo.dtEffective = (dataTranWorkTask != nullptr);
        if (targetInfo.dtEffective)
        {
            targetInfo.setDTInfo(*dataTranWorkTask);
        }
    }

    /* 界面赋值 */
    ui->plan_time->setText(Utility::dateTimeToStr(deviceWorkTask.m_createTime));  //创建时间
    ui->plan_sn->setText(deviceWorkTask.m_plan_serial_number);                    //计划流水号
    ui->plan_employer->setCurrentText(deviceWorkTask.m_work_unit);                //工作单位
    ui->plan_school_phase->setCurrentIndex(deviceWorkTask.m_calibration == TaskCalibration::NoCalibration ? 1
                                                                                                          : 0);  //是否校相  值true false 和索引一致
    ui->plan_school_zero->setCurrentIndex(deviceWorkTask.m_is_zero == TaskCalibration::NoCalibration ? 1 : 0);  //是否校零 值true false 和索引一致

    /* 目标数量改变不触发信号 */
    ui->task_target_total->blockSignals(true);
    ui->task_target_total->setCurrentText(QString::number(deviceWorkTask.m_target_number));
    ui->task_target_total->blockSignals(false);

    /* 阻塞目标数量的targetTabBar的信号 不触发目标数量的刷新 */
    ui->targetTabBar->blockSignals(true);
    taskTargetTotalChanged();
    /* 选择第一个目标 */
    ui->targetTabBar->setCurrentIndex(0);
    ui->targetTabBar->blockSignals(false);

    /* 将数据赋值到当前的缓存数据中 */
    m_impl->indexToTarget = indexToTargetBak;

    /* 刷新当前界面 */
    refreshCurTargetData();

    /* 刷新当前的主跟目标 */
    fullMainTarget();

    /* 主跟目标赋值 */
    ui->task_main_target->setCurrentText(deviceWorkTask.m_lord_with_target);  //主跟目标

    m_impl->taskPlanData.dataTransWorkTasks = dataTranWorkTaskList;
    m_impl->taskPlanData.deviceWorkTask = deviceWorkTask;
}

void DeviceWorkTaskWidget::addACK(const QByteArray& data)
{
    auto ack = Optional<DeviceWorkTaskTarget>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::warning(this, QString("设备工作计划"), QString("添加设备工作计划失败"), QString("确定"));
        return;
    }
    QDialog::accept();
}
void DeviceWorkTaskWidget::updateItemACK(const QByteArray& data)
{
    auto ack = Optional<DeviceWorkTaskTarget>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::warning(this, QString("设备工作计划"), QString("更新设备工作计划失败"), QString("确定"));
        return;
    }
    QDialog::accept();
}

void DeviceWorkTaskWidget::ok()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    //这里是检查部分关系
    QString errMsg;
    if (!ui->targetWidget->checkRelation(errMsg))
    {
        QMessageBox::information(this, "提示", errMsg);
        return;
    }

    TaskPlanData taskPlanData;
    if (m_impl->widgetMode == WidgetMode::Edit)
    {
        taskPlanData = m_impl->taskPlanData;
        taskPlanData.dataTransWorkTasks.clear();
        taskPlanData.deviceWorkTask.m_totalTargetMap.clear();
    }
    else
    {
        //计划流水号,新建的时候计划流水号不设置，到服务器之后自动生成一个
        taskPlanData.deviceWorkTask.m_plan_serial_number = "";
    }

    auto& deviceWorkTask = taskPlanData.deviceWorkTask; /* 测控计划 */

    deviceWorkTask.m_createTime = GlobalData::currentDateTime();  //创建时间
    deviceWorkTask.m_create_userid = "Test";                      //任务的创建者ID 默认的是fep

    deviceWorkTask.m_work_unit = ui->plan_employer->currentText();  //工作单位
    deviceWorkTask.m_equipment_no = "";                             //设备编号
    deviceWorkTask.m_scheduling_mode = "SB";                        //调度模式

    deviceWorkTask.m_task_total_num = 1;                                                           //任务总数
    deviceWorkTask.m_calibration = TaskCalibration(ui->plan_school_phase->currentData().toInt());  //是否校相
    deviceWorkTask.m_is_zero = TaskCalibration(ui->plan_school_zero->currentData().toInt());       //是否校零

    deviceWorkTask.m_target_number = ui->task_target_total->currentData().toInt();  //目标数量
    deviceWorkTask.m_lord_with_target = ui->task_main_target->currentText();        //主跟目标
    deviceWorkTask.m_tracking_order = "x";                                          //跟踪顺序
    deviceWorkTask.m_task_status = TaskPlanStatus::NoStart;                         //任务状态
    deviceWorkTask.m_source = TaskPlanSource::Local;
    deviceWorkTask.m_working_mode = ui->plan_type->currentData().toString();

    /* 先保存当前的数据 */
    saveCurTargetData();

    auto minCount = ui->targetTabBar->count();
    minCount = (minCount > deviceWorkTask.m_target_number) ? deviceWorkTask.m_target_number : minCount;
    /* 获取多目标的数据 */
    for (int index = 0; index < minCount; index++)
    {
        auto curTargetNum = index + 1;
        /* 目标的数据 */
        auto& indexToTarget = m_impl->indexToTarget.at(index);
        /* 测控 */
        if (!indexToTarget.targetDevInfo)
        {
            QMessageBox::critical(this, "提示", indexToTarget.targetDevInfo.msg(), "确定");
            return;
        }

        /* 数传 */
        if (indexToTarget.dtEffective)
        {
            //随机生成一个跟踪计划号
            indexToTarget.targetDevInfo->m_dt_transNum = QString::number(qrand() % 100000000);

            /* 数传计划 */
            if (!indexToTarget.targetDTInfo)
            {
                QMessageBox::critical(this, "提示", indexToTarget.targetDTInfo.msg(), "确定");
                return;
            }
            auto& dataPlan = indexToTarget.targetDTInfo.value();
            dataPlan.m_dt_transNum = indexToTarget.targetDevInfo->m_dt_transNum;
            dataPlan.m_belongTarget = curTargetNum;
            dataPlan.m_plan_serial_number = deviceWorkTask.m_plan_serial_number;
            dataPlan.m_work_unit = deviceWorkTask.m_work_unit;
            dataPlan.m_createTime = deviceWorkTask.m_createTime;  //创建时间

            indexToTarget.targetDevInfo.value().m_dt_starttime = dataPlan.m_task_start_time;
            indexToTarget.targetDevInfo.value().m_dt_endtime = dataPlan.m_task_end_time;

            taskPlanData.dataTransWorkTasks << dataPlan;
        }
        deviceWorkTask.m_totalTargetMap.insert(curTargetNum, indexToTarget.targetDevInfo.value());
    }

    //添加目标时间
    if (!deviceWorkTask.m_totalTargetMap.isEmpty())
    {
        for (auto& item : deviceWorkTask.m_totalTargetMap)
        {
            if (item.m_task_code == deviceWorkTask.m_lord_with_target)
            {
                deviceWorkTask.m_preStartTime = item.m_task_ready_start_time;  //任务准备开始时间
                deviceWorkTask.m_startTime = item.m_task_start_time;           //任务开始时间
                deviceWorkTask.m_endTime = item.m_task_end_time;               //任务结束时间
            }
        }
    }

    //本地校验任务开始时间和任务结束时间是否满足要求
    bool checkTask = TaskPlanSql::checkTask(deviceWorkTask.m_preStartTime, deviceWorkTask.m_endTime, deviceWorkTask.m_uuid);
    if (checkTask)
    {
        QMessageBox::information(this, "提示", "新任务计划时间和已有计划冲突", "确定");
        SystemLogPublish::infoMsg("新任务计划时间和已有计划冲突");
        return;
    }

    //这里来检查模式  如S标准双点频不支持
    errMsg.clear();
    if (!checkModeRelation(taskPlanData, errMsg))
    {
        QMessageBox::information(this, "提示", errMsg);
        return;
    }

    QByteArray json;
    json << taskPlanData;
    if (m_impl->widgetMode == WidgetMode::New)
    {
        emit m_impl->parent->sg_addItem(json);
    }
    else
    {
        emit m_impl->parent->sg_updateItem(json);
    }
}

bool DeviceWorkTaskWidget::checkModeRelation(const TaskPlanData& taskPlanData, QString& errorMsg)
{
    auto plan = taskPlanData;
    int index = 0;
    for (auto target : plan.deviceWorkTask.m_totalTargetMap.keys())
    {
        index++;  //代表目标几
        auto targetPlan = plan.deviceWorkTask.m_totalTargetMap[target];
        AllModeExist modeExist;
        for (auto link : targetPlan.m_linkMap)  //一个目标里的所有链路信息
        {
            if (link.m_work_system == STTC)
            {
                modeExist.sttcMode++;
            }
            else if (link.m_work_system == Skuo2)
            {
                modeExist.skuo2Mode++;
            }
            else if (link.m_work_system == KaKuo2)
            {
                modeExist.kakuo2Mode++;
            }
            else if (link.m_work_system == SKT)
            {
                modeExist.sktMode++;
            }
            else if (link.m_work_system == KaDS)
            {
                modeExist.kadsMode++;
            }
            else if (link.m_work_system == XDS)
            {
                modeExist.xdsMode++;
            }
            else if (link.m_work_system == KaGS)
            {
                modeExist.kagsMode++;
            }
        }
        if (modeExist.sttcMode >= 2)
        {
            errorMsg = QString("目标%1：S标准点频数量超过1个，基带数量不足支撑功能").arg(index);
            return false;
        }
        if (modeExist.sktMode >= 2)
        {
            errorMsg = QString("目标%1：S扩跳点频数量超过1个，基带数量不足支撑功能").arg(index);
            return false;
        }
        if ((modeExist.skuo2Mode > 4) || (modeExist.kakuo2Mode > 4))
        {
            errorMsg = QString("目标%1：扩频点频数量超过4个，基带数量不足支撑功能").arg(index);
            return false;
        }

        if (modeExist.sttcMode >= 1 && (modeExist.skuo2Mode >= 1 || modeExist.kakuo2Mode >= 1))
        {
            errorMsg = QString("目标%1：S标准和扩二不能同时执行任务，系统链路不足支撑功能").arg(index);
            return false;
        }
        if (modeExist.sktMode >= 1 && (modeExist.skuo2Mode >= 1 || modeExist.kakuo2Mode >= 1))
        {
            errorMsg = QString("目标%1：S扩跳和扩二不能同时执行任务，系统链路不足支撑功能").arg(index);
            return false;
        }
    }
    return true;
}
void DeviceWorkTaskWidget::saveCurTargetData()
{
    auto curTarget = ui->targetWidget->curTarget();
    Q_ASSERT(curTarget <= gTaskPlanTotalTagrgetNum && curTarget >= 1);
    auto& indexToTarget = m_impl->indexToTarget.at(curTarget - 1);

    indexToTarget.targetDevInfo = ui->targetWidget->getDeviceWorkTaskTarget();
    indexToTarget.targetDTInfo = ui->targetWidget->getDataTranWorkTask();
    indexToTarget.dtEffective = ui->targetWidget->dtEffective();
    indexToTarget.curTaskCode = ui->targetWidget->curTaskCode();
}
void DeviceWorkTaskWidget::initAllTargetData()
{
    IndexToTargetInfo info;
    info.targetDevInfo = ui->targetWidget->getDeviceWorkTaskTarget();
    info.targetDTInfo = ui->targetWidget->getDataTranWorkTask();
    info.dtEffective = ui->targetWidget->dtEffective();
    info.curTaskCode = ui->targetWidget->curTaskCode();
    m_impl->defaultTargetInfoData = info;
    for (auto& item : m_impl->indexToTarget)
    {
        item = info;
    }
}

void DeviceWorkTaskWidget::refreshCurTargetData()
{
    /* 这里是只管刷新不管原来数据的保存 */
    auto curIndex = ui->targetTabBar->currentIndex();
    Q_ASSERT(curIndex < gTaskPlanTotalTagrgetNum);

    const auto& indexToTarget = m_impl->indexToTarget.at(curIndex);
    // FIXME wjy 设置界面数据的时候不做信号触发
    ui->targetWidget->blockSignals(true);
    ui->targetWidget->setDeviceWorkTaskTarget(indexToTarget.targetDevInfo.notCheckValue());
    if (indexToTarget.dtEffective)
    {
        ui->targetWidget->setDataTranWorkTask(indexToTarget.targetDTInfo.notCheckValue());
    }
    ui->targetWidget->blockSignals(false);
}

void DeviceWorkTaskWidget::fullMainTarget()
{
    /* 先备份当前主跟目标 */
    auto bakMainTarget = ui->task_main_target->currentText();
    ui->task_main_target->clear();

    /* 先获取当前目标 */
    auto curTarget = ui->targetWidget->curTarget();
    auto curTaskCode = ui->targetWidget->curTaskCode();

    int curTargetTotal = ui->task_target_total->currentData().toInt();
    QSet<QString> tempSet;
    tempSet.insert(curTaskCode);
    for (int i = 0; i < curTargetTotal; ++i)
    {
        if (curTarget == i + 1)
        {
            continue;
        }
        Q_ASSERT(gTaskPlanTotalTagrgetNum > i);
        const auto& curTargetData = m_impl->indexToTarget.at(i);
        auto curCode = curTargetData.curTaskCode;
        if (!curCode.isEmpty())
        {
            tempSet.insert(curCode);
        }
    }
    for (auto& item : tempSet)
    {
        ui->task_main_target->addItem(item, item);
    }
    /* 还原当前主跟 */
    if (tempSet.contains(bakMainTarget))
    {
        ui->task_main_target->setCurrentText(bakMainTarget);
    }
}
void DeviceWorkTaskWidget::resetTableBarCount(int targetCount)
{
    Q_ASSERT(targetCount > 0);
    auto curBarCount = ui->targetTabBar->count();

    if (curBarCount >= targetCount)
    {
        for (int i = curBarCount; i > targetCount; --i)
        {
            m_impl->indexToTarget.at(i - 1) = m_impl->defaultTargetInfoData;
            ui->targetTabBar->removeTab(i - 1);
        }
        return;
    }

    for (int i = curBarCount; i < targetCount; ++i)
    {
        m_impl->indexToTarget.at(i) = m_impl->defaultTargetInfoData;
        ui->targetTabBar->addTab(QString("目标%1").arg(i + 1));
    }
}
void DeviceWorkTaskWidget::taskTargetTotalChanged()
{
    int curTargetTotal = ui->task_target_total->currentData().toInt(); /*  1-4 */
    //动态调整targetTabBar的大小
    ui->targetTabBar->setMinimumWidth(75 * curTargetTotal);
    // int index = ui->targetTabBar->currentIndex();                      /*0-3 */
    /* 根据目标数量动态生成table页的数量 */
    resetTableBarCount(curTargetTotal);

    fullMainTarget();
}
