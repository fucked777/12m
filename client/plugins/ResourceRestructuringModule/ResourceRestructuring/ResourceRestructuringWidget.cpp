#include "ResourceRestructuringWidget.h"
#include "Utility.h"
#include "ui_ResourceRestructuringWidget.h"
//#include <CppMicroServices>
#include "CppMicroServicesUtility.h"
#include "Model/ResourceRestructuringTableModel.h"
#include "QssCommonHelper.h"
#include "ResourceRestructuringDialog.h"
#include "ResourceRestructuringMessageDefine.h"
#include "ResourceRestructuringMessageSerialize.h"
#include "ServiceCheck.h"
#include <QDebug>
#include <QMap>
#include <QMenu>
#include <QMessageBox>

ResourceRestructuringWidget::ResourceRestructuringWidget(cppmicroservices::BundleContext /*context*/, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ResourceRestructuringWidget)
{
    ui->setupUi(this);

    // 初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
    init();
}

ResourceRestructuringWidget::~ResourceRestructuringWidget() { delete ui; }

void ResourceRestructuringWidget::showEvent(QShowEvent* event) { Q_UNUSED(event); }

void ResourceRestructuringWidget::timerEvent(QTimerEvent* event)
{
    //    if (m_isInit)
    //    {
    //        killTimer(m_timerId);
    //        m_timerId = -1;
    //        return;
    //    }
}

void ResourceRestructuringWidget::init()
{
    mTableModel = new ResourceRestructuringTableModel(this);
    ui->resourceTableView->setModel(mTableModel);

    // 设置表格样式
    ui->resourceTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->resourceTableView->horizontalHeader()->setStretchLastSection(true);
    ui->resourceTableView->horizontalHeader()->setHighlightSections(false);
    ui->resourceTableView->verticalHeader()->setVisible(false);
    ui->resourceTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->resourceTableView->setFrameShape(QFrame::NoFrame);
    ui->resourceTableView->setAlternatingRowColors(true);
    ui->resourceTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->resourceTableView->setContextMenuPolicy(Qt::CustomContextMenu);

    mDialog = new ResourceRestructuringDialog(this);
    connect(mDialog, &ResourceRestructuringDialog::signalDataReady, this, &ResourceRestructuringWidget::slotDialogDataReady);
    connect(this, &ResourceRestructuringWidget::signalOperationDialog, mDialog, &ResourceRestructuringDialog::slotOperationResult);

    connect(ui->save_btn, &QPushButton::clicked, this, &ResourceRestructuringWidget::slotAddBtnClicked);
    connect(ui->del_btn, &QPushButton::clicked, this, &ResourceRestructuringWidget::slotDeleteBtnClicked);
    connect(ui->refresh_btn, &QPushButton::clicked, this, &ResourceRestructuringWidget::slotRefreshBtnClicked);
    connect(ui->edit_btn, &QPushButton::clicked, this, &ResourceRestructuringWidget::slotEditBtnClicked);
    connect(ui->resourceTableView, &QTableView::clicked, this, &ResourceRestructuringWidget::on_resourceTableView_changed);

    ui->mainStandby->addItem(QString("主"), static_cast<int>(MasterSlave::Master));
    ui->mainStandby->addItem(QString("备"), static_cast<int>(MasterSlave::Slave));

    auto workModeNameMap = mDialog->getWorkModeNameMap();
    for (auto workMode : workModeNameMap.keys())
    {
        ui->workMode->addItem(workModeNameMap[workMode], workMode);
    }

    ui->param_groupBox->setEnabled(false);
    ui->tabwidget->setEnabled(false);

    connect(ui->resourceTableView, &QTableView::doubleClicked, this, &ResourceRestructuringWidget::slotEditBtnClicked);

    mMenu = new QMenu(this);
    mMenu->addAction(QString("编辑"), this, &ResourceRestructuringWidget::slotEditBtnClicked);
    mMenu->addAction(QString("删除"), this, &ResourceRestructuringWidget::slotDeleteBtnClicked);

    connect(ui->resourceTableView, &QTableView::customContextMenuRequested, this, [=]() {
        QPoint pt = ui->resourceTableView->mapFromGlobal(QCursor::pos());
        int height = ui->resourceTableView->horizontalHeader()->height();
        QPoint pt2(0, height);
        pt -= pt2;
        if (ui->resourceTableView->currentIndex().isValid())
        {
            mMenu->exec(QCursor::pos());
        }
    });
}

void ResourceRestructuringWidget::slotAddItemACK(const QByteArray& data)
{
    auto ack = Optional<ResourceRestructuringData>::emptyOptional();
    data >> ack;
    emit signalOperationDialog(ack.success(), ack.msg());

    if (!ack)
    {
        return;
    }

    mTableModel->addData(ack.value());
}

void ResourceRestructuringWidget::slotDeleteItemACK(const QByteArray& data)
{
    auto ack = Optional<ResourceRestructuringData>::emptyOptional();
    data >> ack;

    if (ack.success())
    {
        QMessageBox::information(this, "提示", "删除成功", "确定");
        mTableModel->deleteData(ack.value());
    }
    else
    {
        QMessageBox::critical(this, "删除失败", ack.msg(), "确定");
    }
}

void ResourceRestructuringWidget::slotQueryItemACK(const QByteArray& data)
{
    SERVICEONLINECHECK();

    auto ack = Optional<QList<ResourceRestructuringData>>::emptyOptional();
    data >> ack;
    mTableModel->setDatas(ack.value());
}

void ResourceRestructuringWidget::slotModifyItemACK(const QByteArray& data)
{
    auto ack = Optional<ResourceRestructuringData>::emptyOptional();
    data >> ack;
    emit signalOperationDialog(ack.success(), ack.msg());

    if (!ack)
    {
        return;
    }

    mTableModel->updateData(ack.value());
}

void ResourceRestructuringWidget::slotAddBtnClicked()
{
    mDialog->reset();
    mDialog->setMode(ResourceRestructuringDialog::Mode::Add);
    mDialog->exec();
}

void ResourceRestructuringWidget::slotDeleteBtnClicked()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    auto curSelectData = mTableModel->getData(ui->resourceTableView->currentIndex());
    if (!curSelectData)
    {
        QMessageBox::warning(this, QString("提示"), QString("请选择需要删除的数据"), QString("确定"));
        {
            return;
        }
    }

    if (QMessageBox::information(this, "提示", "您确定要删除数据吗?", QString("确定"), QString("取消")) != 0)
    {
        return;
    }

    QByteArray array;
    array << curSelectData.value();
    emit signalDelete(array);
}

void ResourceRestructuringWidget::slotRefreshBtnClicked()
{
    SERVICEONLINECHECK();

    QByteArray array;
    emit signalQuery(array);
}

void ResourceRestructuringWidget::slotEditBtnClicked()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    auto row = ui->resourceTableView->currentIndex().row();
    if (row == -1)
    {
        QMessageBox::warning(this, QString("提示"), QString("请选择需要编辑的配置"), QString("确定"));
        return;
    }

    auto curSelectData = mTableModel->getData(ui->resourceTableView->currentIndex());

    mDialog->setMode(ResourceRestructuringDialog::Mode::Edit);
    mDialog->setData(curSelectData.value());
    mDialog->exec();
}

void ResourceRestructuringWidget::on_resourceTableView_changed(const QModelIndex& index)
{
    auto row = index.row();
    if (row == -1)
    {
        return;
    }

    ResourceRestructuringData data = mTableModel->getData(index).value();
    auto workModeName = mDialog->getWorkModeName(data.workMode);

    ui->name->setText(data.name);
    ui->workMode->setCurrentText(workModeName);
    ui->mainStandby->setCurrentIndex((data.isMaster == MasterSlave::Master) ? 0 : 1);
    ui->desc->setPlainText(data.desc);
    mDialog->setSubData(data);
    ui->tabwidget->clear();
    QTreeWidget* treeWidget = mDialog->getTreeWidget(data.workMode);
    if (treeWidget != nullptr)
    {
        treeWidget->clearFocus();
        ui->tabwidget->addTab(treeWidget, workModeName);
    }
}

void ResourceRestructuringWidget::slotDialogDataReady(const ResourceRestructuringData& resourceRestructuringData)
{
    QByteArray data;
    data << resourceRestructuringData;

    auto mode = mDialog->getMode();
    if (mode == ResourceRestructuringDialog::Mode::Add)
    {
        emit signalAdd(data);
    }
    else if (mode == ResourceRestructuringDialog::Mode::Edit)
    {
        emit signalEdit(data);
    }
}

void ResourceRestructuringWidget::on_desc_textChanged()
{
    QString textContent = ui->desc->toPlainText();
    int length = textContent.count();
    int maxLength = 40;  // 最大字符数
    if (length > maxLength)
    {
        ui->desc->setPlainText(textContent.mid(0, maxLength));
    }
}
