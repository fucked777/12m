#include "DMEditDialog.h"
#include "DMMessageSerialize.h"
#include "DMParamInputOrDisplay.h"
#include "DMUIUtility.h"
#include "DMWidget.h"
#include "GlobalData.h"
#include "ServiceCheck.h"
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>

DMEditDialog::DMEditDialog(const UIDataParameter& parameter, DMWidget* parent)
    : QDialog(parent)
    , m_parent(parent)
    , m_dmParamInputOrDisplay(new DMParamInputOrDisplay(parameter, this, true))
{
    Q_ASSERT(parent != nullptr);

    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(9, 9, 9, 9);
    vLayout->setSpacing(0);
    vLayout->addWidget(m_dmParamInputOrDisplay);

    auto hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);

    auto okBtn = new QPushButton(this);
    auto cancelBtn = new QPushButton(this);

    okBtn->setText("确认");
    cancelBtn->setText("取消");

    hLayout->addStretch();
    hLayout->addWidget(okBtn);
    hLayout->addWidget(cancelBtn);

    vLayout->addLayout(hLayout);

    m_devName = parameter.name;
    this->setWindowTitle(QString("添加设备-%1").arg(m_devName));

    if (parameter.treeInfo.maxGroup <= 0)
    {
        this->setMinimumSize(420, 550);
    }
    else
    {
        this->setMinimumSize(1020, 550);
    }
    connect(okBtn, &QPushButton::clicked, this, &DMEditDialog::ok);
    connect(cancelBtn, &QPushButton::clicked, this, &DMEditDialog::reject);
    connect(parent, &DMWidget::sg_addACK, this, &DMEditDialog::addACK);
    connect(parent, &DMWidget::sg_modifyACK, this, &DMEditDialog::modifyACK);
}

DMEditDialog::~DMEditDialog() {}
void DMEditDialog::setUIValue(const DMDataItem& value)
{
    m_isAdd = false;
    this->setWindowTitle(QString("编辑设备-%1").arg(m_devName));
    m_id = value.id;
    m_dmParamInputOrDisplay->setUIValue(value);
    m_dmParamInputOrDisplay->setFromOnlyOneDisable();
}
void DMEditDialog::ok()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    auto value = m_dmParamInputOrDisplay->getUIValue();
    if (!value)
    {
        QMessageBox::warning(this, "获取数据失败", value.msg(), "确定");
        return;
    }

    QByteArray array;

    if (m_isAdd)
    {
        value->typeName = m_devName;
        array << value.value();
        emit m_parent->sg_add(array);
    }
    else
    {
        value->id = m_id;
        value->typeName = m_devName;
        array << value.value();
        emit m_parent->sg_modify(array);
    }
}

void DMEditDialog::addACK(const QByteArray& value)
{
    auto ackResult = Optional<DMDataItem>::emptyOptional();
    value >> ackResult;
    if (!ackResult)
    {
        QMessageBox::information(this, "添加失败", ackResult.msg(), QString("确定"));
        return;
    }
    accept();
}

void DMEditDialog::modifyACK(const QByteArray& value)
{
    auto ackResult = Optional<DMDataItem>::emptyOptional();
    value >> ackResult;
    if (!ackResult)
    {
        QMessageBox::information(this, "更新失败", ackResult.msg(), QString("确定"));
        return;
    }
    else
    {
        QMessageBox::information(this, "提示", QString("编辑成功"));
    }

    accept();
}
