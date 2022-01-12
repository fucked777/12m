#include "DeviceStatusLogWidget.h"
#include "ui_DeviceStatusLogWidget.h"

#include "DeviceStatusLogTableModel.h"
#include "GlobalData.h"
#include "PageNavigator.h"
#include "ProtocolXmlTypeDefine.h"
#include "QssCommonHelper.h"
#include "SqlDeviceStatusManager.h"
#include "StatusPersistenceMessageSerialize.h"
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QScrollBar>
#include <QTextStream>

DeviceStatusLogWidget::DeviceStatusLogWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DeviceStatusLogWidget)

{
    ui->setupUi(this);

    // 初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    init();
}

DeviceStatusLogWidget::~DeviceStatusLogWidget() { delete ui; }

void DeviceStatusLogWidget::init()
{
    auto currentTime = GlobalData::currentDateTime();
    ui->startTime->setTime(QTime(8,0,0));
    ui->endTime->setTime(QTime(9,0,0));
    ui->queryDate->setDate(currentTime.date());

    mDeviceStatusLogTableModel = new DeviceStatusLogTableModel(this);
    ui->tableView->setModel(mDeviceStatusLogTableModel);
    connect(mDeviceStatusLogTableModel, &DeviceStatusLogTableModel::signalTotalCount, this, &DeviceStatusLogWidget::slotGetTotalCount);

    m_statusSqlQuery = new SqlDeviceStatusManager();

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setHighlightSections(false);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setFrameShape(QFrame::NoFrame);
    ui->tableView->setAlternatingRowColors(true);

    ui->tableView->setColumnWidth(0, 200);
    ui->tableView->setColumnWidth(1, 200);
    ui->tableView->setColumnWidth(2, 240);
    ui->tableView->setColumnWidth(3, 200);
    ui->tableView->setColumnWidth(4, 200);

    ui->deviceComboBox->addItem("全部设备", -1);
    ui->modeComboBox->addItem("全部模式", -1);
    ui->unitComboBox->addItem("全部单元", -1);

    ui->pagesize->addItem("10", 10);
    ui->pagesize->addItem("20", 20);
    ui->pagesize->addItem("50", 50);

    auto systemMap = GlobalData::getSystemMap();
    for (const auto& system : systemMap)
    {
        for (const auto& device : system.deviceMap)
        {
            for (const auto& exten : device.extensionMap)
            {
                auto deviceID = DeviceID(system.ID, device.ID, exten.ID).toInt();
                auto extenName = QString("%1(%2)").arg(exten.desc).arg(deviceID,0,16);
                ui->deviceComboBox->addItem(extenName, deviceID);
            }
        }
    }

    connect(ui->deviceComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &DeviceStatusLogWidget::slotCurrentDeviceChanged);
    connect(ui->modeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &DeviceStatusLogWidget::slotCurrentModeChanged);
    connect(ui->queryPushButton, &QPushButton::clicked, this, &DeviceStatusLogWidget::query);
    connect(ui->exportPushButton, &QPushButton::clicked, this, &DeviceStatusLogWidget::exportStatus);
    connect(ui->pagesize, &QComboBox::currentTextChanged, this, &DeviceStatusLogWidget::pageSizeChange);

    ui->exportPushButton->setHidden(true);
    d_pageNavigator = new PageNavigator(3, this);
    ui->horizontalLayout_2->addWidget(d_pageNavigator);
    connect(d_pageNavigator, &PageNavigator::currentPageChanged, this, &DeviceStatusLogWidget::currentPageChanged);


}

void DeviceStatusLogWidget::processExport(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::critical(this, "提示", "文件创建失败", "确定");
        return;
    }

    uchar bom[] = { 0xef, 0xbb, 0xbf };
    file.write(reinterpret_cast<char*>(bom), sizeof(bom));

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    auto status_lists = d_dataList;

    for (auto status : status_lists)
    {
        stream << Utility::dateTimeToStr(status.createTime) << QString("     ").toUtf8() << QString("设备：").toUtf8() << status.deviceId.toUtf8()
               << QString("     ").toUtf8() << QString("单元：").toUtf8() << status.unitId.toUtf8() << QString("     ").toUtf8()
               << QString("参数：").toUtf8() << status.param.toUtf8() << QString("     ").toUtf8() << QString("值：").toUtf8()
               << status.paramValue.toUtf8() << '\n';
    }

    file.close();
}

void DeviceStatusLogWidget::slotSearchAck(const QByteArray& data)
{
    QByteArray bak = data;
    DeviceStatusLogDataList dataList;
    bak >> dataList;
    mDeviceStatusLogTableModel->setDeviceStatusLogData(dataList);
}

void DeviceStatusLogWidget::slotCurrentDeviceChanged()
{
    auto deviceId = ui->deviceComboBox->currentData().toString().toInt(nullptr, 16);
    auto extention = GlobalData::getExtension(deviceId);

    ui->modeComboBox->clear();

    if (!extention.haveMode)
    {
        ui->modeComboBox->addItem("全部模式", -1);
        return;
    }

    ui->modeComboBox->addItem("全部模式", -1);
    for (const auto& mode : extention.modeCtrlMap)
    {
        ui->modeComboBox->addItem(mode.desc, mode.ID);
    }
}

void DeviceStatusLogWidget::slotCurrentModeChanged()
{
    auto deviceId = ui->deviceComboBox->currentData().toString().toInt(nullptr, 16);
    auto modeId = ui->modeComboBox->currentData().toInt();

    auto mode = GlobalData::getMode(deviceId, modeId);

    ui->unitComboBox->clear();
    ui->unitComboBox->addItem("全部单元", -1);
    for (const auto& unit : mode.unitMap)
    {
        ui->unitComboBox->addItem(unit.desc, unit.unitCode);
    }
}

void DeviceStatusLogWidget::query()
{
    QTime startTime = ui->startTime->time();
    QTime endTime = ui->endTime->time();
    if (startTime > endTime)
    {
        QMessageBox::warning(this, "提示", "查询开始时间不能晚于结束时间", QString("确定"));
        return;
    }

    QSet<int> deviceSet;
    auto tempData = ui->deviceComboBox->currentData().toInt();
    if (tempData > 0)
    {
        deviceSet << tempData;
    }

    QSet<QString> unitSet;
    auto unitID = ui->unitComboBox->currentData();
    if (unitID.toInt() > 0)
    {
        unitSet << unitID.toString();
    }

    QSet<int> modeSet;
    auto modeID = ui->modeComboBox->currentData().toInt();
    if (modeID > 0)
    {
        modeSet << modeID;
    }

    int pageSize = ui->pagesize->currentData().toInt();
    QDateTime tempTime1(ui->queryDate->date());
    tempTime1.setTime(startTime);

    auto tempTime2 = tempTime1;
    tempTime2.setTime(endTime);
    mDeviceStatusLogTableModel->reset(deviceSet, unitSet,tempTime1,tempTime2, modeSet, 1, pageSize);
}

void DeviceStatusLogWidget::exportStatus()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("导出状态历史数据"));
    if (filename.isEmpty())
    {
        QMessageBox::critical(this, "提示", "文件打开失败", "确定");
        return;
    }

    QMessageBox mb(QMessageBox::Information, tr("保存状态历史数据"), filename, QMessageBox::Ok | QMessageBox::Close);
    if (mb.exec() == QMessageBox::Ok)
    {
        processExport(filename);
    }
}

void DeviceStatusLogWidget::currentPageChanged(int page)
{
    QTime startTime = ui->startTime->time();
    QTime endTime = ui->endTime->time();
    if (startTime > endTime)
    {
        QMessageBox::warning(this, "提示", "查询开始时间不能晚于结束时间", QString("确定"));
        return;
    }

    QSet<int> deviceSet;
    auto tempData = ui->deviceComboBox->currentData().toInt();
    if (tempData > 0)
    {
        deviceSet << tempData;
    }

    QSet<QString> unitSet;
    auto unitID = ui->unitComboBox->currentData();
    if (unitID.toInt() > 0)
    {
        unitSet << unitID.toString();
    }

    QSet<int> modeSet;
    auto modeID = ui->modeComboBox->currentData().toInt();
    if (modeID > 0)
    {
        modeSet << modeID;
    }

    int pageSize = ui->pagesize->currentData().toInt();
    QDateTime tempTime1(ui->queryDate->date());
    tempTime1.setTime(startTime);

    auto tempTime2 = tempTime1;
    tempTime2.setTime(endTime);
    mDeviceStatusLogTableModel->reset(deviceSet, unitSet,tempTime1,tempTime2, modeSet, page, pageSize);
}

void DeviceStatusLogWidget::slotGetTotalCount(const int totalCount, const int currentPage)
{
    int pageSize = ui->pagesize->currentData().toInt();
    int pageNum = totalCount / pageSize + ((totalCount % pageSize) ? 1 : 0);
    d_pageNavigator->setMaxPage(pageNum);
    d_pageNavigator->setCurrentPage(currentPage);
}

void DeviceStatusLogWidget::pageSizeChange(const QString& /*strPage*/) { query(); }

void DeviceStatusLogWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return)
        {
            return;
        }
    }
    QWidget::keyPressEvent(event);
}
