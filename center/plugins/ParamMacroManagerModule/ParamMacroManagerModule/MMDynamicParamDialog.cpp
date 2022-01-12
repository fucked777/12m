#include "MMDynamicParamDialog.h"
#include "ui_MMDynamicParamDialog.h"

#include "ControlFactory.h"

#include <QMessageBox>

MMDynamicParamDialog::MMDynamicParamDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::MMDynamicParamDialog)
    , mMinValue(0)
    , mMaxValue(0)
    , mEnabled(true)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
}

MMDynamicParamDialog::~MMDynamicParamDialog() { delete ui; }

void MMDynamicParamDialog::setWidgetMode(const MMDynamicParamDialog::WidgetMode& currentUIMode) { mCurrentMode = currentUIMode; }

void MMDynamicParamDialog::setParamValueMap(const QMap<int, QMap<QString, QVariant>>& paramValueMap)
{
    QMap<QString, bool> containsMap;

    for (auto groupNo : paramValueMap.keys())
    {
        auto groupParamValueMap = paramValueMap.value(groupNo);
        for (auto item : mParamAttriList)
        {
            containsMap[item.id] = true;
        }
        //上行遥控
        for (int i = 0; i < mParamAttriList.size(); i++)
        {
            if (groupParamValueMap.contains(mParamAttriList[i].id) && i < 1 * 4)
            {
                on_addBtn_clicked();

                for (int col = 0; col < 4; ++col)
                {
                    auto control = qobject_cast<WWidget*>(ui->tableWidget->cellWidget(groupNo - 1, col));
                    if (control != nullptr)
                    {
                        auto value = groupParamValueMap.value(control->objectName());
                        control->setValue(value);
                    }
                }
                break;
            }
        }

        //上行测距
        for (int i = 0; i < mParamAttriList.size(); i++)
        {
            if (groupParamValueMap.contains(mParamAttriList[i].id) && i < 2 * 4 && i >= 1 * 4)
            {
                on_addBtn_2_clicked();
                for (int col = 0; col < 4; ++col)
                {
                    auto control = qobject_cast<WWidget*>(ui->tableWidget_2->cellWidget(groupNo - 1, col));
                    if (control != nullptr)
                    {
                        auto value = groupParamValueMap.value(control->objectName());
                        control->setValue(value);
                    }
                }
                break;
            }
        }
        //下行遥测
        for (int i = 0; i < mParamAttriList.size(); i++)
        {
            if (groupParamValueMap.contains(mParamAttriList[i].id) && i < 3 * 4 && i >= 2 * 4)
            {
                on_addBtn_3_clicked();

                for (int col = 0; col < 4; ++col)
                {
                    auto control = qobject_cast<WWidget*>(ui->tableWidget_3->cellWidget(groupNo - 1, col));
                    if (control != nullptr)
                    {
                        auto value = groupParamValueMap.value(control->objectName());
                        control->setValue(value);
                    }
                }
                break;
            }
        }

        //下行测距
        for (int i = 0; i < mParamAttriList.size(); i++)
        {
            if (groupParamValueMap.contains(mParamAttriList[i].id) && i < 4 * 4 && i >= 3 * 4)
            {
                on_addBtn_4_clicked();

                for (int col = 0; col < 4; ++col)
                {
                    auto control = qobject_cast<WWidget*>(ui->tableWidget_4->cellWidget(groupNo - 1, col));
                    if (control != nullptr)
                    {
                        auto value = groupParamValueMap.value(control->objectName());
                        control->setValue(value);
                    }
                }
                break;
            }
        }
    }
}

QMap<int, QMap<QString, QVariant>> MMDynamicParamDialog::getParamValueMap() { return mParamValueMap; }

void MMDynamicParamDialog::setRowParamAttribute(const QList<ParameterAttribute>& paramAttriList)
{
    for (auto attr : paramAttriList)
    {
        if (attr.isShow)
        {
            mParamAttriList << attr;
        }
    }

    {
        ui->tableWidget->setColumnCount(mParamAttriList.size() / 4);

        QStringList headerLabels;
        for (int col = 0 * 4; col < 1 * 4; ++col)
        {
            auto attr = mParamAttriList.at(col);
            headerLabels << attr.desc;
        }
        ui->tableWidget->setColumnCount(headerLabels.size());
        ui->tableWidget->setHorizontalHeaderLabels(headerLabels);
        ui->tableWidget->setTabKeyNavigation(true);
        ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  // 设置最后一列自适应大小
        ui->tableWidget->setFocusPolicy(Qt::NoFocus);                                     // 取消选中单元格时的虚线框
        ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);             // 选择整行
        ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);            // 选择单行模式
        ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);              // 禁止编辑单元格
        ui->tableWidget->setMouseTracking(true);                                          // 跟踪鼠标一定要先设的值
    }
    {
        ui->tableWidget_2->setColumnCount(mParamAttriList.size() / 4);

        QStringList headerLabels;
        for (int col = 1 * 4; col < 2 * 4; ++col)
        {
            auto attr = mParamAttriList.at(col);
            headerLabels << attr.desc;
        }
        ui->tableWidget_2->setColumnCount(headerLabels.size());
        ui->tableWidget_2->setHorizontalHeaderLabels(headerLabels);

        ui->tableWidget_2->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
        ui->tableWidget_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  // 设置最后一列自适应大小
        ui->tableWidget_2->setFocusPolicy(Qt::NoFocus);                                     // 取消选中单元格时的虚线框
        ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);             // 选择整行
        ui->tableWidget_2->setSelectionMode(QAbstractItemView::SingleSelection);            // 选择单行模式
        ui->tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers);              // 禁止编辑单元格
        ui->tableWidget_2->setMouseTracking(true);                                          // 跟踪鼠标一定要先设的值
    }
    {
        ui->tableWidget_3->setColumnCount(mParamAttriList.size() / 4);

        QStringList headerLabels;
        for (int col = 2 * 4; col < 3 * 4; ++col)
        {
            auto attr = mParamAttriList.at(col);
            headerLabels << attr.desc;
        }
        ui->tableWidget_3->setColumnCount(headerLabels.size());
        ui->tableWidget_3->setHorizontalHeaderLabels(headerLabels);
        ui->tableWidget_3->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
        ui->tableWidget_3->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  // 设置最后一列自适应大小
        ui->tableWidget_3->setFocusPolicy(Qt::NoFocus);                                     // 取消选中单元格时的虚线框
        ui->tableWidget_3->setSelectionBehavior(QAbstractItemView::SelectRows);             // 选择整行
        ui->tableWidget_3->setSelectionMode(QAbstractItemView::SingleSelection);            // 选择单行模式
        ui->tableWidget_3->setEditTriggers(QAbstractItemView::NoEditTriggers);              // 禁止编辑单元格
        ui->tableWidget_3->setMouseTracking(true);                                          // 跟踪鼠标一定要先设的值
    }
    {
        ui->tableWidget_4->setColumnCount(mParamAttriList.size() / 4);

        QStringList headerLabels;
        for (int col = 3 * 4; col < 4 * 4; ++col)
        {
            auto attr = mParamAttriList.at(col);
            headerLabels << attr.desc;
        }
        ui->tableWidget_4->setColumnCount(headerLabels.size());
        ui->tableWidget_4->setHorizontalHeaderLabels(headerLabels);

        ui->tableWidget_4->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
        ui->tableWidget_4->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  // 设置最后一列自适应大小
        ui->tableWidget_4->setFocusPolicy(Qt::NoFocus);                                     // 取消选中单元格时的虚线框
        ui->tableWidget_4->setSelectionBehavior(QAbstractItemView::SelectRows);             // 选择整行
        ui->tableWidget_4->setSelectionMode(QAbstractItemView::SingleSelection);            // 选择单行模式
        ui->tableWidget_4->setEditTriggers(QAbstractItemView::NoEditTriggers);              // 禁止编辑单元格
        ui->tableWidget_4->setMouseTracking(true);                                          // 跟踪鼠标一定要先设的值
    }
}

void MMDynamicParamDialog::setEnumMap(const QMap<QString, DevEnum>& enumMap) { mEnumMap = enumMap; }

void MMDynamicParamDialog::setNumRange(const int minValue, const int maxValue)
{
    mMinValue = minValue;
    mMaxValue = maxValue;
}

void MMDynamicParamDialog::setButtonEnable(bool isEnable) { mEnabled = isEnable; }

void MMDynamicParamDialog::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)

    // 查看时设置控件不可用
    if (mCurrentMode == WidgetMode::Look)
    {
        ui->addBtn->hide();
        ui->removeBtn->hide();
        ui->addBtn_2->hide();
        ui->removeBtn_2->hide();

        ui->addBtn_3->hide();
        ui->removeBtn_3->hide();

        ui->addBtn_4->hide();
        ui->removeBtn_4->hide();

        ui->okBtn->hide();

        auto widgets = ui->tableWidget->findChildren<WWidget*>();
        for (auto widget : widgets)
        {
            widget->setEnabled(false);
        }

        auto tableWidget_2 = ui->tableWidget_2->findChildren<WWidget*>();
        for (auto widget : tableWidget_2)
        {
            widget->setEnabled(false);
        }

        auto tableWidget_3 = ui->tableWidget_3->findChildren<WWidget*>();
        for (auto widget : tableWidget_3)
        {
            widget->setEnabled(false);
        }

        auto tableWidget_4 = ui->tableWidget_4->findChildren<WWidget*>();
        for (auto widget : tableWidget_4)
        {
            widget->setEnabled(false);
        }
    }
}

void MMDynamicParamDialog::on_okBtn_clicked()
{
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
    }

    {
        int rows = ui->tableWidget_2->rowCount();
        int cols = ui->tableWidget_2->columnCount();

        for (int row = 0; row < rows; ++row)
        {
            for (int col = 0; col < cols; ++col)
            {
                auto control = qobject_cast<WWidget*>(ui->tableWidget_2->cellWidget(row, col));
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
    }

    {
        int rows = ui->tableWidget_3->rowCount();
        int cols = ui->tableWidget_3->columnCount();

        for (int row = 0; row < rows; ++row)
        {
            for (int col = 0; col < cols; ++col)
            {
                auto control = qobject_cast<WWidget*>(ui->tableWidget_3->cellWidget(row, col));
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
    }

    {
        int rows = ui->tableWidget_4->rowCount();
        int cols = ui->tableWidget_4->columnCount();

        for (int row = 0; row < rows; ++row)
        {
            for (int col = 0; col < cols; ++col)
            {
                auto control = qobject_cast<WWidget*>(ui->tableWidget_4->cellWidget(row, col));
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
    }
    accept();
}

void MMDynamicParamDialog::on_cancelBtn_clicked() { reject(); }

void MMDynamicParamDialog::on_addBtn_clicked()
{
    auto rows = ui->tableWidget->rowCount();
    if (rows >= 8)
    {
        QMessageBox::information(this, "提示", "最多可添加8组参数", "确定");
        return;
    }

    ui->tableWidget->setRowCount(rows + 1);

    for (int col = 0 * 4; col < 1 * 4; ++col)
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

void MMDynamicParamDialog::on_removeBtn_clicked()
{
    auto rows = ui->tableWidget->rowCount();
    if (rows > 0)
    {
        ui->tableWidget->removeRow(rows - 1);
    }
}

void MMDynamicParamDialog::on_addBtn_2_clicked()
{
    auto rows = ui->tableWidget_2->rowCount();
    if (rows >= 8)
    {
        QMessageBox::information(this, "提示", "最多可添加8组参数", "确定");
        return;
    }

    ui->tableWidget_2->setRowCount(rows + 1);

    for (int col = 1 * 4; col < 2 * 4; ++col)
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
        ui->tableWidget_2->setCellWidget(rows, col - 1 * 4, control);
    }
}

void MMDynamicParamDialog::on_removeBtn_2_clicked()
{
    auto rows = ui->tableWidget_2->rowCount();
    if (rows > 0)
    {
        ui->tableWidget_2->removeRow(rows - 1);
    }
}

void MMDynamicParamDialog::on_addBtn_3_clicked()
{
    auto rows = ui->tableWidget_3->rowCount();
    if (rows >= 8)
    {
        QMessageBox::information(this, "提示", "最多可添加8组参数", "确定");
        return;
    }

    ui->tableWidget_3->setRowCount(rows + 1);

    for (int col = 2 * 4; col < 3 * 4; ++col)
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
        ui->tableWidget_3->setCellWidget(rows, col - 2 * 4, control);
    }
}

void MMDynamicParamDialog::on_removeBtn_3_clicked()
{
    auto rows = ui->tableWidget_3->rowCount();
    if (rows > 0)
    {
        ui->tableWidget_3->removeRow(rows - 1);
    }
}

void MMDynamicParamDialog::on_addBtn_4_clicked()
{
    auto rows = ui->tableWidget_4->rowCount();
    if (rows >= 8)
    {
        QMessageBox::information(this, "提示", "最多可添加8组参数", "确定");
        return;
    }

    ui->tableWidget_4->setRowCount(rows + 1);

    for (int col = 3 * 4; col < 4 * 4; ++col)
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
        ui->tableWidget_4->setCellWidget(rows, col - 3 * 4, control);
    }
}

void MMDynamicParamDialog::on_removeBtn_4_clicked()
{
    auto rows = ui->tableWidget_4->rowCount();
    if (rows > 0)
    {
        ui->tableWidget_4->removeRow(rows - 1);
    }
}
