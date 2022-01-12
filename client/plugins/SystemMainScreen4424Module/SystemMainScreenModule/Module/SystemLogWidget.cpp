#include "SystemLogWidget.h"
#include "ui_SystemLogWidget.h"

#include <QAction>
#include <QMenu>
#include <QToolTip>

#include "MessagePublish.h"
#include "SubscriberHelper.h"
#include "SystemLogMessageSerialize.h"
#include "SystemLogTableModel.h"

SystemLogWidget::SystemLogWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SystemLogWidget)
{
    ui->setupUi(this);

    setMouseTracking(true);
    ui->tableView->setMouseTracking(true);
    init();

    connect(ui->tableView, &QTableView::clicked, this, &SystemLogWidget::slotLogTableClicked);
    connect(SubscriberHelper::getInstance(), &SubscriberHelper::signalSystemLogReadable, this, &SystemLogWidget::slotReadSystemLog);
}

SystemLogWidget::~SystemLogWidget() { delete ui; }

void SystemLogWidget::init()
{
    mSystemLogTableModel = new SystemLogTableModel(this);
    ui->tableView->setModel(mSystemLogTableModel);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setHighlightSections(false);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setFrameShape(QFrame::NoFrame);
    ui->tableView->setAlternatingRowColors(true);

    mMenu = new QMenu(this);
    mMenu->addAction(QString("清空数据"), mSystemLogTableModel, &SystemLogTableModel::clear);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, &QTableView::customContextMenuRequested, this, [=]() {
        QPoint pt = ui->tableView->mapFromGlobal(QCursor::pos());
        int height = ui->tableView->horizontalHeader()->height();
        QPoint pt2(0, height);
        pt -= pt2;

        mMenu->exec(QCursor::pos());
    });
}

void SystemLogWidget::slotReadSystemLog(const SystemLogData& data) { mSystemLogTableModel->prependSystemLogData(data); }

void SystemLogWidget::slotLogTableClicked(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }

    QToolTip::showText(QCursor::pos(), index.data().toString());
}
