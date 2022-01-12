#include "SystemLogFastPhotoWidget.h"
#include "ui_SystemLogFastPhotoWidget.h"

#include <QAction>
#include <QMenu>
#include <QToolTip>

#include "MessagePublish.h"
#include "SubscriberHelper.h"
#include "SystemLogMessageSerialize.h"
#include "RealTimeLogModel.h"

SystemLogFastPhotoWidget::SystemLogFastPhotoWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SystemLogFastPhotoWidget)
{
    ui->setupUi(this);

    setMouseTracking(true);
    ui->tableView->setMouseTracking(true);
    init();

    connect(ui->tableView, &QTableView::clicked, this, &SystemLogFastPhotoWidget::slotLogTableClicked);
    setWindowTitle("快照");
   // setWindowIcon(QIcon(":/image/logPhoto.png"));
}

SystemLogFastPhotoWidget::~SystemLogFastPhotoWidget() { delete ui; }

void SystemLogFastPhotoWidget::setFastLogData(QList<AutoTaskLogData> systemLogDatas)
{
    realTimeLogModel->clear();
    realTimeLogModel->setSystemLogData(systemLogDatas);
}

void SystemLogFastPhotoWidget::init()
{
    realTimeLogModel = new RealtimeLogModel(this);
    ui->tableView->setModel(realTimeLogModel);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setHighlightSections(false);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setFrameShape(QFrame::NoFrame);
    ui->tableView->setAlternatingRowColors(true);

    m_menu = new QMenu(this);
    m_menu->addAction(QString("清空数据"), realTimeLogModel, &RealtimeLogModel::clear);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, &QTableView::customContextMenuRequested, this, [=]() {
        QPoint pt = ui->tableView->mapFromGlobal(QCursor::pos());
        int height = ui->tableView->horizontalHeader()->height();
        QPoint pt2(0, height);
        pt -= pt2;

        m_menu->exec(QCursor::pos());
    });
}

void SystemLogFastPhotoWidget::slotLogTableClicked(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }

    QToolTip::showText(QCursor::pos(), index.data().toString());
}
