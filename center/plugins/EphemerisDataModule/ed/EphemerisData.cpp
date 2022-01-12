#include "EphemerisData.h"
#include "AddEphemerisDataDialog.h"
#include "CppMicroServicesUtility.h"
#include "EDUtility.h"
#include "EphemerisDataSerialize.h"
#include "GlobalData.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "SatelliteManagementSerialize.h"
#include "ServiceCheck.h"
#include "ui_EphemerisData.h"
#include <QMenu>
#include <QMessageBox>
class EphemerisDataImpl
{
public:
    cppmicroservices::BundleContext context;
    EphemerisDataModel dataModel;
    QMenu* menu{ nullptr };

    static EphemerisDataMap getAllData()
    {
        /* 从Redis获取数据 */
        QString json;
        if (!RedisHelper::getInstance().getData("EphemerisData", json))
        {
            return EphemerisDataMap();
        }
        EphemerisDataMap resultMap;
        json >> resultMap;
        return resultMap;
    }
    void queryItem()
    {
        auto data = EphemerisDataImpl::getAllData();
        dataModel.updateData(data);
    }
    static QMap<QString, SatelliteManagementData> getAllSatelliteManagementData()
    {
        /* 从Redis获取数据 */
        QString json;

        if (!RedisHelper::getInstance().getData("SatelliteData", json))
        {
            return SatelliteManagementMap();
        }
        SatelliteManagementDataList dataList;
        json >> dataList;
        QMap<QString, SatelliteManagementData> resultMap;
        for (auto item : dataList)
        {
            resultMap[item.m_satelliteCode] = item;
        }
        return resultMap;
    }
};

EphemerisData::EphemerisData(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::EphemerisData)
    , m_impl(new EphemerisDataImpl)
{
    ui->setupUi(this);

    m_impl->context = context;

    qRegisterMetaType<EphemerisDataData>("EphemerisDataData&");
    qRegisterMetaType<EphemerisDataData>("EphemerisDataData");
    qRegisterMetaType<EphemerisDataData>("const EphemerisDataData&");
    //初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    init();
}

EphemerisData::~EphemerisData()
{
    delete ui;
    delete m_impl;
}

void EphemerisData::init()
{
    EDUtility::uiInit(ui);
    Utility::tableViewInit(ui->tableView, true);
    ui->tableView->setModel(&(m_impl->dataModel));

    EDUtility::setUIEnable(ui, false);

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->addBtn, &QPushButton::clicked, this, &EphemerisData::addItem);
    connect(ui->editBtn, &QPushButton::clicked, this, &EphemerisData::modifyItem);
    connect(ui->delBtn, &QPushButton::clicked, this, &EphemerisData::deleteItem);
    connect(ui->refreshBtn, &QPushButton::clicked, this, &EphemerisData::queryItem);
    connect(ui->tableView, &QTableView::clicked, [=](const QModelIndex& index) {
        auto curSelectData = m_impl->dataModel.getItem(index);
        if (!curSelectData)
        {
            return;
        }
        EDUtility::setUIData(ui, EphemerisDataImpl::getAllSatelliteManagementData(), curSelectData.value());
    });
    connect(ui->tableView, &QTableView::doubleClicked, this, &EphemerisData::modifyItem);

    m_impl->menu = new QMenu(this);
    m_impl->menu->addAction(QString("编辑"), this, &EphemerisData::modifyItem);
    m_impl->menu->addAction(QString("删除"), this, &EphemerisData::deleteItem);

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

void EphemerisData::addItem()
{
    AddEphemerisDataDialog dialog(this);
    dialog.setWindowTitle("添加星历数据");
    dialog.setData(EphemerisDataImpl::getAllSatelliteManagementData());

    if (QDialog::Accepted == dialog.exec())
    {
        queryItem();
        EDUtility::clearUIData(ui);
    }
}

void EphemerisData::modifyItem()
{
    auto curSelectData = m_impl->dataModel.getItem(ui->tableView->currentIndex());
    if (!curSelectData)
    {
        QMessageBox::warning(this, QString("提示"), QString("请选择需要编辑的星历"), QString("确定"));
        return;
    }

    AddEphemerisDataDialog dialog(this);
    dialog.setWindowTitle("编辑星历数据");
    dialog.setData(EphemerisDataImpl::getAllSatelliteManagementData(), curSelectData.value());

    if (QDialog::Accepted == dialog.exec())
    {
        queryItem();
        EDUtility::clearUIData(ui);
    }
}

void EphemerisData::deleteItem()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    auto curSelectData = m_impl->dataModel.getItem(ui->tableView->currentIndex());
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
    EDUtility::clearUIData(ui);
}

void EphemerisData::queryItem()
{
    auto dataList = EphemerisDataImpl::getAllSatelliteManagementData();
    ui->satelliteIdentification->clear();
    ui->satelliteName->clear();
    ui->tableView->clearSelection();
    for (auto& item : dataList)
    {
        ui->satellitCode->addItem(item.m_satelliteCode, item.m_satelliteCode);
        //        ui->satelliteIdentification->setText(item.m_satelliteIdentification);
        //        ui->satelliteName->setText(item.m_satelliteName);
    }
    m_impl->queryItem();
}

void EphemerisData::deleteACK(const QByteArray& data)
{
    auto ack = Optional<EphemerisDataData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::critical(this, "删除失败", ack.msg(), "确定");
        return;
    }

    m_impl->dataModel.deleteItem(ack.value());
    QMessageBox::information(this, "提示", "删除成功", "确定");
    EDUtility::clearUIData(ui);
}
void EphemerisData::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
    queryItem();
    EDUtility::clearUIData(ui);
}
