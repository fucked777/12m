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

SystemWorkMode SCTaskWorkModeTaskCodeSelectWidget::getWorkMode() { return mdefaultSystemWorkMode; }

void SCTaskWorkModeTaskCodeSelectWidget::setWorkMode(SystemWorkMode workMode)
{
    mdefaultSystemWorkMode = workMode;
    slotChangeTarget(mdefaultSystemWorkMode);
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
    linkLine.workMode = mdefaultSystemWorkMode;                 // 工作模式
    linkLine.masterTargetNo = mSCTargetNoBtnGroup.checkedId();  // 主跟目标号

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

void SCTaskWorkModeTaskCodeSelectWidget::refreshRunningTaskPlanData(const ManualMessage& msg)
{
    auto manualMsg = msg;
    for (auto system : manualMsg.linkLineMap.keys())
    {
        auto link = manualMsg.linkLineMap[system];
        for (auto item : mWorkSystem.dtCommands)
        {
            if (mdefaultSystemWorkMode == KaDS && item.id == (int)SystemWorkMode::KaDS && item.id == (int)system)
            {
                int masterTargetNo = link.masterTargetNo;
                //处理主跟目标选择按钮
                refreshMasterTargetNo(masterTargetNo);
                //处理任务代号
                auto targetMap = link.targetMap;
                refreshTargetInfo(targetMap);
            }
            if (mdefaultSystemWorkMode == XDS && item.id == (int)SystemWorkMode::XDS && item.id == (int)system)
            {
                int masterTargetNo = manualMsg.linkLineMap[system].masterTargetNo;
                //处理主跟目标选择按钮
                refreshMasterTargetNo(masterTargetNo);
                //处理任务代号
                auto targetMap = link.targetMap;
                refreshTargetInfo(targetMap);
            }
            if (mdefaultSystemWorkMode == KaGS && item.id == (int)SystemWorkMode::KaGS && item.id == (int)system)
            {
                int masterTargetNo = manualMsg.linkLineMap[system].masterTargetNo;
                //处理主跟目标选择按钮
                refreshMasterTargetNo(masterTargetNo);
                //处理任务代号
                auto targetMap = link.targetMap;
                refreshTargetInfo(targetMap);
            }
        }
    }
}

void SCTaskWorkModeTaskCodeSelectWidget::refreshMasterTargetNo(int checkID)
{
    switch (checkID)
    {
    case 1: ui->scTargetRadioBtn1->setChecked(true); break;
    case 2: ui->scTargetRadioBtn2->setChecked(true); break;
    default: break;
    }
}

void SCTaskWorkModeTaskCodeSelectWidget::refreshTargetInfo(const QMap<int, TargetInfo>& targetMap)
{
    for (auto targetNo : targetMap.keys())
    {
        auto targetInfo = targetMap[targetNo];
        auto taskCode = targetInfo.taskCode;
        auto point = targetInfo.pointFreqNo;
        switch (targetNo)
        {
        case 1:
        {
            ui->scTaskCodeCombo1->setCurrentText(taskCode);
            ui->scPointFreqCombo1->setCurrentText(QString("点频%1").arg(point));
            break;
        }
        case 2:
        {
            ui->scTaskCodeCombo2->setCurrentText(taskCode);
            ui->scPointFreqCombo2->setCurrentText(QString("点频%1").arg(point));
            break;
        }
        default: break;
        }
    }
}

void SCTaskWorkModeTaskCodeSelectWidget::initUI()
{
    // 添加数传工作模式

    mSCTargetNoBtnGroup.addButton(ui->scTargetRadioBtn1, 1);
    mSCTargetNoBtnGroup.addButton(ui->scTargetRadioBtn2, 2);
}

void SCTaskWorkModeTaskCodeSelectWidget::initSlot()
{
    changedPointFreq(ui->scTaskCodeCombo1, ui->scPointFreqCombo1);
    changedPointFreq(ui->scTaskCodeCombo2, ui->scPointFreqCombo2);
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
    //    auto workMode = getWorkMode();
    //    emit signalWorkModeChanged(workMode);

    //    ui->scTargetRadioBtn1->setEnabled(false);
    //    ui->scTargetRadioBtn2->setEnabled(false);

    //    ui->scTaskCodeCombo1->setEnabled(false);
    //    ui->scTaskCodeCombo2->setEnabled(false);

    //    ui->scPointFreqCombo1->setEnabled(false);
    //    ui->scPointFreqCombo2->setEnabled(false);

    //    ui->scTaskCodeCombo1->clear();
    //    ui->scTaskCodeCombo2->clear();

    //    ui->scPointFreqCombo1->clear();
    //    ui->scPointFreqCombo2->clear();

    //    ui->scTaskCodeCombo1->addItem("无", 0);
    //    ui->scTaskCodeCombo2->addItem("无", 0);

    //    // 默认主跟目标第一个
    //    ui->scTargetRadioBtn1->setChecked(true);

    //    // 根据当前数传选择的工作模式去改变可使用的目标
    //    slotChangeTarget(workMode);
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

        ui->scTaskCodeCombo1->clear();
        ui->scTaskCodeCombo2->clear();

        ui->scTaskCodeCombo1->addItem("无", SystemWorkMode::NotDefine);
        ui->scTaskCodeCombo2->addItem("无", SystemWorkMode::NotDefine);

        for (auto taskCode : mWorkModeTaskcodeMap[workMode])
        {
            ui->scTaskCodeCombo1->addItem(taskCode, workMode);
            ui->scTaskCodeCombo2->addItem(taskCode, workMode);
        }
    }
    break;
    }
}
