#include "StartWizard.h"
#include "CppMicroServicesUtility.h"
#include "GlobalData.h"
#include "QssCommonHelper.h"
#include "Utility.h"
#include "ui_StartWizard.h"
#include <QDebug>
#include <QMenu>
#include <QMessageBox>

StartWizard::StartWizard(cppmicroservices::BundleContext /*context*/, QWidget* parent)
    : IStartWizard(parent)
    , ui(new Ui::StartWizard)
{
    ui->setupUi(this);
    // 初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
}

StartWizard::~StartWizard() { delete ui; }
void StartWizard::serviceInstallStatusChange(ModuleInstallStatus /*status*/, const QString& name, const QString& /*msg*/)
{
    ui->listWidget->addItem(name);
}
