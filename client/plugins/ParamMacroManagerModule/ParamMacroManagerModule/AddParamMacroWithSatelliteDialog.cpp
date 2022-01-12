#include "AddParamMacroWithSatelliteDialog.h"
#include "ServiceCheck.h"
#include "ui_AddParamMacroWithSatelliteDialog.h"

#include <QMessageBox>

#include "GlobalData.h"
#include "SatelliteManagementSerialize.h"

AddParamMacroWithSatelliteDialog::AddParamMacroWithSatelliteDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AddParamMacroWithSatelliteDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("添加参数宏");
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    ui->workModeLineEdit->setEnabled(false);

    init();
}

AddParamMacroWithSatelliteDialog::~AddParamMacroWithSatelliteDialog() { delete ui; }

void AddParamMacroWithSatelliteDialog::on_add_clicked()
{
    // 判断服务器是否离线
    if (!SERVICEONLINE)
    {
        QMessageBox::information(this, QString("提示"), "当前监控服务器离线", QString("确定"));
        return;
    }

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    if (ui->satelliteCombo->currentData().isNull())
    {
        QMessageBox::information(this, "提示", "获取卫星数据错误", "确定");
        return;
    }

    mCurrentParamMacro.name = ui->paramMacroName->text();
    mCurrentParamMacro.taskCode = ui->satelliteCombo->currentData().toString();
    mCurrentParamMacro.workMode = ui->workModeLineEdit->text();
    mCurrentParamMacro.desc = ui->desc->toPlainText();

    for (auto paramMacro : mParamMacroList)
    {
        if (paramMacro.name == mCurrentParamMacro.name)
        {
            QMessageBox::information(this, QString("提示"), QString("当前参数宏名称已经存在"), QString("确定"));
            return;
        }

        if (paramMacro.taskCode == mCurrentParamMacro.taskCode)
        {
            QMessageBox::information(this, QString("提示"), QString("当前任务代号已经存在"), QString("确定"));
            return;
        }
    }

    if (!m_codeWorkModeMap.contains(mCurrentParamMacro.taskCode))
    {
        QMessageBox::information(this, "提示", "获取卫星数据错误", "确定");
        return;
    }
    if (mCurrentParamMacro.name.trimmed() == "")
    {
        QMessageBox::information(this, QString("提示"), QString("参数宏名称不能为空"), QString("确定"));
        return;
    }
    auto map = m_codeWorkModeMap[mCurrentParamMacro.taskCode];
    for (auto workMode : map.keys())
    {
        // 根据点频数量添加点频
        for (int i = 1; i <= map[workMode]; i++)
        {
            mCurrentParamMacro.modeParamMap[workMode].pointFreqParamMap[i] = QMap<DeviceID, DeviceData>();
        }
    }
    QDialog::accept();
}

void AddParamMacroWithSatelliteDialog::on_cancel_clicked() { QDialog::close(); }

ParamMacroData AddParamMacroWithSatelliteDialog::getCurrentParamMacro() const { return mCurrentParamMacro; }

void AddParamMacroWithSatelliteDialog::init()
{
    if (!GlobalData::getParamMacroData(mParamMacroList))
    {
        QMessageBox::warning(this, QString("提示"), QString("获取参数宏数据失败"), QString("确定"));
        return;
    }

    SatelliteManagementDataList dataList;
    GlobalData::getSatelliteManagementData(dataList);

    if (!dataList.size())
    {
        QMessageBox::information(this, "提示", "获取卫星数据失败", "确定");
    }

    for (auto item : dataList)
    {
        mWorkModeDescMap[item.m_satelliteCode] = item.getWorkModeDesc();
        m_currentWorkMode[item.m_satelliteCode] = item.m_workMode;
        ui->satelliteCombo->addItem(item.m_satelliteCode, item.m_satelliteCode);
        m_codeWorkModeMap[item.m_satelliteCode] = item.getDpNumMap();
    }
}

void AddParamMacroWithSatelliteDialog::on_desc_textChanged()
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

void AddParamMacroWithSatelliteDialog::on_satelliteCombo_currentIndexChanged(const QString& arg1)
{
    ui->workModeLineEdit->setText(mWorkModeDescMap[ui->satelliteCombo->currentText()].join("+"));
}
