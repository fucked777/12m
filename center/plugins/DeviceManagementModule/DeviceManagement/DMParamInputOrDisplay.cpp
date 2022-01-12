#include "DMParamInputOrDisplay.h"
#include "DMItemDataModel.h"
#include "DMMessageSerialize.h"
#include "DMUIUtility.h"
#include "DynamicConfigToWidgetUtility.h"
#include "GlobalData.h"
#include "PageNavigator.h"
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTableView>
#include <QUuid>

class DMParamInputOrDisplayImpl
{
public:
    DMFromWidgetInfo fromWidgetInfo;
    DMTreeWidgetInfo treeWidgetInfo;
    QTreeWidget* attributeTreeWidget{ nullptr };
    QTabWidget* treeUI{ nullptr };

    UIDataParameter parameter;
    quint32 channel{ 0 };
    bool hideEmptyAttr{ false };

    void macbIPChange(DMParamInputOrDisplay* parent);
};
void DMParamInputOrDisplayImpl::macbIPChange(DMParamInputOrDisplay* parent)
{
    QList<QLineEdit*> desMainWidgetList;
    QList<QLineEdit*> desStandbyWidgetList;
    for (auto& item : treeWidgetInfo.treeWidgetInfoList)
    {
        auto findMain = item.find("IPMain");
        auto findStand = item.find("IPStandby");
        if (findMain == item.end() || findStand == item.end())
        {
            continue;
        }
        auto widget = qobject_cast<QLineEdit*>(findMain->widget);
        if (widget != nullptr)
        {
            desMainWidgetList << widget;
        }
        widget = qobject_cast<QLineEdit*>(findStand->widget);
        if (widget != nullptr)
        {
            desStandbyWidgetList << widget;
        }
    }
    auto size = desMainWidgetList.size();
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            if (j == i)
            {
                continue;
            }
            auto widget1 = desMainWidgetList.at(i);
            auto widget2 = desMainWidgetList.at(j);
            parent->connect(widget1, &QLineEdit::editingFinished, [=]() { widget2->setText(widget1->text()); });
        }
    }
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            if (j == i)
            {
                continue;
            }
            auto widget1 = desStandbyWidgetList.at(i);
            auto widget2 = desStandbyWidgetList.at(j);
            parent->connect(widget1, &QLineEdit::editingFinished, [=]() { widget2->setText(widget1->text()); });
        }
    }
}
DMParamInputOrDisplay::DMParamInputOrDisplay(const UIDataParameter& parameter, QWidget* parent, bool hideEmptyAttr)
    : QWidget(parent)
    , m_impl(new DMParamInputOrDisplayImpl)
{
    m_impl->hideEmptyAttr = hideEmptyAttr;
    m_impl->parameter = parameter;
    auto paramData = DMUIUtility::createFromWidget(parameter.fromInfo);
    if (!paramData)
    {
        return;
    }

    paramData->widget->setFixedWidth(400);

    m_impl->fromWidgetInfo = paramData.value();

    auto hLayout = new QHBoxLayout(this);
    hLayout->setSpacing(30);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(paramData->widget);

    m_impl->treeUI = new QTabWidget;
    hLayout->addWidget(m_impl->treeUI);

    groupAddRemove();
    refreshAttr(1);
}
DMParamInputOrDisplay::~DMParamInputOrDisplay() { delete m_impl; }
QList<DynamicWidgetInfo> DMParamInputOrDisplay::fromWidgetInfo() const { return m_impl->fromWidgetInfo.fromWidgetInfo; }
void DMParamInputOrDisplay::setFromOnlyOneDisable()
{
    for (auto& item : m_impl->fromWidgetInfo.fromWidgetInfo)
    {
        if (item.widgetInfo.onlyOne)
        {
            item.widget->setEnabled(false);
        }
    }
}
void DMParamInputOrDisplay::setReadOnly()
{
    for (auto& item : m_impl->fromWidgetInfo.fromWidgetInfo)
    {
        item.widget->setEnabled(false);
    }
    for (auto& tempList : m_impl->treeWidgetInfo.treeWidgetInfoList)
    {
        for (auto& item : tempList)
        {
            item.widget->setEnabled(false);
        }
    }
    auto cornerWidget = m_impl->treeUI->cornerWidget();
    if (cornerWidget != nullptr)
    {
        cornerWidget->setEnabled(false);
    }
}
void DMParamInputOrDisplay::groupAddRemove()
{
    /* 没有分组直接全部不生成 */
    if (m_impl->parameter.treeInfo.maxGroup <= 0)
    {
        return;
    }

    auto cornerWidget = new QWidget(m_impl->treeUI);
    auto hlayout = new QHBoxLayout(cornerWidget);
    hlayout->setSpacing(5);
    hlayout->setContentsMargins(0, 0, 0, 0);
    m_impl->treeUI->setCornerWidget(cornerWidget, Qt::TopRightCorner);
    /* 大于1才有增加删除 */
    if (m_impl->parameter.treeInfo.maxGroup > 1)
    {
        auto addBtn = new QPushButton(cornerWidget);
        auto subBtn = new QPushButton(cornerWidget);
        addBtn->setIcon(QIcon(":/add.png"));
        subBtn->setIcon(QIcon(":/sub.png"));
        hlayout->addWidget(addBtn);
        hlayout->addWidget(subBtn);
        connect(addBtn, &QPushButton::clicked, [=]() {
            auto newNum = m_impl->channel + 1;
            if (newNum > m_impl->parameter.treeInfo.maxGroup)
            {
                QMessageBox::information(this, "提示", QString("当前通道不能超过%1个").arg(m_impl->parameter.treeInfo.maxGroup), "确定");
                return;
            }
            refreshAttr(newNum);
        });
        connect(subBtn, &QPushButton::clicked, [=]() {
            auto newNum = m_impl->channel - 1;
            if (newNum <= 0)
            {
                QMessageBox::information(this, "提示", "当前通道不能少于1个", QString("确定"));
                return;
            }
            refreshAttr(newNum);
        });
    }
}

void DMParamInputOrDisplay::refreshAttr(uint channelNum)
{
    if (m_impl->parameter.treeInfo.maxGroup <= 0)
    {
        m_impl->channel = 0;
        if (m_impl->hideEmptyAttr)
        {
            m_impl->treeUI->setVisible(false);
        }
        return;
    }
    m_impl->treeUI->setVisible(true);
    m_impl->channel = channelNum;
    auto result = DMUIUtility::refreshTreeWidget(channelNum, m_impl->treeUI, m_impl->treeWidgetInfo, m_impl->parameter.treeInfo);
    if (!result)
    {
        m_impl->channel = 0;
        QMessageBox::information(this, "提示", result.msg(), "确定");
        return;
    }

    /* 20210610 军检以后删除
     * 	IPMain	IPStandby
     */
    if (m_impl->parameter.type == "MACB")
    {
        m_impl->macbIPChange(this);
    }
}
QString DMParamInputOrDisplay::type() const { return m_impl->parameter.type; }
Optional<DMDataItem> DMParamInputOrDisplay::getUIValue() const
{
    using ResType = Optional<DMDataItem>;
    DMDataItem dmDataItem;

    auto fromRes = DMUIUtility::getFromWidgetData(m_impl->fromWidgetInfo.fromWidgetInfo);
    if (!fromRes)
    {
        return ResType(fromRes.errorCode(), fromRes.msg());
    }
    dmDataItem.parameterConfig = fromRes.value();

    auto treeRes = DMUIUtility::getTreeWidgetData(m_impl->treeWidgetInfo.treeWidgetInfoList);
    if (!treeRes)
    {
        return ResType(treeRes.errorCode(), treeRes.msg());
    }
    dmDataItem.attributeConfig = treeRes.value();

    dmDataItem.createTime = GlobalData::currentDateTime();
    dmDataItem.lastTime = dmDataItem.createTime;
    dmDataItem.type = type();
    dmDataItem.id = QUuid::createUuid().toString();

    if (!m_impl->fromWidgetInfo.fromWidgetInfo.isEmpty())
    {
        auto obj = dmDataItem.parameterConfig.value(m_impl->fromWidgetInfo.fromWidgetInfo.first().widgetInfo.id);
        dmDataItem.devID = obj.toString();
    }
    /*20210610 军检后删除 IPMain	IPStandby */
    if (m_impl->parameter.type == "MACB")
    {
        if (!dmDataItem.attributeConfig.isEmpty())
        {
            auto tempData = dmDataItem.attributeConfig.first();

            if (tempData.value("IPMain") == tempData.value("IPStandby"))
            {
                return ResType(ErrorCode::InvalidData, "当前参数中数据有重复,请更改参数");
            }
        }
    }

    /*20210610 军检后删除 */
    return ResType(dmDataItem);
}

void DMParamInputOrDisplay::setUIValue(const DMDataItem& value)
{
    DMUIUtility::setFromWidgetData(m_impl->fromWidgetInfo.fromWidgetInfo, value.parameterConfig);
    if (m_impl->parameter.treeInfo.maxGroup <= 0)
    {
        m_impl->channel = 0;
        if (m_impl->hideEmptyAttr)
        {
            m_impl->treeUI->setVisible(false);
        }
        return;
    }

    m_impl->treeUI->setVisible(true);
    DMUIUtility::setTreeWidgetData(m_impl->treeUI, m_impl->parameter.treeInfo, m_impl->treeWidgetInfo, value.attributeConfig);
    m_impl->channel = m_impl->treeWidgetInfo.treeWidgetInfoList.size();

    if (m_impl->parameter.type == "MACB")
    {
        m_impl->macbIPChange(this);
    }
}

void DMParamInputOrDisplay::clearInput()
{
    for (auto& item : m_impl->fromWidgetInfo.fromWidgetInfo)
    {
        DynamicConfigToWidgetUtility::resetValue(item);
    }
    if (m_impl->parameter.treeInfo.maxGroup > 0)
    {
        refreshAttr(1);
    }
    for (auto& group : m_impl->treeWidgetInfo.treeWidgetInfoList)
    {
        for (auto& item : group)
        {
            DynamicConfigToWidgetUtility::resetValue(item);
        }
    }
}
