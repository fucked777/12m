#include "DistanceCalibration.h"
#include "BusinessMacroCommon.h"
#include "CppMicroServicesUtility.h"
#include "DistanceCalibrationModel.h"
#include "DistanceCalibrationSerialize.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "ParamMacroMessageSerialize.h"
#include "PlatformInterface.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "SatelliteManagementSerialize.h"
#include "ServiceCheck.h"
#include "SystemWorkMode.h"
#include "ui_DistanceCalibration.h"
#include <QButtonGroup>
#include <QMessageBox>

class DistanceCalibrationImpl
{
public:
    /* 通道单选的分组,这里的策略是无论用不用的上都是正常初始化 */
    QButtonGroup* channelGroup{ nullptr };
    /* 所有的卫星数据 */
    SatelliteManagementDataMap satelliteManagementDataMap;
    SatelliteManagementData curSatelliteManagementData;
    cpp_redis::subscriber logSubscriber;
    DistanceCalibrationModel* model{ nullptr };
    int timerID{ -1 };
};

DistanceCalibration::DistanceCalibration(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DistanceCalibration)
    , m_impl(new DistanceCalibrationImpl)
{
    qRegisterMetaType<DistanceCalibrationItem>("DistanceCalibrationItem&");
    qRegisterMetaType<DistanceCalibrationItem>("DistanceCalibrationItem");
    qRegisterMetaType<DistanceCalibrationItem>("const DistanceCalibrationItem&");

    Q_UNUSED(context)
    ui->setupUi(this);
    // 初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
    init();
}

DistanceCalibration::~DistanceCalibration()
{
    stop();
    delete ui;
    delete m_impl;
}
void DistanceCalibration::start() { Utility::startTimer(this, m_impl->timerID, 1000); }
void DistanceCalibration::stop() { Utility::killTimer(this, m_impl->timerID); }
bool DistanceCalibration::isStart() { return m_impl->timerID != -1; }
void DistanceCalibration::init()
{
    /* 首先隐藏掉,通道选项,只有在24的一体化上有这个 */
    ui->channelGroupBox->setVisible(false);
    m_impl->channelGroup = new QButtonGroup(this);
    /*
     * 1 通道1(一体化)
     * 2 通道2(扩频）
     */
    m_impl->channelGroup->addButton(ui->channel_yth, 1);
    m_impl->channelGroup->addButton(ui->channel_kp, 2);

    auto projectName = ExtendedConfig::curTKID();
    ui->projectCode->addItem(projectName, projectName);
    ui->projectCodeSearch->addItem(projectName, projectName);
    controlButtonEnable(true);

    m_impl->model = new DistanceCalibrationModel(this);

    auto& tableView = ui->tableView;
    ui->tableView->setModel(m_impl->model);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->horizontalHeader()->setHighlightSections(false);
    tableView->verticalHeader()->setVisible(false);
    tableView->setFrameShape(QFrame::NoFrame);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->taskCode, &QComboBox::currentTextChanged, this, &DistanceCalibration::taskCodeChange);
    connect(ui->workMode, &QComboBox::currentTextChanged, this, &DistanceCalibration::workModeChange);
    connect(ui->dotDrequency, &QComboBox::currentTextChanged, this, &DistanceCalibration::dpChange);

    connect(ui->startDC, &QPushButton::clicked, this, &DistanceCalibration::startDistanceCalibration);
    connect(ui->endDC, &QPushButton::clicked, this, &DistanceCalibration::stopDistanceCalibration);
    connect(ui->clearHistry, &QPushButton::clicked, this, &DistanceCalibration::clearHistry);
    connect(ui->remove, &QPushButton::clicked, this, &DistanceCalibration::deleteDistanceCalibrationResult);
    connect(ui->query, &QPushButton::clicked, this, &DistanceCalibration::queryHistry);
    connect(this, &DistanceCalibration::sg_addLog, this, &DistanceCalibration::addLog);

    auto currentDate = GlobalData::currentDate();
    ui->beginDate->setDate(currentDate.addMonths(-1));
    ui->endDate->setDate(currentDate);
}
void DistanceCalibration::reloadUIData()
{
    /* 先清空ui的数据 */
    ui->taskCode->clear();
    ui->taskCodeSearch->clear();
    m_impl->satelliteManagementDataMap.clear();

    /* 获取卫星管理数据 */
    if (!GlobalData::getSatelliteManagementData(m_impl->satelliteManagementDataMap))
    {
        return;
    }
    for (auto iter = m_impl->satelliteManagementDataMap.begin(); iter != m_impl->satelliteManagementDataMap.end(); ++iter)
    {
        auto key = iter.key();
        /* 只添加有测控的 */
        for (auto modeIter = iter->m_workModeParamMap.begin(); modeIter != iter->m_workModeParamMap.end(); ++modeIter)
        {
            if (SystemWorkModeHelper::isMeasureContrlWorkMode(modeIter.key()))
            {
                ui->taskCode->addItem(iter->m_satelliteCode, key);
                ui->taskCodeSearch->addItem(iter->m_satelliteCode, key);
                break;
            }
        }
    }
}
void DistanceCalibration::taskCodeChange()
{
    ui->workMode->clear();

    auto satelliteCode = ui->taskCode->currentData().toString();
    auto find = m_impl->satelliteManagementDataMap.find(satelliteCode);
    if (find == m_impl->satelliteManagementDataMap.end())
    {
        m_impl->curSatelliteManagementData = SatelliteManagementData();
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
void DistanceCalibration::workModeChange()
{
    ui->dotDrequency->clear();
    auto tempWorkMode = (SystemWorkMode)ui->workMode->currentData().toInt();
    auto dpNum = m_impl->curSatelliteManagementData.getDpNumByWorkMode(tempWorkMode);
    for (int i = 0; i < dpNum; ++i)
    {
        ui->dotDrequency->addItem(QString::number(i + 1), i + 1);
    }
    /* 20210527 因为卫星管理模式变为单模式了 这里先隐藏看后续怎么说 wp?? 20210527 */
    /* 在一体化会有一个特别的参数 */
    //    if (tempWorkMode == STTC || tempWorkMode == Skuo2 || tempWorkMode == SKT)
    //    {
    //        ui->channelGroupBox->setVisible(false);
    //    }
    //    else
    //    {
    //        ui->channelGroupBox->setVisible(true);
    //    }
}
void DistanceCalibration::dpChange()
{
    ui->upFreq->clear();
    ui->downFreq->clear();
    auto tempDPData = ui->dotDrequency->currentData();
    auto tempModeData = ui->workMode->currentData();
    auto tempTaskCodeVar = ui->taskCode->currentData();

    QVariant up;
    QVariant down;
    if (MacroCommon::getFrequency(tempTaskCodeVar.toString(), SystemWorkMode(tempModeData.toInt()), tempDPData.toInt(), up, down))
    {
        ui->downFreq->setText(QString::number(down.toDouble(), 'f', 6));
        ui->upFreq->setText(QString::number(up.toDouble(), 'f', 6));
    }
}

void DistanceCalibration::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    reloadUIData();
}
void DistanceCalibration::timerEvent(QTimerEvent* /*event*/)
{
    if (m_impl->logSubscriber.is_connected())
    {
        return;
    }
    if (!RedisHelper::getInstance().getSubscriber(m_impl->logSubscriber))
    {
        return;
    }

    m_impl->logSubscriber.subscribe("DistanceCalibrationLog", [=](const std::string& /*chanel*/, const std::string& msg) {
        auto qmsg = QString::fromStdString(msg);
        emit sg_addLog(qmsg);
    });
    m_impl->logSubscriber.commit();
}
void DistanceCalibration::addLog(const QString& qmsg) { ui->logs->append(qmsg); }
void DistanceCalibration::controlButtonEnable(bool enable)
{
    ui->startDC->setEnabled(enable);
    ui->endDC->setEnabled(!enable);
}

void DistanceCalibration::startDistanceCalibration()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }
    ui->logs->setPlainText("");
    if (isStart())
    {
        return;
    }
    auto tempModeVar = ui->workMode->currentData();
    auto tempDPVar = ui->dotDrequency->currentData();
    auto tempTaskCodeVar = ui->taskCode->currentData();
    auto up = ui->upFreq->text();
    auto down = ui->downFreq->text();
    if (!tempModeVar.isValid() || !tempDPVar.isValid() || !tempTaskCodeVar.isValid())
    {
        QMessageBox::information(this, "提示", "参数不完整", "确定");
        return;
    }
    if (up.isEmpty() || down.isEmpty())
    {
        QMessageBox::information(this, "提示", "未找到参数宏数据", "确定");
        return;
    }
    auto tempWorkMode = SystemWorkMode(tempModeVar.toInt());

    QVariant polarization;
    if (!MacroCommon::getRecvPolarization(tempTaskCodeVar.toString(), SystemWorkMode(tempWorkMode), tempDPVar.toInt(), polarization))
    {
        QMessageBox::information(this, "提示", "当前极化获取失败", "确定");
        return;
    }

    /* 获取界面参数 */
    DistanceCalibrationItem param;
    param.projectCode = ui->projectCode->currentData().toString();

    if (tempWorkMode == STTC)
    {
        param.channel = 0;
    }
    else if (tempWorkMode == Skuo2 || tempWorkMode == SKT)
    {
        param.channel = 1;
    }
    else
    {
        param.channel = m_impl->channelGroup->checkedId();
    }
    param.downFreq = down.toDouble();
    param.uplinkFreq = up.toDouble();
    param.statisPoint = ui->statisPoint->value();
    param.taskIdent = m_impl->curSatelliteManagementData.m_satelliteIdentification;
    param.taskCode = m_impl->curSatelliteManagementData.m_satelliteCode;
    param.dpNum = tempDPVar.toInt();  // add by wjy
    param.correctValue = ui->correctValue->value();
    param.workMode = tempWorkMode;
    param.signalRot = polarization.toInt();
    /* 20210620 更新  设备组合号在服务端获取 */
    param.equipComb = 0x00;

    start();
    /* 禁用界面 */
    controlButtonEnable(false);

    QByteArray json;
    json << param;
    emit sg_startDistanceCalibration(json);
}
void DistanceCalibration::stopDistanceCalibration()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    stop();
    controlButtonEnable(true);
    emit sg_stopDistanceCalibration(QByteArray());
}
void DistanceCalibration::deleteDistanceCalibrationResult()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    DistanceCalibrationItem item;
    if (!m_impl->model->itemAt(item, ui->tableView->currentIndex()))
    {
        QMessageBox::information(this, "提示", "请选择需要删除的数据", "确定");
        return;
    }
    QByteArray json;
    json << item;
    emit sg_deleteDistanceCalibrationResult(json);
}
void DistanceCalibration::clearHistry()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    if (1 == QMessageBox::warning(this, "是否继续?", "此操作会清空90天之前的数据,不可恢复", "继续", "取消"))
    {
        return;
    }
    QByteArray json;
    ClearDistanceCalibrationHistry item;
    json << item;
    emit sg_clearHistry(json);
}
void DistanceCalibration::queryHistry()
{
    QueryDistanceCalibrationHistry query;
    /* 20210617 wp??
     * 这里认为查询不需要这么精确 精确到秒没什么意义 你基本都不会去动后面的秒的到天足够了
     * 接口是完整的包括服务器的都是OK的 如果实在是需要精确到秒直接界面改成datetime就OK
     */
    query.beginTime = ui->beginDate->dateTime();
    query.endTime = ui->endDate->dateTime();

    auto satelliteCode = ui->taskCodeSearch->currentData().toString();
    query.taskCode = satelliteCode;
    if (query.beginTime > query.endTime)
    {
        QMessageBox::critical(this, "提示", "开始时间不能晚于结束时间", "确定");
        return;
    }
    if (satelliteCode.isEmpty())
    {
        QMessageBox::critical(this, "提示", "任务代号不能为空", "确定");
        return;
    }

    m_impl->model->clear();
    QByteArray json;
    json << query;
    emit sg_queryHistry(json);
}

void DistanceCalibration::startDistanceCalibrationACK(const QByteArray& /*data*/)
{
    stop();
    ui->logs->append("校零结束");
    controlButtonEnable(true);
}
void DistanceCalibration::stopDistanceCalibrationACK(const QByteArray& /*data*/) {}
void DistanceCalibration::deleteDistanceCalibrationResultACK(const QByteArray& data)
{
    auto result = Optional<DistanceCalibrationItem>::emptyOptional();
    data >> result;

    if (!result)
    {
        QMessageBox::information(this, "删除错误", result.msg(), "确定");
        return;
    }
    QMessageBox::information(this, "提示", "删除完成");
    m_impl->model->removeValue(result.value());
}
void DistanceCalibration::clearHistryACK(const QByteArray& data)
{
    OptionalNotValue result;
    data >> result;
    if (!result)
    {
        QMessageBox::information(this, "清除错误", result.msg(), "确定");
        return;
    }
    QMessageBox::information(this, "提示", "清除成功");
}
void DistanceCalibration::queryHistryACK(const QByteArray& data)
{
    auto result = Optional<DistanceCalibrationItemList>::emptyOptional();
    data >> result;
    if (!result)
    {
        QMessageBox::information(this, "提示", "查询失败" /*result.msg()*/);  //这里注释掉是因为弹出的语句是专业术语，不是给用户看的
        return;
    }
    m_impl->model->setValue(result.value());
}
