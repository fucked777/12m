#include "SCTaskWorkModeTaskCodeSelectWidget.h"
#include "ui_SCTaskWorkModeTaskCodeSelectWidget.h"

SCTaskWorkModeTaskCodeSelectWidget::SCTaskWorkModeTaskCodeSelectWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SCTaskWorkModeTaskCodeSelectWidget)
{
    ui->setupUi(this);

    // 读取工作模式配置文件
    WorkModeXMLReader reader;
    mWorkSystem = reader.getWorkSystem();

    initUI();

    initSlot();
}

SCTaskWorkModeTaskCodeSelectWidget::~SCTaskWorkModeTaskCodeSelectWidget() { delete ui; }

SystemWorkMode SCTaskWorkModeTaskCodeSelectWidget::getWorkMode() { return (SystemWorkMode)ui->scWorkModeCombo->currentData().toInt(); }

void SCTaskWorkModeTaskCodeSelectWidget::setWorkMode(SystemWorkMode workMode)
{
    auto index = ui->scWorkModeCombo->findData((int)workMode);
    if (index != -1)
    {
        // 如果工作模式和原来一样，也触发一下信号
        if (index == ui->scWorkModeCombo->currentIndex())
        {
            emit ui->scWorkModeCombo->currentIndexChanged(index);
        }
        else
        {
            ui->scWorkModeCombo->setCurrentIndex(index);
        }
    }
}

bool SCTaskWorkModeTaskCodeSelectWidget::isSelectedTaskCode()
{
    if (ui->scTaskCodeCombo1->isEnabled() && ui->scTaskCodeCombo1->currentText() != QString("无"))
    {
        return true;
    }

    if (ui->scTaskCodeCombo2->isEnabled() && ui->scTaskCodeCombo2->currentText() != QString("无"))
    {
        return true;
    }

    return false;
}

void SCTaskWorkModeTaskCodeSelectWidget::setWorkModeTaskCodeMap(const QMap<SystemWorkMode, QList<QString>>& workModeTaskCodeMap)
{
    mWorkModeTaskcodeMap = workModeTaskCodeMap;
}

void SCTaskWorkModeTaskCodeSelectWidget::setTaskCodeWorkModeDpMap(const QMap<QString, QMap<SystemWorkMode, int>>& taskCodeWorkModeDpMap)
{
    mTaskCodeWorkModeDpMap = taskCodeWorkModeDpMap;
}

LinkLine SCTaskWorkModeTaskCodeSelectWidget::getLinkLine()
{
    LinkLine linkLine;
    linkLine.workMode = (SystemWorkMode)ui->scWorkModeCombo->currentData().toUInt();  // 工作模式
    linkLine.masterTargetNo = mSCTargetNoBtnGroup.checkedId();                        // 主跟目标号

    if (ui->scTaskCodeCombo1->currentText() != "无")
    {
        TargetInfo targetInfo;
        targetInfo.targetNo = 1;
        targetInfo.taskCode = ui->scTaskCodeCombo1->currentText();
        targetInfo.pointFreqNo = ui->scPointFreqCombo1->currentData().toUInt();
        targetInfo.workMode = SystemWorkModeHelper::getTargetWorkMode(linkLine.workMode, targetInfo.targetNo);

        linkLine.targetMap[targetInfo.targetNo] = targetInfo;
    }

    if (ui->scTaskCodeCombo2->currentText() != "无")
    {
        TargetInfo targetInfo;
        targetInfo.targetNo = 2;
        targetInfo.taskCode = ui->scTaskCodeCombo2->currentText();
        targetInfo.pointFreqNo = ui->scPointFreqCombo2->currentData().toUInt();
        targetInfo.workMode = SystemWorkModeHelper::getTargetWorkMode(linkLine.workMode, targetInfo.targetNo);

        linkLine.targetMap[targetInfo.targetNo] = targetInfo;
    }

    // 如果主用目标没有包含，默认使用第一个作为主用目标
    if (!linkLine.targetMap.isEmpty() && !linkLine.targetMap.contains(linkLine.masterTargetNo))
    {
        linkLine.masterTargetNo = linkLine.targetMap.firstKey();
    }

    return linkLine;
}

void SCTaskWorkModeTaskCodeSelectWidget::initUI()
{
    // 添加数传工作模式
    ui->scWorkModeCombo->addItem("无", NotDefine);
    for (auto item : mWorkSystem.dtCommands)
    {
        ui->scWorkModeCombo->addItem(item.desc, item.id);
    }

    mSCTargetNoBtnGroup.addButton(ui->scTargetRadioBtn1, 1);
    mSCTargetNoBtnGroup.addButton(ui->scTargetRadioBtn2, 2);
}

void SCTaskWorkModeTaskCodeSelectWidget::initSlot()
{
    connect(ui->scWorkModeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &SCTaskWorkModeTaskCodeSelectWidget::slotWorkModeChanged);

    changedPointFreq(ui->scTaskCodeCombo1, ui->scPointFreqCombo1);
    changedPointFreq(ui->scTaskCodeCombo2, ui->scPointFreqCombo2);

    // 触发信号
    emit ui->scWorkModeCombo->currentIndexChanged(0);
}

void SCTaskWorkModeTaskCodeSelectWidget::changedPointFreq(QComboBox* taskCodeCombo, QComboBox* pointFreqCombo)
{
    connect(taskCodeCombo, &QComboBox::currentTextChanged, [=]() {
        pointFreqCombo->clear();

        auto tempTaskCode = taskCodeCombo->currentText();
        auto targetWorkMode = (SystemWorkMode)taskCodeCombo->currentData().toInt();  // 获取目标的工作模式
        for (auto i = 1; i <= mTaskCodeWorkModeDpMap[tempTaskCode][targetWorkMode]; ++i)
        {
            pointFreqCombo->addItem(QString("点频%1").arg(i), i);
        }
    });
}

void SCTaskWorkModeTaskCodeSelectWidget::slotWorkModeChanged(int)
{
    auto workMode = getWorkMode();
    emit signalWorkModeChanged(workMode);

    ui->scTargetRadioBtn1->setEnabled(false);
    ui->scTargetRadioBtn2->setEnabled(false);

    ui->scTaskCodeCombo1->setEnabled(false);
    ui->scTaskCodeCombo2->setEnabled(false);

    ui->scPointFreqCombo1->setEnabled(false);
    ui->scPointFreqCombo2->setEnabled(false);

    ui->scTaskCodeCombo1->clear();
    ui->scTaskCodeCombo2->clear();

    ui->scPointFreqCombo1->clear();
    ui->scPointFreqCombo2->clear();

    ui->scTaskCodeCombo1->addItem("无", 0);
    ui->scTaskCodeCombo2->addItem("无", 0);

    // 默认主跟目标第一个
    ui->scTargetRadioBtn1->setChecked(true);

    // 根据当前数传选择的工作模式去改变可使用的目标
    slotChangeTarget(workMode);
}

void SCTaskWorkModeTaskCodeSelectWidget::slotChangeTarget(SystemWorkMode workMode)
{
    if (workMode == NotDefine)
    {
        return;
    }

    switch (workMode)
    {
    case KaGS:  // Ka高速
    case XGS:   // X高速
    case KaDS:  // Ka低速
    case XDS:   // X低速
    {
        ui->scTargetRadioBtn1->setEnabled(true);
        ui->scTargetRadioBtn2->setEnabled(true);

        ui->scTaskCodeCombo1->setEnabled(true);
        ui->scTaskCodeCombo2->setEnabled(true);

        ui->scPointFreqCombo1->setEnabled(true);
        ui->scPointFreqCombo2->setEnabled(true);

        for (auto taskCode : mWorkModeTaskcodeMap[workMode])
        {
            ui->scTaskCodeCombo1->addItem(taskCode, workMode);
            ui->scTaskCodeCombo2->addItem(taskCode, workMode);
        }
    }
    break;
    }
}
