#include "ui_ParamMacroManagerWidget.h"

#include "AddParamMacroDialog.h"
#include "AddParamMacroWithSatelliteDialog.h"
#include "CopyParamMacroDialog.h"
#include "CppMicroServicesUtility.h"
#include "ParamMacroManagerWidget.h"
#include "ParamMacroTableViewDelegate.h"
#include "ParamMacroTableViewModel.h"
#include "ParamMacroXMLReader.h"
#include "QssCommonHelper.h"
#include "ServiceCheck.h"

#include <QMenu>
#include <QMessageBox>

class ParamMacroManagerImpl
{
public:
    QMenu* menu = nullptr;

    QList<ParamMacroModeInfo> mParamMacroModeInfoList;  // 生成参数宏界面数据

    ParamMacroTableViewModel* mParamMacroTableModel = nullptr;

    //  ParamMacroTableViewDelegate* mParamMacroTableDelegate = nullptr;  // 参数宏的代理Delegate
};
ParamMacroManagerWidget::ParamMacroManagerWidget(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ParamMacroManagerWidget)
    , mImpl(new ParamMacroManagerImpl)
{
    Q_UNUSED(context)
    ui->setupUi(this);
    loadUI();

    // 初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
}

ParamMacroManagerWidget::~ParamMacroManagerWidget()
{
    delete ui;
    delete mImpl;
}

void ParamMacroManagerWidget::loadUI()
{
    ParamMacroXMLReader paramMacroModeXmlReader;
    paramMacroModeXmlReader.loadParamMacroWorksystemXML(mImpl->mParamMacroModeInfoList);

    mImpl->mParamMacroTableModel = new ParamMacroTableViewModel;
    ui->tableView->setModel(mImpl->mParamMacroTableModel);

    // mImpl->mParamMacroTableDelegate = new ParamMacroTableViewDelegate(this);
    //    ui->tableView->setItemDelegateForColumn(4, mImpl->mParamMacroTableDelegate);

    emit mImpl->mParamMacroTableModel->layoutChanged();

    // 设置单行选中、表头不高亮、无边框等
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->horizontalHeader()->setHighlightSections(false);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setFrameShape(QFrame::NoFrame);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->tableView, &QTableView::doubleClicked, this, &ParamMacroManagerWidget::slotShowBtnClicked);
    //    connect(mImpl->mParamMacroTableDelegate, &ParamMacroTableViewDelegate::signal_show, this, &ParamMacroManagerWidget::slotShowBtnClicked);
    //    connect(mImpl->mParamMacroTableDelegate, &ParamMacroTableViewDelegate::signal_edit, this, &ParamMacroManagerWidget::slotEditBtnClicked);
    //    connect(mImpl->mParamMacroTableDelegate, &ParamMacroTableViewDelegate::signal_del, this, &ParamMacroManagerWidget::slotDeleteBtnClicked);

    connect(ui->addBtn, &QPushButton::clicked, this, &ParamMacroManagerWidget::slotAddBtnClicked);
    connect(ui->deleteBtn, &QPushButton::clicked, this, &ParamMacroManagerWidget::slotDeleteBtnClicked);
    connect(ui->editBtn, &QPushButton::clicked, this, &ParamMacroManagerWidget::slotEditBtnClicked);
    connect(ui->refreshBtn, &QPushButton::clicked, this, &ParamMacroManagerWidget::slotRefreshBtnClicked);
    connect(ui->copyBtn, &QPushButton::clicked, this, &ParamMacroManagerWidget::slotCopyBtnClicked);

    mImpl->menu = new QMenu(this);
    mImpl->menu->addAction(QString("查看"), this, &ParamMacroManagerWidget::slotShowBtnClicked);
    mImpl->menu->addAction(QString("编辑"), this, &ParamMacroManagerWidget::slotEditBtnClicked);
    mImpl->menu->addAction(QString("删除"), this, &ParamMacroManagerWidget::slotDeleteBtnClicked);
    mImpl->menu->addAction(QString("复制"), this, &ParamMacroManagerWidget::slotCopyBtnClicked);

    connect(ui->tableView, &QTableView::customContextMenuRequested, this, [=]() {
        QPoint pt = ui->tableView->mapFromGlobal(QCursor::pos());
        int height = ui->tableView->horizontalHeader()->height();
        QPoint pt2(0, height);
        pt -= pt2;
        if (ui->tableView->currentIndex().isValid())
        {
            mImpl->menu->exec(QCursor::pos());
        }
    });
}

bool ParamMacroManagerWidget::check()
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

void ParamMacroManagerWidget::slotAddItemACK(const QByteArray& data)
{
    auto ack = Optional<ParamMacroData>::emptyOptional();
    data >> ack;
    emit signalOperationDialog(ack.success(), ack.msg());

    if (!ack)
    {
        return;
    }

    mImpl->mParamMacroTableModel->addParamMacroData(ack.value());
}

void ParamMacroManagerWidget::slotDeleteItemACK(const QByteArray& data)
{
    auto ack = Optional<ParamMacroData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::critical(this, "删除失败", ack.msg(), "确定");
        return;
    }

    mImpl->mParamMacroTableModel->deleteParamMacroData(ack.value());
    QMessageBox::information(this, "提示", "删除成功", "确定");
}

void ParamMacroManagerWidget::slotModifyItemACK(const QByteArray& data)
{
    auto ack = Optional<ParamMacroData>::emptyOptional();
    data >> ack;
    emit signalOperationDialog(ack.success(), ack.msg());

    if (!ack)
    {
        return;
    }

    mImpl->mParamMacroTableModel->updateParamMacroData(ack.value());
}

void ParamMacroManagerWidget::slotAddBtnClicked()
{
    AddParamMacroWithSatelliteDialog satelliteDialog;
    if (satelliteDialog.exec())
    {
        AddParamMacroDialog dialog;

        dialog.setParamMacroModeInfoList(mImpl->mParamMacroModeInfoList);

        dialog.setWidgetMode(AddParamMacroDialog::Add);

        auto paramMacroData = satelliteDialog.getCurrentParamMacro();
        dialog.setParamMacroData(paramMacroData);
        dialog.setWindowTitle(QString("添加参数宏数据--%1").arg(paramMacroData.taskCode));

        connect(&dialog, &AddParamMacroDialog::signalAddParamMacro, this, &ParamMacroManagerWidget::signalAddParamMacro);
        connect(&dialog, &AddParamMacroDialog::signalEditParamMacro, this, &ParamMacroManagerWidget::signalEditParamMacro);
        connect(this, &ParamMacroManagerWidget::signalOperationDialog, &dialog, &AddParamMacroDialog::slotOperationResult);

        dialog.initUI();
        dialog.exec();
    }
}

void ParamMacroManagerWidget::slotShowBtnClicked()
{
    auto curSelectDataResult = mImpl->mParamMacroTableModel->getParamMacroData(ui->tableView->currentIndex());
    if (!curSelectDataResult)
    {
        QMessageBox::warning(this, QString("提示"), QString("请选择需要查看的数据"), QString("确定"));
        {
            return;
        }
    }
    auto curSelectData = curSelectDataResult.value();

    AddParamMacroDialog dialog;
    dialog.setWindowTitle(QString("查看参数宏数据--%1").arg(curSelectData.taskCode));

    dialog.setWidgetMode(AddParamMacroDialog::Look);
    dialog.setParamMacroModeInfoList(mImpl->mParamMacroModeInfoList);
    dialog.setParamMacroData(curSelectData);

    connect(&dialog, &AddParamMacroDialog::signalAddParamMacro, this, &ParamMacroManagerWidget::signalAddParamMacro);
    connect(&dialog, &AddParamMacroDialog::signalEditParamMacro, this, &ParamMacroManagerWidget::signalEditParamMacro);
    connect(this, &ParamMacroManagerWidget::signalOperationDialog, &dialog, &AddParamMacroDialog::slotOperationResult);

    dialog.initUI();

    dialog.exec();
}

void ParamMacroManagerWidget::slotDeleteBtnClicked()
{
    if (!check())
    {
        return;
    }

    auto curSelectData = mImpl->mParamMacroTableModel->getParamMacroData(ui->tableView->currentIndex());
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
    emit signalDelParamMacro(array);
}

void ParamMacroManagerWidget::slotEditBtnClicked()
{
    if (!check())
    {
        return;
    }

    auto curSelectDataResult = mImpl->mParamMacroTableModel->getParamMacroData(ui->tableView->currentIndex());
    if (!curSelectDataResult)
    {
        QMessageBox::warning(this, QString("提示"), QString("请选择需要编辑的数据"), QString("确定"));
        return;
    }
    auto curSelectData = curSelectDataResult.value();

    AddParamMacroDialog dialog;
    dialog.setWindowTitle(QString("编辑参数宏数据--%1").arg(curSelectData.taskCode));
    dialog.setParamMacroModeInfoList(mImpl->mParamMacroModeInfoList);

    dialog.setWidgetMode(AddParamMacroDialog::Edit);
    dialog.setParamMacroData(curSelectData);

    connect(&dialog, &AddParamMacroDialog::signalAddParamMacro, this, &ParamMacroManagerWidget::signalAddParamMacro);
    connect(&dialog, &AddParamMacroDialog::signalEditParamMacro, this, &ParamMacroManagerWidget::signalEditParamMacro);
    connect(this, &ParamMacroManagerWidget::signalOperationDialog, &dialog, &AddParamMacroDialog::slotOperationResult);

    dialog.initUI();
    dialog.exec();
}

void ParamMacroManagerWidget::slotRefreshBtnClicked()
{
    ParamMacroDataList paramMacroDataList;

    GlobalData::getParamMacroData(paramMacroDataList);
    mImpl->mParamMacroTableModel->setParamMacroData(paramMacroDataList);
}

void ParamMacroManagerWidget::slotCopyBtnClicked()
{
    if (!check())
    {
        return;
    }

    CopyParamMacroDialog copyDialog;
    if (copyDialog.exec() == QDialog::Accepted)
    {
        auto curParamMacroData = copyDialog.getCopyParamMacroData();
        AddParamMacroDialog dialog;
        dialog.setWindowTitle(QString("复制参数宏数据--%1").arg(curParamMacroData.taskCode));
        dialog.setParamMacroModeInfoList(mImpl->mParamMacroModeInfoList);

        // 获取当前复制模式
        auto currentCopyMode = copyDialog.getCopyMode();
        if (currentCopyMode == CopyMode::NewParamMacro)
        {
            dialog.setWidgetMode(AddParamMacroDialog::Add);
        }
        else
        {
            dialog.setWidgetMode(AddParamMacroDialog::Edit);
        }

        dialog.setParamMacroData(curParamMacroData);

        connect(&dialog, &AddParamMacroDialog::signalAddParamMacro, this, &ParamMacroManagerWidget::signalAddParamMacro);
        connect(&dialog, &AddParamMacroDialog::signalEditParamMacro, this, &ParamMacroManagerWidget::signalEditParamMacro);
        connect(this, &ParamMacroManagerWidget::signalOperationDialog, &dialog, &AddParamMacroDialog::slotOperationResult);

        dialog.initUI();
        dialog.exec();
    }
}

void ParamMacroManagerWidget::showEvent(QShowEvent*) { slotRefreshBtnClicked(); }
