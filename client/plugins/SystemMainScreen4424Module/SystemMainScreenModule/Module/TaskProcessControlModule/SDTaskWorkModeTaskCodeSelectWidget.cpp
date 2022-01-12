#include "SDTaskWorkModeTaskCodeSelectWidget.h"
#include "ui_SDTaskWorkModeTaskCodeSelectWidget.h"

#include "ConfigMacroXmlWorkModeDefine.h"
#include "ExtendedConfig.h"

#include <QMessageBox>
#include <QTreeWidget>

SDTaskWorkModeTaskCodeSelectWidget::SDTaskWorkModeTaskCodeSelectWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SDTaskWorkModeTaskCodeSelectWidget)
{
    ui->setupUi(this);

    // 读取工作模式配置文件
    WorkModeXMLReader reader;
    mWorkSystem = reader.getWorkSystem();

    initUI();
    initSlot();
}

SDTaskWorkModeTaskCodeSelectWidget::~SDTaskWorkModeTaskCodeSelectWidget() { delete ui; }

SystemWorkMode SDTaskWorkModeTaskCodeSelectWidget::getWorkMode() { return (SystemWorkMode)ui->sdWorkModeCombo->currentData().toInt(); }

void SDTaskWorkModeTaskCodeSelectWidget::setWorkMode(SystemWorkMode workMode)
{
    auto index = ui->sdWorkModeCombo->findData((int)workMode);
    if (index != -1)
    {
        // 如果工作模式和原来一样，也触发一下信号
        if (index == ui->sdWorkModeCombo->currentIndex())
        {
            emit ui->sdWorkModeCombo->currentIndexChanged(index);
        }
        else
        {
            ui->sdWorkModeCombo->setCurrentIndex(index);
        }
    }
}

QComboBox* SDTaskWorkModeTaskCodeSelectWidget::getWorkModeComboBox() { return ui->sdWorkModeCombo; }

bool SDTaskWorkModeTaskCodeSelectWidget::isSelectedTaskCode()
{
    if (ui->sdTaskCodeCombo1->isEnabled() && ui->sdTaskCodeCombo1->currentText() != QString("无"))
    {
        return true;
    }

    if (ui->sdTaskCodeCombo2->isEnabled() && ui->sdTaskCodeCombo2->currentText() != QString("无"))
    {
        return true;
    }

    if (ui->sdTaskCodeCombo3->isEnabled() && ui->sdTaskCodeCombo3->currentText() != QString("无"))
    {
        return true;
    }

    if (ui->sdTaskCodeCombo4->isEnabled() && ui->sdTaskCodeCombo4->currentText() != QString("无"))
    {
        return true;
    }

    return false;
}

void SDTaskWorkModeTaskCodeSelectWidget::setWorkModeTaskCodeMap(const QMap<SystemWorkMode, QList<QString>>& workModeTaskCodeMap)
{
    mWorkModeTaskcodeMap = workModeTaskCodeMap;
}

void SDTaskWorkModeTaskCodeSelectWidget::setTaskCodeWorkModeDpMap(const QMap<QString, QMap<SystemWorkMode, int>>& taskCodeWorkModeDpMap)
{
    mTaskCodeWorkModeDpMap = taskCodeWorkModeDpMap;
}

LinkLine SDTaskWorkModeTaskCodeSelectWidget::getLinkLink()
{
    if (getWorkMode() == NotDefine)
    {
        return LinkLine();
    }

    LinkLine linkLine;
    linkLine.workMode = (SystemWorkMode)ui->sdWorkModeCombo->currentData().toUInt();  // 工作模式
    linkLine.masterTargetNo = mSDTargetNoBtnGroup.checkedId();                        // 主跟目标号

    // 一体化+XXX 模式，固定第一个目标为一体化，第二个目标为扩频
    if (SystemWorkModeHelper::isMixWorkMode(linkLine.workMode))
    {
        if (ui->sdTaskCodeCombo1->currentText() != "无")
        {
            TargetInfo targetInfo;
            targetInfo.targetNo = 1;
            targetInfo.taskCode = ui->sdTaskCodeCombo1->currentText();
            targetInfo.pointFreqNo = ui->sdPointFreqCombo1->currentData().toUInt();
            targetInfo.workMode = SystemWorkModeHelper::getTargetWorkMode(linkLine.workMode, targetInfo.targetNo);

            linkLine.targetMap[targetInfo.targetNo] = targetInfo;
        }

        if (ui->sdTaskCodeCombo2->currentText() != "无")
        {
            TargetInfo targetInfo;
            targetInfo.targetNo = 2;
            targetInfo.taskCode = ui->sdTaskCodeCombo2->currentText();
            targetInfo.pointFreqNo = ui->sdPointFreqCombo2->currentData().toUInt();
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
        if (ui->sdTaskCodeCombo1->currentText() != "无")
        {
            TargetInfo targetInfo;
            targetInfo.taskCode = ui->sdTaskCodeCombo1->currentText();
            targetInfo.pointFreqNo = ui->sdPointFreqCombo1->currentData().toUInt();
            targetInfo.workMode = linkLine.workMode;

            targetInfoList << targetInfo;
        }

        if (ui->sdTaskCodeCombo2->currentText() != "无")
        {
            TargetInfo targetInfo;
            targetInfo.taskCode = ui->sdTaskCodeCombo2->currentText();
            targetInfo.pointFreqNo = ui->sdPointFreqCombo2->currentData().toUInt();
            targetInfo.workMode = linkLine.workMode;

            targetInfoList << targetInfo;
        }

        if (ui->sdTaskCodeCombo3->currentText() != "无")
        {
            TargetInfo targetInfo;
            targetInfo.taskCode = ui->sdTaskCodeCombo3->currentText();
            targetInfo.pointFreqNo = ui->sdPointFreqCombo3->currentData().toUInt();
            targetInfo.workMode = linkLine.workMode;

            targetInfoList << targetInfo;
        }

        if (ui->sdTaskCodeCombo4->currentText() != "无")
        {
            TargetInfo targetInfo;
            targetInfo.taskCode = ui->sdTaskCodeCombo4->currentText();
            targetInfo.pointFreqNo = ui->sdPointFreqCombo4->currentData().toUInt();
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

        // 获取手动控制界面选择的设备信息
        ConfigMacroData configMacroData;
        getConfigMacroData(configMacroData);

        if (!linkLine.targetMap.isEmpty())
        {
            // 判断选择的设备主备是不是一样的
            if (SystemWorkModeHelper::isMeasureContrlWorkMode(linkLine.workMode))
            {
                DeviceID masterCKJDDeviceID;
                DeviceID slaveCKJDDeviceID;
                configMacroData.getCKJDDeviceID(linkLine.workMode, masterCKJDDeviceID, 1);
                configMacroData.getCKJDDeviceID(linkLine.workMode, slaveCKJDDeviceID, 2);

                if (masterCKJDDeviceID == slaveCKJDDeviceID)
                {
                    QMessageBox::information(this, "提示", "测控基带主备不能相同", "确定");
                    return LinkLine();
                }
            }
            else if (SystemWorkModeHelper::isDsDataTransmissionWorkMode(linkLine.workMode))
            {
                DeviceID masterDSJDDeviceID;
                DeviceID slaveDSJDDeviceID;
                configMacroData.getDSJDDeviceID(linkLine.workMode, masterDSJDDeviceID, 1);
                configMacroData.getDSJDDeviceID(linkLine.workMode, slaveDSJDDeviceID, 2);

                if (masterDSJDDeviceID == slaveDSJDDeviceID)
                {
                    QMessageBox::information(this, "提示", "低速基带主备不能相同", "确定");
                    return LinkLine();
                }
            }
            else if (SystemWorkModeHelper::isGsDataTransmissionWorkMode(linkLine.workMode))
            {
                auto curProject = ExtendedConfig::curProjectID();
                if (curProject == "4424")
                {
                    DeviceID GSJDDeviceID1;
                    DeviceID GSJDDeviceID2;
                    DeviceID GSJDDeviceID3;

                    configMacroData.getGSJDDeviceID(linkLine.workMode, GSJDDeviceID1, 1);
                    configMacroData.getGSJDDeviceID(linkLine.workMode, GSJDDeviceID2, 2);
                    configMacroData.getGSJDDeviceID(linkLine.workMode, GSJDDeviceID3, 3);

                    if (GSJDDeviceID1 == GSJDDeviceID2 || GSJDDeviceID1 == GSJDDeviceID2 || GSJDDeviceID2 == GSJDDeviceID3)
                    {
                        QMessageBox::information(this, "提示", "高速基带主备不能相同", "确定");
                        return LinkLine();
                    }
                }
                // 4426只有两台高速基带 一主一备
                else if (curProject == "4426")
                {
                    DeviceID masterGSJDDeviceID;
                    DeviceID slaveGSJDDeviceID;
                    configMacroData.getGSJDDeviceID(linkLine.workMode, masterGSJDDeviceID, 1);
                    configMacroData.getGSJDDeviceID(linkLine.workMode, slaveGSJDDeviceID, 2);

                    if (masterGSJDDeviceID == slaveGSJDDeviceID)
                    {
                        QMessageBox::information(this, "提示", "高速基带主备不能相同", "确定");
                        return LinkLine();
                    }
                }
            }
        }
    }

    return linkLine;
}

void SDTaskWorkModeTaskCodeSelectWidget::initUI()
{
    // 添加手动控制的工作模式
    ui->sdWorkModeCombo->addItem("无", NotDefine);
    for (auto item : mWorkSystem.ckCommands)
    {
        ui->sdWorkModeCombo->addItem(item.desc, item.id);
    }
    for (auto item : mWorkSystem.dtCommands)
    {
        ui->sdWorkModeCombo->addItem(item.desc, item.id);
    }

    mSDTargetNoBtnGroup.addButton(ui->sdTargetRadioBtn1, 1);
    mSDTargetNoBtnGroup.addButton(ui->sdTargetRadioBtn2, 2);
    mSDTargetNoBtnGroup.addButton(ui->sdTargetRadioBtn3, 3);
    mSDTargetNoBtnGroup.addButton(ui->sdTargetRadioBtn4, 4);

    auto widget = new QWidget();
    mDeviceWidgetMap.insert(NotDefine, widget);
    ui->deviceStackedWidget->addWidget(widget);

    // 从配置文件中读取配置，初始化界面
    ConfigMacroWorkModeList configMacroWorkModeList;
    GlobalData::getConfigMacroWorkModeData(configMacroWorkModeList);

    for (int index = 0; index < configMacroWorkModeList.size(); ++index)
    {
        auto configMacroWorkMode = configMacroWorkModeList.at(index);

        auto treeWidget = new QTreeWidget(this);
        treeWidget->setHidden(true);
        treeWidget->setColumnCount(2);
        treeWidget->setColumnWidth(0, 150);
        treeWidget->setColumnWidth(1, 150);
        treeWidget->setHeaderLabels(QStringList() << QString("名称") << QString("值"));

        for (auto item : configMacroWorkMode.items)
        {
            auto treeWidgetItem = new QTreeWidgetItem;

            treeWidgetItem->setText(0, item.desc);
            treeWidget->addTopLevelItem(treeWidgetItem);

            auto comboBox = new QComboBox(this);
            for (auto enumEntry : item.enums)
            {
                comboBox->addItem(enumEntry.desc, enumEntry.value);
                comboBox->setProperty("Item", QVariant::fromValue<Item>(item));
            }
            treeWidget->setItemWidget(treeWidgetItem, 1, comboBox);
        }

        ui->deviceStackedWidget->addWidget(treeWidget);
        mDeviceWidgetMap.insert(configMacroWorkMode.workMode, treeWidget);
    }
}

void SDTaskWorkModeTaskCodeSelectWidget::initSlot()
{
    // 手动控制界面工作模式改变
    connect(ui->sdWorkModeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &SDTaskWorkModeTaskCodeSelectWidget::slotWorkModeChanged);
    // 4424中，手动控制界面高速数传模式中的任务代号改变
    connect(ui->sdTaskCodeCombo1, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &SDTaskWorkModeTaskCodeSelectWidget::slotGSTaskCodeComboChanged);
    connect(ui->sdTaskCodeCombo2, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &SDTaskWorkModeTaskCodeSelectWidget::slotGSTaskCodeComboChanged);

    // 根据任务代号添加对应的点频
    changedPointFreq(ui->sdTaskCodeCombo1, ui->sdPointFreqCombo1);
    changedPointFreq(ui->sdTaskCodeCombo2, ui->sdPointFreqCombo2);
    changedPointFreq(ui->sdTaskCodeCombo3, ui->sdPointFreqCombo3);
    changedPointFreq(ui->sdTaskCodeCombo4, ui->sdPointFreqCombo4);

    // 触发信号
    emit ui->sdWorkModeCombo->currentIndexChanged(0);
}

void SDTaskWorkModeTaskCodeSelectWidget::changedPointFreq(QComboBox* taskCodeCombo, QComboBox* pointFreqCombo)
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

void SDTaskWorkModeTaskCodeSelectWidget::slotWorkModeChanged(int)
{
    auto workMode = getWorkMode();
    signalWorkModeChanged(workMode);

    auto widget = mDeviceWidgetMap[workMode];
    if (widget != nullptr)
    {
        ui->deviceStackedWidget->setCurrentWidget(widget);
    }

    ui->sdTargetRadioBtn1->setEnabled(false);
    ui->sdTargetRadioBtn2->setEnabled(false);
    ui->sdTargetRadioBtn3->setEnabled(false);
    ui->sdTargetRadioBtn4->setEnabled(false);

    ui->sdTaskCodeCombo1->setEnabled(false);
    ui->sdTaskCodeCombo2->setEnabled(false);
    ui->sdTaskCodeCombo3->setEnabled(false);
    ui->sdTaskCodeCombo4->setEnabled(false);

    ui->sdPointFreqCombo1->setEnabled(false);
    ui->sdPointFreqCombo2->setEnabled(false);
    ui->sdPointFreqCombo3->setEnabled(false);
    ui->sdPointFreqCombo4->setEnabled(false);

    ui->sdTaskCodeCombo1->clear();
    ui->sdTaskCodeCombo2->clear();
    ui->sdTaskCodeCombo3->clear();
    ui->sdTaskCodeCombo4->clear();

    ui->sdPointFreqCombo1->clear();
    ui->sdPointFreqCombo2->clear();
    ui->sdPointFreqCombo3->clear();
    ui->sdPointFreqCombo4->clear();

    ui->sdTaskCodeCombo1->addItem("无", 0);
    ui->sdTaskCodeCombo2->addItem("无", 0);
    ui->sdTaskCodeCombo3->addItem("无", 0);
    ui->sdTaskCodeCombo4->addItem("无", 0);

    // 默认主跟目标第一个
    ui->sdTargetRadioBtn1->setChecked(true);

    // 根据当前选择的工作模式去改变可使用的目标
    slotChangeTarget(workMode);
}

void SDTaskWorkModeTaskCodeSelectWidget::slotChangeTarget(SystemWorkMode workMode)
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
        ui->sdTargetRadioBtn1->setEnabled(true);
        ui->sdTaskCodeCombo1->setEnabled(true);
        ui->sdPointFreqCombo1->setEnabled(true);

        ui->sdTargetRadioBtn2->setEnabled(false);
        ui->sdTaskCodeCombo2->setEnabled(false);
        ui->sdPointFreqCombo2->setEnabled(false);

        ui->sdTargetRadioBtn3->setEnabled(false);
        ui->sdTaskCodeCombo3->setEnabled(false);
        ui->sdPointFreqCombo3->setEnabled(false);

        ui->sdTargetRadioBtn4->setEnabled(false);
        ui->sdTaskCodeCombo4->setEnabled(false);
        ui->sdPointFreqCombo4->setEnabled(false);

        // 添加任务代号
        for (auto taskCode : mWorkModeTaskcodeMap[workMode])
        {
            ui->sdTaskCodeCombo1->addItem(taskCode, workMode);
        }
    }
    break;
    case Skuo2:   // S扩二
    case KaKuo2:  // Ka扩二
    {
        ui->sdTargetRadioBtn1->setEnabled(true);
        ui->sdTaskCodeCombo1->setEnabled(true);
        ui->sdPointFreqCombo1->setEnabled(true);

        ui->sdTargetRadioBtn2->setEnabled(true);
        ui->sdTaskCodeCombo2->setEnabled(true);
        ui->sdPointFreqCombo2->setEnabled(true);

        ui->sdTargetRadioBtn3->setEnabled(true);
        ui->sdTaskCodeCombo3->setEnabled(true);
        ui->sdPointFreqCombo3->setEnabled(true);

        ui->sdTargetRadioBtn4->setEnabled(true);
        ui->sdTaskCodeCombo4->setEnabled(true);
        ui->sdPointFreqCombo4->setEnabled(true);

        // 添加任务代号
        for (auto taskCode : mWorkModeTaskcodeMap[workMode])
        {
            ui->sdTaskCodeCombo1->addItem(taskCode, workMode);
            ui->sdTaskCodeCombo2->addItem(taskCode, workMode);
            ui->sdTaskCodeCombo3->addItem(taskCode, workMode);
            ui->sdTaskCodeCombo4->addItem(taskCode, workMode);
        }
    }
    break;
    case SYTHSMJK2GZB:  // 一体化上面级+扩二共载波
    {
        ui->sdTargetRadioBtn1->setEnabled(true);
        ui->sdTaskCodeCombo1->setEnabled(true);
        ui->sdPointFreqCombo1->setEnabled(true);

        ui->sdTargetRadioBtn2->setEnabled(true);
        ui->sdTaskCodeCombo2->setEnabled(true);
        ui->sdPointFreqCombo2->setEnabled(true);

        ui->sdTargetRadioBtn3->setEnabled(false);
        ui->sdTaskCodeCombo3->setEnabled(false);
        ui->sdPointFreqCombo3->setEnabled(false);

        ui->sdTargetRadioBtn4->setEnabled(false);
        ui->sdTaskCodeCombo4->setEnabled(false);
        ui->sdPointFreqCombo4->setEnabled(false);

        // 添加任务代号
        for (auto tempWorkMode : mWorkModeTaskcodeMap.keys())
        {
            const auto& taskCodeList = mWorkModeTaskcodeMap.value(tempWorkMode);
            if (tempWorkMode == SYTHSMJ)  // 第一个目标为一体化上面级
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->sdTaskCodeCombo1->addItem(taskCode, tempWorkMode);
                }
            }
            else if (tempWorkMode == Skuo2)  // 第二个目标为扩频
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->sdTaskCodeCombo2->addItem(taskCode, tempWorkMode);
                }
            }
        }
    }
    break;
    case SYTHSMJK2BGZB:  // 一体化上面级+扩二不共载波
    {
        ui->sdTargetRadioBtn1->setEnabled(true);
        ui->sdTaskCodeCombo1->setEnabled(true);
        ui->sdPointFreqCombo1->setEnabled(true);

        ui->sdTargetRadioBtn2->setEnabled(true);
        ui->sdTaskCodeCombo2->setEnabled(true);
        ui->sdPointFreqCombo2->setEnabled(true);

        ui->sdTargetRadioBtn3->setEnabled(false);
        ui->sdTaskCodeCombo3->setEnabled(false);
        ui->sdPointFreqCombo3->setEnabled(false);

        ui->sdTargetRadioBtn4->setEnabled(false);
        ui->sdTaskCodeCombo4->setEnabled(false);
        ui->sdPointFreqCombo4->setEnabled(false);

        // 添加任务代号
        for (auto tempWorkMode : mWorkModeTaskcodeMap.keys())
        {
            const auto& taskCodeList = mWorkModeTaskcodeMap.value(tempWorkMode);
            if (tempWorkMode == SYTHSMJ)  // 第一个目标为一体化上面级
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->sdTaskCodeCombo1->addItem(taskCode, tempWorkMode);
                }
            }
            else if (tempWorkMode == Skuo2)  // 第二个目标为扩频
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->sdTaskCodeCombo2->addItem(taskCode, tempWorkMode);
                }
            }
        }
    }
    break;
    case SYTHWXSK2:  // 一体化卫星+扩二
    {
        ui->sdTargetRadioBtn1->setEnabled(true);
        ui->sdTaskCodeCombo1->setEnabled(true);
        ui->sdPointFreqCombo1->setEnabled(true);

        ui->sdTargetRadioBtn2->setEnabled(true);
        ui->sdTaskCodeCombo2->setEnabled(true);
        ui->sdPointFreqCombo2->setEnabled(true);

        ui->sdTargetRadioBtn3->setEnabled(false);
        ui->sdTaskCodeCombo3->setEnabled(false);
        ui->sdPointFreqCombo3->setEnabled(false);

        ui->sdTargetRadioBtn4->setEnabled(false);
        ui->sdTaskCodeCombo4->setEnabled(false);
        ui->sdPointFreqCombo4->setEnabled(false);

        // 添加任务代号
        for (auto tempWorkMode : mWorkModeTaskcodeMap.keys())
        {
            const auto& taskCodeList = mWorkModeTaskcodeMap.value(tempWorkMode);
            if (tempWorkMode == SYTHWX)  // 第一个目标为一体化卫星
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->sdTaskCodeCombo1->addItem(taskCode, tempWorkMode);
                }
            }
            else if (tempWorkMode == Skuo2)  // 第二个目标为扩频
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->sdTaskCodeCombo2->addItem(taskCode, tempWorkMode);
                }
            }
        }
    }
    break;
    case SYTHGMLSK2:  // 一体化高码率+扩二
    {
        ui->sdTargetRadioBtn1->setEnabled(true);
        ui->sdTaskCodeCombo1->setEnabled(true);
        ui->sdPointFreqCombo1->setEnabled(true);

        ui->sdTargetRadioBtn2->setEnabled(true);
        ui->sdTaskCodeCombo2->setEnabled(true);
        ui->sdPointFreqCombo2->setEnabled(true);

        ui->sdTargetRadioBtn3->setEnabled(false);
        ui->sdTaskCodeCombo3->setEnabled(false);
        ui->sdPointFreqCombo3->setEnabled(false);

        ui->sdTargetRadioBtn4->setEnabled(false);
        ui->sdTaskCodeCombo4->setEnabled(false);
        ui->sdPointFreqCombo4->setEnabled(false);

        // 添加任务代号
        for (auto tempWorkMode : mWorkModeTaskcodeMap.keys())
        {
            const auto& taskCodeList = mWorkModeTaskcodeMap.value(tempWorkMode);
            if (tempWorkMode == SYTHGML)  // 第一个目标为一体化高码率
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->sdTaskCodeCombo1->addItem(taskCode, tempWorkMode);
                }
            }
            else if (tempWorkMode == Skuo2)  // 第二个目标为扩频
            {
                for (auto taskCode : taskCodeList)
                {
                    ui->sdTaskCodeCombo2->addItem(taskCode, tempWorkMode);
                }
            }
        }
    }
    break;
    case KaGS:  // Ka高速
    case XGS:   // X高速
    {
        ui->sdTargetRadioBtn1->setEnabled(true);
        ui->sdTaskCodeCombo1->setEnabled(true);
        ui->sdPointFreqCombo1->setEnabled(true);

        ui->sdTargetRadioBtn2->setEnabled(true);
        ui->sdTaskCodeCombo2->setEnabled(true);
        ui->sdPointFreqCombo2->setEnabled(true);

        ui->sdTargetRadioBtn3->setEnabled(false);
        ui->sdTaskCodeCombo3->setEnabled(false);
        ui->sdPointFreqCombo3->setEnabled(false);

        ui->sdTargetRadioBtn4->setEnabled(false);
        ui->sdTaskCodeCombo4->setEnabled(false);
        ui->sdPointFreqCombo4->setEnabled(false);

        // 添加任务代号
        for (auto taskCode : mWorkModeTaskcodeMap[workMode])
        {
            ui->sdTaskCodeCombo1->addItem(taskCode, workMode);
            ui->sdTaskCodeCombo2->addItem(taskCode, workMode);
        }
    }
    break;
    case KaDS:  // Ka低速
    case XDS:   // X低速
    {
        ui->sdTargetRadioBtn1->setEnabled(true);
        ui->sdTaskCodeCombo1->setEnabled(true);
        ui->sdPointFreqCombo1->setEnabled(true);

        ui->sdTargetRadioBtn2->setEnabled(true);
        ui->sdTaskCodeCombo2->setEnabled(true);
        ui->sdPointFreqCombo2->setEnabled(true);

        ui->sdTargetRadioBtn3->setEnabled(false);
        ui->sdTaskCodeCombo3->setEnabled(false);
        ui->sdPointFreqCombo3->setEnabled(false);

        ui->sdTargetRadioBtn4->setEnabled(false);
        ui->sdTaskCodeCombo4->setEnabled(false);
        ui->sdPointFreqCombo4->setEnabled(false);

        // 添加任务代号
        for (auto taskCode : mWorkModeTaskcodeMap[workMode])
        {
            ui->sdTaskCodeCombo1->addItem(taskCode, workMode);
            ui->sdTaskCodeCombo2->addItem(taskCode, workMode);
        }
    }
    break;
    }
}

void SDTaskWorkModeTaskCodeSelectWidget::slotGSTaskCodeComboChanged()
{
    auto curProject = ExtendedConfig::curProjectID();
    auto currentWorkMode = (SystemWorkMode)ui->sdWorkModeCombo->currentData().toInt();
    // 当前项目是4424并且手动控制界面工作模式为数传模式
    if (curProject == "4424" && SystemWorkModeHelper::isDataTransmissionWorkMode(currentWorkMode))
    {
        auto text1 = ui->sdTaskCodeCombo1->currentText();
        auto text2 = ui->sdTaskCodeCombo2->currentText();

        int pointNum = 0;
        if (text1 != "无")
        {
            ++pointNum;
        }
        if (text2 != "无")
        {
            ++pointNum;
        }

        // 获取显示高速基带描述的 QTreeWidgetItem
        auto treeWidget = qobject_cast<QTreeWidget*>(mDeviceWidgetMap.value(currentWorkMode));
        if (treeWidget == nullptr)
        {
            return;
        }

        QList<QTreeWidgetItem*> gsjdTreeWidgetItemList;
        QList<QComboBox*> gsjdComboBoxList;
        for (int topLeveItemIndex = 0; topLeveItemIndex < treeWidget->topLevelItemCount(); ++topLeveItemIndex)
        {
            auto topLeveItem = treeWidget->topLevelItem(topLeveItemIndex);

            auto comboBox = qobject_cast<QComboBox*>(treeWidget->itemWidget(topLeveItem, 1));
            if (comboBox == nullptr)
            {
                continue;
            }

            auto item = comboBox->property("Item").value<Item>();
            // 获取高速基带描述的item
            if (item.id.contains("GSJD"))  // 配置文件中必须包含GSJD
            {
                gsjdTreeWidgetItemList << topLeveItem;
                gsjdComboBoxList << comboBox;
            }
        }

        // 4424配置宏高速模式中高数基带配置必须有3台
        if (gsjdTreeWidgetItemList.size() != 3)
        {
            return;
        }

        // 4424根据点频数进行改变id
        // 单点频情况下一主两备
        // 双点频情况下两主一备

        // 点频数只有1个
        if (pointNum == 1)
        {
            gsjdTreeWidgetItemList[0]->setText(0, "高速基带主");
            gsjdTreeWidgetItemList[1]->setText(0, "高速基带备1");
            gsjdTreeWidgetItemList[2]->setText(0, "高速基带备2");

            auto item = gsjdComboBoxList[0]->property("Item").value<Item>();
            item.id = "GSJD1";
            item.desc = "高速基带主";
            gsjdComboBoxList[0]->setProperty("Item", QVariant::fromValue<Item>(item));

            item = gsjdComboBoxList[1]->property("Item").value<Item>();
            item.id = "GSJD2";
            item.desc = "高速基带备1";
            gsjdComboBoxList[1]->setProperty("Item", QVariant::fromValue<Item>(item));

            item = gsjdComboBoxList[2]->property("Item").value<Item>();
            item.id = "GSJD3";
            item.desc = "高速基带备2";
            gsjdComboBoxList[2]->setProperty("Item", QVariant::fromValue<Item>(item));
        }
        // 点频数有2个
        else if (pointNum == 2)
        {
            gsjdTreeWidgetItemList[0]->setText(0, "高速基带主1");
            gsjdTreeWidgetItemList[1]->setText(0, "高速基带主2");
            gsjdTreeWidgetItemList[2]->setText(0, "高速基带备");

            auto item = gsjdComboBoxList[0]->property("Item").value<Item>();
            item.id = "GSJD1";
            item.desc = "高速基带主1";
            gsjdComboBoxList[0]->setProperty("Item", QVariant::fromValue<Item>(item));

            item = gsjdComboBoxList[1]->property("Item").value<Item>();
            item.id = "GSJD2";
            item.desc = "高速基带主2";
            gsjdComboBoxList[1]->setProperty("Item", QVariant::fromValue<Item>(item));

            item = gsjdComboBoxList[2]->property("Item").value<Item>();
            item.id = "GSJD3";
            item.desc = "高速基带备";
            gsjdComboBoxList[2]->setProperty("Item", QVariant::fromValue<Item>(item));
        }
    }
}

void SDTaskWorkModeTaskCodeSelectWidget::getConfigMacroData(ConfigMacroData& configMacroData)
{
    auto currentWorkMode = (SystemWorkMode)ui->sdWorkModeCombo->currentData().toInt();
    if (currentWorkMode == NotDefine)
    {
        return;
    }

    configMacroData.configMacroID = QString("手动控制界面设备信息");
    configMacroData.configMacroName = configMacroData.configMacroID;

    auto treeWidget = qobject_cast<QTreeWidget*>(mDeviceWidgetMap.value(currentWorkMode));
    if (treeWidget == nullptr)
    {
        return;
    }

    ConfigMacroCmdMap cmdMap;
    for (int topLeveItemIndex = 0; topLeveItemIndex < treeWidget->topLevelItemCount(); ++topLeveItemIndex)
    {
        auto topLeveItem = treeWidget->topLevelItem(topLeveItemIndex);

        auto comboBox = qobject_cast<QComboBox*>(treeWidget->itemWidget(topLeveItem, 1));
        if (comboBox == nullptr)
        {
            continue;
        }

        auto item = comboBox->property("Item").value<Item>();

        // Item 转为 ConfigMacroCmd
        ConfigMacroCmd configMacroCmd;
        configMacroCmd.id = item.id;                     // id
        configMacroCmd.desc = item.desc;                 // 描述
        configMacroCmd.value = comboBox->currentData();  // 当前选择的值
        for (auto enumEntry : item.enums)
        {
            if (enumEntry.value == configMacroCmd.value)
            {
                configMacroCmd.deviceID = enumEntry.deviceId;  // 当前选中的设备id（测控前端、跟踪前端、下变频器）
                break;
            }
        }

        cmdMap.insert(configMacroCmd.id, configMacroCmd);
    }
    // 获取模式id
    auto& configMacroCmdModeMap = configMacroData.configMacroCmdModeMap[currentWorkMode];
    configMacroCmdModeMap.configMacroCmdMap = cmdMap;
    configMacroCmdModeMap.workMode = currentWorkMode;
}
