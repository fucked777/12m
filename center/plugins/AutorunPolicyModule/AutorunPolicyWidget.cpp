#include "AutorunPolicyWidget.h"
#include "AutorunPolicyMessageSerialize.h"
#include "AutorunPolicyUtility.h"
#include "CppMicroServicesUtility.h"
#include "DynamicConfigToWidgetUtility.h"
#include "GlobalData.h"
#include "PlatformInterface.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTreeWidget>

class AutorunPolicyWidgetImpl
{
public:
    cppmicroservices::BundleContext context;
    QPushButton* save{ nullptr };
    QPushButton* refresh{ nullptr };
    QPushButton* reload{ nullptr }; /* 这个用来调试的,正常情况下隐藏 */
    QTabWidget* tabWidget{ nullptr };
    QList<ATTreeWidgetInfo> listWidgetInfo;

    static AutorunPolicyData getAllData()
    {
        /* 从Redis获取数据 */
        QString json;
        if (!RedisHelper::getInstance().getData("AutorunPolicy", json))
        {
            return AutorunPolicyData();
        }
        AutorunPolicyData autorunPolicyData;
        json >> autorunPolicyData;
        return autorunPolicyData;
    }
};

AutorunPolicyWidget::AutorunPolicyWidget(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , m_impl(new AutorunPolicyWidgetImpl)
{
    m_impl->context = context;

    auto vLayouts = new QVBoxLayout(this);
    auto hLayouts = new QHBoxLayout;
    m_impl->tabWidget = new QTabWidget(this);
    vLayouts->addWidget(m_impl->tabWidget);
    /* 按钮 */
    m_impl->save = new QPushButton(this);
    m_impl->refresh = new QPushButton(this);
    m_impl->reload = new QPushButton(this);
    m_impl->save->setText("保存");
    m_impl->refresh->setText("刷新");
    m_impl->reload->setText("重载配置");
    m_impl->save->setFixedSize(100, 32);
    m_impl->refresh->setFixedSize(100, 32);
    m_impl->reload->setFixedSize(100, 32);
    hLayouts->addStretch();
    hLayouts->addWidget(m_impl->save);
    hLayouts->addWidget(m_impl->refresh);
    hLayouts->addWidget(m_impl->reload);

    vLayouts->addLayout(hLayouts);
    m_impl->reload->setVisible(false); /* 隐藏调试用的按钮 */

    connect(m_impl->reload, &QPushButton::clicked, this, &AutorunPolicyWidget::reloadConfig);
    connect(m_impl->save, &QPushButton::clicked, this, &AutorunPolicyWidget::updateAutorunPolicy);
    connect(m_impl->refresh, &QPushButton::clicked, this, &AutorunPolicyWidget::refreshUI);

    this->setMinimumWidth(700);
    this->setMinimumHeight(800);

    reloadConfig();
}

AutorunPolicyWidget::~AutorunPolicyWidget() { delete m_impl; }

void AutorunPolicyWidget::reloadConfig()
{
    /* 清空原来的数据 */
    m_impl->tabWidget->clear();
    for (auto& item : m_impl->listWidgetInfo)
    {
        item.widget->setParent(nullptr);
        delete item.widget;
    }
    m_impl->listWidgetInfo.clear();

    auto fileDir = PlatformConfigTools::configBusiness("AutorunPolicy");
    auto loadResult = AutorunPolicyUtility::loadConfig(fileDir);
    if (!loadResult)
    {
        qWarning() << loadResult.msg();
        return;
    }

    auto tempTreeInfoList = loadResult.value();
    for (auto& item : tempTreeInfoList)
    {
        auto info = AutorunPolicyUtility::createTreeWidget(item.treeList);
        if (!info)
        {
            qWarning() << info.msg();
            continue;
        }
        if (info->treeWidgetMap.isEmpty())
        {
            delete info->widget;
            continue;
        }
        info->widget->setColumnWidth(0, 300);
        info->widget->setColumnWidth(1, 250);
        info->widget->expandAll();
        m_impl->listWidgetInfo << info.value();
        m_impl->tabWidget->addTab(info->widget, item.name);
    }
    refreshUI();
}

void AutorunPolicyWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    refreshUI();
}

void AutorunPolicyWidget::updateAutorunPolicy()
{
    SERVICEONLINECHECK();

    if (QMessageBox::information(this, "提示", "是否保存当前配置!", "确定", "取消") != 0)
        return;

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    AutorunPolicyData autorunPolicyData;
    for (auto& page : m_impl->listWidgetInfo)
    {
        for (auto& item : page.treeWidgetMap)
        {
            auto value = DynamicConfigToWidgetUtility::getWidgetValue(item);
            if (!value)
            {
                QMessageBox::warning(this, "提示", value.msg(), QString("确定"));
                return;
            }
            autorunPolicyData.policy[item.widgetInfo.id] = value.value();
        }
    }

    QByteArray json;
    json << autorunPolicyData;
    emit sg_modify(json);
}

void AutorunPolicyWidget::refreshUI()
{
    auto data = AutorunPolicyWidgetImpl::getAllData();
    for (auto& page : m_impl->listWidgetInfo)
    {
        for (auto& item : page.treeWidgetMap)
        {
            auto find = data.policy.find(item.widgetInfo.id);
            if (find == data.policy.end())
            {
                DynamicConfigToWidgetUtility::resetValue(item);
            }
            else
            {
                DynamicConfigToWidgetUtility::setWidgetValue(item, find.value());
            }
        }
    }
}
void AutorunPolicyWidget::modifyACK(const QByteArray& data)
{
    if (isHidden())
    {
        return;
    }
    auto ack = Optional<QVariantMap>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::information(this, "更新失败", ack.msg(), QString("确定"));
        return;
    }
    QMessageBox::information(this, "提示", "更新成功", QString("确定"));
    refreshUI();
}
