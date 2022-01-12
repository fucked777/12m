#include "AddTaskCenterDialog.h"
#include "GlobalData.h"
#include "QssCommonHelper.h"
#include "ServiceCheck.h"
#include "TCUtility.h"
#include "TaskCenter.h"
#include "TaskCenterModel.h"
#include "ui_AddTaskCenterDialog.h"
#include <QMessageBox>
#include <QPushButton>
#include <QRegExp>

// 匹配UAC地址十六进制，8字符
static bool matchingUAC(const QString& str)
{
    QRegExp reg(R"(^[0-9a-fA-F]{1,8}$)");
    return reg.exactMatch(str);
}

// 匹配中心代号十六进制，6字符
static bool matchingZXDH(const QString& str)
{
    QRegExp reg(R"([0-9a-zA-Z_]+)");
    return reg.exactMatch(str);
}

AddTaskCenterDialog::AddTaskCenterDialog(TaskCenter* parent)
    : QDialog(parent)
    , ui(new Ui::AddTaskCenterDialog)
    , m_parent(parent)
{
    Q_ASSERT(m_parent != nullptr);

    ui->setupUi(this);
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
    TCUtility::uiInit(ui);
    connect(ui->confirmBtn, &QPushButton::clicked, this, &AddTaskCenterDialog::ok);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &AddTaskCenterDialog::reject);

    connect(m_parent, &TaskCenter::sg_addACK, this, &AddTaskCenterDialog::addACK);
    connect(m_parent, &TaskCenter::sg_modifyACK, this, &AddTaskCenterDialog::modifyACK);
}

AddTaskCenterDialog::~AddTaskCenterDialog() { delete ui; }

TaskCenterData AddTaskCenterDialog::data() const { return m_taskCenter; }
void AddTaskCenterDialog::setData(const TaskCenterData& data)
{
    TCUtility::setUIData(ui, data);
    m_isAdd = false;
    ui->UACLineEdit->setEnabled(false);  // 编辑时不可用，以 UAC 作为主键
}

void AddTaskCenterDialog::ok()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    auto tempData = TCUtility::getUIData(ui);
    if (tempData.centerName.isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("中心名称不能为空"), QString("确定"));
        return;
    }
    if (tempData.centerName.size() > 80)
    {
        QMessageBox::information(this, QString("提示"), QString("中心名称过长"), QString("确定"));
        return;
    }

    if (!matchingZXDH(tempData.centerCode))
    {
        QMessageBox::information(this, QString("提示"), QString("中心代号必须为数字或字母或_等字符，且最多6个字符"), QString("确定"));
        return;
    }

    if (!matchingUAC(tempData.uacAddr))
    {
        QMessageBox::information(this, QString("提示"), QString("UAC地址必须为十六进制，且最多8个字符"), QString("确定"));
        return;
    }

    m_taskCenter = tempData;

    QByteArray array;
    array << tempData;

    /* 数据重复检查是在服务端做的 */
    if (m_isAdd)
    {
        emit m_parent->sg_add(array);
    }
    else
    {
        emit m_parent->sg_modify(array);
    }
}

void AddTaskCenterDialog::addACK(const QByteArray& data)
{
    auto ack = Optional<TaskCenterData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::information(this, "添加数据失败", ack.msg(), "确定");
        return;
    }

    QMessageBox::information(this, "提示", "添加数据成功", "确定");
    accept();
}
void AddTaskCenterDialog::modifyACK(const QByteArray& data)
{
    auto ack = Optional<TaskCenterData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::information(this, "更新数据失败", ack.msg(), "确定");
        return;
    }

    QMessageBox::information(this, "提示", "更新数据成功", "确定");
    accept();
}
