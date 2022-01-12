#include "CreateTestPlanWidget.h"
#include "AutomateTestConfig.h"
#include "AutomateTestDefine.h"
#include "AutomateTestSerialize.h"
#include "AutomateTestUtility.h"
#include "AutomateTestWidget.h"
#include "BusinessMacroCommon.h"
#include "ConfigMacroMessageDefine.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "SatelliteManagementDefine.h"
#include "ServiceCheck.h"
#include "SystemWorkMode.h"
#include "TestAttributeWidget.h"
#include "ui_CreateTestPlanWidget.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QEvent>
#include <QListWidget>
#include <QMap>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QTableView>
#include <QTreeWidgetItem>
#include <QUuid>
#include <QVariant>

enum ATTreeUserRole
{
    Role = Qt::UserRole + 100
};

class CreateTestPlanWidgetImpl
{
public:
    AutomateTestWidget* parent{ nullptr };
    AutomateTestParameterBindMap parameterBindMap;         /* 参数绑定的数据 */
    QList<QTreeWidgetItem*> treeWidgetItemList;            /* 当前的所有的树节点的Item */
    SatelliteManagementDataMap satelliteManagementDataMap; /* 卫星的数据 */
    SatelliteManagementData curSatelliteManagementData;
    NoneParam* noneParam{ nullptr }; /* 未找到参数的界面 */

    CreateTestPlanWidgetImpl(AutomateTestWidget* parent_)
        : parent(parent_)
    {
        GlobalData::getAutomateTestParameterBindMap(parameterBindMap);
    }
};

CreateTestPlanWidget::CreateTestPlanWidget(AutomateTestWidget* parent)
    : QDialog(parent)
    , ui(new Ui::CreateTestPlanWidget)
    , m_impl(new CreateTestPlanWidgetImpl(parent))
{
    ui->setupUi(this);
    Q_ASSERT(parent != nullptr);

    setMinimumSize(1200, 800);
    m_impl->noneParam = new NoneParam(this);
    ui->stackedWidget->addWidget(m_impl->noneParam);

    connect(ui->save, &QAbstractButton::clicked, this, &CreateTestPlanWidget::addTestItem);
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &CreateTestPlanWidget::selectTestItem);

    connect(ui->taskCode, &QComboBox::currentTextChanged, this, &CreateTestPlanWidget::taskCodeChanged);
    connect(ui->workMode, &QComboBox::currentTextChanged, this, &CreateTestPlanWidget::workModeChanged);
    connect(parent, &AutomateTestWidget::sg_addTestItemACK, this, &CreateTestPlanWidget::addTestItemACK);
}

CreateTestPlanWidget::~CreateTestPlanWidget()
{
    delete ui;
    delete m_impl;
}
void CreateTestPlanWidget::init(const AutomateTestItemInfo& testItemInfo)
{
    ui->treeWidget->clear();
    m_impl->treeWidgetItemList.clear();
    /* 测试参数列表 */
    TestItemNodeData testItemNodeData;
    auto paramList = ui->treeWidget;
    for (auto& group : testItemInfo.groupList)
    {
        auto parentParam = new QTreeWidgetItem();
        parentParam->setText(0, group.groupName);

        for (auto& item : group.itemList)
        {
            testItemNodeData.autoTestParam = item;
            testItemNodeData.groupName = group.groupName;
            auto varData = QVariant::fromValue<TestItemNodeData>(testItemNodeData);
            auto treeItem = new QTreeWidgetItem();
            treeItem->setCheckState(0, Qt::Unchecked);
            treeItem->setText(0, item.name);
            treeItem->setData(0, int(ATTreeUserRole::Role), varData);
            parentParam->addChild(treeItem);
            m_impl->treeWidgetItemList << treeItem;
        }
        paramList->addTopLevelItem(parentParam);
    }
}

void CreateTestPlanWidget::setNotFoundParam(const QString& tips)
{
    m_impl->noneParam->setText(tips);
    ui->stackedWidget->setCurrentWidget(m_impl->noneParam);
}

void CreateTestPlanWidget::selectTestItem(QTreeWidgetItem* item, int /*column*/)
{
    /* 获取当前工作模式 */
    if (ui->workMode->count() <= 0)
    {
        setNotFoundParam();
        return;
    }
    auto tempMode = SystemWorkMode(ui->workMode->currentData().toInt());

    auto varData = item->data(0, int(ATTreeUserRole::Role));
    /* 选中父节点了 */
    if (!varData.canConvert<TestItemNodeData>())
    {
        return;
    }
    auto testItemNodeData = varData.value<TestItemNodeData>();

    if (testItemNodeData.testParameterWidget == nullptr)
    {
        testItemNodeData.testParameterWidget = new TestParameterWidget(this);
        testItemNodeData.workMode = tempMode;
        testItemNodeData.testParameterWidget->init(testItemNodeData);
        ui->stackedWidget->addWidget(testItemNodeData.testParameterWidget);
    }
    else if (testItemNodeData.workMode != tempMode)
    {
        ui->stackedWidget->removeWidget(testItemNodeData.testParameterWidget);
        testItemNodeData.testParameterWidget->setParent(nullptr);
        delete testItemNodeData.testParameterWidget;

        testItemNodeData.testParameterWidget = new TestParameterWidget(this);
        testItemNodeData.workMode = tempMode;
        testItemNodeData.testParameterWidget->init(testItemNodeData);
        ui->stackedWidget->addWidget(testItemNodeData.testParameterWidget);
    }
    varData = QVariant::fromValue<TestItemNodeData>(testItemNodeData);
    item->setData(0, int(ATTreeUserRole::Role), varData);
    ui->stackedWidget->setCurrentWidget(testItemNodeData.testParameterWidget);
}

void CreateTestPlanWidget::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    reloadUIData();
}
void CreateTestPlanWidget::reloadUIData()
{
    ui->taskCode->clear();
    m_impl->satelliteManagementDataMap.clear();

    /* 获取卫星管理数据 */
    if (!GlobalData::getSatelliteManagementData(m_impl->satelliteManagementDataMap))
    {
        return;
    }
    for (auto iter = m_impl->satelliteManagementDataMap.begin(); iter != m_impl->satelliteManagementDataMap.end(); ++iter)
    {
        auto key = iter.key();
        ui->taskCode->addItem(iter->m_satelliteCode, key);
    }
}
void CreateTestPlanWidget::taskCodeChanged()
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
        auto tempString = SystemWorkModeHelper::systemWorkModeToDesc(SystemWorkMode(key));
        ui->workMode->addItem(tempString, key);
    }
}

void CreateTestPlanWidget::workModeChanged()
{
    ui->dotDrequency->clear();
    auto tempWorkMode = (SystemWorkMode)ui->workMode->currentData().toInt();
    auto dpNum = m_impl->curSatelliteManagementData.getDpNumByWorkMode(tempWorkMode);
    for (int i = 0; i < dpNum; ++i)
    {
        ui->dotDrequency->addItem(QString::number(i + 1), i + 1);
    }
    /* 工作模式改变之后,清除所有的显示 */
    clearAllItem();
    setNotFoundParam("未选中测试项");
}

void CreateTestPlanWidget::clearAllItem()
{
    for (auto& item : m_impl->treeWidgetItemList)
    {
        item->setCheckState(0, Qt::CheckState::Unchecked);
    }
    ui->treeWidget->clearSelection();
    setNotFoundParam();
}

void CreateTestPlanWidget::addTestItem()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    /* 1.先检查上方的公共参数的数据是否完整 */
    AutomateTestPlanItem automateTestPlanItem;
    automateTestPlanItem.testName = ui->testName->text().trimmed();
    if (automateTestPlanItem.testName.isEmpty())
    {
        QMessageBox::critical(this, "提示", "测试名称不能为空", "确定");
        return;
    }
    automateTestPlanItem.testDesc = ui->testDesc->text().trimmed();

    automateTestPlanItem.taskCode = ui->taskCode->currentData().toString();
    if (automateTestPlanItem.taskCode.isEmpty())
    {
        QMessageBox::critical(this, "提示", "请新增卫星", "确定");
        return;
    }
    automateTestPlanItem.taskIdent = m_impl->curSatelliteManagementData.m_satelliteIdentification;

    /* 获取当前工作模式 */
    if (ui->workMode->count() <= 0)
    {
        QMessageBox::critical(this, "提示", "请选择一个工作模式", "确定");
        return;
    }
    automateTestPlanItem.workMode = SystemWorkMode(ui->workMode->currentData().toInt());
    automateTestPlanItem.dotDrequency = ui->dotDrequency->currentData().toInt();
    if (automateTestPlanItem.dotDrequency <= 0)
    {
        QMessageBox::critical(this, "提示", "请选择一个工作点频", "确定");
        return;
    }

    //    QVariant up;
    //    QVariant down;
    //    MacroCommon::getFrequency(automateTestPlanItem.taskCode, automateTestPlanItem.workMode, automateTestPlanItem.dotDrequency, up, down);

    //    automateTestPlanItem.upFreq = up.toDouble();
    //    automateTestPlanItem.downFreq = down.toDouble();
    automateTestPlanItem.createTime = GlobalData::currentDateTime();
    automateTestPlanItem.equipComb = 0; /* 20210526 现在没有设备组合号 */

    AutomateTestPlanItemList automateTestPlanItemList;
    /* 2.遍历所偶有的测试项 */
    for (auto& treeItem : m_impl->treeWidgetItemList)
    {
        /* 未选中测试项 */
        if (Qt::Checked != treeItem->checkState(0))
        {
            continue;
        }
        /* 这里和鼠标点击节点不一样,鼠标点击是可能点在分组的父节点上,内个节点是啊没有数据的,但是这里的treeItem一定是有效的测试项 */
        auto varData = treeItem->data(0, int(ATTreeUserRole::Role));
        Q_ASSERT(varData.canConvert<TestItemNodeData>());
        auto testItemNodeData = varData.value<TestItemNodeData>();
        /* 为空就是界面都没创建,所以当前测试项不支持这个模式 */
        if (testItemNodeData.testParameterWidget == nullptr)
        {
            QMessageBox::critical(this, testItemNodeData.autoTestParam.name, "此工作体制下不能进行这项测试", "确定");
            return;
        }

        auto resultParameterData = testItemNodeData.testParameterWidget->data();
        if (!resultParameterData)
        {
            QMessageBox::critical(this, testItemNodeData.autoTestParam.name, resultParameterData.msg(), "确定");
            return;
        }

        auto tempBak = automateTestPlanItem;
        tempBak.groupName = testItemNodeData.groupName;
        tempBak.testParameterMap = resultParameterData->infoMap;
        tempBak.automateTestParameterBind = resultParameterData->automateTestParameterBind;
        tempBak.testTypeID = testItemNodeData.autoTestParam.id;
        tempBak.testTypeName = testItemNodeData.autoTestParam.name;
        tempBak.projectCode = ExtendedConfig::curTKID();
        tempBak.id = QString("%1-%2").arg(tempBak.testName, tempBak.testTypeID);
        automateTestPlanItemList << tempBak;
    }

    QByteArray tempData;
    tempData << automateTestPlanItemList;
    emit m_impl->parent->sg_addTestItem(tempData);
}
void CreateTestPlanWidget::addTestItemACK(const QByteArray& data)
{
    auto ack = Optional<AutomateTestPlanItemList>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::warning(this, "添加失败", "测试计划名称重复", "确认");
        return;
    }

    this->accept();
}
// void CreateTestPlanWidget::slotSendCmdAutoTestACK(const QByteArray& data)
//{
//    //    int index = 0;
//    //    QByteArray dataArray;
//    //    // PackCommonHelper::unpackBusinessDataWithBA(value, index, dataArray);

//    //    CalibrationItemACK res;
//    //    res.fromByteArray(data);

//    //    if (!res.result || res.value.isEmpty())
//    //    {
//    //        QMessageBox::critical(this, "提示", "添加测试项失败", "确定");
//    //        return;
//    //    }
//    //    m_impl->resultInfo.clear();
//    //    for (auto& itemValue : res.value)
//    //    {
//    //        AutoTestInfo info;
//    //        info.scItem = itemValue;
//    //        auto doc = QJsonDocument::fromJson(info.scItem.content);
//    //        auto jsonObject = doc.object();

//    //        auto var = jsonObject.find(CPara(TestName))->toVariant();
//    //        info.testName = var.toString();

//    //        var = jsonObject.find(CPara(ProjectCode))->toVariant();
//    //        info.antennaName = ProjectCode(var.toInt());

//    //        var = jsonObject.find(CPara(SCWorkMode))->toVariant();
//    //        info.scWorkMode = SCWorkMode(var.toUInt() % gWMArrayNum);
//    //        info.workMode = EnumStr1(SCWorkMode, info.scWorkMode);

//    //        var = jsonObject.find(atPara(SubSystem))->toVariant();
//    //        info.subSystem = var.toString();

//    //        var = jsonObject.find(atPara(TestTypeName))->toVariant();
//    //        info.testTypeName = var.toString();

//    //        m_impl->resultInfo << info;
//    //    }

//    //    done(1);
//    //    this->hide();
//}
