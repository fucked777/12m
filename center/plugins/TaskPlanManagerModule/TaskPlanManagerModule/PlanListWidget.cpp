#include "PlanListWidget.h"
#include "ui_PlanListWidget.h"

#include <QDateTime>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QMouseEvent>
#include <QTableView>
#include <QTimer>

#include "AutoRunTaskLogMessageSerialize.h"
#include "CConverter.h"
#include "CommonSelectDelegate.h"
#include "DataTransferTaskModel.h"
#include "DeviceWorkTaskModel.h"
#include "DeviceWorkTaskWidget.h"
#include "FepSendUnit.h"
#include "GlobalData.h"
#include "LocalPlanDialog.h"
#include "MessagePublish.h"
#include "PlatformInterface.h"
#include "QssCommonHelper.h"
#include "RealTimeLogModel.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "SubscriberHelper.h"
#include "TaskPlanMessageSerialize.h"
#include "TaskPlanSql.h"
#include "Utility.h"

/*
 * 20210428 更改
 * 去掉分页
 * 默认刷新只显示未执行和当前正在执行的计划
 * 按时间查询会查询到当前时间段内的所有计划
 */
class PlanListWidgetImpl
{
public:
    FepSendUnit* fepSend{ nullptr };
    RealtimeLogModel* realTimeLogModel{ nullptr };
    DeviceWorkTaskModel* deviceWorkTaskModel{ nullptr };
    DataTransferTaskModel* dtModel{ nullptr };
    QMenu* dtMenu{ nullptr };
    QMenu* devMenu{ nullptr };
    QMenu* logMenu{ nullptr };

    bool runStatus = false;
    bool isAutoRefresh = false;  // 是否进行自动刷新
    int currentDeviceTaskPage = 1;
    /* 自动刷新后需要还原选中的行,不然会一直闪烁选不中 */
    int deviceIndex{ 0 };
    int dtIndex{ 0 };

    /* 用于解决任务计划界面会生成多个界面用于展示,其中有部分操作会导致两边界面同时操作,比如同时弹窗
     * 现在加一个最后一次的操作时间
     * 当两次操作时间在operateInterval秒以内认为是当前界面的操作的
     */
    qint64 lastOperateTimeMS;
    qint64 operateInterval{ 10000 };

    QDate beginDate;
    QDate endDate;

    bool mShowRunningStatus = false;

    QPixmap mTaskPreparationGrayPixmap;
    QPixmap mTaskPreparationGreenPixmap;
    QPixmap mTaskPreparationRedPixmap;

    QPixmap mZeroCorrectionGrayPixmap;
    QPixmap mZeroCorrectionGreenPixmap;
    QPixmap mZeroCorrectionRedPixmap;

    QPixmap mTurnToWaitingPointGrayPixmap;
    QPixmap mTurnToWaitingPointGreenPixmap;
    QPixmap mTurnToWaitingPointRedPixmap;

    QPixmap mPhaseCorrectionGrayPixmap;
    QPixmap mPhaseCorrectionGreenPixmap;
    QPixmap mPhaseCorrectionRedPixmap;

    QPixmap mCaptureTrackGrayPixmap;
    QPixmap mCaptureTrackGreenPixmap;
    QPixmap mCaptureTrackRedPixmap;

    QPixmap mGoupGrayPixmap;
    QPixmap mGoupGreenPixmap;
    QPixmap mGoupRedPixmap;

    QPixmap mTaskEndGrayPixmap;
    QPixmap mTaskEndGreenPixmap;
    QPixmap mTaskEndRedPixmap;
};

PlanListWidget::PlanListWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PlanListWidget)
    , m_impl(new PlanListWidgetImpl)
{
    ui->setupUi(this);

    // 初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    initUI();

    connect(SubscriberHelper::getInstance(), &SubscriberHelper::signalAutoRunTaskStepStatusReadable, this,
            &PlanListWidget::slotReadAutoRunTaskStepStatus);
    connect(SubscriberHelper::getInstance(), &SubscriberHelper::signalAutoRunTaskLogReadable, this, &PlanListWidget::slotReadAutoRunTaskLog);

    startTimer(3000);
}

PlanListWidget::~PlanListWidget()
{
    m_impl->fepSend->setParent(nullptr);
    m_impl->fepSend->stop();
    delete m_impl->fepSend;
    delete ui;
    delete m_impl;
}

void PlanListWidget::initUI()
{
    initPixmap();

    m_impl->fepSend = new FepSendUnit(nullptr);
    ui->tabWidget->setMinimumHeight(370);
    m_impl->deviceWorkTaskModel = new DeviceWorkTaskModel(this);
    m_impl->dtModel = new DataTransferTaskModel(this);
    m_impl->realTimeLogModel = new RealtimeLogModel(this);
    ui->projectTable->setModel(m_impl->deviceWorkTaskModel);
    ui->dtTable->setModel(m_impl->dtModel);
    ui->logTableView->setModel(m_impl->realTimeLogModel);

    ui->projectTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->dtTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->logTableView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->projectTable->setItemDelegate(new CommonSelectDelegate(ui->projectTable));
    ui->dtTable->setItemDelegate(new CommonSelectDelegate(ui->dtTable));
    ui->logTableView->setItemDelegate(new CommonSelectDelegate(ui->logTableView));

    ui->projectTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    Utility::tableViewInit(ui->projectTable);
    Utility::tableViewInit(ui->dtTable);
    Utility::tableViewInit(ui->logTableView);

    ui->dtTable->setColumnWidth(DataTransferTaskModel::SerialNumber, 155);
    ui->dtTable->setColumnWidth(DataTransferTaskModel::PlanType, 150);
    ui->dtTable->setColumnWidth(DataTransferTaskModel::TaskPlanSource, 60);
    ui->dtTable->setColumnWidth(DataTransferTaskModel::Priority, 80);
    ui->dtTable->setColumnWidth(DataTransferTaskModel::Acu, 140);
    ui->dtTable->setColumnWidth(DataTransferTaskModel::Priority, 140);
    ui->dtTable->setColumnWidth(DataTransferTaskModel::TaskStartTime, 180);
    ui->dtTable->setColumnWidth(DataTransferTaskModel::TransportTime, 180);
    ui->dtTable->setColumnWidth(DataTransferTaskModel::TaskEndTime, 180);
    ui->dtTable->setColumnWidth(DataTransferTaskModel::CreateTime, 180);

    ui->projectTable->setColumnWidth(DeviceWorkTaskModel::SerialNumber, 155);
    ui->projectTable->setColumnWidth(DeviceWorkTaskModel::TaskPlanType, 150);
    ui->projectTable->setColumnWidth(DeviceWorkTaskModel::TaskPlanSource, 60);
    ui->projectTable->setColumnWidth(DeviceWorkTaskModel::CircleNo, 60);
    ui->projectTable->setColumnWidth(DeviceWorkTaskModel::WorkMode, 140);
    ui->projectTable->setColumnWidth(DeviceWorkTaskModel::TaskStartTime, 180);
    ui->projectTable->setColumnWidth(DeviceWorkTaskModel::CreateTime, 180);
    ui->projectTable->setColumnWidth(DeviceWorkTaskModel::TaskStatus, 140);
    ui->projectTable->setColumnWidth(DeviceWorkTaskModel::TaskEndTime, 180);
    ui->projectTable->setColumnWidth(DeviceWorkTaskModel::TrackStartTime, 180);
    ui->projectTable->setColumnWidth(DeviceWorkTaskModel::TrackEndTime, 180);

    ui->logTableView->setColumnWidth(RealtimeLogModel::Time, 200);
    ui->logTableView->setColumnWidth(RealtimeLogModel::TaskId, 150);
    ui->logTableView->horizontalHeader()->setSectionResizeMode(RealtimeLogModel::Context, QHeaderView::Stretch);
    /*************************************************************************************************/
    m_impl->dtMenu = new QMenu(this);
    m_impl->devMenu = new QMenu(this);
    m_impl->logMenu = new QMenu(this);
    m_impl->devMenu->addAction(QString("编辑"), this, &PlanListWidget::editItem);
    m_impl->devMenu->addAction(QString("查看"), this, &PlanListWidget::viewItem);
    m_impl->devMenu->addAction(QString("删除"), this, &PlanListWidget::deleteItem);

    m_impl->dtMenu->addAction(QString("查看"), this, &PlanListWidget::viewItem);
    m_impl->dtMenu->addAction(QString("编辑"), this, &PlanListWidget::editItem);

    m_impl->logMenu->addAction(QString("清空数据"), [=]() { m_impl->realTimeLogModel->clear(); });

    ui->tabWidget->setCurrentIndex(0);

    m_btnGroup.addButton(ui->manualRunRadioBtn, 0);
    m_btnGroup.addButton(ui->autoRunRadioBtn, 1);

    //    ui->manualRunRadioBtn->hide();
    //    ui->autoRunRadioBtn->hide();

    connect(&m_btnGroup, static_cast<void (QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked), this,
            &PlanListWidget::slotShowSwitchAutoRunDialog);

    connect(ui->refreshBtn, &QPushButton::clicked, [=]() { autoRefreshChange(true); });
    connect(ui->queryByTime, &QPushButton::clicked, this, &PlanListWidget::queryByTime);
    connect(ui->addBtn, &QPushButton::clicked, this, &PlanListWidget::addItem);
    connect(ui->viewBtn, &QPushButton::clicked, this, &PlanListWidget::viewItem);
    connect(ui->editBtn, &QPushButton::clicked, this, &PlanListWidget::editItem);
    connect(ui->deleteBtn, &QPushButton::clicked, this, &PlanListWidget::deleteItem);
    connect(ui->getPlan, &QPushButton::clicked, this, &PlanListWidget::getPlan);
    connect(ui->projectTable, &QTableView::doubleClicked, this, &PlanListWidget::viewItem);
    connect(ui->dtTable, &QTableView::doubleClicked, this, &PlanListWidget::viewItem);
    connect(ui->projectTable, &QTableView::clicked, this, &PlanListWidget::projectTableSelectChange);
    connect(ui->dtTable, &QTableView::clicked, this, &PlanListWidget::dtTableSelectChange);
    connect(ui->tabWidget, &QTabWidget::currentChanged, [=](int index) {
        /* 数传没有删除 */
        ui->deleteBtn->setVisible(index != 1);
        autoRefreshData();
    });

    connect(ui->projectTable, &QTableView::customContextMenuRequested, this, [=]() {
        QPoint pt = ui->projectTable->mapFromGlobal(QCursor::pos());
        int height = ui->projectTable->horizontalHeader()->height();
        QPoint pt2(0, height);
        pt -= pt2;
        QModelIndex index = ui->projectTable->indexAt(pt);
        bool flag = m_impl->deviceWorkTaskModel->judgeData(index.row());
        if (ui->projectTable->currentIndex().isValid() && flag)
        {
            m_impl->devMenu->exec(QCursor::pos());
        }
    });
    connect(ui->dtTable, &QTableView::customContextMenuRequested, this, [=]() {
        QPoint pt = ui->dtTable->mapFromGlobal(QCursor::pos());
        int height = ui->dtTable->horizontalHeader()->height();
        QPoint pt2(0, height);
        pt -= pt2;
        QModelIndex index = ui->dtTable->indexAt(pt);
        bool flag = m_impl->dtModel->judgeData(index.row());
        if (ui->dtTable->currentIndex().isValid() && flag)
        {
            m_impl->dtMenu->exec(QCursor::pos());
        }
    });

    connect(ui->logTableView, &QTableView::customContextMenuRequested, this, [=]() {
        QPoint pt = ui->logTableView->mapFromGlobal(QCursor::pos());
        int height = ui->logTableView->horizontalHeader()->height();
        QPoint pt2(0, height);
        pt -= pt2;
        //        QModelIndex index = ui->logTableView->indexAt(pt);
        //        bool flag = m_impl->realTimeLogModel->judgeData(index.row());
        //        if (ui->logTableView->currentIndex().isValid() && flag)
        //        {
        m_impl->logMenu->exec(QCursor::pos());
        //        }
    });

    connect(m_impl->fepSend, &FepSendUnit::sg_error, [=](const QString& errMsg) { QMessageBox::critical(this, "错误", errMsg, "确定"); });
    connect(m_impl->fepSend, &FepSendUnit::sg_msg,
            [=](bool msg, const QString& errMsg) { QMessageBox::information(this, "提示", msg ? errMsg : "发送完成", "确定"); });
}

void PlanListWidget::initPixmap()
{
    m_impl->mTaskPreparationGrayPixmap = QPixmap(":/image/task_preparation_gray.png");
    m_impl->mTaskPreparationGreenPixmap = QPixmap(":/image/task_preparation_green.png");
    m_impl->mTaskPreparationRedPixmap = QPixmap(":/image/task_preparation_red.png");

    m_impl->mZeroCorrectionGrayPixmap = QPixmap(":/image/zero_correction_gray.png");
    m_impl->mZeroCorrectionGreenPixmap = QPixmap(":/image/zero_correction_green.png");
    m_impl->mZeroCorrectionRedPixmap = QPixmap(":/image/zero_correction_red.png");

    m_impl->mTurnToWaitingPointGrayPixmap = QPixmap(":/image/turn_to_waiting_point_gray.png");
    m_impl->mTurnToWaitingPointGreenPixmap = QPixmap(":/image/turn_to_waiting_point_green.png");
    m_impl->mTurnToWaitingPointRedPixmap = QPixmap(":/image/turn_to_waiting_point_red.png");

    m_impl->mPhaseCorrectionGrayPixmap = QPixmap(":/image/phase_correction_gray.png");
    m_impl->mPhaseCorrectionGreenPixmap = QPixmap(":/image/phase_correction_green.png");
    m_impl->mPhaseCorrectionRedPixmap = QPixmap(":/image/phase_correction_red.png");

    m_impl->mCaptureTrackGrayPixmap = QPixmap(":/image/capture_track_gray.png");
    m_impl->mCaptureTrackGreenPixmap = QPixmap(":/image/capture_track_green.png");
    m_impl->mCaptureTrackRedPixmap = QPixmap(":/image/capture_track_red.png");

    m_impl->mGoupGrayPixmap = QPixmap(":/image/goup_gray.png");
    m_impl->mGoupGreenPixmap = QPixmap(":/image/goup_green.png");
    m_impl->mGoupRedPixmap = QPixmap(":/image/goup_red.png");

    m_impl->mTaskEndGrayPixmap = QPixmap(":/image/task_end_gray.png");
    m_impl->mTaskEndGreenPixmap = QPixmap(":/image/task_end_green.png");
    m_impl->mTaskEndRedPixmap = QPixmap(":/image/task_end_red.png");

    // 设置默认图片
    ui->taskPreparationLabel->setPixmap(m_impl->mTaskPreparationGrayPixmap);
    ui->zeroCorrectionLabel->setPixmap(m_impl->mZeroCorrectionGrayPixmap);
    ui->turnToWaitingPointLabel->setPixmap(m_impl->mTurnToWaitingPointGrayPixmap);
    ui->phaseCorrectionLabel->setPixmap(m_impl->mPhaseCorrectionGrayPixmap);
    ui->captureTrackLabel->setPixmap(m_impl->mCaptureTrackGrayPixmap);
    ui->goUpLabel->setPixmap(m_impl->mGoupGrayPixmap);
    ui->taskEndLabel->setPixmap(m_impl->mTaskEndGrayPixmap);
}

void PlanListWidget::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);
    autoRefreshChange(true);
    auto tempDate = GlobalData::currentDate();
    m_impl->beginDate = tempDate.addDays(-3);
    m_impl->endDate = tempDate;

    ui->endDate->setDate(m_impl->endDate);
    ui->beginDate->setDate(m_impl->beginDate);
}

void PlanListWidget::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event);
    autoRefreshChange(false);
}

void PlanListWidget::addItem()
{
    DeviceWorkTaskWidget deviceWorkTaskWidget(WidgetMode::New, this);
    deviceWorkTaskWidget.setWindowTitle(QString("新建工作计划"));
    if (QDialog::Accepted == deviceWorkTaskWidget.exec())
    {
        auto isAutoTaskPlan = GlobalData::getAutoRunTaskFlag();

        if (!isAutoTaskPlan)
        {
            QMessageBox::information(this, "提示", "当前为手动任务,添加任务后请切换为自动运行任务!", "确定");
        }
        /* 手动刷新显示数据 */
        autoRefreshData();
    }
    deviceWorkTaskWidget.setParent(nullptr);
}
void PlanListWidget::deleteItem()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    if (GlobalData::getAutoRunTaskFlag())
    {
        QMessageBox::information(this, QString("提示"), QString("自动化运行时不能进行删除任务计划，手动时才能删除"), QString("确定"));
        return;
    }

    m_impl->lastOperateTimeMS = GlobalData::currentDateTime().toMSecsSinceEpoch();
    /* 数传计划不能直接删除 */
    Q_ASSERT(1 != ui->tabWidget->currentIndex());

    auto devPlanResult = m_impl->deviceWorkTaskModel->getItem(ui->projectTable->currentIndex());
    if (!devPlanResult)
    {
        QMessageBox::warning(this, QString("提示"), QString("请选择需要删除的数据,然后进行删除操作!!!!"), "确定");
        return;
    }
    if (devPlanResult->m_task_status == TaskPlanStatus::Running)
    {
        if (QMessageBox::information(this, "提示", "当前计划正在运行,删除会导致任务终止,是否需要删除,请谨慎操作!!!!", "确定", "取消") != 0)
            return;
    }
    else
    {
        if (QMessageBox::information(this, "提示", "您确定要删除该项任务计划吗?", "确定", "取消") != 0)
            return;
    }

    QByteArray data;
    DeviceWorkTaskDelete deviceWorkTaskDelete;
    deviceWorkTaskDelete.uuid = devPlanResult->m_uuid;
    data << deviceWorkTaskDelete;
    emit sg_deletePlanItem(data);
}
void PlanListWidget::viewItem()
{
    SERVICEONLINECHECK();

    m_impl->lastOperateTimeMS = GlobalData::currentDateTime().toMSecsSinceEpoch();

    // 由于客户端没有保存全部的设备计划和数传计划,所以需要通过计划流水号查询出设备计划和关联的数传计划
    int currentTabIndex = ui->tabWidget->currentIndex();

    DeviceWorkTaskView deviceWorkTaskView;
    // 选中的是设备计划查看
    if (currentTabIndex == 0)
    {
        auto devPlanResult = m_impl->deviceWorkTaskModel->getItem(ui->projectTable->currentIndex());
        if (!devPlanResult)
        {
            QMessageBox::warning(this, QString("提示"), QString("请选择要查看的数据!!!!"), "确定");
            return;
        }

        deviceWorkTaskView.id = devPlanResult->m_uuid;
    }
    // 选中的是实时数传计划
    else if (currentTabIndex == 1)
    {
        auto dtItemResult = m_impl->dtModel->getItem(ui->dtTable->currentIndex());
        if (!dtItemResult)
        {
            QMessageBox::warning(this, QString("提示"), QString("请选择要查看的数据!!!!"), "确定");
            return;
        }
        deviceWorkTaskView.sn = dtItemResult->m_plan_serial_number;
    }

    if (deviceWorkTaskView.id.isEmpty() && deviceWorkTaskView.sn.isEmpty())
    {
        QMessageBox::warning(this, QString("提示"), QString("请选择需要查看的任务计划"), QString("确定"));
        return;
    }

    QByteArray data;
    data << deviceWorkTaskView;
    emit sg_viewItem(data);
}

void PlanListWidget::editItem()
{
    SERVICEONLINECHECK();
    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    m_impl->lastOperateTimeMS = GlobalData::currentDateTime().toMSecsSinceEpoch();

    // 由于客户端没有保存全部的设备计划和数传计划,所以需要通过计划流水号查询出设备计划和关联的数传计划
    int currentTabIndex = ui->tabWidget->currentIndex();

    DeviceWorkTaskView deviceWorkTaskView;
    // 选中的是设备计划查看
    if (currentTabIndex == 0)
    {
        auto devPlanResult = m_impl->deviceWorkTaskModel->getItem(ui->projectTable->currentIndex());
        if (!devPlanResult)
        {
            QMessageBox::warning(this, QString("提示"), QString("请选择要编辑的数据!!!!"), "确定");
            return;
        }
        if (devPlanResult->m_task_status != TaskPlanStatus::NoStart)
        {
            QMessageBox::warning(this, QString("提示"), QString("要编辑的任务必须是未运行的任务"), "确定");
            return;
        }

        deviceWorkTaskView.id = devPlanResult->m_uuid;
    }
    // 选中的是实时数传计划
    else if (currentTabIndex == 1)
    {
        auto dtItemResult = m_impl->dtModel->getItem(ui->dtTable->currentIndex());
        if (!dtItemResult)
        {
            QMessageBox::warning(this, QString("提示"), QString("请选择要编辑的数据!!!!"), "确定");
            return;
        }
        deviceWorkTaskView.sn = dtItemResult->m_plan_serial_number;
        if (dtItemResult->m_task_status != TaskPlanStatus::NoStart)
        {
            QMessageBox::warning(this, QString("提示"), QString("要编辑的任务必须是未运行的任务"), "确定");
            return;
        }
    }

    if (deviceWorkTaskView.id.isEmpty() && deviceWorkTaskView.sn.isEmpty())
    {
        QMessageBox::warning(this, QString("提示"), QString("请选择需要编辑的任务计划"), QString("确定"));
        return;
    }

    QByteArray data;
    data << deviceWorkTaskView;
    emit sg_editInfoItem(data);
}

/* 获取中心计划 */
void PlanListWidget::getPlan()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }
    //    auto configPath = PlatformConfigTools::configBusiness("Fep/FepConfig.xml");
    //    // FepConfig
    //    QString dbConnnectPath = QApplication::applicationDirPath() + "/resources/config/FepConfig.ini";
    //    QSettings setting(dbConnnectPath, QSettings::IniFormat);

    //    auto addr = setting.value("FepConfig/addr", "192.20.1.156").toString();
    //    auto file = setting.value("FepConfig/file", false).toBool();
    //    auto port = static_cast<quint16>(setting.value("FepConfig/port", 5556).toUInt());
    //    auto workDir = setting.value("FepConfig/path", "FepSend").toString();
    //    m_impl->fepSend->setServerAddr(addr, port);

    //    /* 发送文件 */
    //    if (file)
    //    {
    //        QFileInfo info(workDir);
    //        QString errMsg;
    //        if (!m_impl->fepSend->startSendFile(workDir, errMsg))
    //        {
    //            QMessageBox::information(this, "发送错误", errMsg);
    //        }
    //        return;
    //    }

    //    m_impl->fepSend->setWorkDir(workDir);
    //    QString errMsg;
    //    if (!m_impl->fepSend->start(errMsg))
    //    {
    //        QMessageBox::information(this, "发送错误", errMsg);
    //        return;
    //    }
}
void PlanListWidget::autoRefreshChange(bool clickAuto)
{
    /*
     * 两个按钮
     * 点击refreshBtn     时 查询一下数据 切换为自动刷新状态
     * 点击queryByTime    时 如果处于自动刷新则停止自动刷新 否则不处理  然后按照当前的时间段查询数据
     */
    /* 当查询历史数据的时候,停止自动刷新 */
    /* 恢复自动刷新 */
    if (clickAuto)
    {
        //点击刷新按钮的时候取消选中 ，客户奇怪的需求
        ui->projectTable->clearSelection();
        ui->dtTable->clearSelection();
        ui->refreshBtn->setText("手动刷新");
        startTimerOutRefresh();
        autoRefreshData();
    }
    else
    {
        //点击刷新按钮的时候取消选中 ，客户奇怪的需求
        ui->projectTable->clearSelection();
        ui->dtTable->clearSelection();
        ui->refreshBtn->setText("自动刷新");
        stopTimerOutRefresh();
    }
}
void PlanListWidget::autoRefreshData()
{
    if (enableTimerOutRefresh())
    {
        timerOutRefresh();
        return;
    }
    manualRefreshData(m_impl->beginDate, m_impl->endDate);
}
void PlanListWidget::queryByTime()
{
    /* 时间检查 */
    auto beginDate = ui->beginDate->date();
    auto endDate = ui->endDate->date();

    if (beginDate > endDate)
    {
        QMessageBox::information(this, "提示", "查询结束时间不能早于开始时间", "确定");
        return;
    }
    autoRefreshChange(false);
    manualRefreshData(beginDate, endDate);
}
void PlanListWidget::timerOutRefresh()
{
    int index = ui->tabWidget->currentIndex();
    switch (index)
    {
    case 0:
    {
        // DeviceWorkTaskList pdataList;
        // GlobalData::getDeviceWorkTaskRunAndNoStart(pdataList);
        // m_impl->deviceWorkTaskModel->setTaskdata(pdataList);
        auto queryResult = TaskPlanSql::queryRunningOrNoStartDevPlanMark();
        if (queryResult)
        {
            m_impl->deviceWorkTaskModel->setTaskdata(queryResult.value());
            if (m_impl->deviceIndex >= 0)
            {
                ui->projectTable->selectRow(m_impl->deviceIndex);
            }
        }
        else
        {
            m_impl->deviceWorkTaskModel->clear();
        }

        break;
    }
    case 1:
    {
        // DataTranWorkTaskList pdataList;
        // GlobalData::getDataTransWorkTaskRunAndNoStart(pdataList);
        // m_impl->dtModel->setTaskdata(pdataList);
        auto queryResult = TaskPlanSql::queryRunningOrNoStartDTPlanMark();
        if (queryResult)
        {
            m_impl->dtModel->setTaskdata(queryResult.value());
            if (m_impl->dtIndex >= 0)
            {
                ui->dtTable->selectRow(m_impl->dtIndex);
            }
        }
        else
        {
            m_impl->dtModel->clear();
        }
        break;
    }
    }
}
void PlanListWidget::manualRefreshData(const QDate& startDt, const QDate& endDt)
{
    m_impl->beginDate = startDt;
    m_impl->endDate = endDt;

    //    DeviceWorkTaskQuery deviceWorkTaskQuery;
    //    deviceWorkTaskQuery.beginDate = startDt;
    //    deviceWorkTaskQuery.endDate = endDt;
    //    QByteArray data;
    //    data << deviceWorkTaskQuery;

    int index = ui->tabWidget->currentIndex();
    switch (index)
    {
    case 0:
    {
        // emit sg_queryDeviceItem(data);
        auto queryResult = TaskPlanSql::queryHistoryDevPlanMark(startDt, endDt);
        if (!queryResult)
        {
            QMessageBox::warning(this, QString("设备工作计划"), QString("查询设备工作计划失败"), QString("确定"));
            return;
        }
        m_impl->deviceWorkTaskModel->setTaskdata(queryResult.value());

        return;
    }
    case 1:
    {
        // emit sg_queryDTItem(data);
        auto queryResult = TaskPlanSql::queryHistoryDTPlanMark(startDt, endDt);
        if (!queryResult)
        {
            QMessageBox::warning(this, QString("设备工作计划"), QString("查询设备工作计划失败"), QString("确定"));
            return;
        }
        m_impl->dtModel->setTaskdata(queryResult.value());
        return;
    }
    }
}
void PlanListWidget::startTimerOutRefresh() { m_impl->isAutoRefresh = true; }

void PlanListWidget::stopTimerOutRefresh() { m_impl->isAutoRefresh = false; }

bool PlanListWidget::enableTimerOutRefresh() const { return m_impl->isAutoRefresh; }

void PlanListWidget::deletePlanItemACK(const QByteArray& data)
{
    auto bak = m_impl->lastOperateTimeMS;
    m_impl->lastOperateTimeMS = 0;
    if (qAbs(GlobalData::currentDateTime().toMSecsSinceEpoch() - bak) > m_impl->operateInterval)
    {
        return;
    }

    auto ack = OptionalNotValue::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::warning(this, QString("设备工作计划"), QString("删除设备工作计划失败"), QString("确定"));
        return;
    }
    QMessageBox::warning(this, QString("设备工作计划"), QString("删除设备工作计划成功"), QString("确定"));

    autoRefreshData();
}

void PlanListWidget::viewItemACK(const QByteArray& data)
{
    auto bak = m_impl->lastOperateTimeMS;
    m_impl->lastOperateTimeMS = 0;
    if (qAbs(GlobalData::currentDateTime().toMSecsSinceEpoch() - bak) > m_impl->operateInterval)
    {
        return;
    }

    auto ack = Optional<TaskPlanData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::warning(this, QString("查看计划失败"), ack.msg(), QString("确定"));
        return;
    }

    DeviceWorkTaskWidget deviceWorkTaskWidget(WidgetMode::View, this);
    deviceWorkTaskWidget.setWindowTitle(QString("查看工作计划"));
    deviceWorkTaskWidget.setWidgetData(ack->deviceWorkTask, ack->dataTransWorkTasks);
    deviceWorkTaskWidget.exec();
    deviceWorkTaskWidget.setParent(nullptr);
}

void PlanListWidget::editInfoItemACK(const QByteArray& data)
{
    auto bak = m_impl->lastOperateTimeMS;
    m_impl->lastOperateTimeMS = 0;
    if (qAbs(GlobalData::currentDateTime().toMSecsSinceEpoch() - bak) > m_impl->operateInterval)
    {
        return;
    }

    auto ack = Optional<TaskPlanData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::warning(this, QString("编辑计划失败"), ack.msg(), QString("确定"));
        return;
    }

    DeviceWorkTaskWidget deviceWorkTaskWidget(WidgetMode::Edit, this);
    deviceWorkTaskWidget.setWindowTitle(QString("编辑工作计划"));
    deviceWorkTaskWidget.setWidgetData(ack->deviceWorkTask, ack->dataTransWorkTasks);
    if (QDialog::Accepted == deviceWorkTaskWidget.exec())
    {
        /* 手动刷新显示数据 */
        autoRefreshData();
    }
    deviceWorkTaskWidget.setParent(nullptr);
}

void PlanListWidget::slotReadAutoRunTaskStepStatus(const CurrentRunningTaskPlanData& data)
{
    // 更新图片
    slotUpdateTaskRunStepStatus(data.taskStepStatusMap);
    // 更新当前运行任务的时间信息
    slotUpdateTaskTime(data.taskPlanData.deviceWorkTask);
}

void PlanListWidget::slotReadAutoRunTaskLog(const AutoTaskLogData& data) { m_impl->realTimeLogModel->prependLogData(data); }

void PlanListWidget::slotUpdateTaskRunStepStatus(const QMap<TaskStep, TaskStepStatus>& stepStatusMap)
{
    for (TaskStep step : stepStatusMap.keys())
    {
        TaskStepStatus status = stepStatusMap[step];

        switch (step)
        {
        case TaskStep::START:  // 开始
        {
        }
        break;
        case TaskStep::TASK_PREPARATION:  // 任务准备
        {
            switch (status)
            {
            case TaskStepStatus::NoStart:
            case TaskStepStatus::Continue:
            {
                ui->taskPreparationLabel->setPixmap(m_impl->mTaskPreparationGrayPixmap);
            }
            break;
            case TaskStepStatus::Running:
            {
                m_impl->mShowRunningStatus ? ui->taskPreparationLabel->setPixmap(m_impl->mTaskPreparationGreenPixmap)
                                           : ui->taskPreparationLabel->setPixmap(m_impl->mTaskPreparationGrayPixmap);
            }
            break;
            case TaskStepStatus::Finish:
            {
                ui->taskPreparationLabel->setPixmap(m_impl->mTaskPreparationGreenPixmap);
            }
            break;
            case TaskStepStatus::Error:
            case TaskStepStatus::Exception:
            {
                ui->taskPreparationLabel->setPixmap(m_impl->mTaskPreparationGreenPixmap);
            }
            break;
            default: break;
            }
        }
        break;
        case TaskStep::ZERO_CORRECTION:  // 校零处理
        {
            switch (status)
            {
            case TaskStepStatus::NoStart:
            case TaskStepStatus::Continue:
            {
                ui->zeroCorrectionLabel->setPixmap(m_impl->mZeroCorrectionGrayPixmap);
            }
            break;
            case TaskStepStatus::Running:
            {
                m_impl->mShowRunningStatus ? ui->zeroCorrectionLabel->setPixmap(m_impl->mZeroCorrectionGreenPixmap)
                                           : ui->zeroCorrectionLabel->setPixmap(m_impl->mZeroCorrectionGrayPixmap);
            }
            break;
            case TaskStepStatus::Finish:
            {
                ui->zeroCorrectionLabel->setPixmap(m_impl->mZeroCorrectionGreenPixmap);
            }
            break;
            case TaskStepStatus::Error:
            case TaskStepStatus::Exception:
            {
                ui->zeroCorrectionLabel->setPixmap(m_impl->mZeroCorrectionRedPixmap);
            }
            break;
            default: break;
            }
        }
        break;
        case TaskStep::TASK_START:  // 任务开始
        {
        }
        break;
        case TaskStep::TURN_TOWAITINGPOINT:  // 转等待点
        {
            switch (status)
            {
            case TaskStepStatus::NoStart:
            case TaskStepStatus::Continue:
            {
                ui->turnToWaitingPointLabel->setPixmap(m_impl->mTurnToWaitingPointGrayPixmap);
            }
            break;
            case TaskStepStatus::Running:
            {
                m_impl->mShowRunningStatus ? ui->turnToWaitingPointLabel->setPixmap(m_impl->mTurnToWaitingPointGreenPixmap)
                                           : ui->turnToWaitingPointLabel->setPixmap(m_impl->mTurnToWaitingPointGrayPixmap);
            }
            break;
            case TaskStepStatus::Finish:
            {
                ui->turnToWaitingPointLabel->setPixmap(m_impl->mTurnToWaitingPointGreenPixmap);
            }
            break;
            case TaskStepStatus::Error:
            case TaskStepStatus::Exception:
            {
                ui->turnToWaitingPointLabel->setPixmap(m_impl->mTurnToWaitingPointRedPixmap);
            }
            break;
            default: break;
            }
        }
        break;
        case TaskStep::CALIBRATION:  // 校相处理
        {
            switch (status)
            {
            case TaskStepStatus::NoStart:
            case TaskStepStatus::Continue:
            {
                ui->phaseCorrectionLabel->setPixmap(m_impl->mPhaseCorrectionGrayPixmap);
            }
            break;
            case TaskStepStatus::Running:
            {
                m_impl->mShowRunningStatus ? ui->phaseCorrectionLabel->setPixmap(m_impl->mPhaseCorrectionGreenPixmap)
                                           : ui->phaseCorrectionLabel->setPixmap(m_impl->mPhaseCorrectionGrayPixmap);
            }
            break;
            case TaskStepStatus::Finish:
            {
                ui->phaseCorrectionLabel->setPixmap(m_impl->mPhaseCorrectionGreenPixmap);
            }
            break;
            case TaskStepStatus::Error:
            case TaskStepStatus::Exception:
            {
                ui->phaseCorrectionLabel->setPixmap(m_impl->mPhaseCorrectionRedPixmap);
            }
            break;
            default: break;
            }
        }
        break;
        case TaskStep::CAPTURE_TRACKLING:  // 捕获跟踪
        {
            switch (status)
            {
            case TaskStepStatus::NoStart:
            case TaskStepStatus::Continue:
            {
                ui->captureTrackLabel->setPixmap(m_impl->mCaptureTrackGrayPixmap);
            }
            break;
            case TaskStepStatus::Running:
            {
                m_impl->mShowRunningStatus ? ui->captureTrackLabel->setPixmap(m_impl->mCaptureTrackGreenPixmap)
                                           : ui->captureTrackLabel->setPixmap(m_impl->mCaptureTrackGrayPixmap);
            }
            break;
            case TaskStepStatus::Finish:
            {
                ui->captureTrackLabel->setPixmap(m_impl->mCaptureTrackGreenPixmap);
            }
            break;
            case TaskStepStatus::Error:
            case TaskStepStatus::Exception:
            {
                ui->captureTrackLabel->setPixmap(m_impl->mCaptureTrackRedPixmap);
            }
            break;
            default: break;
            }
        }
        break;
        case TaskStep::GO_UP:  // 发上行
        {
            switch (status)
            {
            case TaskStepStatus::NoStart:
            case TaskStepStatus::Continue:
            {
                ui->goUpLabel->setPixmap(m_impl->mGoupGrayPixmap);
            }
            break;
            case TaskStepStatus::Running:
            {
                m_impl->mShowRunningStatus ? ui->goUpLabel->setPixmap(m_impl->mGoupGreenPixmap) : ui->goUpLabel->setPixmap(m_impl->mGoupGrayPixmap);
            }
            break;
            case TaskStepStatus::Finish:
            {
                ui->goUpLabel->setPixmap(m_impl->mGoupGreenPixmap);
            }
            break;
            case TaskStepStatus::Error:
            case TaskStepStatus::Exception:
            {
                ui->goUpLabel->setPixmap(m_impl->mGoupRedPixmap);
            }
            break;
            default: break;
            }
        }
        break;
        case TaskStep::TRACK_END:        // 跟踪结束
        case TaskStep::POST_PROCESSING:  // 事后处理
        {
            switch (status)
            {
            case TaskStepStatus::NoStart:
            case TaskStepStatus::Continue:
            {
                ui->taskEndLabel->setPixmap(m_impl->mTaskEndGrayPixmap);
            }
            break;
            case TaskStepStatus::Running:
            {
                m_impl->mShowRunningStatus ? ui->taskEndLabel->setPixmap(m_impl->mTaskEndGreenPixmap)
                                           : ui->taskEndLabel->setPixmap(m_impl->mTaskEndGrayPixmap);
            }
            break;
            case TaskStepStatus::Finish:
            {
                ui->taskEndLabel->setPixmap(m_impl->mTaskEndGreenPixmap);
            }
            break;
            case TaskStepStatus::Error:
            case TaskStepStatus::Exception:
            {
                ui->taskEndLabel->setPixmap(m_impl->mTaskEndRedPixmap);
            }
            break;
            default: break;
            }
        }
        break;
        }
    }

    m_impl->mShowRunningStatus = !m_impl->mShowRunningStatus;
}

void PlanListWidget::slotUpdateTaskTime(const DeviceWorkTask& deviceWorkTask)
{
    DeviceWorkTaskTarget mainTarget;
    // 有主跟目标代表任务有待执行的任务
    if (deviceWorkTask.getMainTarget(mainTarget))
    {
        ui->taskPreparationTimeLable->setText(mainTarget.m_task_ready_start_time.toString(DATETIME_DISPLAY_FORMAT2));
        ui->trackStartTimeLabel->setText(mainTarget.m_track_start_time.toString(DATETIME_DISPLAY_FORMAT2));
        ui->trackEndTimeLabel->setText(mainTarget.m_track_end_time.toString(DATETIME_DISPLAY_FORMAT2));
        ui->taskEndTimeLabel->setText(mainTarget.m_task_end_time.toString(DATETIME_DISPLAY_FORMAT2));

        QDateTime currentDateTime = GlobalData::currentDateTime();

        QString countDownText;
        // 该任务已经在执行，倒计时完成
        if (currentDateTime > mainTarget.m_task_ready_start_time)
        {
            countDownText = QString("00:00:00");
        }
        else
        {
            auto secs = currentDateTime.secsTo(mainTarget.m_task_ready_start_time);

            int mi = 60;
            int hh = mi * 60;
            int dd = hh * 24;
            long day = secs / dd;
            long hour = (secs - day * dd) / hh;
            long minute = (secs - day * dd - hour * hh) / mi;
            long secend = (secs - day * dd - hour * hh - minute * mi);

            if (day > 0)
            {
                countDownText = QString("%0天%1:%2:%3")
                                    .arg(day, 2, 10, QLatin1Char('0'))
                                    .arg(hour, 2, 10, QLatin1Char('0'))
                                    .arg(minute, 2, 10, QLatin1Char('0'))
                                    .arg(secend, 2, 10, QLatin1Char('0'));
            }
            else
            {
                countDownText =
                    QString("%1:%2:%3").arg(hour, 2, 10, QLatin1Char('0')).arg(minute, 2, 10, QLatin1Char('0')).arg(secend, 2, 10, QLatin1Char('0'));
            }
        }

        ui->taskCountDownLabel->setText(countDownText);
    }
    else
    {
        ui->taskPreparationTimeLable->setText("0000年00月00日 00:00:00");
        ui->trackStartTimeLabel->setText("0000年00月00日 00:00:00");
        ui->trackEndTimeLabel->setText("0000年00月00日 00:00:00");
        ui->taskEndTimeLabel->setText("0000年00月00日 00:00:00");

        ui->taskCountDownLabel->setText("--:--:--");
    }
}

void PlanListWidget::slotShowSwitchAutoRunDialog(QAbstractButton* btn)
{
    bool isAutoRun = false;
    QString text;
    if (ui->autoRunRadioBtn == btn)
    {
        text = QString("自动化运行任务");
        isAutoRun = true;
    }
    else if (ui->manualRunRadioBtn == btn)
    {
        text = QString("手动运行任务");
        isAutoRun = false;
    }
    else
    {
        return;
    }

    // 和原来的一样不进行切换
    auto oldFlag = GlobalData::getAutoRunTaskFlag();
    if (oldFlag == isAutoRun)
    {
        return;
    }

    if (QMessageBox::information(this, "提示", QString("您确定要切换为%1吗?").arg(text), QString("确定"), QString("取消")) != 0)
    {
        return;
    }

    // 设置任务运行控制类型
    GlobalData::setAutoRunTaskFlag(isAutoRun);
}

void PlanListWidget::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event);

    // 是否进行自动刷新数据
    if (m_impl->isAutoRefresh)
    {
        timerOutRefresh();
    }
    // 获取任务运行控制类型
    if (GlobalData::getAutoRunTaskFlag())
    {
        ui->autoRunRadioBtn->setChecked(true);
    }
    else
    {
        ui->manualRunRadioBtn->setChecked(true);
    }
}
void PlanListWidget::projectTableSelectChange(const QModelIndex& index) { m_impl->deviceIndex = index.row(); }
void PlanListWidget::dtTableSelectChange(const QModelIndex& index) { m_impl->dtIndex = index.row(); }
