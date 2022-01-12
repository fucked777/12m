#include "DynamicParamDialog.h"
#include "ui_DynamicParamDialog.h"

#include "ControlFactory.h"

#include <QMessageBox>

DynamicParamDialog::DynamicParamDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DynamicParamDialog)
    , mMinValue(0)
    , mMaxValue(0)
    , mEnabled(true)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
}

DynamicParamDialog::~DynamicParamDialog() { delete ui; }

void DynamicParamDialog::setWidgetMode(const DynamicParamDialog::WidgetMode& currentUIMode) { mCurrentMode = currentUIMode; }

void DynamicParamDialog::setParamValueMap(const QMap<int, QMap<QString, QVariant>>& paramValueMap)
{
    for (auto groupNo : paramValueMap.keys())
    {
        auto groupParamValueMap = paramValueMap.value(groupNo);

        on_addBtn_clicked();

        for (int col = 0; col < mParamAttriList.size(); ++col)
        {
            auto control = qobject_cast<WWidget*>(ui->tableWidget->cellWidget(groupNo - 1, col));
            if (control != nullptr)
            {
                auto value = groupParamValueMap.value(control->objectName());
                control->setValue(value);
            }
        }
    }
}

QMap<int, QMap<QString, QVariant>> DynamicParamDialog::getParamValueMap() { return mParamValueMap; }

void DynamicParamDialog::setRowParamAttribute(const QList<ParameterAttribute>& paramAttriList)
{
    for (auto attr : paramAttriList)
    {
        if (attr.isShow)
        {
            mParamAttriList << attr;
        }
    }
    ui->tableWidget->setColumnCount(mParamAttriList.size());

    QStringList headerLabels;
    for (int col = 0; col < mParamAttriList.size(); ++col)
    {
        auto attr = mParamAttriList.at(col);
        headerLabels << attr.desc;
    }
    ui->tableWidget->setColumnCount(headerLabels.size());
    ui->tableWidget->setHorizontalHeaderLabels(headerLabels);

    ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    //    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  // 设置最后一列自适应大小
    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(150);
    // tableWidget->setShowGrid(false);                                 // 隐藏单元格边框
    ui->tableWidget->setFocusPolicy(Qt::NoFocus);                           // 取消选中单元格时的虚线框
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);   // 选择整行
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);  // 选择单行模式
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);    // 禁止编辑单元格
    ui->tableWidget->setMouseTracking(true);                                // 跟踪鼠标一定要先设的值
}

void DynamicParamDialog::setEnumMap(const QMap<QString, DevEnum>& enumMap) { mEnumMap = enumMap; }

void DynamicParamDialog::setNumRange(const int minValue, const int maxValue)
{
    mMinValue = minValue;
    mMaxValue = maxValue;
}

void DynamicParamDialog::setButtonEnable(bool isEnable) { mEnabled = isEnable; }

void DynamicParamDialog::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)

    // 查看时设置控件不可用
    if (mCurrentMode == WidgetMode::Look)
    {
        ui->addBtn->hide();
        ui->removeBtn->hide();
        ui->okBtn->hide();

        auto widgets = ui->tableWidget->findChildren<WWidget*>();
        for (auto widget : widgets)
        {
            widget->setEnabled(false);
        }
    }
}

void DynamicParamDialog::on_okBtn_clicked()
{
    int rows = ui->tableWidget->rowCount();
    int cols = ui->tableWidget->columnCount();

    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            auto control = qobject_cast<WWidget*>(ui->tableWidget->cellWidget(row, col));
            if (control != nullptr)
            {
                auto paramId = control->objectName();

                bool isValid = false;
                auto value = control->value(isValid);
                if (!isValid)
                {
                    QMessageBox::information(this, "提示", "参数数据非法,参数名称：" + control->paramAttr.desc);
                    return;
                }
                mParamValueMap[row + 1][paramId] = value;
            }
        }
    }

    accept();
}

void DynamicParamDialog::on_cancelBtn_clicked() { reject(); }

void DynamicParamDialog::on_addBtn_clicked()
{
    auto rows = ui->tableWidget->rowCount();
    if (rows >= 8)
    {
        QMessageBox::information(this, "提示", "最多可添加8组参数", "确定");
        return;
    }

    ui->tableWidget->setRowCount(rows + 1);

    for (int col = 0; col < mParamAttriList.size(); ++col)
    {
        auto attr = mParamAttriList.at(col);

        // 枚举值
        QMap<QString, QVariant> enumMap;
        if (attr.displayFormat == DisplayFormat_Enum)
        {
            auto paramEnumList = mEnumMap.value(attr.enumType);
            for (auto paramEnum : paramEnumList.emumEntryList)
            {
                enumMap[paramEnum.desc] = paramEnum.uValue;
            }
        }

        auto control = ControlFactory::createSetControlOnly(attr, enumMap);

        //如果保存过最大值和最小值,那么使用保存值。否则使用默认值。
        if (mMinValue != 0 || mMaxValue != 0)
        {
            control->setNumRange(mMinValue, mMaxValue);
        }
        control->setEnabled(mEnabled);
        ui->tableWidget->setCellWidget(rows, col, control);
    }
}

void DynamicParamDialog::on_removeBtn_clicked()
{
    auto rows = ui->tableWidget->rowCount();
    if (rows > 0)
    {
        ui->tableWidget->removeRow(rows - 1);
    }
}
