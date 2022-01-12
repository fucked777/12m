#include "NavBarWidget.h"
#include "ui_NavBarWidget.h"

#include "IGuiService.h"
#include "ServiceHelper.h"
#include "SystemMainScreen.h"

#include <QListWidgetItem>

NavBarWidget::NavBarWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::NavBarWidget)
{
    ui->setupUi(this);

    connect(ui->listWidget, &QListWidget::itemClicked, this, &NavBarWidget::signalCurrentItemClicked);
}

NavBarWidget::~NavBarWidget() { delete ui; }

void NavBarWidget::appendItem(const QString& label, const QString& moduleName)
{
    auto item = new QListWidgetItem(label, ui->listWidget);
    item->setData(Qt::UserRole, moduleName);

    ui->listWidget->addItem(item);
}

void NavBarWidget::setCurrentRow(int row)
{
    auto item = ui->listWidget->item(row);
    if (item == nullptr)
    {
        return;
    }
    ui->listWidget->setCurrentRow(row);
    emit ui->listWidget->itemClicked(item);
}
