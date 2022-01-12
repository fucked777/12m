#include "TableView.h"
#include "ButtonDelegate.h"
#include "CppMicroServicesUtility.h"
#include "MircoServiceInfo.h"
#include "TableModel.h"
#include <QApplication>
#include <QDebug>
#include <QDir>

TableView::TableView(QWidget* parent)
    : QTableView(parent)
{
    iniData();
}

TableView::~TableView() { delete m_model; }

void TableView::updateData(const QList<MircoServiceInfo>& microServiceInfoList)
{
    if (m_model)
    {
        m_model->setData(microServiceInfoList);
    }
}

void TableView::iniData()
{
    m_model = new TableModel();
    this->setModel(m_model);

    //    m_model->setHorizontalHeader(headers);

    // m_model->setData(data);

    m_buttonDelegate = new ButtonDelegate(this);
    connect(m_buttonDelegate, &ButtonDelegate::signal_install, this, [=](int row) { install(m_model->rowData(row)); });
    connect(m_buttonDelegate, &ButtonDelegate::signal_uninstall, this, [=](int row) { uninstall(m_model->rowData(row)); });

    this->setItemDelegateForColumn(TableModel::NUM, m_buttonDelegate);
    emit m_model->layoutChanged();
}

void TableView::setM_context(const BundleContext& m_context) { m_context_ = m_context; }

void TableView::install(MircoServiceInfo info)
{
#if defined(Q_OS_LINUX)
    auto location =
        QApplication::applicationDirPath() + QDir::separator() + "plugins" + QDir::separator() + QString("lib%1.so").arg(info.m_symbolicName);
#else
    auto location =
        QApplication::applicationDirPath() + QDir::separator() + "plugins" + QDir::separator() + QString("%1.dll").arg(info.m_symbolicName);
#endif
    std::vector<ServiceReferenceU> refs = m_context_.GetServiceReferences("");
    for (auto ref : refs)
    {
        auto&& bundle = ref.GetBundle();
        qDebug() << bundle.GetSymbolicName().c_str();
        if (bundle.GetSymbolicName().c_str() == info.m_symbolicName)
        {
            if (bundle.GetState() == Bundle::STATE_UNINSTALLED)
            {
                auto new_bundles = m_context_.InstallBundles(location.toStdString());
                for (auto new_bundle : new_bundles)
                {
                    new_bundle.Start();
                }
            }
        }
    }
}

void TableView::uninstall(MircoServiceInfo info)
{
    std::vector<ServiceReferenceU> refs = m_context_.GetServiceReferences("");
    for (auto ref : refs)
    {
        auto&& bundle = ref.GetBundle();
        if (bundle.GetSymbolicName().c_str() == info.m_symbolicName)
        {
            qDebug() << bundle.GetState() << "   " << bundle.GetSymbolicName().c_str();
            if (bundle.GetState() == Bundle::STATE_INSTALLED)
            {
                bundle.Uninstall();
            }
            if (bundle.GetState() == Bundle::STATE_ACTIVE)
            {
                bundle.Stop();
                bundle.Uninstall();
            }
        }
    }
}
