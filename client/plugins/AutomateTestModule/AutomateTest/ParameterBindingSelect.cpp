#include "ParameterBindingSelect.h"
#include "AutomateTestUtility.h"
#include "QssCommonHelper.h"
#include "ui_ParameterBindingSelect.h"
#include <QMessageBox>

ParameterBindingSelect::ParameterBindingSelect(const AutomateTestItemInfo& info, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ParameterBindingSelect)
{
    ui->setupUi(this);

    for (auto& item : info.itemMap)
    {
        if (!item.bind.isEmpty())
        {
            ui->planList->addItem(item.name, item.id);
        }
    }
    setMinimumSize(400, 200);
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
    connect(ui->ok, &QPushButton::clicked, this, &ParameterBindingSelect::accept);
    connect(ui->cancel, &QPushButton::clicked, this, &ParameterBindingSelect::reject);
}

ParameterBindingSelect::~ParameterBindingSelect() { delete ui; }

QString ParameterBindingSelect::text() { return ui->planList->currentText(); }
QString ParameterBindingSelect::id() { return ui->planList->currentData().toString(); }
