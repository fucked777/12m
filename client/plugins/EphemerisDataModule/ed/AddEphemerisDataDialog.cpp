#include "AddEphemerisDataDialog.h"
#include "EDUtility.h"
#include "EphemerisData.h"
#include "GlobalData.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "ui_AddEphemerisDataDialog.h"
#include <QMessageBox>
#include <QPushButton>

AddEphemerisDataDialog::AddEphemerisDataDialog(EphemerisData* parent)
    : QDialog(parent)
    , ui(new Ui::AddEphemerisDataDialog)
    , m_parent(parent)
{
    Q_ASSERT(parent != nullptr);
    ui->setupUi(this);
    QssCommonHelper::setWidgetStyle(this, "Common.qss");
    EDUtility::uiInit(ui);
    connect(ui->confirmBtn, &QPushButton::clicked, this, &AddEphemerisDataDialog::ok);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &AddEphemerisDataDialog::reject);
    connect(ui->satellitCode, &QComboBox::currentTextChanged, this, &AddEphemerisDataDialog::satellitCodeChange);

    connect(m_parent, &EphemerisData::sg_addACK, this, &AddEphemerisDataDialog::addACK);
    connect(m_parent, &EphemerisData::sg_modifyACK, this, &AddEphemerisDataDialog::modifyACK);
}

AddEphemerisDataDialog::~AddEphemerisDataDialog() { delete ui; }

EphemerisDataData AddEphemerisDataDialog::data() const { return m_taskCenter; }
void AddEphemerisDataDialog::setData(const SatelliteManagementMap& smm, const EphemerisDataData& data)
{
    setData(smm);
    EDUtility::setUIData(ui, smm, data);
    m_isAdd = false;
    ui->satellitCode->setEnabled(false);  // 编辑时不可用
}
void AddEphemerisDataDialog::setData(const SatelliteManagementMap& smm)
{
    m_map = smm;
    ui->satellitCode->clear();
    for (auto& item : smm)
    {
        ui->satellitCode->addItem(item.m_satelliteCode, item.m_satelliteCode);
    }
}

void AddEphemerisDataDialog::ok()
{
    SERVICEONLINECHECK();

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    auto tempData = EDUtility::getUIData(ui);
    if (tempData.satellitCode.isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("任务代号不能为空"), QString("确定"));
        return;
    }

    if (ui->satelliteName->text().isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("卫星名称不能为空"), QString("确定"));
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

void AddEphemerisDataDialog::satellitCodeChange(const QString& arg1)
{
    Q_UNUSED(arg1);
    auto taskCode = ui->satellitCode->currentText();
    if (m_map.contains(taskCode))
    {
        ui->satelliteIdentification->setText(m_map[taskCode].m_satelliteIdentification);
        ui->satelliteName->setText(m_map[taskCode].m_satelliteName);
    }
}

void AddEphemerisDataDialog::addACK(const QByteArray& data)
{
    auto ack = Optional<EphemerisDataData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::information(this, "添加数据失败", ack.msg(), "确定");
        return;
    }

    QMessageBox::information(this, "提示", "添加数据成功", "确定");
    accept();
}
void AddEphemerisDataDialog::modifyACK(const QByteArray& data)
{
    auto ack = Optional<EphemerisDataData>::emptyOptional();
    data >> ack;

    if (!ack)
    {
        QMessageBox::information(this, "更新数据失败", ack.msg(), "确定");
        return;
    }

    QMessageBox::information(this, "提示", "更新数据成功", "确定");
    accept();
}
