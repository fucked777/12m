#include "StartWizard.h"
#include "IStartWizard.h"
#include "PlatformInterface.h"
#include "SingleModuleLoad.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>
#include <QString>

class StartWizardImpl
{
public:
    std::shared_ptr<IStartWizard> startWizard;
    SingleModuleLoad singleModuleLoad;
    QString errMsg;
};

StartWizard::StartWizard()
    : QObject(nullptr)
    , m_impl(new StartWizardImpl)
{
}
StartWizard::~StartWizard()
{
    m_impl->startWizard.reset();
    delete m_impl;
}
QString StartWizard::errMsg() const { return m_impl->errMsg; }
WizardInstallResult StartWizard::loadWizard(const PlatformConfigInfo& info)
{
    if (info.uiInfo.start.isEmpty())
    {
        return WizardInstallResult::Skip;
    }

    if (!m_impl->singleModuleLoad.loadModule(info.uiInfo.start))
    {
        m_impl->errMsg = m_impl->singleModuleLoad.errMsg();
        return WizardInstallResult::Error;
    }
    m_impl->startWizard = m_impl->singleModuleLoad.instance<IStartWizard>();
    if (m_impl->startWizard == nullptr)
    {
        m_impl->errMsg = "向导模块加载错误";
        return WizardInstallResult::Error;
    }

    m_impl->startWizard->show();

    auto width = m_impl->startWizard->width();
    auto deskTopWidget = QApplication::desktop();
    auto height = m_impl->startWizard->height();
    m_impl->startWizard->move((deskTopWidget->width() - width) / 2, (deskTopWidget->height() - height) / 2);

    return WizardInstallResult::Finish;
}
void StartWizard::serviceInstallStatusChange(ModuleInstallStatus status, const QString& name, const QString& msg)
{
    if (m_impl->startWizard != nullptr)
    {
        m_impl->startWizard->serviceInstallStatusChange(status, name, msg);
    }
}
void StartWizard::setControlPowerThis(bool controlPower)
{
    if (m_impl->startWizard != nullptr)
    {
        m_impl->startWizard->setControlPowerThis(controlPower);
    }
}

//#include <QHBoxLayout>
//#include <QListWidget>
// ExecStartWidget::ExecStartWidget(QWidget* parent)
//    : QDialog(parent)
//    , listWidget(new QListWidget(this))
//{
//    auto layout = new QHBoxLayout(this);
//    layout->setContentsMargins(0, 0, 0, 0);
//    layout->addWidget(listWidget);
//}
// ExecStartWidget::~ExecStartWidget() {}
// void ExecStartWidget::appendInfo(const QString& txt) { listWidget->addItem(txt); }
