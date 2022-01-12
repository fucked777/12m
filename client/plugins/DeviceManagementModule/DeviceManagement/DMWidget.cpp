#include "DMWidget.h"
#include "CppMicroServicesUtility.h"
#include "DMEditDialog.h"
#include "DMItemWidget.h"
#include "DMMessageSerialize.h"
#include "DMUIUtility.h"
#include "GlobalData.h"
#include "PlatformInterface.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "Utility.h"
#include "ui_DMWidget.h"
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QUuid>

class DMWidgetImpl
{
public:
    cppmicroservices::BundleContext context;
    QMap<QString, UIDataParameter> parameterMap;
    QMap<QString, DMItemWidget*> widgetMap;

    static DMTypeMap getDMAllData()
    {
        /* 从Redis获取数据 */
        QString json;
        if (!RedisHelper::getInstance().getData("DeviceManagement", json))
        {
            return DMTypeMap();
        }
        DMTypeMap resultMap;
        json >> resultMap;
        return resultMap;
    }
};

DMWidget::DMWidget(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DMWidget)
    , m_impl(new DMWidgetImpl)
{
    ui->setupUi(this);
    m_impl->context = context;

    //初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    parseConfig();
    init();
}

DMWidget::~DMWidget()
{
    delete ui;
    delete m_impl;
}
void DMWidget::parseConfig()
{
    auto configDir = PlatformConfigTools::configBusiness("DeviceManagement");
    QDir dir(configDir);

    if (!dir.exists())
    {
        auto msg = QString("未找到设备管理配置文件目录:%1").arg(configDir);
        // Error(ErrorCode::NotFound, msg);
        return;
    }

    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    auto dirInfoList = dir.entryInfoList();

    for (auto& info : dirInfoList)
    {
        auto parameter = DMUIUtility::loadConfigXml(info.filePath());
        if (!parameter)
        {
            auto msg = QString("设备管理配置文件解析失败:%1").arg(parameter.msg());
            qWarning() << msg;
            // Error(ErrorCode::NotFound, msg);
            continue;
        }
        m_impl->parameterMap.insert(parameter->type, parameter.value());
    }
}
void DMWidget::init()
{
    for (auto& item : m_impl->parameterMap)
    {
        auto widget = new DMItemWidget(item, this);
        ui->tabWidget->addTab(widget, item.name);
        m_impl->widgetMap.insert(item.type, widget);
    }

    //    connect(m_impl->antenna, &DMAntenna::siganlMenuDeletes, this, &DMWidget::deleteItem);
    //    connect(m_impl->did, &DMDID::siganlMenuDeletes, this, &DMWidget::deleteItem);
    //    connect(m_impl->dsame, &DMDSAME::siganlMenuDeletes, this, &DMWidget::deleteItem);
    //    connect(m_impl->lsdb, &DMLSDB::siganlMenuDeletes, this, &DMWidget::deleteItem);
    //    connect(m_impl->hsdb, &DMHSDB::siganlMenuDeletes, this, &DMWidget::deleteItem);
    //    connect(m_impl->macb, &DMMACB::siganlMenuDeletes, this, &DMWidget::deleteItem);

    connect(ui->btn_add, &QAbstractButton::clicked, this, &DMWidget::addItem);
    connect(ui->btn_delete, &QAbstractButton::clicked, this, &DMWidget::deleteItem);
    connect(ui->btn_modify, &QAbstractButton::clicked, this, &DMWidget::modifyItem);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &DMWidget::queryItem);
    connect(ui->btn_refresh, &QAbstractButton::clicked, [=]() {
        SERVICEONLINECHECK();  //判断服务器是否离线

        auto result = queryItem();
        if (!result)
        {
            QMessageBox::information(this, "提示", result.msg(), QString("确定"));
        }
    });

    //    m_impl->timerID = startTimer(1000);
}

void DMWidget::addItem()
{
    auto tempWidget = qobject_cast<DMItemWidget*>(ui->tabWidget->currentWidget());
    if (tempWidget == nullptr)
    {
        QMessageBox::critical(this, "错误", "系统错误", "确定");
        return;
    }
    DMEditDialog dmEditDialog(tempWidget->uiDataParameter(), this);
    dmEditDialog.exec();
    queryItem();
}
void DMWidget::modifyItem()
{
    auto tempWidget = qobject_cast<DMItemWidget*>(ui->tabWidget->currentWidget());
    if (tempWidget == nullptr)
    {
        QMessageBox::critical(this, "错误", "系统错误", "确定");
        return;
    }
    auto oldData = tempWidget->getSelectItem();
    /* 获取原来的数据 */
    if (!oldData)
    {
        QMessageBox::information(this, "提示", oldData.msg(), QString("确定"));
        return;
    }

    DMEditDialog dmEditDialog(tempWidget->uiDataParameter(), this);
    dmEditDialog.setUIValue(oldData.value());
    dmEditDialog.exec();
    queryItem();
}

void DMWidget::deleteItem()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    auto tempWidget = qobject_cast<DMItemWidget*>(ui->tabWidget->currentWidget());
    if (tempWidget == nullptr)
    {
        QMessageBox::critical(this, "错误", "系统错误", "确定");
        return;
    }
    auto value = tempWidget->getSelectItem();
    if (!value)
    {
        QMessageBox::information(this, "提示", value.msg(), "确定");
        return;
    }
    if (QMessageBox::information(this, "提示", "您确定要删除数据吗?", "确定", "取消") != 0)
        return;

    /* 基带配置关联 没写  20210228 wp?? */
    QByteArray array;
    value->typeName = ui->tabWidget->tabText(ui->tabWidget->currentIndex());
    array << value.value();
    emit sg_delete(array);
}
OptionalNotValue DMWidget::queryItem()
{
    auto tempWidget = qobject_cast<DMItemWidget*>(ui->tabWidget->currentWidget());
    if (tempWidget == nullptr)
    {
        return OptionalNotValue(ErrorCode::UnknownError, "系统错误");
    }

    auto dmValueMap = DMWidgetImpl::getDMAllData();

    auto find = dmValueMap.find(tempWidget->type());
    if (find == dmValueMap.end())
    {
        //如果删除到没有记录的时候,应该清除表格
        tempWidget->setItems(QList<DMDataItem>());
        return OptionalNotValue(ErrorCode::NotFound, "未能查询到当前的数据");
    }

    tempWidget->setItems(find.value());
    tempWidget->clearInput();

    return OptionalNotValue();
}

void DMWidget::deleteACK(const QByteArray& value)
{
    auto ackResult = Optional<DMDataItem>::emptyOptional();
    value >> ackResult;
    if (!ackResult)
    {
        QMessageBox::information(this, "删除失败", ackResult.msg(), QString("确定"));
        return;
    }
    auto find = m_impl->widgetMap.find(ackResult->type);
    if (find == m_impl->widgetMap.end())
    {
        QMessageBox::information(this, "删除失败", "删除的数据是未知的类型", QString("确定"));
        return;
    }
    auto& widget = find.value();
    widget->deleteItem(ackResult.value());
    widget->clearInput();
    QMessageBox::information(this, "提示", "删除数据成功", QString("确定"));
}

void DMWidget::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
    queryItem();
}

// void DMWidget::queryACK(const QByteArray& value)
//{
//    auto ackResult = Optional<DMTypeMap>::emptyOptional();
//    value >> ackResult;
//    if (!ackResult)
//    {
//        QMessageBox::information(this, "查询失败", ackResult.msg(), QString("确定"));
//        return;
//    }
//    auto& ackValue = ackResult.value();

//    if (ackResult->isEmpty())
//    {
//        QMessageBox::information(this, "查询成功", "当前数据为空", QString("确定"));
//        return;
//    }
//    auto find = m_impl->widgetMap.find(ackResult->begin().value().);
//    if (find == m_impl->widgetMap.end())
//    {
//        QMessageBox::information(this, "删除失败", "删除的数据是未知的类型", QString("确定"));
//        return;
//    }
//    auto& widget = find.value();
//    widget->deleteItem(ackResult.value());
//    widget->clearInput();
//    QMessageBox::information(this, "提示", "删除数据成功", QString("确定"));
// }
