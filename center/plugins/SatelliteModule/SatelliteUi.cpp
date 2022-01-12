#include "SatelliteUi.h"
#include "AddSatelliteDialog.h"
#include "CppMicroServicesUtility.h"
#include "QssCommonHelper.h"
#include "SatelliteTableViewDelegate.h"
#include "SatelliteTableViewModel.h"
#include "SatelliteXMLReader.h"
#include "ServiceCheck.h"
#include "ui_SatelliteUi.h"
#include <QMenu>
#include <QMessageBox>

class SatelliteConfigImpl
{
public:
    SatelliteConfigImpl() {}

    QMenu* menu = nullptr;

    QList<SalliteModeInfo> m_satelliteDataList;

    //卫星的model
    SatelliteTableViewModel* m_model = nullptr;
    //卫星的代理Delegate
    // SatelliteTableViewDelegate* m_delegate = nullptr;
    //当前的数据
    QList<SatelliteManagementData> m_currentDataList;
    //工作模式对应的名称
    QMap<int, QString> m_workModeMap;
    //最大的模式组合
    int m_maxWorkMode = 0;
    //当前站是什么站
    int m_stationBz = 0;

    inline bool updateSatelliteData(SatelliteManagementData request)
    {
        for (auto i = 0; i < m_currentDataList.size(); i++)
        {
            if (request.m_satelliteCode == m_currentDataList[i].m_satelliteCode)
            {
                m_currentDataList[i] = request;
                return true;
            }
        }

        return false;
    }
    inline bool removeSatelliteData(SatelliteManagementData request)
    {
        for (auto i = 0; i < m_currentDataList.size(); i++)
        {
            if (request.m_satelliteCode == m_currentDataList[i].m_satelliteCode)
            {
                m_currentDataList.removeAt(i);
                return true;
            }
        }

        return false;
    }
};
SatelliteUi::SatelliteUi(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SatelliteUi)
    , m_impl(new SatelliteConfigImpl)
{
    Q_UNUSED(context)
    ui->setupUi(this);
    loadUI();
}

SatelliteUi::~SatelliteUi() { delete ui; }

void SatelliteUi::loadUI()
{
    initStyleQss();
    SalliteModeXMLReader salliteModeXmlReader;

    salliteModeXmlReader.loadConfigMacroWorksystemXML(m_impl->m_satelliteDataList, m_impl->m_maxWorkMode, m_impl->m_stationBz);
    for (auto item : m_impl->m_satelliteDataList)
    {
        m_impl->m_workModeMap[item.m_modeID] = item.m_modeName;
    }

    m_impl->m_model = new SatelliteTableViewModel;
    ui->tableView->setModel(m_impl->m_model);
    QStringList headers;

    headers << "任务代号"
            << "卫星标识"
            << "卫星名称"
            << "任务中心"
            << "数传中心"
            << "轨道类型"
            << "卫星跟踪方式"
            << "工作模式";
    m_impl->m_model->setHorizontalHeader(headers);
    m_impl->m_model->setWorkModeMap(m_impl->m_workModeMap);

    //    m_impl->m_delegate = new SatelliteTableViewDelegate(this);
    //    ui->tableView->setItemDelegateForColumn(8, m_impl->m_delegate);

    emit m_impl->m_model->layoutChanged();

    // 设置单行选中、表头不高亮、无边框等
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->horizontalHeader()->setHighlightSections(false);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setFrameShape(QFrame::NoFrame);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    //  ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->setColumnWidth(0, 171);
    ui->tableView->setColumnWidth(1, 171);
    ui->tableView->setColumnWidth(2, 171);
    ui->tableView->setColumnWidth(3, 171);
    ui->tableView->setColumnWidth(4, 171);
    ui->tableView->setColumnWidth(5, 171);
    ui->tableView->setColumnWidth(6, 171);
    ui->tableView->setColumnWidth(7, 420);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    //    connect(m_impl->m_delegate, &SatelliteTableViewDelegate::signal_show, [&](QModelIndex index) {
    //        auto satellite = m_impl->m_model->SatelliteData(index);
    //        if (!satellite.m_satelliteCode.isEmpty())
    //        {
    //            AddSatelliteDialog dialog;
    //            dialog.setMaxWorkMode(m_impl->m_maxWorkMode);
    //            dialog.setStationBz(m_impl->m_stationBz);
    //            dialog.setSatelliteDataList(m_impl->m_satelliteDataList);
    //            dialog.initUI();
    //            dialog.setCurrentUIMode(AddSatelliteDialog::SHOW);

    //            dialog.updateUI(satellite);
    //            if (dialog.exec() == QDialog::Accepted) {}
    //        }
    //    });

    //    connect(m_impl->m_delegate, &SatelliteTableViewDelegate::signal_edit, [&](QModelIndex index) {
    //        SERVICEONLINECHECK();  //判断服务器是否离线

    //        QString errorMsg;
    //        if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    //        {
    //            QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
    //            return;
    //        }

    //        auto satellite = m_impl->m_model->SatelliteData(index);
    //        if (!satellite.m_satelliteCode.isEmpty())
    //        {
    //            AddSatelliteDialog dialog;
    //            dialog.setMaxWorkMode(m_impl->m_maxWorkMode);
    //            dialog.setStationBz(m_impl->m_stationBz);
    //            dialog.setSatelliteDataList(m_impl->m_satelliteDataList);
    //            dialog.initUI();
    //            dialog.setCurrentUIMode(AddSatelliteDialog::EDIT);
    //            dialog.updateUI(satellite);

    //            connect(&dialog, &AddSatelliteDialog::signalAddSatellite, this, &SatelliteUi::signalAddSatellite);
    //            connect(&dialog, &AddSatelliteDialog::signalEditSatellite, this, &SatelliteUi::signalEditSatellite);
    //            connect(this, &SatelliteUi::signalAddSatelliteResponce, &dialog, &AddSatelliteDialog::slotAddSatelliteResponce);
    //            connect(this, &SatelliteUi::signalEditSatelliteResponce, &dialog, &AddSatelliteDialog::slotEditSatelliteResponce);

    //            if (dialog.exec())
    //            {
    //                auto data = dialog.currentsatelliteData();

    //                m_impl->updateSatelliteData(data);
    //                m_impl->m_model->setSatelliteData(m_impl->m_currentDataList);
    //            }
    //        }
    //    });

    //    connect(m_impl->m_delegate, &SatelliteTableViewDelegate::signal_del, [&](QModelIndex index) {
    //        SERVICEONLINECHECK();  //判断服务器是否离线

    //        QString errorMsg;
    //        if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    //        {
    //            QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
    //            return;
    //        }

    //        auto satellite = m_impl->m_model->SatelliteData(index);
    //        if (!satellite.m_satelliteCode.isEmpty())
    //        {
    //            if (!QMessageBox::information(this, "提示", "是否删除当前选择的卫星", "确定", "取消"))
    //            {
    //                QByteArray json;
    //                json << satellite;
    //                emit signalDelSatellite(json);
    //            }
    //        }
    //    });

    connect(ui->tableView, &QTableView::doubleClicked, this, &SatelliteUi::on_pushButton_edit_clicked);

    m_impl->menu = new QMenu(this);
    m_impl->menu->addAction(QString("编辑"), this, &SatelliteUi::on_pushButton_edit_clicked);
    m_impl->menu->addAction(QString("删除"), this, &SatelliteUi::on_pushButton_delete_clicked);

    connect(ui->tableView, &QTableView::customContextMenuRequested, this, [=]() {
        QPoint pt = ui->tableView->mapFromGlobal(QCursor::pos());
        int height = ui->tableView->horizontalHeader()->height();
        QPoint pt2(0, height);
        pt -= pt2;
        if (ui->tableView->currentIndex().isValid())
        {
            m_impl->menu->exec(QCursor::pos());
        }
    });
}

void SatelliteUi::slotDelSatelliteResponce(bool flag, SatelliteManagementData request)
{
    Q_UNUSED(flag);

    if (m_impl->removeSatelliteData(request))
    {
        m_impl->m_model->setSatelliteData(m_impl->m_currentDataList);
        QMessageBox::information(this, "提示", "删除卫星成功", "确定");
    }
    else
    {
        //如果本地删除失败就刷新一个表格
        on_pushButton_refresh_clicked();
    }
}

void SatelliteUi::slotAllSatelliteResponce(bool flag, QList<SatelliteManagementData> requestList)
{
    Q_UNUSED(flag);
    m_impl->m_model->setSatelliteData(requestList);
    m_impl->m_currentDataList = requestList;
}

void SatelliteUi::on_pushButton_add_clicked()
{
    AddSatelliteDialog dialog;
    dialog.setMaxWorkMode(m_impl->m_maxWorkMode);
    dialog.setStationBz(m_impl->m_stationBz);
    dialog.setWindowTitle("添加卫星数据");
    dialog.setSatelliteDataList(m_impl->m_satelliteDataList);

    dialog.setCurrentUIMode(AddSatelliteDialog::ADD);

    connect(&dialog, &AddSatelliteDialog::signalAddSatellite, this, &SatelliteUi::signalAddSatellite);
    connect(&dialog, &AddSatelliteDialog::signalEditSatellite, this, &SatelliteUi::signalEditSatellite);
    connect(this, &SatelliteUi::signalAddSatelliteResponce, &dialog, &AddSatelliteDialog::slotAddSatelliteResponce);
    connect(this, &SatelliteUi::signalEditSatelliteResponce, &dialog, &AddSatelliteDialog::slotEditSatelliteResponce);

    dialog.initUI();
    if (dialog.exec())
    {
        auto data = dialog.currentsatelliteData();

        m_impl->m_currentDataList.append(data);
        m_impl->m_model->setSatelliteData(m_impl->m_currentDataList);
    }
}

void SatelliteUi::on_pushButton_refresh_clicked()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    AllSatelliteMessageRequest request;

    JsonWriter writer;
    writer& request;
    auto json = writer.GetByteArray();

    emit signalGetAllSatellite(json);
}

void SatelliteUi::initStyleQss()
{
    //初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
    //    ui->tableView->setStyleSheet("QTableView {                                                                   "
    //                                 "    color: rgb(102,102,102);                           /*表格内文字颜色*/"
    //                                 "    gridline-color: rgb(230,230,230);                 /*表格内框颜色*/"
    //                                 "    background-color: rgb(255,255,255);               /*表格内背景色*/"
    //                                 "    alternate-background-color: rgb(242,242,242);"
    //                                 "    selection-color: rgb(102,102,102);                /*选中区域的文字颜色*/"
    //                                 "    selection-background-color: rgb(242,242,242);        /*选中区域的背景色*/"
    //                                 "    border: 2px groove rgb(230,230,230);"
    //                                 "    border-radius: 0px;"
    //                                 "    padding: 2px 4px;"
    //                                 "}"
    //                                 "QHeaderView {                                      "
    //                                 "    color:  rgb(102,102,102);                      "
    //                                 "    font: bold 10pt;                               "
    //                                 "    background-color: rgb(242,242,242);            "
    //                                 "    border: 0px solid rgb(230,230,230);            "
    //                                 "    border:0px solid rgb(230,230,230);             "
    //                                 "    border-left-color: rgba(230,230,230, 0);       "
    //                                 "    border-top-color: rgba(230,230,230, 0);        "
    //                                 "    border-radius:0px;                             "
    //                                 "    min-height:30px;                               "
    //                                 "}                                                  "

    //    );

    //    ui->tableView->setFocusPolicy(Qt::NoFocus);  //隐藏选择虚线框
}

void SatelliteUi::showEvent(QShowEvent*)
{
    AllSatelliteMessageRequest request;

    JsonWriter writer;
    writer& request;
    auto json = writer.GetByteArray();

    emit signalGetAllSatellite(json);
}

void SatelliteUi::on_pushButton_edit_clicked()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    auto index = ui->tableView->currentIndex();
    if (index.row() >= 0)
    {
        auto satellite = m_impl->m_model->SatelliteData(index);
        if (!satellite.m_satelliteCode.isEmpty())
        {
            AddSatelliteDialog dialog;
            dialog.setMaxWorkMode(m_impl->m_maxWorkMode);
            dialog.setStationBz(m_impl->m_stationBz);
            dialog.setSatelliteDataList(m_impl->m_satelliteDataList);
            dialog.initUI();
            dialog.setCurrentUIMode(AddSatelliteDialog::EDIT);
            dialog.updateUI(satellite);

            connect(&dialog, &AddSatelliteDialog::signalAddSatellite, this, &SatelliteUi::signalAddSatellite);
            connect(&dialog, &AddSatelliteDialog::signalEditSatellite, this, &SatelliteUi::signalEditSatellite);
            connect(this, &SatelliteUi::signalAddSatelliteResponce, &dialog, &AddSatelliteDialog::slotAddSatelliteResponce);
            connect(this, &SatelliteUi::signalEditSatelliteResponce, &dialog, &AddSatelliteDialog::slotEditSatelliteResponce);

            if (dialog.exec())
            {
                auto data = dialog.currentsatelliteData();

                m_impl->updateSatelliteData(data);
                m_impl->m_model->setSatelliteData(m_impl->m_currentDataList);
            }
        }
    }
    else
    {
        QMessageBox::information(this, "提示", "请勾选需要编辑的卫星", "确定");
    }
}

void SatelliteUi::on_pushButton_delete_clicked()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    auto index = ui->tableView->currentIndex();
    if (index.row() >= 0)
    {
        auto satellite = m_impl->m_model->SatelliteData(index);
        if (!satellite.m_satelliteCode.isEmpty())
        {
            if (!QMessageBox::information(this, "提示", "是否删除当前选择的卫星", "确定", "取消"))
            {
                QByteArray json;
                json << satellite;
                emit signalDelSatellite(json);
            }
        }
    }
    else
    {
        QMessageBox::information(this, "提示", "请勾选需要删除的卫星", "确定");
    }
}
