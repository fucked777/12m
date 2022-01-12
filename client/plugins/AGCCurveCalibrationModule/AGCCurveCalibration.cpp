#include "AGCCurveCalibration.h"
#include "AGCAndSBFModel.h"
#include "AGCSerialize.h"
#include "BusinessMacroCommon.h"
#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "HistryDataModel.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "SatelliteManagementSerialize.h"
#include "ServiceCheck.h"
#include "Utility.h"
#include "SubscriberHelper.h"
#include "ui_AGCCurveCalibration.h"
#include <QMenu>
#include <QMessageBox>
#include <QShowEvent>

class AGCCurveCalibrationImpl
{
public:
    AGCAndSBFModel realtimeMode;
    AGCAndSBFModel histryPointMode;
    HistryDataModel histryDataMode;
    QMenu* pointMenu{ nullptr }; /* 实时/历史数据右键菜单 */

    SatelliteManagementDataMap satelliteManagementDataMap;
    SatelliteManagementData curSatelliteManagementData;
    cppmicroservices::BundleContext context;

    bool refreshUI { true };// 是否刷新界面
    // double down{ 0.0 };
    // double up{ 0.0 };

    cpp_redis::subscriber logSuber;

    Optional<AGCCalibrationItem> getInteractiveItem(Ui::AGCCurveCalibration* ui);
    static void enableChange(Ui::AGCCurveCalibration* ui, bool enable);
    static void enableHistoryControlBtn(Ui::AGCCurveCalibration* ui, bool enable);
    static void enableRealControlBtn(Ui::AGCCurveCalibration* ui, bool enable);
};

/* 获取交互界面的数据 */
Optional<AGCCalibrationItem> AGCCurveCalibrationImpl::getInteractiveItem(Ui::AGCCurveCalibration* ui)
{
    using ResType = Optional<AGCCalibrationItem>;
    AGCCalibrationItem item;
    //赋值变量初始化
    item.projectCode = ui->projectCode->currentText();
    item.taskCode = curSatelliteManagementData.m_satelliteCode;
    item.taskID = curSatelliteManagementData.m_satelliteIdentification;

    if (item.taskCode.isEmpty())
    {
        return ResType(ErrorCode::DataNotEmpty, "任务代号不能为空");
    }
    if (ui->workMode->currentText().isEmpty())
    {
        return ResType(ErrorCode::DataNotEmpty, "工作模式不能为空");
    }
    item.workMode = SystemWorkMode(ui->workMode->currentData().toInt());

    auto data = ui->dotDrequency->currentText();
    if (data.isEmpty())
    {
        return ResType(ErrorCode::DataNotEmpty, "工作点频不能为空");
    }
    item.dotDrequencyNum = data.toUInt();

    /* 20210816 wp??
     * 设备组合号这里变为保留的数据
     * 在下发给基带或者ACU的时候再计算
     */
    item.equipComb = 0x00;
    /* 下行频率 */
    // item.downFreq = down;

    // if (qFuzzyIsNull(down))
    //{
    //    return ResType(ErrorCode::DataNotEmpty, "当前参数宏数据解析错误");
    //}

    return ResType(item);
}
void AGCCurveCalibrationImpl::enableHistoryControlBtn(Ui::AGCCurveCalibration* ui, bool enable)
{
    ui->history_addPoint->setEnabled(enable);
    ui->history_clearBtn->setEnabled(enable);
    ui->history_enableEdit->setEnabled(enable);
    ui->history_removePoint->setEnabled(enable);
    ui->history_saveBtn->setEnabled(enable);
}
void AGCCurveCalibrationImpl::enableRealControlBtn(Ui::AGCCurveCalibration* ui, bool enable)
{
    ui->real_addPoint->setEnabled(enable);
    ui->real_clearBtn->setEnabled(enable);
    ui->real_enableEdit->setEnabled(enable);
    ui->real_removePoint->setEnabled(enable);
    ui->real_saveBtn->setEnabled(enable);
}

void AGCCurveCalibrationImpl::enableChange(Ui::AGCCurveCalibration* ui, bool enable)
{
    ui->beginBtn->setEnabled(enable);
    ui->endBtn->setEnabled(!enable);
    ui->sendMACB->setEnabled(enable);
    ui->sendACU->setEnabled(enable);

    ui->real_saveBtn->setEnabled(enable);
    ui->real_clearBtn->setEnabled(enable);
    ui->real_addPoint->setEnabled(enable);
    ui->real_enableEdit->setEnabled(enable);
    ui->real_removePoint->setEnabled(enable);

    ui->history_saveBtn->setEnabled(enable);
    ui->history_clearBtn->setEnabled(enable);
    ui->history_addPoint->setEnabled(enable);
    ui->history_enableEdit->setEnabled(enable);
    ui->history_removePoint->setEnabled(enable);

    ui->tabWidget->tabBar()->setEnabled(enable);
    if (!enable)
    {
        ui->tabWidget->setCurrentIndex(0);
    }
}

AGCCurveCalibration::AGCCurveCalibration(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::AGCCurveCalibration)
    , m_impl(new AGCCurveCalibrationImpl)
{
    m_impl->context = context;
    ui->setupUi(this);
    // 初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    AGCCurveCalibrationImpl::enableChange(ui, true);
    QString qssPath = QApplication::applicationDirPath() + "/resources/newqss/common.qss";
    QssCommonHelper::setWidgetStyle(this, qssPath);

    ui->realtimeTableView->setModel(&(m_impl->realtimeMode));
    ui->tableViewDataPoint->setModel(&(m_impl->histryPointMode));
    ui->tableViewHistryData->setModel(&(m_impl->histryDataMode));

    Utility::tableViewInit(ui->realtimeTableView, true);
    Utility::tableViewInit(ui->tableViewDataPoint, true);
    Utility::tableViewInit(ui->tableViewHistryData, true);
    ui->realtimeTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->logs->setContextMenuPolicy(Qt::CustomContextMenu);

    auto curDate = GlobalData::currentDate();
    ui->beginDate->setDate(curDate.addDays(-7));
    ui->endDate->setDate(curDate);
    // ui->agcWidget->setMinimumSize(715, 600);

    setEnableEditAGCImpl(0);
    setEnableEditAGCImpl(1);
    m_impl->enableHistoryControlBtn(ui, m_impl->histryPointMode.isValid());
    m_impl->enableRealControlBtn(ui, m_impl->realtimeMode.isValid());

    connect(ui->taskCode, &QComboBox::currentTextChanged, this, &AGCCurveCalibration::taskCodeChanged);
    connect(ui->workMode, &QComboBox::currentTextChanged, this, &AGCCurveCalibration::workModeChanged);
    // connect(ui->dotDrequency, &QComboBox::currentTextChanged, this, &AGCCurveCalibration::dotDrequencyChanged);

    connect(ui->beginBtn, &QAbstractButton::clicked, this, &AGCCurveCalibration::startAGCData);
    connect(ui->endBtn, &QAbstractButton::clicked, this, &AGCCurveCalibration::endAGCData);
    connect(ui->sendMACB, &QAbstractButton::clicked, this, &AGCCurveCalibration::sendToMACB);
    connect(ui->sendACU, &QAbstractButton::clicked, this, &AGCCurveCalibration::sendToACU);
    connect(ui->search, &QAbstractButton::clicked, this, &AGCCurveCalibration::selectAGCData);
    connect(ui->finishing, &QAbstractButton::clicked, this, &AGCCurveCalibration::finishingItem);
    connect(ui->remove, &QAbstractButton::clicked, this, &AGCCurveCalibration::deleteAGCData);
    connect(ui->manual, &QAbstractButton::clicked, this, &AGCCurveCalibration::manualCreateAGC);
    connect(ui->tableViewHistryData, &QTableView::clicked, this, &AGCCurveCalibration::historyShowPoint);

    connect(ui->real_saveBtn, &QAbstractButton::clicked, this, &AGCCurveCalibration::saveAGCData);
    connect(ui->real_clearBtn, &QAbstractButton::clicked, this, &AGCCurveCalibration::clearPoint);
    connect(ui->real_enableEdit, &QAbstractButton::clicked, this, &AGCCurveCalibration::setEnableEditAGC);
    connect(ui->real_removePoint, &QAbstractButton::clicked, this, &AGCCurveCalibration::removePoint);
    connect(ui->real_addPoint, &QAbstractButton::clicked, this, &AGCCurveCalibration::addPoint);
    connect(ui->history_saveBtn, &QAbstractButton::clicked, this, &AGCCurveCalibration::saveAGCData);
    connect(ui->history_clearBtn, &QAbstractButton::clicked, this, &AGCCurveCalibration::clearPoint);
    connect(ui->history_enableEdit, &QAbstractButton::clicked, this, &AGCCurveCalibration::setEnableEditAGC);
    connect(ui->history_removePoint, &QAbstractButton::clicked, this, &AGCCurveCalibration::removePoint);
    connect(ui->history_addPoint, &QAbstractButton::clicked, this, &AGCCurveCalibration::addPoint);

    connect(&(m_impl->realtimeMode), &AGCAndSBFModel::sg_dataChange, [=]() { ui->agcWidget->resetData(m_impl->realtimeMode.value()); });
    connect(&(m_impl->histryPointMode), &AGCAndSBFModel::sg_dataChange, [=]() { ui->agcWidget->resetData(m_impl->histryPointMode.value()); });
    connect(&(m_impl->realtimeMode), &AGCAndSBFModel::sg_dataIsValid,
            [=](bool enable) { AGCCurveCalibrationImpl::enableRealControlBtn(ui, enable); });
    connect(&(m_impl->histryPointMode), &AGCAndSBFModel::sg_dataIsValid,
            [=](bool enable) { AGCCurveCalibrationImpl::enableHistoryControlBtn(ui, enable); });

    connect(this, &AGCCurveCalibration::sg_recvLogs, this, &AGCCurveCalibration::recvLogs);

    m_impl->pointMenu = new QMenu(this);
    m_impl->pointMenu->addAction(QString("清空"), this, &AGCCurveCalibration::clearPoint);
    m_impl->pointMenu->addAction(QString("增加"), this, &AGCCurveCalibration::addPoint);
    m_impl->pointMenu->addAction(QString("删除"), this, &AGCCurveCalibration::removePoint);

    connect(ui->realtimeTableView, &QTableView::customContextMenuRequested, this, [=]() {
        QPoint pt = ui->realtimeTableView->mapFromGlobal(QCursor::pos());
        int height = ui->realtimeTableView->horizontalHeader()->height();
        QPoint pt2(0, height);
        pt -= pt2;
        auto index = ui->realtimeTableView->indexAt(pt);
        bool flag = m_impl->realtimeMode.judgeData(index);
        if (ui->realtimeTableView->currentIndex().isValid() && flag)
        {
            m_impl->pointMenu->exec(QCursor::pos());
        }
    });

    connect(ui->logs, &QTextEdit::customContextMenuRequested, this, [=]() {
        QMenu logMenu(ui->logs);
        logMenu.addAction(QString("清空"), ui->logs, &QTextEdit::clear);
        logMenu.exec(QCursor::pos());
    });

    connect(SubscriberHelper::getInstance(),&SubscriberHelper::sg_statusChange,this,&AGCCurveCalibration::masterStatusChange);
}

AGCCurveCalibration::~AGCCurveCalibration() { delete ui; }

void AGCCurveCalibration::closeEvent(QCloseEvent* event)
{
    QWidget::closeEvent(event);
    m_impl->refreshUI = event->isAccepted();
}

void AGCCurveCalibration::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if(m_impl->refreshUI)
    {
        reloadUIParam();
    }
    m_impl->enableHistoryControlBtn(ui, m_impl->histryPointMode.isValid());
    m_impl->enableRealControlBtn(ui, m_impl->realtimeMode.isValid());
}
void AGCCurveCalibration::reloadUIParam()
{
    ui->projectCode->clear();
    ui->taskCode->clear();
    ui->findTaskCode->clear();
    ui->findTaskCode->addItem("所有");

    ui->projectCode->addItem(ExtendedConfig::curTKID());
    /* 获取卫星管理数据 */
    if (!GlobalData::getSatelliteManagementData(m_impl->satelliteManagementDataMap))
    {
        m_impl->refreshUI = true;
        return;
    }
    for (auto iter = m_impl->satelliteManagementDataMap.begin(); iter != m_impl->satelliteManagementDataMap.end(); ++iter)
    {
        auto key = iter.key();
        ui->taskCode->addItem(iter->m_satelliteCode, key);
        ui->findTaskCode->addItem(iter->m_satelliteCode, key);
    }
    m_impl->refreshUI = false;
}

void AGCCurveCalibration::taskCodeChanged()
{
    ui->workMode->clear();

    auto satelliteCode = ui->taskCode->currentData().toString();
    auto find = m_impl->satelliteManagementDataMap.find(satelliteCode);
    if (find == m_impl->satelliteManagementDataMap.end())
    {
        return;
    }
    m_impl->curSatelliteManagementData = find.value();
    /* 刷新工作模式 */
    auto workModeMap = m_impl->curSatelliteManagementData.getworkMode();
    for (auto iter = workModeMap.begin(); iter != workModeMap.end(); ++iter)
    {
        auto key = iter.key();
        if (!SystemWorkModeHelper::isMeasureContrlWorkMode(SystemWorkMode(key)))
        {
            continue;
        }
        auto tempString = SystemWorkModeHelper::systemWorkModeToDesc(SystemWorkMode(key));
        ui->workMode->addItem(tempString, key);
    }
}
void AGCCurveCalibration::workModeChanged()
{
    ui->dotDrequency->clear();
    auto tempWorkMode = (SystemWorkMode)ui->workMode->currentData().toInt();
    auto dpNum = m_impl->curSatelliteManagementData.getDpNumByWorkMode(tempWorkMode);
    for (int i = 0; i < dpNum; ++i)
    {
        ui->dotDrequency->addItem(QString::number(i + 1), i + 1);
    }
}

// void AGCCurveCalibration::dotDrequencyChanged()
//{
//    m_impl->down = 0.0;
//    m_impl->up = 0.0;

//    auto tempDPData = ui->dotDrequency->currentData();
//    auto tempWorkMode = (SystemWorkMode)ui->workMode->currentData().toInt();
//    auto tempTaskCodeVar = ui->taskCode->currentData();

//    QVariant up;
//    QVariant down;
//    if (MacroCommon::getFrequency(tempTaskCodeVar.toString(), tempWorkMode, tempDPData.toInt(), up, down))
//    {
//        m_impl->down = down.toDouble();
//        m_impl->up = up.toDouble();
//    }
//}
void AGCCurveCalibration::clearPoint()
{
    ui->agcWidget->resetData(AGCResultList());

    if (ui->tabWidget->currentIndex() == 0)
    {
        m_impl->realtimeMode.resetItemValue();
    }
    else
    {
        m_impl->histryPointMode.resetItemValue();
    }
}
void AGCCurveCalibration::clearAll()
{
    clearPoint();
    ui->logs->clear();
}

void AGCCurveCalibration::selectAGCData()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }
    /* 20210617 wp??
     * 这里认为查询不需要这么精确 精确到秒没什么意义 你基本都不会去动后面的秒的到天足够了
     * 接口是完整的包括服务器的都是OK的 如果实在是需要精确到秒直接界面改成datetime就OK
     */
    AGCResultQuery agcResultQuery;
    agcResultQuery.beginTime = ui->beginDate->dateTime();
    agcResultQuery.endTime = ui->endDate->dateTime();
    agcResultQuery.taskCode = ui->findTaskCode->currentData().toString();

    if (agcResultQuery.beginTime > agcResultQuery.endTime)
    {
        QMessageBox::critical(this, "提示", "开始时间不能晚于结束时间", "确定");
        return;
    }
    /* 这里有一个查询所有,不能根据currentData查询 */
    if (ui->findTaskCode->currentText().isEmpty())
    {
        QMessageBox::critical(this, "提示", "任务代号不能为空", "确定");
        return;
    }

    QByteArray array;
    array << agcResultQuery;
    emit sg_selectAGCData(array);
}
void AGCCurveCalibration::selectAGCDataACK(const QByteArray& data)
{
    auto result = Optional<AGCCalibrationItemList>::emptyOptional();
    data >> result;
    if (!result)
    {
        QMessageBox::information(this, "查询历史数据失败", result.msg(), QString("确定"));
        return;
    }
    m_impl->histryDataMode.updateData(result.value());
}

void AGCCurveCalibration::saveAGCData()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    AGCCalibrationItem item;
    if (ui->tabWidget->currentIndex() == 0)
    {
        item = m_impl->realtimeMode.itemValue();
    }
    else
    {
        item = m_impl->histryPointMode.itemValue();
    }

    if (item.result.isEmpty())
    {
        QMessageBox::information(this, "提示", "当前无可保存的数据", "确认");
        return;
    }
    if (item.taskCode.isEmpty())
    {
        QMessageBox::information(this, "当前数据无效", "手动创建需要在左下方点击新建", "确认");
        return;
    }
    // item.createTime = GlobalData::currentDateTime();
    // item.itemGUID = QString("%1-%2").arg(item.createTime.toString("yyyyMMdd"), QUuid::createUuid().toString());

    QByteArray array;
    array << item;
    emit sg_saveAGCData(array);
}
void AGCCurveCalibration::saveAGCDataACK(const QByteArray& data)
{
    auto result = Optional<AGCCalibrationItem>::emptyOptional();
    data >> result;
    if (!result)
    {
        QMessageBox::information(this, "保存曲线数据失败", result.msg(), QString("确定"));
        return;
    }

    QMessageBox::information(this, "提示", "保存曲线数据成功", QString("确定"));
    m_impl->realtimeMode.setItemValue(result.value());
}

void AGCCurveCalibration::startAGCData()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    m_impl->realtimeMode.resetItemValue(); /* 每次都要先清空这里的数据 */

    /* 获取界面的数据 */
    auto result = m_impl->getInteractiveItem(ui);
    if (!result)
    {
        QMessageBox::information(this, "提示", result.msg(), QString("确定"));
        return;
    }

    AGCCurveCalibrationImpl::enableChange(ui, false);
    clearAll();
    registerSuber();
    m_impl->realtimeMode.setItemValue(result.value());

    QByteArray array;
    array << result.value();
    emit sg_startAGCData(array);
}
void AGCCurveCalibration::startAGCDataACK(const QByteArray& bak)
{
    auto data = bak;
    AGCCalibrationProcessACK res;
    data >> res;

    switch (res.result)
    {
    case AGCCalibrationStatus::End: /* 标定结束 */
    {
        QMessageBox::information(this, "提示", "测试结束", "确认");
        AGCCurveCalibrationImpl::enableChange(ui, true);
        m_impl->realtimeMode.setItemValue(res.item);
        unregisterSuber();
        break;
    }
    case AGCCalibrationStatus::Failed: /* 失败 */
    {
        QMessageBox::information(this, "测试结束", res.msg.isEmpty() ? "测试错误" : res.msg, "确认");
        AGCCurveCalibrationImpl::enableChange(ui, true);
        unregisterSuber();
        break;
    }
    case AGCCalibrationStatus::Busy: /* 忙(正在执行) */
    {
        QMessageBox::information(this, "提示", "当前已经有测试在进行", "确认");
        AGCCurveCalibrationImpl::enableChange(ui, true);
        unregisterSuber();
        break;
    }
    case AGCCalibrationStatus::Info: /* 过程状态信息 */
    {
        m_impl->realtimeMode.addItem(res.value);
        ui->agcWidget->addData(res.value);
        break;
    }
    }
}

void AGCCurveCalibration::endAGCData()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }
    AGCCurveCalibrationImpl::enableChange(ui, true);
    unregisterSuber();

    emit sg_endAGCData(QByteArray());
}
void AGCCurveCalibration::endAGCDataACK(const QByteArray&) {}

void AGCCurveCalibration::deleteAGCData()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    AGCCalibrationItem item;
    if (!m_impl->histryDataMode.agcItemData(ui->tableViewHistryData->currentIndex(), item))
    {
        QMessageBox::warning(this, QString("提示"), "请选择要删除的项", QString("确认"));
        return;
    }

    clearPoint();
    QByteArray array;
    array << item;
    emit sg_deleteAGCData(array);
}
void AGCCurveCalibration::deleteAGCDataACK(const QByteArray& data)
{
    auto result = Optional<AGCCalibrationItem>::emptyOptional();
    data >> result;
    if (!result)
    {
        QMessageBox::information(this, "删除曲线数据失败", result.msg(), QString("确定"));
        return;
    }

    m_impl->histryDataMode.removeValue(result.value());
    QMessageBox::information(this, "提示", "删除曲线数据成功", QString("确定"));
}

void AGCCurveCalibration::finishingItem()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    ClearAGCCalibrationHistry clearAGCCalibrationHistry;
    QByteArray array;
    array << clearAGCCalibrationHistry;
    emit sg_finishingItem(array);
}
void AGCCurveCalibration::finishingItemACK(const QByteArray& data)
{
    auto result = OptionalNotValue::emptyOptional();
    data >> result;
    if (result)
    {
        QMessageBox::information(this, QString("提示"), "清除成功", QString("确认"));
    }
    else
    {
        QMessageBox::information(this, QString("提示"), "清除失败", QString("确认"));
    }
}
void AGCCurveCalibration::manualCreateAGC()
{
    m_impl->realtimeMode.resetItemValue(); /* 每次都要先清空这里的数据 */
    /* 获取界面的数据 */
    auto result = m_impl->getInteractiveItem(ui);
    if (!result)
    {
        QMessageBox::information(this, "提示", result.msg(), QString("确定"));
        return;
    }

    ui->tabWidget->setCurrentIndex(0);
    clearAll();
    m_impl->realtimeMode.setItemValue(result.value());
    QMessageBox::information(this, "提示", "数据创建完成,请增加AGC点数后保存即可", "确定");
}
void AGCCurveCalibration::sendToMACB()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    /* 获取当前的点数 */
    AGCCalibrationItem item;
    if (ui->tabWidget->currentIndex() == 0)
    {
        /* 获取界面的数据 */
        auto result = m_impl->getInteractiveItem(ui);
        if (!result)
        {
            QMessageBox::information(this, "提示", result.msg(), QString("确定"));
            return;
        }
        item = result.value();
        item.result = m_impl->realtimeMode.value();
    }
    else
    {
        item = m_impl->histryPointMode.itemValue();
    }

    if (item.result.isEmpty())
    {
        QMessageBox::warning(this, "提示", "当前无可发送的数据", "确认");
        return;
    }
    if (item.taskCode.isEmpty())
    {
        QMessageBox::warning(this, "当前装订数据错误", "任务代号为空", "确认");
        return;
    }
    if (!SystemWorkModeHelper::isMeasureContrlWorkMode(item.workMode))
    {
        QMessageBox::information(this, "提示", "只有测控模式的才能向测控基带装订", "确认");
        return;
    }

    QByteArray array;
    array << item;
    emit sg_sendToMACB(array);
}
void AGCCurveCalibration::sendToMACBACK(const QByteArray& data)
{
    auto result = OptionalNotValue::emptyOptional();
    data >> result;
    if (result)
    {
        QMessageBox::information(this, "提示", "装订完成", "确定");
        return;
    }
    QMessageBox::warning(this, "装订错误", result.msg(), "确定");
}

void AGCCurveCalibration::sendToACU()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }
    /* 获取当前的点数 */
    AGCCalibrationItem item;
    if (ui->tabWidget->currentIndex() == 0)
    {
        item = m_impl->realtimeMode.itemValue();
    }
    else
    {
        item = m_impl->histryPointMode.itemValue();
    }
    if (item.result.isEmpty())
    {
        QMessageBox::warning(this, "提示", "当前无可发送的数据", "确认");
        return;
    }
    if (item.taskCode.isEmpty())
    {
        QMessageBox::warning(this, "当前装订数据错误", "任务代号为空", "确认");
        return;
    }

    QByteArray array;
    array << item;
    emit sg_sendToACU(array);
}
void AGCCurveCalibration::sendToACUACK(const QByteArray& data)
{
    auto result = OptionalNotValue::emptyOptional();
    data >> result;
    if (result)
    {
        QMessageBox::information(this, "提示", "装订完成", "确定");
        return;
    }
    QMessageBox::warning(this, "装订错误", result.msg(), "确定");
}

void AGCCurveCalibration::recvLogs(const QString& syslog) { ui->logs->append(syslog); }
void AGCCurveCalibration::unregisterSuber() { m_impl->logSuber.disconnect(); }
void AGCCurveCalibration::masterStatusChange()
{
    if (m_impl->logSuber.is_connected())
    {
        m_impl->logSuber.disconnect();
        registerSuber();
    }
}
void AGCCurveCalibration::registerSuber()
{
    if (m_impl->logSuber.is_connected())
    {
        return;
    }
    if (!RedisHelper::getInstance().getSubscriber(m_impl->logSuber))
    {
        return;
    }
    m_impl->logSuber.subscribe("AGC", [=](const std::string& /*chanel*/, const std::string& msg) {
        QString qmsg = QString::fromStdString(msg);
        emit sg_recvLogs(qmsg);
    });
    m_impl->logSuber.commit();
}
void AGCCurveCalibration::setEnableEditAGC() { setEnableEditAGCImpl(ui->tabWidget->currentIndex()); }
void AGCCurveCalibration::setEnableEditAGCImpl(int index)
{
    AGCAndSBFModel* model = nullptr;
    QPushButton* enableEdit = nullptr;
    if (index == 0)
    {
        model = &m_impl->realtimeMode;
        enableEdit = ui->real_enableEdit;
    }
    else
    {
        model = &m_impl->histryPointMode;
        enableEdit = ui->history_enableEdit;
    }

    auto isEnableEdit = model->enableEdit();
    isEnableEdit = !isEnableEdit;
    model->setEnableEdit(isEnableEdit);

    /* 20200826 wp?? 这里采用的是显示当前的状态
     * 如果需要改为显示下次操作的状态把这里的两个文本换个位置就行
     */
    enableEdit->setText(isEnableEdit ? "启用编辑" : "禁用编辑");
}
void AGCCurveCalibration::removePoint()
{
    AGCAndSBFModel* model = nullptr;
    QTableView* tableView = nullptr;
    if (ui->tabWidget->currentIndex() == 0)
    {
        model = &m_impl->realtimeMode;
        tableView = ui->realtimeTableView;
    }
    else
    {
        model = &m_impl->histryPointMode;
        tableView = ui->tableViewDataPoint;
    }

    /* 删除点，如果未选中则提示错误 */
    auto result = model->removeItem(tableView->currentIndex());
    if (!result)
    {
        QMessageBox::warning(this, "提示", "请选择需要删除的点", "确定");
        return;
    }
    ui->agcWidget->resetData(model->value());
}
void AGCCurveCalibration::addPoint()
{
    /* 增加点有效就在点上面增加 否则就在末尾添加 */
    AGCAndSBFModel* model = nullptr;
    QTableView* tableView = nullptr;
    if (ui->tabWidget->currentIndex() == 0)
    {
        model = &m_impl->realtimeMode;
        tableView = ui->realtimeTableView;
    }
    else
    {
        model = &m_impl->histryPointMode;
        tableView = ui->tableViewDataPoint;
    }

    model->addItem(tableView->currentIndex());
    ui->agcWidget->resetData(model->value());
}
void AGCCurveCalibration::historyShowPoint(const QModelIndex& index)
{
    AGCCalibrationItem item;
    auto result = m_impl->histryDataMode.agcItemData(index, item);
    if (!result)
    {
        clearPoint();
        return;
    }
    m_impl->histryPointMode.setItemValue(item);
    ui->agcWidget->resetData(m_impl->histryPointMode.value());

    /* 刷新左侧界面显示 */
//    item.taskCode = curSatelliteManagementData.m_satelliteCode;
//    item.taskID = curSatelliteManagementData.m_satelliteIdentification;

//    if (item.taskCode.isEmpty())
//    {
//        return ResType(ErrorCode::DataNotEmpty, "任务代号不能为空");
//    }
//    if (ui->workMode->currentText().isEmpty())
//    {
//        return ResType(ErrorCode::DataNotEmpty, "工作模式不能为空");
//    }
//    item.workMode = SystemWorkMode(ui->workMode->currentData().toInt());

//    auto data = ui->dotDrequency->currentText();
//    if (data.isEmpty())
//    {
//        return ResType(ErrorCode::DataNotEmpty, "工作点频不能为空");
//    }
//    item.dotDrequencyNum = data.toUInt();
}
