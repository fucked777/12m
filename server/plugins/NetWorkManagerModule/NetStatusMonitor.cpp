#include "NetStatusMonitor.h"
#include "CommunicationSerialize.h"
#include "GlobalData.h"
#include "NetStatusDataModel.h"
#include "NetStatusInterface.h"
#include "Utility.h"
#include "ui_NetStatusMonitor.h"
class NetStatusMonitorImpl
{
public:
    NetStatusDataModel dataModel;
    QModelIndex curSelectIndex;
    QList<NetStatusDataInfo> records;
};

NetStatusMonitor::NetStatusMonitor(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::NetStatusMonitor)
    , m_impl(new NetStatusMonitorImpl)
{
    ui->setupUi(this);
    Utility::tableViewInit(ui->statusView);
    ui->statusView->setModel(&(m_impl->dataModel));
    //    ui->statusView->setColumnWidth(1, 240);
    //    ui->statusView->setColumnWidth(5, 80);
    //    ui->statusView->setColumnWidth(6, 80);
    //    ui->statusView->setColumnWidth(7, 100);
    //    ui->statusView->setColumnWidth(8, 120);
    //    ui->statusView->setColumnWidth(10, 100);
    //    ui->statusView->setColumnWidth(11, 120);

    ui->statusView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->statusView->horizontalHeader()->setHighlightSections(false);
    ui->statusView->verticalHeader()->setVisible(false);
    ui->statusView->setFrameShape(QFrame::NoFrame);
    ui->statusView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->statusView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->statusView->setAlternatingRowColors(true);

    resetUI();
    connect(ui->statusView, &QTableView::clicked, this, &NetStatusMonitor::showDataChannel);
    startTimer(1000);
}

NetStatusMonitor::~NetStatusMonitor()
{
    delete ui;
    delete m_impl;
}
void NetStatusMonitor::resetUI()
{
    /* 重新加载通道数据信息 */
    auto infoMap = GlobalData::communicationInfo();
    m_impl->dataModel.resetCommunicationInfo(infoMap);
    m_impl->records = m_impl->dataModel.getRecords();
}
void NetStatusMonitor::showDataChannel(const QModelIndex& index)
{
    m_impl->curSelectIndex = index;
    NetStatusDataInfo info;
    if (!m_impl->dataModel.curDataInfo(index, info))
    {
        return;
    }

    ui->recvData->setText(info.recvData.toHex());
    ui->sendData->setText(info.sendData.toHex());
}

void NetStatusMonitor::channelRecvDataChange(const NetStatusItemInfo& info)
{
    /* info.index 直接映射过来就是行 */
    if (info.index < 0 || info.index >= m_impl->records.size())
    {
        return;
    }
    // beginResetModel();
    auto& item = m_impl->records[info.index];
    item.recvCount = QString::number(info.count);
    item.recvFailedCount = QString::number(info.failCount);
    item.recvState = info.state;
    item.recvData = info.data;

    // m_impl->dataModel.updateRecvData(info);
    // ui->statusView->setCurrentIndex(m_impl->curSelectIndex);
}
void NetStatusMonitor::channelSendDataChange(const NetStatusItemInfo& info)
{
    /* info.index 直接映射过来就是行 */
    if (info.index < 0 || info.index >= m_impl->records.size())
    {
        return;
    }
    // beginResetModel();
    auto& item = m_impl->records[info.index];
    item.sendCount = QString::number(info.count);
    item.sendFailedCount = QString::number(info.failCount);
    item.sendState = info.state;
    item.sendData = info.data;
    // m_impl->dataModel.updateSendData(info);
    // ui->statusView->setCurrentIndex(m_impl->curSelectIndex);
}

void NetStatusMonitor::timerEvent(QTimerEvent* /*event*/)
{
    if (!isHidden())
    {
        m_impl->dataModel.updateByTimer(m_impl->records);
        ui->statusView->setCurrentIndex(m_impl->curSelectIndex);
    }
}
