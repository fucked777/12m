#include "SystemLogWidget.h"
#include "ui_SystemLogWidget.h"

#include <QAction>
#include <QMenu>
#include <QToolTip>

#include "MessagePublish.h"
#include "SubscriberHelper.h"
#include "SystemLogFastPhotoWidget.h"
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
    ui->tableView->setVerticalScrollMode(QTableView::ScrollPerPixel);
}

SystemLogWidget::~SystemLogWidget() { delete ui; }

void SystemLogWidget::init()
{
    mSystemLogTableModel = new SystemLogTableModel(this);
    m_fastWidget = new SystemLogFastPhotoWidget;
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
    mMenu->addAction(QString("快照"), this, &SystemLogWidget::slotShowFastPhotoLogWidget);
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

void SystemLogWidget::slotShowFastPhotoLogWidget()
{
    if (m_fastWidget)
    {
        auto currentLogData = mSystemLogTableModel->getCurrentLogData();
        m_fastWidget->setFastLogData(currentLogData);
        if (m_fastWidget->isHidden())
        {
            m_fastWidget->show();
        }
        m_fastWidget->raise();
        m_fastWidget->activateWindow();
    }
}
