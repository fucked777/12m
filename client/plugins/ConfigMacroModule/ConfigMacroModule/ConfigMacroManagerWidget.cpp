#include "ConfigMacroManagerWidget.h"
#include "ui_ConfigMacroManagerWidget.h"

#include "ConfigMacroDialog.h"
#include "ConfigMacroMessageSerialize.h"
#include "ConfigMacroTableModel.h"
#include "GlobalData.h"
#include "QssCommonHelper.h"
#include "ServiceCheck.h"

#include <QMenu>
#include <QMessageBox>

ConfigMacroManagerWidget::ConfigMacroManagerWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ConfigMacroManagerWidget)
{
    ui->setupUi(this);
    // 初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    init();
}

ConfigMacroManagerWidget::~ConfigMacroManagerWidget() { delete ui; }

void ConfigMacroManagerWidget::showEvent(QShowEvent* event)
{
    slotRefreshBtnClicked();
    QWidget::showEvent(event);
}

void ConfigMacroManagerWidget::init()
{
    mConfigMacroTableModel = new ConfigMacroTableModel(this);
    ui->tableView->setModel(mConfigMacroTableModel);

    // 设置表格样式
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setHighlightSections(false);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setFrameShape(QFrame::NoFrame);
    ui->tableView->setAlternatingRowColors(true);
    //    ui->tableView->setColumnWidth(0, 400);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    mConfigMacroDialog = new ConfigMacroDialog(this);
    connect(mConfigMacroDialog, &ConfigMacroDialog::signalDataReady, this, &ConfigMacroManagerWidget::slotConfigMacroDialogDataReady);
    connect(this, &ConfigMacroManagerWidget::signalOperationDialog, mConfigMacroDialog, &ConfigMacroDialog::slotOperationResult);

    connect(ui->addBtn, &QPushButton::clicked, this, &ConfigMacroManagerWidget::slotAddBtnClicked);
    connect(ui->deleteBtn, &QPushButton::clicked, this, &ConfigMacroManagerWidget::slotDeleteBtnClicked);
    connect(ui->refreshBtn, &QPushButton::clicked, this, &ConfigMacroManagerWidget::slotRefreshBtnClicked);
    connect(ui->editBtn, &QPushButton::clicked, this, &ConfigMacroManagerWidget::slotEditBtnClicked);

    connect(ui->tableView, &QTableView::doubleClicked, this, &ConfigMacroManagerWidget::slotEditBtnClicked);

    mMenu = new QMenu(this);
    mMenu->addAction(QString("编辑"), this, &ConfigMacroManagerWidget::slotEditBtnClicked);
    mMenu->addAction(QString("删除"), this, &ConfigMacroManagerWidget::slotDeleteBtnClicked);

    connect(ui->tableView, &QTableView::customContextMenuRequested, this, [=]() {
        QPoint pt = ui->tableView->mapFromGlobal(QCursor::pos());
        int height = ui->tableView->horizontalHeader()->height();
        QPoint pt2(0, height);
        pt -= pt2;
        if (ui->tableView->currentIndex().isValid())
        {
            mMenu->exec(QCursor::pos());
        }
    });
}

bool ConfigMacroManagerWidget::check()
{
    // 判断服务器是否离线
    if (!SERVICEONLINE)
    {
        QMessageBox::information(this, QString("提示"), "当前监控服务器离线", QString("确定"));
        return false;
    }

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return false;
    }

    return true;
}

void ConfigMacroManagerWidget::slotAddItemACK(const QByteArray& data)
{
    auto ack = Optional<ConfigMacroData>::emptyOptional();
    data >> ack;
    emit signalOperationDialog(ack.success(), ack.msg());

    if (!ack)
    {
        return;
    }

    mConfigMacroTableModel->addConfigMacroData(ack.value());
}

void ConfigMacroManagerWidget::slotDeleteItemACK(const QByteArray& data)
{
    auto ack = Optional<ConfigMacroData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::critical(this, "删除失败", ack.msg(), "确定");
        return;
    }

    mConfigMacroTableModel->deleteConfigMacroData(ack.value());
    QMessageBox::information(this, "提示", "删除成功", "确定");
}

void ConfigMacroManagerWidget::slotModifyItemACK(const QByteArray& data)
{
    auto ack = Optional<ConfigMacroData>::emptyOptional();
    data >> ack;
    emit signalOperationDialog(ack.success(), ack.msg());

    if (!ack)
    {
        return;
    }

    mConfigMacroTableModel->updateConfigMacroData(ack.value());
}

void ConfigMacroManagerWidget::slotAddBtnClicked()
{
    mConfigMacroDialog->reset();
    mConfigMacroDialog->setWidgetMode(ConfigMacroDialog::WidgetMode::Add);
    mConfigMacroDialog->exec();
}

void ConfigMacroManagerWidget::slotDeleteBtnClicked()
{
    if (!check())
    {
        return;
    }

    auto curSelectData = mConfigMacroTableModel->getConfigMacroData(ui->tableView->currentIndex());
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
    emit signalDeleteConfigMacro(array);
}

void ConfigMacroManagerWidget::slotRefreshBtnClicked()
{
    ConfigMacroDataList configMacroDataList;

    GlobalData::getConfigMacroData(configMacroDataList);
    mConfigMacroTableModel->setConfigMacroData(configMacroDataList);
}

void ConfigMacroManagerWidget::slotEditBtnClicked()
{
    auto curSelectData = mConfigMacroTableModel->getConfigMacroData(ui->tableView->currentIndex());
    if (!curSelectData)
    {
        QMessageBox::warning(this, QString("提示"), QString("请选择需要编辑的数据"), QString("确定"));
        return;
    }

    mConfigMacroDialog->reset();
    mConfigMacroDialog->setWidgetMode(ConfigMacroDialog::WidgetMode::Edit);
    mConfigMacroDialog->setConfigMacroData(curSelectData.value());
    mConfigMacroDialog->exec();
}

void ConfigMacroManagerWidget::slotConfigMacroDialogDataReady(const ConfigMacroData& configMacroData)
{
    QByteArray data;
    data << configMacroData;

    auto mode = mConfigMacroDialog->getWidgetMode();
    if (mode == ConfigMacroDialog::WidgetMode::Add)
    {
        emit signalAddConfigMacro(data);
    }
    else if (mode == ConfigMacroDialog::WidgetMode::Edit)
    {
        emit signalEditConfigMacro(data);
    }
}
