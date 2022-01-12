#include "OrbitProdictionUI.h"
#include "CppMicroServicesUtility.h"
#include "ui_OrbitProdictionUI.h"

OrbitProdictionUI::OrbitProdictionUI(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::OrbitProdictionUI)
{
    Q_UNUSED(context)
    ui->setupUi(this);
}

OrbitProdictionUI::~OrbitProdictionUI() { delete ui; }
