#include "AddStationResManagement.h"
#include "GlobalData.h"
#include "QssCommonHelper.h"
#include "SRMUtility.h"
#include "ServiceCheck.h"
#include "StationResManagement.h"
#include "StationResManagementModel.h"
#include "ui_AddStationResManagement.h"
#include <QMessageBox>
#include <QPushButton>

AddStationResManagement::AddStationResManagement(StationResManagement* parent)
    : QDialog(parent)
    , ui(new Ui::AddStationResManagement)
    , m_parent(parent)
{
    Q_ASSERT(m_parent != nullptr);

    ui->setupUi(this);
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
    SRMUtility::uiInit(ui);
    connect(ui->confirmBtn, &QPushButton::clicked, this, &AddStationResManagement::ok);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &AddStationResManagement::reject);
    connect(m_parent, &StationResManagement::sg_addACK, this, &AddStationResManagement::addACK);
    connect(m_parent, &StationResManagement::sg_modifyACK, this, &AddStationResManagement::modifyACK);
}

AddStationResManagement::~AddStationResManagement() { delete ui; }

StationResManagementData AddStationResManagement::data() const { return m_centerData; }
void AddStationResManagement::setData(const StationResManagementData& data)
{
    SRMUtility::setUIData(ui, data);
    m_isAdd = false;
    ui->stationIdentifying->setEnabled(false);
}

void AddStationResManagement::ok()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    auto tempData = SRMUtility::getUIData(ui);
    if (tempData.stationIdentifying.isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("站标识不能为空"), QString("确定"));
        return;
    }
    if (tempData.stationIdentifying.size() > 80)
    {
        QMessageBox::information(this, QString("提示"), QString("站标识过长"), QString("确定"));
        return;
    }
    if (tempData.stationName.isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("站名不能为空"), QString("确定"));
        return;
    }

    if (tempData.ttcUACAddr.isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("测控UAC地址不能为空"), QString("确定"));
        return;
    }
    if (tempData.dataTransUACAddr.isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("数传UAC地址不能为空"), QString("确定"));
        return;
    }
    //    if (tempData.descText.isEmpty())
    //    {
    //        QMessageBox::information(this, QString("提示"), QString("站描述不能为空"), QString("确定"));
    //        return;
    //    }
    if (tempData.descText.size() > 400)
    {
        QMessageBox::information(this, QString("提示"), QString("站描述过长"), QString("确定"));
        return;
    }
    m_centerData = tempData;

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

void AddStationResManagement::addACK(const QByteArray& data)
{
    auto ack = Optional<StationResManagementData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::information(this, "添加数据失败", ack.msg(), "确定");
        return;
    }

    QMessageBox::information(this, "提示", "添加数据成功", "确定");
    accept();
}
void AddStationResManagement::modifyACK(const QByteArray& data)
{
    auto ack = Optional<StationResManagementData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::information(this, "更新数据失败", ack.msg(), "确定");
        return;
    }

    QMessageBox::information(this, "提示", "更新数据成功", "确定");
    accept();
}

void AddStationResManagement::on_desc_textChanged()
{
    QString textContent = ui->desc->toPlainText();
    int length = textContent.count();
    int maxLength = 40;  // 最大字符数
    if (length > maxLength)
    {
        int position = ui->desc->textCursor().position();
        QTextCursor textCursor = ui->desc->textCursor();
        textContent.remove(position - (length - maxLength), length - maxLength);
        ui->desc->setPlainText(textContent);
        textCursor.setPosition(position - (length - maxLength));
        ui->desc->setTextCursor(textCursor);
    }
}
