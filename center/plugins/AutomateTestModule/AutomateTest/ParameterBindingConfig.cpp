#include "ParameterBindingConfig.h"
#include "AutomateTestDefine.h"
#include "AutomateTestSerialize.h"
#include "AutomateTestUtility.h"
#include "AutomateTestWidget.h"
#include "DynamicConfigToWidgetUtility.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "ParameterBindingModel.h"
#include "QssCommonHelper.h"
#include "ServiceCheck.h"
#include "SystemWorkMode.h"
#include "ui_ParameterBindingConfig.h"
#include <QComboBox>
#include <QDateTime>
#include <QDomDocument>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <QUuid>

class ParameterBindingConfigImpl
{
public:
    AutomateTestWidget* parent{ nullptr };
    QMap<QString, DynamicWidgetInfo> dataWidget;
    QWidget* bindingConvertWidget{ nullptr };
    QHBoxLayout* bindingLayout{ nullptr };
    ParameterBindingConfigRelation parameterBindingConfigRelation;
    AutomateTestItem testItem;
    ParameterBindingModel model;
    QString relationKey;

    ParameterBindingConfigImpl(AutomateTestWidget* parent_)
        : parent(parent_)

    {
    }
    void refreshBind()
    {
        AutomateTestParameterBindMap tempMap;
        if (!GlobalData::getAutomateTestParameterBindMap(tempMap))
        {
            return;
        }
        model.updateData(tempMap.value(testItem.id));
    }
};

ParameterBindingConfig::ParameterBindingConfig(AutomateTestWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ParameterBindingConfig)
    , m_impl(new ParameterBindingConfigImpl(parent))
{
    Q_ASSERT(parent != nullptr);
    ui->setupUi(this);
    setWindowTitle("测试计划参数装订");
    setMinimumSize(960, 530);
    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    connect(ui->add, &QPushButton::clicked, this, &ParameterBindingConfig::add);
    connect(ui->del, &QPushButton::clicked, this, &ParameterBindingConfig::del);
    connect(ui->tableView, &QTableView::clicked, this, &ParameterBindingConfig::bindItemClicket);

    ui->tableView->setModel(&(m_impl->model));
    Utility::tableViewInit(ui->tableView, true);

    m_impl->bindingLayout = new QHBoxLayout(ui->binding);
    m_impl->bindingLayout->setSpacing(0);
    m_impl->bindingLayout->setContentsMargins(0, 0, 0, 0);
}

ParameterBindingConfig::~ParameterBindingConfig() { delete m_impl; }
OptionalNotValue ParameterBindingConfig::init(const AutomateTestItem& testItem)
{
    m_impl->testItem = testItem;
    /* 读取当前绑定参数的配置文件 */
    auto result = AutomateTestUtility::parseParameterBindingConfig(testItem);
    if (!result)
    {
        return OptionalNotValue(result.errorCode(), result.msg());
    }
    m_impl->parameterBindingConfigRelation = result.value();
    if (!m_impl->parameterBindingConfigRelation.configMap.isEmpty())
    {
        resetUI(m_impl->parameterBindingConfigRelation.configMap.firstKey());
    }

    return OptionalNotValue();
}
void ParameterBindingConfig::resetUI(const QString& tempText)
{
    auto& parameterBindingConfigRelation = m_impl->parameterBindingConfigRelation;
    auto size = parameterBindingConfigRelation.configMap.size();
    ParameterBindingConfigInfo parameterBindingConfigInfo;
    if (size > 1)
    {
        //        if (!parameterBindingConfigRelation.configMap.contains(tempText))
        //        {
        //            m_impl->model.setIndexData(ParameterBindingConfigInfo());
        //            return;
        //        }
        parameterBindingConfigInfo = parameterBindingConfigRelation.configMap.value(tempText);
    }
    else
    {
        parameterBindingConfigInfo = parameterBindingConfigRelation.configMap.first();
    }

    /* 先保存数据 */
    QMap<QString, QVariant> tempControlData;
    for (auto& item : m_impl->dataWidget)
    {
        auto varResult = DynamicConfigToWidgetUtility::getWidgetValue(item);
        if (!varResult)
        {
            continue;
        }
        tempControlData[item.widgetInfo.id] = varResult.value();
    }
    /* 删除原来的数据 */
    if (m_impl->bindingConvertWidget != nullptr)
    {
        m_impl->bindingConvertWidget->setParent(nullptr);
        delete m_impl->bindingConvertWidget;
        m_impl->bindingConvertWidget = nullptr;
        m_impl->dataWidget.clear();
    }
    auto tempWidget = new QWidget(ui->binding);
    m_impl->bindingConvertWidget = tempWidget;

    /* 创建界面 */
    auto widgetLayout = new QGridLayout(tempWidget);
    widgetLayout->setContentsMargins(0, 0, 0, 0);
    widgetLayout->setHorizontalSpacing(10);
    widgetLayout->setVerticalSpacing(20);
    int row = 0;
    int col = 0;
    for (auto& widgetItem : parameterBindingConfigInfo.testItemParameter)
    {
        auto widgetResult = DynamicConfigToWidgetUtility::createWidget(widgetItem);
        if (!widgetResult || widgetResult->widget == nullptr)
        {
            continue;
        }
        auto& dynamicWidgetInfo = widgetResult.value();
        dynamicWidgetInfo.widget->setParent(ui->binding);

        auto label = new QLabel(ui->binding);
        label->setText(widgetItem.name + ":");

        label->setFixedWidth(150);
        dynamicWidgetInfo.widget->setFixedWidth(200);

        widgetLayout->addWidget(label, row, col++);
        widgetLayout->addWidget(widgetResult->widget, row, col++);
        if (col >= 4)
        {
            col = 0;
            ++row;
        }
        m_impl->dataWidget.insert(widgetItem.id, dynamicWidgetInfo);
    }

    /* 还原 */
    for (auto& item : m_impl->dataWidget)
    {
        DynamicConfigToWidgetUtility::setWidgetValue(item, tempControlData.value(item.widgetInfo.id));
    }
    if (m_impl->dataWidget.contains(parameterBindingConfigRelation.relation))
    {
        auto info = m_impl->dataWidget.value(parameterBindingConfigRelation.relation);
        m_impl->relationKey = parameterBindingConfigRelation.relation;
        if (info.widgetInfo.type == DynamicConfigToWidgetType::Enum)
        {
            auto combobox = qobject_cast<QComboBox*>(info.widget);
            if (combobox != nullptr)
            {
                combobox->setCurrentText(tempText);
                /* 槽函数里面套了一个定时器原因是
                 * 这个是在控件的槽函数触发的函数
                 * 但是这里会销控件,所以这里在程序返回的时候会出现程序崩溃
                 */
                connect(combobox, &QComboBox::currentTextChanged, [=](const QString& text) { QTimer::singleShot(300, [=]() { resetUI(text); }); });
            }
        }
    }
    tempWidget->setLayout(widgetLayout);
    m_impl->bindingLayout->addWidget(tempWidget);
    //    m_impl->refreshBind();
    return;
}

void ParameterBindingConfig::add()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    AutomateTestParameterBind automateTestParameterBind;
    automateTestParameterBind.createTime = GlobalData::currentDateTime();
    automateTestParameterBind.testTypeID = m_impl->testItem.id;

    for (auto& item : m_impl->dataWidget)
    {
        auto varResult = DynamicConfigToWidgetUtility::getWidgetValue(item);
        if (!varResult)
        {
            QMessageBox::warning(this, "提示", varResult.msg());
            return;
        }
        AutomateTestItemParameterInfo info;
        info.displayName = item.widgetInfo.name;
        info.rawValue = varResult.value();
        info.id = item.widgetInfo.id;
        if (item.widgetInfo.type == DynamicConfigToWidgetType::Enum)
        {
            auto temp = qobject_cast<QComboBox*>(item.widget);
            info.displayValue = temp->currentText();
        }
        else
        {
            info.displayValue = info.rawValue.toString();
        }
        if (item.widgetInfo.onlyOne)
        {
            automateTestParameterBind.id.append(QString("%1*").arg(info.displayValue));
        }

        automateTestParameterBind.testBindParameterMap.insert(info.id, info);
    }
    /* 这个1是*号 */
    if (automateTestParameterBind.id.size() <= 1)
    {
        automateTestParameterBind.id = QUuid::createUuid().toString();
    }
    else
    {
        automateTestParameterBind.id.push_back(automateTestParameterBind.testTypeID);
    }

    automateTestParameterBind.workMode = SystemWorkMode(automateTestParameterBind.testBindParameterMap.value("WorkMode").rawValue.toInt());

    QByteArray sendArray;
    sendArray << automateTestParameterBind;
    emit m_impl->parent->sg_addTestBindItem(sendArray);
}
void ParameterBindingConfig::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

    auto& parameterBindingConfigRelation = m_impl->parameterBindingConfigRelation;
    auto parameterBindingConfigInfo = parameterBindingConfigRelation.configMap.first();
    m_impl->model.setIndexData(parameterBindingConfigInfo);
    m_impl->refreshBind();
}
void ParameterBindingConfig::del()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    AutomateTestParameterBind item;
    if (!m_impl->model.itemAt(item, ui->tableView->currentIndex()))
    {
        QMessageBox::warning(this, "提示", "请选择删除的项", QString("确定"));
        return;
    }
    QByteArray sendArray;
    sendArray << item;
    emit m_impl->parent->sg_deleteTestBindItem(sendArray);
}
void ParameterBindingConfig::bindItemClicket(const QModelIndex& /*index*/)
{
    AutomateTestParameterBind automateTestParameterBind;
    if (!m_impl->model.itemAt(automateTestParameterBind, ui->tableView->currentIndex()))
    {
        return;
    }
    /* 主动重置UI */
    auto& tempMap = automateTestParameterBind.testBindParameterMap;
    if (tempMap.contains(m_impl->relationKey))
    {
        auto value = tempMap.value(m_impl->relationKey);
        resetUI(value.displayValue);
    }

    for (auto& item : automateTestParameterBind.testBindParameterMap)
    {
        auto widget = m_impl->dataWidget.value(item.id);
        if (widget.widget == nullptr)
        {
            continue;
        }
        DynamicConfigToWidgetUtility::setWidgetValue(widget, item.rawValue);
    }
}

void ParameterBindingConfig::addTestBindItemACK(const QByteArray& data)
{
    auto ack = Optional<AutomateTestParameterBind>::emptyOptional();
    data >> ack;

    m_impl->refreshBind();
    if (!ack)
    {
        QMessageBox::warning(this, "更新数据失败", ack.msg(), "确认");
        return;
    }

    // this->accept();
}
void ParameterBindingConfig::deleteTestBindItemACK(const QByteArray& data)
{
    auto ack = Optional<AutomateTestParameterBind>::emptyOptional();
    data >> ack;

    m_impl->refreshBind();
    if (!ack)
    {
        QMessageBox::warning(this, "删除失败", ack.msg(), "确认");
        return;
    }

    // this->accept();
}
