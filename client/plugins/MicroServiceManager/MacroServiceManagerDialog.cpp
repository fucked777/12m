#include "MacroServiceManagerDialog.h"
#include "CppMicroServicesUtility.h"
#include "MircoServiceInfo.h"
#include "TableView.h"
#include "Utility.h"
#include "ui_MacroServiceManagerDialog.h"
#include <QDebug>
#include <QHeaderView>
#include <QVBoxLayout>

class MacroServiceManagerDialogImpl
{
public:
    TableView* tableView{ nullptr };
    BundleContext context;
    int timerID{ -1 };
};

using namespace cppmicroservices;
MacroServiceManagerDialog::MacroServiceManagerDialog(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::MacroServiceManagerDialog)
    , m_impl(new MacroServiceManagerDialogImpl)
{
    ui->setupUi(this);
    initTableView();
    m_impl->context = context;
    m_impl->timerID = startTimer(1000);
}

MacroServiceManagerDialog::~MacroServiceManagerDialog()
{
    Utility::killTimer(this, m_impl->timerID);
    delete ui;
    delete m_impl;
}

void MacroServiceManagerDialog::initTableView()
{
    m_impl->tableView = new TableView(this);
    m_impl->tableView->setM_context(m_impl->context);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(m_impl->tableView);
    this->setLayout(layout);

    m_impl->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_impl->tableView->horizontalHeader()->setStretchLastSection(true);
    m_impl->tableView->horizontalHeader()->setHighlightSections(false);
    m_impl->tableView->verticalHeader()->setVisible(false);

    m_impl->tableView->setFrameShape(QFrame::NoFrame);
    m_impl->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    m_impl->tableView->setColumnWidth(5, 230);
    m_impl->tableView->setColumnWidth(6, 230);

    //    std::vector<ServiceReference<IGuiService>> refs =
    //        context_.GetServiceReferences<IGuiService>("");
    //    QList<MircoServiceInfo> mircoServiceInfoList;
    //    for (auto ref : refs) {
    //        MircoServiceInfo microInfo;
    //        for (auto item : ref.GetPropertyKeys()) {

    //            if (QString(item.c_str()) == "servicename")
    //                microInfo.m_serviceName = ref.GetProperty("servicename").ToString().c_str();
    //            if (QString(item.c_str()) == "bundle.symbolic_name")
    //                microInfo.m_symbolicName =
    //                    ref.GetProperty("bundle.symbolic_name").ToString().c_str();
    //            if (QString(item.c_str()) == "bundle.version")
    //                microInfo.m_serviceVersion =
    //                ref.GetProperty("bundle.version").ToString().c_str();
    //            if (QString(item.c_str()) == "authors")
    //                microInfo.m_serviceAuthors = ref.GetProperty("authors").ToString().c_str();
    //            if (QString(item.c_str()) == "bundle.description")
    //                microInfo.m_serviceDescription =
    //                    ref.GetProperty("bundle.description").ToString().c_str();
    //            if (QString(item.c_str()) == "update_log")
    //                microInfo.m_serviceLog = ref.GetProperty("update_log").ToString().c_str();
    //        }
    //        mircoServiceInfoList.append(microInfo);
    //    }
    //    m_tableView->updateData(mircoServiceInfoList);
    //    std::vector<ServiceReferenceU> refs = context_.GetServiceReferences("");
    //    for (auto ref : refs) {
    //        for (auto item : ref.GetPropertyKeys()) {
    //            qDebug() << "item   ---------   " << item.c_str() << "    "
    //                     << ref.GetProperty(item.c_str()).ToString().c_str();
    //        }
    //        auto &&bundle = ref.GetBundle();
    //        for (auto item : bundle.GetPropertyKeys()) {

    //            if (QString(item.c_str()) == "servicename")
    //                qDebug() << bundle.GetProperty("servicename").ToString().c_str();
    //            if (QString(item.c_str()) == "bundle.symbolic_name")
    //                qDebug() << bundle.GetProperty("bundle.symbolic_name").ToString().c_str();
    //            if (QString(item.c_str()) == "bundle.version")
    //                qDebug() << bundle.GetProperty("bundle.version").ToString().c_str();
    //            if (QString(item.c_str()) == "authors")
    //                qDebug() << bundle.GetProperty("authors").ToString().c_str();
    //            if (QString(item.c_str()) == "bundle.description")
    //                qDebug() << bundle.GetProperty("bundle.description").ToString().c_str();
    //            if (QString(item.c_str()) == "update_log")
    //                qDebug() << bundle.GetProperty("update_log").ToString().c_str();
    //        }
    //        if (bundle.GetState() == Bundle::STATE_UNINSTALLED) {
    //            qDebug() << bundle.GetProperty("bundle.symbolic_name").ToString().c_str()
    //                     << "Bundle::STATE_UNINSTALLED";
    //        }
    //        if (bundle.GetState() == Bundle::STATE_INSTALLED) {
    //            qDebug() << bundle.GetProperty("bundle.symbolic_name").ToString().c_str()
    //                     << "Bundle::STATE_INSTALLED";
    //        }
    //        if (bundle.GetState() == Bundle::STATE_RESOLVED) {
    //            qDebug() << bundle.GetProperty("bundle.symbolic_name").ToString().c_str()
    //                     << "Bundle::STATE_RESOLVED";
    //        }
    //        if (bundle.GetState() == Bundle::STATE_STARTING) {
    //            qDebug() << bundle.GetProperty("bundle.symbolic_name").ToString().c_str()
    //                     << "Bundle::STATE_STARTING";
    //        }
    //        if (bundle.GetState() == Bundle::STATE_STOPPING) {
    //            qDebug() << ""
    //                     << "Bundle::STATE_STOPPING";
    //        }
    //        if (bundle.GetState() == Bundle::STATE_ACTIVE) {
    //            qDebug() << bundle.GetProperty("bundle.symbolic_name").ToString().c_str()
    //                     << "Bundle::STATE_ACTIVE";
    //        }

    //        qDebug() << bundle.GetState();
    //        //        bundle.Stop();
    //        //        bundle.Start();
    //    }

    //以下代码已弃用
    //    QList<MircoServiceInfo> mircoServiceInfoList;
    //    for (auto const bundle : context_.GetBundles()) {

    //        //跳过系统的符号
    //        if (bundle.GetSymbolicName().c_str() == QString("system_bundle")) {
    //            continue;
    //        }
    //        MircoServiceInfo microInfo;
    //        for (auto item : bundle.GetPropertyKeys()) {

    //            if (QString(item.c_str()) == "servicename")
    //                microInfo.m_serviceName =
    //                bundle.GetProperty("servicename").ToString().c_str();
    //            if (QString(item.c_str()) == "bundle.symbolic_name")
    //                microInfo.m_symbolicName =
    //                    bundle.GetProperty("bundle.symbolic_name").ToString().c_str();
    //            if (QString(item.c_str()) == "bundle.version")
    //                microInfo.m_serviceVersion =
    //                    bundle.GetProperty("bundle.version").ToString().c_str();
    //            if (QString(item.c_str()) == "authors")
    //                microInfo.m_serviceAuthors = bundle.GetProperty("authors").ToString().c_str();
    //            if (QString(item.c_str()) == "bundle.description")
    //                microInfo.m_serviceDescription =
    //                    bundle.GetProperty("bundle.description").ToString().c_str();
    //            if (QString(item.c_str()) == "update_log")
    //                microInfo.m_serviceLog = bundle.GetProperty("update_log").ToString().c_str();
    //        }
    //        mircoServiceInfoList.append(microInfo);
    //    }
    //    m_tableView->updateData(mircoServiceInfoList);
}

template<typename T>
static void findBundle(AnyMap& anyMap, T& res, const char* key)
{
    auto find = anyMap.find(key);
    if (find != anyMap.end())
    {
        auto stdString = find->second.ToString();
        res = stdString.c_str();
    }
}
void MacroServiceManagerDialog::timerEvent(QTimerEvent* /*event*/)
{
    // 卸载顺序问题 当context已经无效了但是定时器还在跑所以会抛异常这里判断一下
    if (!m_impl->context)
    {
        // 待定是否停止计时 wp??
        // Utility::killTimer(this, m_timerID);
        return;
    }
    QList<MircoServiceInfo> mircoServiceInfoList;
    auto bundles = m_impl->context.GetBundles();
    for (auto const& bundle : bundles)
    {
        //        if (bundle.GetState() == Bundle::STATE_UNINSTALLED)
        //            continue;
        //跳过系统的符号
        if (bundle.GetSymbolicName().c_str() == QString("system_bundle"))
        {
            continue;
        }
        MircoServiceInfo microInfo;
        auto headers = bundle.GetHeaders();

        findBundle(headers, microInfo.m_serviceName, "servicename");
        findBundle(headers, microInfo.m_symbolicName, "bundle.symbolic_name");
        findBundle(headers, microInfo.m_serviceVersion, "bundle.version");
        findBundle(headers, microInfo.m_serviceAuthors, "authors");
        findBundle(headers, microInfo.m_serviceDescription, "bundle.description");
        findBundle(headers, microInfo.m_serviceLog, "update_log");
        mircoServiceInfoList.append(microInfo);
    }

    m_impl->tableView->updateData(mircoServiceInfoList);
}
