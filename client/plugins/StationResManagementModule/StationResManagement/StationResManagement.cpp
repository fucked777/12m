#include "StationResManagement.h"
#include "AddStationResManagement.h"
#include "CppMicroServicesUtility.h"
#include "GlobalData.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "SRMUtility.h"
#include "ServiceCheck.h"
#include "StationResManagementModel.h"
#include "StationResManagementSerialize.h"
#include "Utility.h"
#include "ui_StationResManagement.h"
#include <QMessageBox>

class StationResManagementImpl
{
public:
    cppmicroservices::BundleContext context;
    StationResManagementModel dataModel;
    QMenu* menu{ nullptr };

    static StationResManagementMap getAllData()
    {
        /* 从Redis获取数据 */
        QString json;

        if (!RedisHelper::getInstance().getData("StationResManagement", json))
        {
            return StationResManagementMap();
        }
        StationResManagementMap resultMap;
        json >> resultMap;
        return resultMap;
    }
    void queryItem()
    {
        auto data = StationResManagementImpl::getAllData();
        dataModel.updateData(data);
    }
};

StationResManagement::StationResManagement(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StationResManagement)
    , m_impl(new StationResManagementImpl)
{
    ui->setupUi(this);

    m_impl->context = context;

    qRegisterMetaType<StationResManagementData>("StationResManagementData&");
    qRegisterMetaType<StationResManagementData>("StationResManagementData");
    qRegisterMetaType<StationResManagementData>("const StationResManagementData&");
    //初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    init();
}

StationResManagement::~StationResManagement()
{
    delete ui;
    delete m_impl;
}

void StationResManagement::init()
{
    SRMUtility::uiInit(ui);
    Utility::tableViewInit(ui->tableView, true);
    ui->tableView->setModel(&(m_impl->dataModel));

    SRMUtility::setUIEnable(ui, false);

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->addBtn, &QPushButton::clicked, this, &StationResManagement::addItem);
    connect(ui->editBtn, &QPushButton::clicked, this, &StationResManagement::modifyItem);
    connect(ui->delBtn, &QPushButton::clicked, this, &StationResManagement::deleteItem);
    connect(ui->refreshBtn, &QPushButton::clicked, this, &StationResManagement::queryItem);
    connect(ui->tableView, &QTableView::clicked, [=](const QModelIndex& index) {
        auto curSelectData = m_impl->dataModel.getDataCenterItem(index);
        if (!curSelectData)
        {
            return;
        }
        SRMUtility::setUIData(ui, curSelectData.value());
    });
    connect(ui->tableView, &QTableView::doubleClicked, this, &StationResManagement::modifyItem);

    m_impl->menu = new QMenu(this);
    m_impl->menu->addAction(QString("编辑"), this, &StationResManagement::modifyItem);
    m_impl->menu->addAction(QString("删除"), this, &StationResManagement::deleteItem);

    connect(ui->tableView, &QTableView::customContextMenuRequested, this, [=]() {
        QPoint pt = ui->tableView->mapFromGlobal(QCursor::pos());
        int height = ui->tableView->horizontalHeader()->height();
        QPoint pt2(0, height);
        pt -= pt2;
        QModelIndex index = ui->tableView->indexAt(pt);
        bool flag = m_impl->dataModel.judgeData(index.row());
        if (ui->tableView->currentIndex().isValid() && flag)
        {
            m_impl->menu->exec(QCursor::pos());
        }
    });
}

void StationResManagement::addItem()
{
    AddStationResManagement dialog(this);
    dialog.setWindowTitle("添加站资源数据");

    if (QDialog::Accepted == dialog.exec())
    {
        queryItem();
        SRMUtility::clearUIData(ui);
    }
}

void StationResManagement::modifyItem()
{
    auto curSelectData = m_impl->dataModel.getDataCenterItem(ui->tableView->currentIndex());
    if (!curSelectData)
    {
        QMessageBox::warning(this, QString("提示"), QString("请选择需要编辑的数据"), QString("确定"));
        return;
    }
    AddStationResManagement dialog(this);
    dialog.setWindowTitle("编辑站资源数据");
    dialog.setData(curSelectData.value());

    if (QDialog::Accepted == dialog.exec())
    {
        queryItem();
        SRMUtility::clearUIData(ui);
    }
}

void StationResManagement::deleteItem()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    auto curSelectData = m_impl->dataModel.getDataCenterItem(ui->tableView->currentIndex());
    if (!curSelectData)
    {
        QMessageBox::warning(this, QString("提示"), QString("请选择需要删除的数据"), QString("确定"));
        return;
    }

    if (QMessageBox::information(this, "提示", "您确定要删除数据吗?", QString("确定"), QString("取消")) != 0)
        return;

    QByteArray array;
    array << curSelectData.value();
    emit sg_delete(array);
}

void StationResManagement::queryItem() { m_impl->queryItem(); }

void StationResManagement::deleteACK(const QByteArray& data)
{
    auto ack = Optional<StationResManagementData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::critical(this, "删除失败", ack.msg(), "确定");
        return;
    }

    m_impl->dataModel.deleteItem(ack.value());
    QMessageBox::information(this, "提示", "删除成功", "确定");
    SRMUtility::clearUIData(ui);
}
void StationResManagement::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
    emit ui->refreshBtn->clicked();
    SRMUtility::clearUIData(ui);
}
