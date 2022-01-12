#include "ui_CopyPointFreqDialog.h"

#include "CopyPointFreqDialog.h"
#include "WWidget.h"

#include <QTreeWidget>

CopyPointFreqDialog::CopyPointFreqDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::CopyPointFreqDialog)
{
    ui->setupUi(this);
    setWindowTitle("复制点频参数");
}

CopyPointFreqDialog::~CopyPointFreqDialog() { delete ui; }

void CopyPointFreqDialog::setPointFreqMap(const QMap<QString, QTreeWidget*> pointFreqMap)
{
    mPointFreqMap = pointFreqMap;
    for (auto pointFreqItemName : pointFreqMap.keys())
    {
        ui->sourcePointFreqComboBox->addItem(pointFreqItemName);
        ui->destPointFreqComboBox->addItem(pointFreqItemName);
    }
}

void CopyPointFreqDialog::on_cancleBtn_clicked() { this->close(); }

void CopyPointFreqDialog::on_okBtn_clicked()
{
    auto sourcePointFreqName = ui->sourcePointFreqComboBox->currentText();
    auto destPointFreqName = ui->destPointFreqComboBox->currentText();

    QMap<QString, QVariant> paramMap;     // QMap<4001_1/4002_1/AAAA_1&&K2_WorkRate, 值>
    QMap<QString, QVariant> btnParamMap;  // QMap<按钮文本, 值>
    // 从控件中取出源点频控件中的参数
    auto srcTreeWidget = mPointFreqMap[sourcePointFreqName];
    auto groupCount = srcTreeWidget->topLevelItemCount();
    for (int i = 0; i < groupCount; ++i)
    {
        auto groupTreeItem = srcTreeWidget->topLevelItem(i);
        auto controlCount = groupTreeItem->childCount();
        for (int j = 0; j < controlCount; ++j)
        {
            auto control = qobject_cast<WWidget*>(srcTreeWidget->itemWidget(groupTreeItem->child(j), 0));

            if (control != nullptr)
            {
                auto objName = control->objectName();
                auto key = objName.section("&&", 2);
                paramMap[key] = control->value();
            }

            //  动态参数
            auto btn = qobject_cast<QPushButton*>(srcTreeWidget->itemWidget(groupTreeItem->child(j), 0));
            if (btn != nullptr)
            {
                auto value = btn->property("ChangeValue");
                btnParamMap[btn->text()] = value;
            }
        }
    }

    // 把源点频参数值设置到目标点频中
    auto destTreeWidget = mPointFreqMap[destPointFreqName];
    groupCount = destTreeWidget->topLevelItemCount();
    for (int i = 0; i < groupCount; ++i)
    {
        auto groupTreeItem = destTreeWidget->topLevelItem(i);

        auto controlCount = groupTreeItem->childCount();
        for (int j = 0; j < controlCount; ++j)
        {
            auto control = qobject_cast<WWidget*>(destTreeWidget->itemWidget(groupTreeItem->child(j), 0));

            if (control != nullptr)
            {
                auto objName = control->objectName();
                auto paramName = objName.section("&&", 2);

                control->setValue(paramMap.value(paramName));
            }

            //  动态参数
            auto btn = qobject_cast<QPushButton*>(destTreeWidget->itemWidget(groupTreeItem->child(j), 0));
            if (btn != nullptr)
            {
                btn->setProperty("ChangeValue", btnParamMap.value(btn->text()));
            }
        }
    }

    close();
}
