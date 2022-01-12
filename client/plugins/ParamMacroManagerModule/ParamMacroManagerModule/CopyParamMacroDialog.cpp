#include "CopyParamMacroDialog.h"
#include "ui_CopyParamMacroDialog.h"

#include "GlobalData.h"
#include "ParamMacroMessage.h"
#include "SatelliteManagementDefine.h"

#include <QCheckBox>
#include <QMessageBox>
#include <QVBoxLayout>

CopyParamMacroDialog::CopyParamMacroDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::CopyParamMacroDialog)
{
    ui->setupUi(this);
    setWindowTitle("复制参数宏");
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

    init();
    initSlot();
}

CopyParamMacroDialog::~CopyParamMacroDialog() { delete ui; }

ParamMacroData CopyParamMacroDialog::getCopyParamMacroData() const { return mCopyParamMacro; }

CopyMode CopyParamMacroDialog::getCopyMode() { return (CopyMode)(ui->modeComboBox->currentData().toInt()); }

void CopyParamMacroDialog::init()
{
    ui->modeComboBox->addItem("复制到新参数宏", static_cast<int>(CopyMode::NewParamMacro));
    ui->modeComboBox->addItem("复制到已存在参数宏", static_cast<int>(CopyMode::ExistParamMacro));

    // 获取全部参数宏数据
    if (!GlobalData::getParamMacroData(mParamMacroDataList))
    {
        QMessageBox::warning(this, QString("提示"), QString("获取参数宏数据失败"), QString("确定"));
        return;
    }

    // 初始化参数宏下拉框
    for (auto item : mParamMacroDataList)
    {
        ui->srcParamMacroComboBox->addItem(item.name, item.taskCode);

        ui->existSrcParamMacroComboBox->addItem(item.name, item.taskCode);
        ui->destParamMacroComboBox->addItem(item.name, item.taskCode);
    }

    // 初始化复制已存在参数宏界面
    auto modeVLayout = new QVBoxLayout();
    auto workModeList = SystemWorkModeHelper::getWorkModes();
    for (auto workMode : workModeList)
    {
        auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(workMode);

        auto checkBox = new QCheckBox(workModeDesc);
        checkBox->setProperty("systemWorkModeId", workMode);

        modeVLayout->addWidget(checkBox);
        mModeCheckBoxList.append(checkBox);
    }

    ui->selecteParamMacrModeWidget->setLayout(modeVLayout);
}

void CopyParamMacroDialog::initSlot()
{
    /***** 复制到新参数宏界面 信号槽 ****/
    connect(ui->srcParamMacroComboBox, &QComboBox::currentTextChanged, this,
            &CopyParamMacroDialog::slotNewCreateScrParamMacroComboBoxCurrentTextChanged);

    /***** 复制到已存在参数宏界面 信号槽 ****/
    connect(ui->existSrcParamMacroComboBox, &QComboBox::currentTextChanged, this,
            &CopyParamMacroDialog::slotExistSrcParamMacroComboBoxCurrentTextChanged);
    connect(ui->destParamMacroComboBox, &QComboBox::currentTextChanged, this,
            &CopyParamMacroDialog::slotExistDescParamMacroComboBoxCurrentTextChanged);

    connect(ui->modeComboBox, &QComboBox::currentTextChanged, this, &CopyParamMacroDialog::slotCopyModeChanged);

    connect(ui->okBtn, &QPushButton::clicked, this, &CopyParamMacroDialog::slotOkBtnClicked);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &CopyParamMacroDialog::slotCancelBtnClicked);

    // 触发一次信号
    emit ui->modeComboBox->currentTextChanged(ui->modeComboBox->currentText());
    emit ui->existSrcParamMacroComboBox->currentTextChanged(ui->existSrcParamMacroComboBox->currentText());
    emit ui->destParamMacroComboBox->currentTextChanged(ui->destParamMacroComboBox->currentText());
}

void CopyParamMacroDialog::updateSelectableMode()
{
    // 根据当前参数宏数据，显示当前参数宏的模式
    for (auto checkBox : mModeCheckBoxList)
    {
        checkBox->setChecked(false);

        auto systemModeId = SystemWorkMode(checkBox->property("systemWorkModeId").toInt());
        if (mSrcParamMacroData.modeParamMap.contains(systemModeId) && mDestParamMacroData.modeParamMap.contains(systemModeId))
        {
            checkBox->setHidden(false);
        }
        else
        {
            checkBox->setHidden(true);
        }
    }
}

void CopyParamMacroDialog::slotCopyModeChanged(const QString& text)
{
    auto copyMode = CopyMode(ui->modeComboBox->currentData().toInt());
    switch (copyMode)
    {
    case CopyMode::NewParamMacro: ui->stackedWidget->setCurrentIndex(0); break;
    case CopyMode::ExistParamMacro: ui->stackedWidget->setCurrentIndex(1); break;
    default: ui->stackedWidget->setCurrentIndex(0);
    }
}

void CopyParamMacroDialog::slotNewCreateScrParamMacroComboBoxCurrentTextChanged(const QString& text)
{
    auto taskCode = ui->srcParamMacroComboBox->currentData().toString();

    // 通过参数宏的任务代号获取当前参数宏对应的卫星数据
    SatelliteManagementData currentParamMacroSatelliteData;
    SatelliteManagementDataList satelliteList;
    GlobalData::getSatelliteManagementData(satelliteList);
    for (auto item : satelliteList)
    {
        if (item.m_satelliteCode == taskCode)
        {
            currentParamMacroSatelliteData = item;
        }
    }

    ui->taskCodeComboBox->clear();
    // 添加和当前参数宏相同模式的任务代号
    for (auto item : satelliteList)
    {
        if (item.m_workModeParamMap.keys() == currentParamMacroSatelliteData.m_workModeParamMap.keys())
        {
            ui->taskCodeComboBox->addItem(item.m_satelliteCode);
        }
    }
}

void CopyParamMacroDialog::slotExistSrcParamMacroComboBoxCurrentTextChanged(const QString& text)
{
    auto taskCode = ui->existSrcParamMacroComboBox->currentData().toString();

    // 获取当前参数宏数据
    for (auto item : mParamMacroDataList)
    {
        if (item.taskCode == taskCode)
        {
            mSrcParamMacroData = item;
            break;
        }
    }

    QStringList modeList;
    for (auto workMode : mSrcParamMacroData.modeParamMap.keys())
    {
        modeList << SystemWorkModeHelper::systemWorkModeToDesc(workMode);
    }
    ui->existSrcParamMacroModeLabel->setText(modeList.join("+"));

    // 更新可选择的模式
    updateSelectableMode();
}

void CopyParamMacroDialog::slotExistDescParamMacroComboBoxCurrentTextChanged(const QString& text)
{
    auto taskCode = ui->destParamMacroComboBox->currentData().toString();

    // 获取当前参数宏数据
    for (auto item : mParamMacroDataList)
    {
        if (item.taskCode == taskCode)
        {
            mDestParamMacroData = item;
            break;
        }
    }

    QStringList modeList;
    for (auto workMode : mDestParamMacroData.modeParamMap.keys())
    {
        modeList << SystemWorkModeHelper::systemWorkModeToDesc(workMode);
    }
    ui->existDestParamMacroModeLabel->setText(modeList.join("+"));

    // 更新可选择的模式
    updateSelectableMode();
}

void CopyParamMacroDialog::slotOkBtnClicked()
{
    auto copyMode = CopyMode(ui->modeComboBox->currentData().toInt());

    // 当前是复制到新参数宏
    if (copyMode == CopyMode::NewParamMacro)
    {
        auto srcParamMacroName = ui->srcParamMacroComboBox->currentText();
        auto newParamMacroName = ui->newMacroNameLineEdit->text();
        auto newTaskCode = ui->taskCodeComboBox->currentText();
        auto newParamMacroDesc = ui->newMacroDescPlainTextEdit->toPlainText();

        for (auto item : mParamMacroDataList)
        {
            if (item.taskCode == newTaskCode)
            {
                QMessageBox::information(this, QString("提示"), QString("已经存在当前任务代号的参数宏"), QString("确定"));
                return;
            }
        }

        if (newParamMacroName.isEmpty())
        {
            QMessageBox::information(this, QString("提示"), QString("参数宏名称不能为空"), QString("确定"));
            return;
        }
        if (newTaskCode.isEmpty())
        {
            QMessageBox::information(this, QString("提示"), QString("任务代号不能为空"), QString("确定"));
            return;
        }

        ParamMacroData srcParamMacro;
        for (auto item : mParamMacroDataList)
        {
            if (item.name == newParamMacroName)
            {
                QMessageBox::information(this, QString("提示"), QString("当前参数宏名称已经存在"), QString("确定"));
                return;
            }
            else if (item.name == srcParamMacroName)
            {
                srcParamMacro = item;
            }
        }

        mCopyParamMacro = srcParamMacro;
        mCopyParamMacro.name = newParamMacroName;
        mCopyParamMacro.taskCode = newTaskCode;
        mCopyParamMacro.desc = newParamMacroDesc;
    }
    else  // 当前是复制到已存在参数宏
    {
        bool hasSelected = false;
        for (auto checkBox : mModeCheckBoxList)
        {
            // 模式选中了的进行复制
            if (!checkBox->isHidden() && checkBox->isChecked())
            {
                auto workMode = (SystemWorkMode)checkBox->property("systemWorkModeId").toInt();

                mDestParamMacroData.modeParamMap[workMode] = mSrcParamMacroData.modeParamMap[workMode];

                hasSelected = true;
            }
        }
        if (!hasSelected)
        {
            QMessageBox::information(this, QString("提示"), QString("请选择需要复制的模式"), QString("确定"));
            return;
        }

        // 赋值到复制参数宏，供外界获取
        mCopyParamMacro = mDestParamMacroData;
    }
    QDialog::accept();
}

void CopyParamMacroDialog::slotCancelBtnClicked() { close(); }
