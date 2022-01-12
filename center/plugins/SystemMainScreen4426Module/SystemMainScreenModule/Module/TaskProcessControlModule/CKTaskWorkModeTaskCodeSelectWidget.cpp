#include "CKTaskWorkModeTaskCodeSelectWidget.h"
#include "ui_CKTaskWorkModeTaskCodeSelectWidget.h"

CKTaskWorkModeTaskCodeSelectWidget::CKTaskWorkModeTaskCodeSelectWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CKTaskWorkModeTaskCodeSelectWidget)
{
    ui->setupUi(this);

    // 读取工作模式配置文件
    WorkModeXMLReader reader;
    mWorkSystem = reader.getWorkSystem();

    // initUI();

    initSlot();
}

CKTaskWorkModeTaskCodeSelectWidget::~CKTaskWorkModeTaskCodeSelectWidget() { delete ui; }

QComboBox* CKTaskWorkModeTaskCodeSelectWidget::getWorkModeComboBox() { return ui->ckWorkModeCombo; }

SystemWorkMode CKTaskWorkModeTaskCodeSelectWidget::getWorkMode() { return (SystemWorkMode)ui->ckWorkModeCombo->currentData().toInt(); }

void CKTaskWorkModeTaskCodeSelectWidget::setWorkMode(SystemWorkMode workMode)
{
    auto index = ui->ckWorkModeCombo->findData((int)workMode);
    if (index != -1)
    {
        // 如果工作模式和原来一样，也触发一下信号
        if (index == ui->ckWorkModeCombo->currentIndex())
        {
            emit ui->ckWorkModeCombo->currentIndexChanged(index);
        }
        else
        {
            ui->ckWorkModeCombo->setCurrentIndex(index);
        }
    }
}

bool CKTaskWorkModeTaskCodeSelectWidget::isSelectedTaskCode()
{
    if (ui->ckTaskCodeCombo1->isEnabled() && ui->ckTaskCodeCombo1->currentText() != QString("无"))
    {
        return true;
    }

    if (ui->ckTaskCodeCombo2->isEnabled() && ui->ckTaskCodeCombo2->currentText() != QString("无"))
    {
        return true;
    }

    if (ui->ckTaskCodeCombo3->isEnabled() && ui->ckTaskCodeCombo3->currentText() != QString("无"))
    {
        return true;
    }

    if (ui->ckTaskCodeCombo4->isEnabled() && ui->ckTaskCodeCombo4->currentText() != QString("无"))
    {
        return true;
    }

    return false;
}

void CKTaskWorkModeTaskCodeSelectWidget::setWorkModeTaskCodeMap(const QMap<SystemWorkMode, QList<QString>>& workModeTaskCodeMap)
{
    mWorkModeTaskcodeMap = workModeTaskCodeMap;
}

void CKTaskWorkModeTaskCodeSelectWidget::setTaskCodeWorkModeDpMap(const QMap<QString, QMap<SystemWorkMode, int>>& taskCodeWorkModeDpMap)
{
    mTaskCodeWorkModeDpMap = taskCodeWorkModeDpMap;
}

LinkLine CKTaskWorkModeTaskCodeSelectWidget::getLinkLine()
{
    auto workMode = getWorkMode();
    if (workMode == NotDefine)
    {
        return LinkLine();
    }

    LinkLine linkLine;
    linkLine.workMode = getWorkMode();                          // 工作模式
    linkLine.masterTargetNo = mCkTargetNoBtnGroup.checkedId();  // 主跟目标号

    // 一体化+XXX 模式，固定第一个目标为一体化，第二个目标为扩频
    if (SystemWorkModeHelper::isMixWorkMode(linkLine.workMode))
    {
        if (ui->ckTaskCodeCombo1->currentText() != "无")
        {
            TargetInfo targetInfo;
            targetInfo.targetNo = 1;
            targetInfo.taskCode = ui->ckTaskCodeCombo1->currentText();
            targetInfo.pointFreqNo = ui->ckPointFreqCombo1->currentData().toUInt();
            targetInfo.workMode = SystemWorkModeHelper::getTargetWorkMode(linkLine.workMode, targetInfo.targetNo);

            linkLine.targetMap[targetInfo.targetNo] = targetInfo;
        }

        if (ui->ckTaskCodeCombo2->currentText() != "无")
        {
            TargetInfo targetInfo;
            targetInfo.targetNo = 2;
            targetInfo.taskCode = ui->ckTaskCodeCombo2->currentText();
            targetInfo.pointFreqNo = ui->ckPointFreqCombo2->currentData().toUInt();
            targetInfo.workMode = SystemWorkModeHelper::getTargetWorkMode(linkLine.workMode, targetInfo.targetNo);

            linkLine.targetMap[targetInfo.targetNo] = targetInfo;
        }

        // TODO add by wjy,主要用于判断校零使用一体化还是扩频
        if (linkLine.masterTargetNo == 1)
        {
            linkLine.masterType = MasterType::YTH_DT;
        }
        else if (linkLine.masterTargetNo == 2)
        {
            linkLine.masterType = MasterType::YTH_KP;
        }
    }
    else
    {
        QList<TargetInfo> targetInfoList;
        if (ui->ckTaskCodeCombo1->currentText() != "无")
        {
            TargetInfo targetInfo;
            targetInfo.taskCode = ui->ckTaskCodeCombo1->currentText();
            targetInfo.pointFreqNo = ui->ckPointFreqCombo1->currentData().toUInt();
            targetInfo.workMode = linkLine.workMode;

            targetInfoList << targetInfo;
        }

        if (ui->ckTaskCodeCombo2->currentText() != "无")
        {
            TargetInfo targetInfo;
            targetInfo.taskCode = ui->ckTaskCodeCombo2->currentText();
            targetInfo.pointFreqNo = ui->ckPointFreqCombo2->currentData().toUInt();
            targetInfo.workMode = linkLine.workMode;

            targetInfoList << targetInfo;
        }

        if (ui->ckTaskCodeCombo3->currentText() != "无")
        {
            TargetInfo targetInfo;
            targetInfo.taskCode = ui->ckTaskCodeCombo3->currentText();
            targetInfo.pointFreqNo = ui->ckPointFreqCombo3->currentData().toUInt();
            targetInfo.workMode = linkLine.workMode;

            targetInfoList << targetInfo;
        }

        if (ui->ckTaskCodeCombo4->currentText() != "无")
        {
            TargetInfo targetInfo;
            targetInfo.taskCode = ui->ckTaskCodeCombo4->currentText();
            targetInfo.pointFreqNo = ui->ckPointFreqCombo4->currentData().toUInt();
            targetInfo.workMode = linkLine.workMode;

            targetInfoList << targetInfo;
        }

        // 如果扩频4目标，按照选择的几个就下给前几个目标
        for (int targetNo = 1; targetNo <= targetInfoList.size(); ++targetNo)
        {
            auto& targetInfo = targetInfoList[targetNo - 1];
            targetInfo.targetNo = targetNo;
            linkLine.targetMap[targetInfo.targetNo] = targetInfo;
        }

        // 如果主用目标没有包含，默认使用第一个作为主用目标
        if (!linkLine.targetMap.isEmpty() && !linkLine.targetMap.contains(linkLine.masterTargetNo))
        {
            linkLine.masterTargetNo = linkLine.targetMap.firstKey();
        }
    }

    return linkLine;
}

void CKTaskWorkModeTaskCodeSelectWidget::initUI()
{
    // 添加测控工作模式
    ui->ckWorkModeCombo->addItem("无", NotDefine);
    for (auto item : mWorkSystem.ckCommands)
    {
        if (m_workModeType == 0 && item.id != (int)SystemWorkMode::KaKuo2)
        {
            ui->ckWorkModeCombo->addItem(item.desc, item.id);
        }
        if (m_workModeType != 0 && item.id == (int)SystemWorkMode::KaKuo2)
        {
            ui->ckWorkModeCombo->addItem(item.desc, item.id);
        }
    }

    mCkTargetNoBtnGroup.addButton(ui->ckTargetRadioBtn1, 1);
    mCkTargetNoBtnGroup.addButton(ui->ckTargetRadioBtn2, 2);
    mCkTargetNoBtnGroup.addButton(ui->ckTargetRadioBtn3, 3);
    mCkTargetNoBtnGroup.addButton(ui->ckTargetRadioBtn4, 4);
}

void CKTaskWorkModeTaskCodeSelectWidget::refreshRunningTaskPlanData(const ManualMessage& msg)
{
    auto manualMsg = msg;
    for (auto system : manualMsg.linkLineMap.keys())
    {
        auto link = manualMsg.linkLineMap[system];
        for (auto item : mWorkSystem.ckCommands)
        {
            if (m_workModeType == 0 && item.id != (int)SystemWorkMode::KaKuo2 && item.id == (int)system)
            {
                ui->ckWorkModeCombo->setCurrentText(item.desc);
                int masterTargetNo = link.masterTargetNo;
                //处理主跟目标选择按钮
                refreshMasterTargetNo(masterTargetNo);
                //处理任务代号
                auto targetMap = link.targetMap;
                refreshTargetInfo(targetMap);
            }
            if (m_workModeType != 0 && item.id == (int)SystemWorkMode::KaKuo2 && item.id == (int)system)
            {
                ui->ckWorkModeCombo->setCurrentText(item.desc);
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

void CKTaskWorkModeTaskCodeSelectWidget::refreshMasterTargetNo(int checkID)
{
    switch (checkID)
    {
    case 1: ui->ckTargetRadioBtn1->setChecked(true); break;
    case 2: ui->ckTargetRadioBtn2->setChecked(true); break;
    case 3: ui->ckTargetRadioBtn3->setChecked(true); break;
    case 4: ui->ckTargetRadioBtn4->setChecked(true); break;
    default: break;
    }
}

void CKTaskWorkModeTaskCodeSelectWidget::refreshTargetInfo(const QMap<int, TargetInfo>& targetMap)
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
            ui->ckTaskCodeCombo1->setCurrentText(taskCode);
            ui->ckPointFreqCombo1->setCurrentText(QString("点频%1").arg(point));
            break;
        }
        case 2:
        {
            ui->ckTaskCodeCombo2->setCurrentText(taskCode);
            ui->ckPointFreqCombo2->setCurrentText(QString("点频%1").arg(point));
            break;
        }
        case 3:
        {
            ui->ckTaskCodeCombo3->setCurrentText(taskCode);
            ui->ckPointFreqCombo3->setCurrentText(QString("点频%1").arg(point));
            break;
        }
        case 4:
        {
            ui->ckTaskCodeCombo4->setCurrentText(taskCode);
            ui->ckPointFreqCombo4->setCurrentText(QString("点频%1").arg(point));
            break;
        }
        default: break;
        }
    }
}

void CKTaskWorkModeTaskCodeSelectWidget::initSlot()
{
    connect(ui->ckWorkModeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &CKTaskWorkModeTaskCodeSelectWidget::slotWorkModeChanged);

    // 根据任务代号添加对应的点频
    changedPointFreq(ui->ckTaskCodeCombo1, ui->ckPointFreqCombo1);
    changedPointFreq(ui->ckTaskCodeCombo2, ui->ckPointFreqCombo2);
    changedPointFreq(ui->ckTaskCodeCombo3, ui->ckPointFreqCombo3);
    changedPointFreq(ui->ckTaskCodeCombo4, ui->ckPointFreqCombo4);

    // 触发信号
    emit ui->ckWorkModeCombo->currentIndexChanged(0);
}

void CKTaskWorkModeTaskCodeSelectWidget::changedPointFreq(QComboBox* taskCodeCombo, QComboBox* pointFreqCombo)
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

void CKTaskWorkModeTaskCodeSelectWidget::slotWorkModeChanged(int)
{
    auto workMode = getWorkMode();
    emit signalWorkModeChanged(workMode);

    ui->ckTargetRadioBtn1->setEnabled(false);
    ui->ckTargetRadioBtn2->setEnabled(false);
    ui->ckTargetRadioBtn3->setEnabled(false);
    ui->ckTargetRadioBtn4->setEnabled(false);

    ui->ckTaskCodeCombo1->setEnabled(false);
    ui->ckTaskCodeCombo2->setEnabled(false);
    ui->ckTaskCodeCombo3->setEnabled(false);
    ui->ckTaskCodeCombo4->setEnabled(false);

    ui->ckPointFreqCombo1->setEnabled(false);
    ui->ckPointFreqCombo2->setEnabled(false);
    ui->ckPointFreqCombo3->setEnabled(false);
    ui->ckPointFreqCombo4->setEnabled(false);

    ui->ckTaskCodeCombo1->clear();
    ui->ckTaskCodeCombo2->clear();
    ui->ckTaskCodeCombo3->clear();
    ui->ckTaskCodeCombo4->clear();

    ui->ckPointFreqCombo1->clear();
    ui->ckPointFreqCombo2->clear();
    ui->ckPointFreqCombo3->clear();
    ui->ckPointFreqCombo4->clear();

    ui->ckTaskCodeCombo1->addItem("无", 0);
    ui->ckTaskCodeCombo2->addItem("无", 0);
    ui->ckTaskCodeCombo3->addItem("无", 0);
    ui->ckTaskCodeCombo4->addItem("无", 0);

    // 默认主跟目标第一个
    ui->ckTargetRadioBtn1->setChecked(true);

    // 根据当前测控选择的工作模式去改变可使用的目标
    slotChangeTarget(workMode);
}

void CKTaskWorkModeTaskCodeSelectWidget::slotChangeTarget(SystemWorkMode workMode)
{
    if (workMode == NotDefine)
    {
        return;
    }

    // 对于单体制，只能选择第一行，一体化+XXX的可以选择两行，扩频的可以选择4行
    switch (workMode)
    {
    case STTC:     // S标准TTC
    case SYTHSMJ:  // 一体化上面级
    case SYTHWX:   // 一体化卫星
    case SYTHGML:  // 一体化高码率
    case SKT:      // 扩跳
    {
        ui->ckTargetRadioBtn1->setEnabled(true);
        ui->ckTaskCodeCombo1->setEnabled(true);
        ui->ckPointFreqCombo1->setEnabled(true);

        ui->ckTargetRadioBtn2->setEnabled(false);
        ui->ckTaskCodeCombo2->setEnabled(false);
        ui->ckPointFreqCombo2->setEnabled(false);

        ui->ckTargetRadioBtn3->setEnabled(false);
        ui->ckTaskCodeCombo3->setEnabled(false);
        ui->ckPointFreqCombo3->setEnabled(false);

        ui->ckTargetRadioBtn4->setEnabled(false);
        ui->ckTaskCodeCombo4->setEnabled(false);
        ui->ckPointFreqCombo4->setEnabled(false);

        // 添加任务代号
        for (auto taskCode : mWorkModeTaskcodeMap[workMode])
        {
            ui->ckTaskCodeCombo1->addItem(taskCode, workMode);
        }
    }
    break;
    case Skuo2:   // S扩二
    case KaKuo2:  // Ka扩二
    {
        ui->ckTargetRadioBtn1->setEnabled(true);
        ui->ckTaskCodeCombo1->setEnabled(true);
        ui->ckPointFreqCombo1->setEnabled(true);

        ui->ckTargetRadioBtn2->setEnabled(true);
        ui->ckTaskCodeCombo2->setEnabled(true);
        ui->ckPointFreqCombo2->setEnabled(true);

        ui->ckTargetRadioBtn3->setEnabled(true);
        ui->ckTaskCodeCombo3->setEnabled(true);
        ui->ckPointFreqCombo3->setEnabled(true);

        ui->ckTargetRadioBtn4->setEnabled(true);
        ui->ckTaskCodeCombo4->setEnabled(true);
        ui->ckPointFreqCombo4->setEnabled(true);

        // 添加任务代号
        for (auto taskCode : mWorkModeTaskcodeMap[workMode])
        {
            ui->ckTaskCodeCombo1->addItem(taskCode, workMode);
            ui->ckTaskCodeCombo2->addItem(taskCode, workMode);
            ui->ckTaskCodeCombo3->addItem(taskCode, workMode);
            ui->ckTaskCodeCombo4->addItem(taskCode, workMode);
        }
    }
    break;
    case SYTHSMJK2GZB:  // 一体化上面级+扩二共载波
    {
        ui->ckTargetRadioBtn1->setEnabled(true);
        ui->ckTaskCodeCombo1->setEnabled(true);
        ui->ckPointFreqCombo1->setEnabled(true);

        ui->ckTargetRadioBtn2->setEnabled(true);
        ui->ckTaskCodeCombo2->setEnabled(true);
        ui->ckPointFreqCombo2->setEnabled(true);

        ui->ckTargetRadioBtn3->setEnabled(false);
        ui->ckTaskCodeCombo3->setEnabled(false);
        ui->ckPointFreqCombo3->setEnabled(false);

        ui->ckTargetRadioBtn4->setEnabled(false);
        ui->ckTaskCodeCombo4->setEnabled(false);
        ui->ckPointFreqCombo4->setEnabled(false);

        // 添加任务代号
        for (auto tempWorkMode : mWorkModeTaskcodeMap.keys())
        {
            const auto& taskCodeList = mWorkModeTaskcodeMap.value(tempWorkMode);
            if (tempWorkMode == SYTHSMJ)  // 第一个目标为一体化上面级
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->ckTaskCodeCombo1->addItem(taskCode, tempWorkMode);
                }
            }
            else if (tempWorkMode == Skuo2)  // 第二个目标为扩频
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->ckTaskCodeCombo2->addItem(taskCode, tempWorkMode);
                }
            }
        }
    }
    break;
    case SYTHSMJK2BGZB:  // 一体化上面级+扩二不共载波
    {
        ui->ckTargetRadioBtn1->setEnabled(true);
        ui->ckTaskCodeCombo1->setEnabled(true);
        ui->ckPointFreqCombo1->setEnabled(true);

        ui->ckTargetRadioBtn2->setEnabled(true);
        ui->ckTaskCodeCombo2->setEnabled(true);
        ui->ckPointFreqCombo2->setEnabled(true);

        ui->ckTargetRadioBtn3->setEnabled(false);
        ui->ckTaskCodeCombo3->setEnabled(false);
        ui->ckPointFreqCombo3->setEnabled(false);

        ui->ckTargetRadioBtn4->setEnabled(false);
        ui->ckTaskCodeCombo4->setEnabled(false);
        ui->ckPointFreqCombo4->setEnabled(false);

        // 添加任务代号
        for (auto tempWorkMode : mWorkModeTaskcodeMap.keys())
        {
            const auto& taskCodeList = mWorkModeTaskcodeMap.value(tempWorkMode);
            if (tempWorkMode == SYTHSMJ)  // 第一个目标为一体化上面级
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->ckTaskCodeCombo1->addItem(taskCode, tempWorkMode);
                }
            }
            else if (tempWorkMode == Skuo2)  // 第二个目标为扩频
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->ckTaskCodeCombo2->addItem(taskCode, tempWorkMode);
                }
            }
        }
    }
    break;
    case SYTHWXSK2:  // 一体化卫星+扩二
    {
        ui->ckTargetRadioBtn1->setEnabled(true);
        ui->ckTaskCodeCombo1->setEnabled(true);
        ui->ckPointFreqCombo1->setEnabled(true);

        ui->ckTargetRadioBtn2->setEnabled(true);
        ui->ckTaskCodeCombo2->setEnabled(true);
        ui->ckPointFreqCombo2->setEnabled(true);

        ui->ckTargetRadioBtn3->setEnabled(false);
        ui->ckTaskCodeCombo3->setEnabled(false);
        ui->ckPointFreqCombo3->setEnabled(false);

        ui->ckTargetRadioBtn4->setEnabled(false);
        ui->ckTaskCodeCombo4->setEnabled(false);
        ui->ckPointFreqCombo4->setEnabled(false);

        // 添加任务代号
        for (auto tempWorkMode : mWorkModeTaskcodeMap.keys())
        {
            const auto& taskCodeList = mWorkModeTaskcodeMap.value(tempWorkMode);
            if (tempWorkMode == SYTHWX)  // 第一个目标为一体化卫星
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->ckTaskCodeCombo1->addItem(taskCode, tempWorkMode);
                }
            }
            else if (tempWorkMode == Skuo2)  // 第二个目标为扩频
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->ckTaskCodeCombo2->addItem(taskCode, tempWorkMode);
                }
            }
        }
    }
    break;
    case SYTHGMLSK2:  // 一体化高码率+扩二
    {
        ui->ckTargetRadioBtn1->setEnabled(true);
        ui->ckTaskCodeCombo1->setEnabled(true);
        ui->ckPointFreqCombo1->setEnabled(true);

        ui->ckTargetRadioBtn2->setEnabled(true);
        ui->ckTaskCodeCombo2->setEnabled(true);
        ui->ckPointFreqCombo2->setEnabled(true);

        ui->ckTargetRadioBtn3->setEnabled(false);
        ui->ckTaskCodeCombo3->setEnabled(false);
        ui->ckPointFreqCombo3->setEnabled(false);

        ui->ckTargetRadioBtn4->setEnabled(false);
        ui->ckTaskCodeCombo4->setEnabled(false);
        ui->ckPointFreqCombo4->setEnabled(false);

        // 添加任务代号
        for (auto tempWorkMode : mWorkModeTaskcodeMap.keys())
        {
            const auto& taskCodeList = mWorkModeTaskcodeMap.value(tempWorkMode);
            if (tempWorkMode == SYTHGML)  // 第一个目标为一体化高码率
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->ckTaskCodeCombo1->addItem(taskCode, tempWorkMode);
                }
            }
            else if (tempWorkMode == Skuo2)  // 第二个目标为扩频
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->ckTaskCodeCombo2->addItem(taskCode, tempWorkMode);
                }
            }
        }
    }
    break;
    }
}

void CKTaskWorkModeTaskCodeSelectWidget::setWorkModeType(int workModeType) { m_workModeType = workModeType; }
