#include "CheckBoxComboBox.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScrollBar>

CheckBoxComboBox::CheckBoxComboBox(QWidget* parent)
    : QComboBox(parent)
{
    mListWidget = new QListWidget(this);

    setModel(mListWidget->model());
    setView(mListWidget);
    mListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mListWidget->showFullScreen();

    mLineEdit = new QLineEdit(this);
    setLineEdit(mLineEdit);

    connect(mLineEdit, &QLineEdit::textChanged, this, &CheckBoxComboBox::slotTextChanged);
}

void CheckBoxComboBox::addItem(const QString& text, const QVariant& userData)
{
    auto checkBox = new QCheckBox(text, this);
    checkBox->setProperty("UserData", userData);

    auto listItem = new QListWidgetItem(mListWidget);
    mListWidget->addItem(listItem);
    mListWidget->setItemWidget(listItem, checkBox);

    connect(checkBox, &QCheckBox::stateChanged, this, &CheckBoxComboBox::slotStateChanged);
}

QStringList CheckBoxComboBox::getCheckedItemsText()
{
    QStringList list;
    for (int i = 0; i < mListWidget->count(); ++i)
    {
        auto listItem = mListWidget->item(i);
        auto checkBox = qobject_cast<QCheckBox*>(mListWidget->itemWidget(listItem));
        if (checkBox->isChecked())
        {
            list << checkBox->text();
        }
    }

    return list;
}

void CheckBoxComboBox::clearSelected()
{
    for (int i = 0; i < mListWidget->count(); ++i)
    {
        auto listItem = mListWidget->item(i);
        auto checkBox = (QCheckBox*)(mListWidget->itemWidget(listItem));
        checkBox->setChecked(false);
    }
}

void CheckBoxComboBox::setCheckedItem(const QString& text)
{
    for (int i = 0; i < mListWidget->count(); ++i)
    {
        auto listItem = mListWidget->item(i);
        auto checkBox = (QCheckBox*)(mListWidget->itemWidget(listItem));
        if (checkBox->text() == text)
        {
            checkBox->setChecked(true);
        }
    }
}

void CheckBoxComboBox::setCheckedItem(const QVariant& userData)
{
    for (int i = 0; i < mListWidget->count(); ++i)
    {
        auto listItem = mListWidget->item(i);
        auto checkBox = (QCheckBox*)(mListWidget->itemWidget(listItem));
        if (checkBox->property("UserData") == userData)
        {
            checkBox->setChecked(true);
        }
    }
}

void CheckBoxComboBox::hidePopup()
{
    QComboBox::hidePopup();
    // 解决多次点击下拉按钮后，或者是点击某一item后，再点击下拉菜单就会显示错乱或者空白
    //    view()->verticalScrollBar()->setRange()
}

void CheckBoxComboBox::slotStateChanged()
{
    QStringList list;
    for (int i = 0; i < mListWidget->count(); ++i)
    {
        auto listItem = mListWidget->item(i);
        auto checkBox = (QCheckBox*)(mListWidget->itemWidget(listItem));
        if (checkBox->isChecked())
        {
            list << checkBox->text();
        }
    }
    mSelectedText = list.join(",");
    mLineEdit->setText(mSelectedText);

    auto stateChangedCheckBox = qobject_cast<QCheckBox*>(sender());
    emit signalSelectedChanged(stateChangedCheckBox);
}

void CheckBoxComboBox::slotTextChanged()
{
    if (!mSelectedText.isEmpty())
    {
        mLineEdit->setText(mSelectedText);
    }
}

void CheckBoxComboBox::showPopup()
{
    QComboBox::showPopup();
    //    view()->verticalScrollBar()->setValue(0);
}
