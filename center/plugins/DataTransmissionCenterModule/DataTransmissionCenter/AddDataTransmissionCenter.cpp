#include "AddDataTransmissionCenter.h"
#include "DTCUtility.h"
#include "DataTransmissionCenter.h"
#include "DataTransmissionModel.h"
#include "GlobalData.h"
#include "QssCommonHelper.h"
#include "ServiceCheck.h"
#include "ui_AddDataTransmissionCenter.h"
#include <QMessageBox>
#include <QPushButton>

AddDataTransmissionCenter::AddDataTransmissionCenter(DataTransmissionCenter* parent)
    : QDialog(parent)
    , ui(new Ui::AddDataTransmissionCenter)
    , m_parent(parent)
{
    Q_ASSERT(m_parent != nullptr);

    ui->setupUi(this);
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
    DTCUtility::uiInit(ui);
    connect(ui->confirmBtn, &QPushButton::clicked, this, &AddDataTransmissionCenter::ok);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &AddDataTransmissionCenter::rejected);

    connect(m_parent, &DataTransmissionCenter::sg_addACK, this, &AddDataTransmissionCenter::addACK);
    connect(m_parent, &DataTransmissionCenter::sg_modifyACK, this, &AddDataTransmissionCenter::modifyACK);
}

AddDataTransmissionCenter::~AddDataTransmissionCenter() { delete ui; }

DataTransmissionCenterData AddDataTransmissionCenter::data() const { return m_centerData; }
void AddDataTransmissionCenter::setData(const DataTransmissionCenterData& data)
{
    DTCUtility::setUIData(ui, data);
    m_isAdd = false;
    ui->centerName->setEnabled(false);  // 编辑时不可用，以 中心名 作为主键
}

void AddDataTransmissionCenter::ok()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    auto tempData = DTCUtility::getUIData(ui);
    if (tempData.centerName.isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("中心名称不能为空"), QString("确定"));
        return;
    }
    if (tempData.centerName.size() > 40)
    {
        QMessageBox::information(this, QString("提示"), QString("中心名称过长"), QString("确定"));
        return;
    }

    if (!Regexp::matching(Regexp::ip(), tempData.centerIP))
    {
        QMessageBox::information(this, QString("提示"), QString("IP地址不合法"), QString("确定"));
        return;
    }
    if (tempData.centerXS.isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("中心信宿不能为空"), QString("确定"));
        return;
    }
    if (tempData.desc.size() > 400)
    {
        QMessageBox::information(this, QString("提示"), QString("描述过长"), QString("确定"));
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
void AddDataTransmissionCenter::addACK(const QByteArray& data)
{
    auto ack = Optional<DataTransmissionCenterData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::information(this, "添加数据失败", ack.msg(), "确定");
        return;
    }

    QMessageBox::information(this, "提示", "添加数据成功", "确定");
    accept();
}
void AddDataTransmissionCenter::modifyACK(const QByteArray& data)
{
    auto ack = Optional<DataTransmissionCenterData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::information(this, "更新数据失败", ack.msg(), "确定");
        return;
    }

    QMessageBox::information(this, "提示", "更新数据成功", "确定");
    accept();
}

void AddDataTransmissionCenter::on_desc_textChanged()
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

void AddDataTransmissionCenter::on_cancelBtn_clicked() { accept(); }
