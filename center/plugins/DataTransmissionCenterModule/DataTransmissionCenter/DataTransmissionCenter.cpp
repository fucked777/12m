#include "DataTransmissionCenter.h"
#include "AddDataTransmissionCenter.h"
#include "CppMicroServicesUtility.h"
#include "DTCUtility.h"
#include "DataTransmissionCenterSerialize.h"
#include "GlobalData.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "Utility.h"
#include "ui_DataTransmissionCenter.h"
#include <QMessageBox>

class DataTransmissionCenterImpl
{
public:
    cppmicroservices::BundleContext context;
    DataTransmissionModel dataModel;
    QMenu* menu{ nullptr };

    static DataTransmissionCenterMap getAllData()
    {
        /* 从Redis获取数据 */
        QString json;
        if (!RedisHelper::getInstance().getData("DataTransmissionCenter", json))
        {
            return DataTransmissionCenterMap();
        }
        DataTransmissionCenterMap resultMap;
        json >> resultMap;
        return resultMap;
    }
    void queryItem()
    {
        auto data = DataTransmissionCenterImpl::getAllData();
        dataModel.updateData(data);
    }
};

DataTransmissionCenter::DataTransmissionCenter(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DataTransmissionCenter)
    , m_impl(new DataTransmissionCenterImpl)
{
    ui->setupUi(this);

    m_impl->context = context;

    qRegisterMetaType<DataTransmissionCenterData>("DataTransmissionCenterData&");
    qRegisterMetaType<DataTransmissionCenterData>("DataTransmissionCenterData");
    qRegisterMetaType<DataTransmissionCenterData>("const DataTransmissionCenterData&");
    //初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    init();
}

DataTransmissionCenter::~DataTransmissionCenter()
{
    delete ui;
    delete m_impl;
}

void DataTransmissionCenter::init()
{
    DTCUtility::uiInit(ui);
    Utility::tableViewInit(ui->tableView, true);
    ui->tableView->setModel(&(m_impl->dataModel));

    DTCUtility::setUIEnable(ui, false);

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->addBtn, &QPushButton::clicked, this, &DataTransmissionCenter::addItem);
    connect(ui->editBtn, &QPushButton::clicked, this, &DataTransmissionCenter::modifyItem);
    connect(ui->delBtn, &QPushButton::clicked, this, &DataTransmissionCenter::deleteItem);
    connect(ui->refreshBtn, &QPushButton::clicked, this, &DataTransmissionCenter::queryItem);
    connect(ui->tableView, &QTableView::clicked, [=](const QModelIndex& index) {
        auto curSelectData = m_impl->dataModel.getDataCenterItem(index);
        if (!curSelectData)
        {
            return;
        }
        DTCUtility::setUIData(ui, curSelectData.value());
    });
    connect(ui->tableView, &QTableView::doubleClicked, this, &DataTransmissionCenter::modifyItem);

    m_impl->menu = new QMenu(this);
    m_impl->menu->addAction(QString("编辑"), this, &DataTransmissionCenter::modifyItem);
    m_impl->menu->addAction(QString("删除"), this, &DataTransmissionCenter::deleteItem);

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

void DataTransmissionCenter::addItem()
{
    AddDataTransmissionCenter dialog(this);
    dialog.setWindowTitle("添加数传中心数据");

    if (QDialog::Accepted == dialog.exec())
    {
        queryItem();
        DTCUtility::clearUIData(ui);
    }
}

void DataTransmissionCenter::modifyItem()
{
    auto curSelectData = m_impl->dataModel.getDataCenterItem(ui->tableView->currentIndex());
    if (!curSelectData)
    {
        QMessageBox::warning(this, QString("提示"), QString("请选择需要编辑的任务中心"), QString("确定"));
        return;
    }

    AddDataTransmissionCenter dialog(this);
    dialog.setWindowTitle("编辑数传中心数据");
    dialog.setData(curSelectData.value());

    if (QDialog::Accepted == dialog.exec())
    {
        queryItem();
        DTCUtility::clearUIData(ui);
    }
}

void DataTransmissionCenter::deleteItem()
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

void DataTransmissionCenter::queryItem() { m_impl->queryItem(); }

void DataTransmissionCenter::deleteACK(const QByteArray& data)
{
    auto ack = Optional<DataTransmissionCenterData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::critical(this, "删除失败", ack.msg(), "确定");
        return;
    }

    m_impl->dataModel.deleteItem(ack.value());
    QMessageBox::information(this, "提示", "删除成功", "确定");
    DTCUtility::clearUIData(ui);
}
void DataTransmissionCenter::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
    // m_impl->queryItem();
    emit ui->refreshBtn->clicked();
    DTCUtility::clearUIData(ui);
}
