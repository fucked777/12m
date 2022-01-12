#include "TaskCenter.h"
#include "AddTaskCenterDialog.h"
#include "CppMicroServicesUtility.h"
#include "GlobalData.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "TCUtility.h"
#include "TaskCenterSerialize.h"
#include "Utility.h"
#include "ui_TaskCenter.h"
#include <QMessageBox>

#include <QDebug>
class TaskCenterImpl
{
public:
    cppmicroservices::BundleContext context;
    TaskCenterModel dataModel;
    QMenu* menu{ nullptr };

    static TaskCenterMap getAllData()
    {
        TaskCenterMap resultMap;
        GlobalData::getTaskCenterInfo(resultMap);
        return resultMap;
    }
    void queryItem()
    {
        auto data = TaskCenterImpl::getAllData();
        dataModel.updateData(data);
    }
};

TaskCenter::TaskCenter(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TaskCenter)
    , m_impl(new TaskCenterImpl)
{
    ui->setupUi(this);

    m_impl->context = context;

    qRegisterMetaType<TaskCenterData>("TaskCenterData&");
    qRegisterMetaType<TaskCenterData>("TaskCenterData");
    qRegisterMetaType<TaskCenterData>("const TaskCenterData&");
    //初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    init();
}

TaskCenter::~TaskCenter()
{
    delete ui;
    delete m_impl;
}

void TaskCenter::init()
{
    TCUtility::uiInit(ui);
    Utility::tableViewInit(ui->tableView, true);
    ui->tableView->setModel(&(m_impl->dataModel));

    TCUtility::setUIEnable(ui, false);

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->addBtn, &QPushButton::clicked, this, &TaskCenter::addItem);
    connect(ui->editBtn, &QPushButton::clicked, this, &TaskCenter::modifyItem);
    connect(ui->delBtn, &QPushButton::clicked, this, &TaskCenter::deleteItem);
    connect(ui->refreshBtn, &QPushButton::clicked, this, &TaskCenter::queryItem);
    connect(ui->tableView, &QTableView::clicked, [=](const QModelIndex& index) {
        auto curSelectData = m_impl->dataModel.getDataCenterItem(index);
        if (!curSelectData)
        {
            return;
        }
        TCUtility::setUIData(ui, curSelectData.value());
    });
    connect(ui->tableView, &QTableView::doubleClicked, this, &TaskCenter::modifyItem);

    m_impl->menu = new QMenu(this);
    m_impl->menu->addAction(QString("编辑"), this, &TaskCenter::modifyItem);
    m_impl->menu->addAction(QString("删除"), this, &TaskCenter::deleteItem);

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

void TaskCenter::addItem()
{
    AddTaskCenterDialog dialog(this);
    dialog.setWindowTitle("添加任务中心数据");
    if (QDialog::Accepted == dialog.exec())
    {
        m_impl->queryItem();
        TCUtility::clearUIData(ui);
    }
}

void TaskCenter::modifyItem()
{
    auto curSelectData = m_impl->dataModel.getDataCenterItem(ui->tableView->currentIndex());
    if (!curSelectData)
    {
        QMessageBox::warning(this, QString("提示"), QString("请选择需要编辑的任务中心"), QString("确定"));
        return;
    }

    AddTaskCenterDialog dialog(this);
    dialog.setWindowTitle("编辑任务中心数据");
    dialog.setData(curSelectData.value());

    if (QDialog::Accepted == dialog.exec())
    {
        m_impl->queryItem();
        TCUtility::clearUIData(ui);
    }
}

void TaskCenter::deleteItem()
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

void TaskCenter::queryItem() { m_impl->queryItem(); }

void TaskCenter::deleteACK(const QByteArray& data)
{
    auto ack = Optional<TaskCenterData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::critical(this, "删除失败", ack.msg(), "确定");
        return;
    }

    m_impl->dataModel.deleteItem(ack.value());
    QMessageBox::information(this, "提示", "删除成功", "确定");
    TCUtility::clearUIData(ui);
}
void TaskCenter::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
    m_impl->queryItem();
    TCUtility::clearUIData(ui);
}
