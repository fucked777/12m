#include "ConfigMacroDialog.h"
#include "ui_ConfigMacroDialog.h"

#include "CheckBoxComboBox.h"
#include "ConfigMacroMessageSerialize.h"
#include "ConfigMacroXMLReader.h"
#include "ConfigMacroXmlWorkModeDefine.h"
#include "GlobalData.h"
#include <QCheckBox>
#include <QMessageBox>
#include <QTreeWidget>
#include <QUuid>

ConfigMacroDialog::ConfigMacroDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ConfigMacroDialog)
{
    ui->setupUi(this);
    // 只显示关闭按钮
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    init();
}

ConfigMacroDialog::~ConfigMacroDialog() { delete ui; }

void ConfigMacroDialog::reset()
{
    ui->configMacroName->clear();
    ui->stationMark->setCurrentIndex(0);
    ui->mainAndStandby->setCurrentIndex(0);
    mWorkModeComboBox->clearSelected();
    mEditModeConfigMacroData = ConfigMacroData();
}

void ConfigMacroDialog::setWidgetMode(const ConfigMacroDialog::WidgetMode& mode)
{
    mMode = mode;
    switch (mMode)
    {
    case WidgetMode::Add:
    {
        ui->workMode->setEnabled(true);
        ui->mainAndStandby->setEnabled(true);
        setWindowTitle("添加配置宏");
        mTipsMessage = QString("添加");
    }
    break;
    case WidgetMode::Edit:
    {
        ui->workMode->setEnabled(false);
        ui->mainAndStandby->setEnabled(false);
        setWindowTitle("编辑配置宏");
        mTipsMessage = QString("编辑");
    }
    break;
    }
}

ConfigMacroDialog::WidgetMode ConfigMacroDialog::getWidgetMode() const { return mMode; }

void ConfigMacroDialog::setConfigMacroData(const ConfigMacroData& configMacroData)
{
    mEditModeConfigMacroData = configMacroData;

    ui->configMacroName->setText(configMacroData.configMacroName);
    ui->stationMark->setCurrentText(configMacroData.stationID);
    //  ui->mainAndStandby->setCurrentIndex(static_cast<int>(configMacroData.isMaster));
    ui->mainAndStandby->setCurrentText(configMacroData.isMaster == MasterSlave::Master ? "主" : "备");

    mWorkModeComboBox->clearSelected();

    for (auto& configMacroCmdList : configMacroData.configMacroCmdModeMap)
    {
        mWorkModeComboBox->setCheckedItem(configMacroCmdList.workMode);

        auto treeWidget = mWorkModeTreeWidgetMap[configMacroCmdList.workMode];
        if (treeWidget == nullptr)
        {
            continue;
        }
        for (int topLeveItemIndex = 0; topLeveItemIndex < treeWidget->topLevelItemCount(); ++topLeveItemIndex)
        {
            auto topLeveItem = treeWidget->topLevelItem(topLeveItemIndex);

            auto comboBox = qobject_cast<QComboBox*>(treeWidget->itemWidget(topLeveItem, 1));
            if (comboBox == nullptr)
            {
                continue;
            }
            comboBox->setCurrentIndex(0);

            auto item = comboBox->property("Item").value<Item>();
            for (auto configMacroCmd : configMacroCmdList.configMacroCmdMap)
            {
                if (item.id == configMacroCmd.id)
                {
                    auto index = comboBox->findData(configMacroCmd.value);
                    if (index != -1)
                    {
                        comboBox->setCurrentIndex(index);
                    }
                    break;
                }
            }
        }
    }
}

void ConfigMacroDialog::init()
{
    connect(ui->okBtn, &QPushButton::clicked, this, &ConfigMacroDialog::slotOkBtnClicked);
    connect(ui->cancleBtn, &QPushButton::clicked, this, &ConfigMacroDialog::reject);

    QList<ConfigMacroWorkMode> configMacroWorkModes;  //不使用
    QString stationName;
    ConfigMacroXMLReader reader;
    reader.getConfigMacroWorkMode(configMacroWorkModes, stationName);

    ui->stationMark->addItem(stationName, 0);

    ui->mainAndStandby->addItem(QString("主"), static_cast<int>(MasterSlave::Master));
    ui->mainAndStandby->addItem(QString("备"), static_cast<int>(MasterSlave::Slave));

    //模式+主备作为主键，因此不需要配置宏名称这个参数
    ui->label_4->setHidden(true);
    ui->configMacroName->setHidden(true);

    mWorkModeComboBox = new CheckBoxComboBox(this);
    ui->workMode->layout()->addWidget(mWorkModeComboBox);
    connect(mWorkModeComboBox, &CheckBoxComboBox::signalSelectedChanged, this, &ConfigMacroDialog::slotWorkModeSelectedChanged);

    // 读取配置，初始化界面
    ConfigMacroWorkModeList workModes;
    GlobalData::getConfigMacroWorkModeData(workModes);
    for (int index = 0; index < workModes.size(); ++index)
    {
        auto workMode = workModes.at(index);
        mWorkModeComboBox->addItem(workMode.desc, static_cast<int>(workMode.workMode));

        auto treeWidget = new QTreeWidget(this);
        treeWidget->setHidden(true);
        treeWidget->setColumnCount(3);
        treeWidget->setColumnWidth(0, 200);
        treeWidget->setColumnWidth(1, 200);
        treeWidget->setHeaderLabels(QStringList() << QString("名称") << QString("值") << QString("描述"));

        for (auto item : workMode.items)
        {
            if (item.isManualControl == true)  // 如果是主界面手动控制界面才显示的，在配置宏界面不进行显示
            {
                continue;
            }

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

        mWorkModeTreeWidgetMap[workMode.workMode] = treeWidget;
    }
}

void ConfigMacroDialog::slotOperationResult(bool result, const QString& msg)
{
    if (result)
    {
        QMessageBox::information(this, "提示", mTipsMessage + "成功", "确定");
        this->close();
    }
    else
    {
        QMessageBox::critical(this, mTipsMessage + "失败", msg, "确定");
    }
}

void ConfigMacroDialog::slotWorkModeSelectedChanged(QCheckBox* checkBox)
{
    // 为选中
    if (!checkBox->isChecked())
    {
        for (int i = 0; i < ui->tabWidget->count(); ++i)
        {
            auto tabText = ui->tabWidget->tabText(i);
            if (checkBox->text() == tabText)
            {
                ui->tabWidget->removeTab(i);  // 移除该tab
                return;
            }
        }
    }
    else
    {
        // 获取对应模式的TreeWidget
        auto treeWidget = mWorkModeTreeWidgetMap[(SystemWorkMode)checkBox->property("UserData").toInt()];
        if (treeWidget != nullptr)
        {
            for (int topLeveItemIndex = 0; topLeveItemIndex < treeWidget->topLevelItemCount(); ++topLeveItemIndex)
            {
                auto topLeveItem = treeWidget->topLevelItem(topLeveItemIndex);

                auto comboBox = qobject_cast<QComboBox*>(treeWidget->itemWidget(topLeveItem, 1));
                if (comboBox != nullptr)
                {
                    // 重置选中第一个
                    //                    comboBox->setCurrentIndex(0);
                }
            }

            ui->tabWidget->addTab(treeWidget, checkBox->text());
        }
    }
}

void ConfigMacroDialog::slotOkBtnClicked()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    /*
     * 20211021
     * 更改一下配置宏的ID方便获取
     * 改为模式用加号连接然后下划线主备的形式
     * STTC+SKUO2+XDS_1
     */
    ConfigMacroData configMacroData;
    configMacroData.workModeDesc = mWorkModeComboBox->getCheckedItemsText().join("+");
    configMacroData.stationID = ui->stationMark->currentText();
    configMacroData.isMaster = (MasterSlave)ui->mainAndStandby->currentData().toInt();
    configMacroData.desc = ui->desc->toPlainText();

    // 当前所有配置宏模式
    QSet<SystemWorkMode> curModeSet;
    // 循环当前的模式tabWidget
    // 得到所有数据
    for (int i = 0; i < ui->tabWidget->count(); ++i)
    {
        auto treeWidget = qobject_cast<QTreeWidget*>(ui->tabWidget->widget(i));

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
            configMacroCmd.startBit = item.startBit;         // 设备组合号里面的开始位
            configMacroCmd.endBit = item.endBit;             // 设备组合号里面的结束位
            configMacroCmd.offset = item.offset;             // 设备组合号里面的偏移量
            configMacroCmd.linkType = item.linkType;         // 设备组合号类设备上下行区分
            configMacroCmd.value = comboBox->currentData();  // 当前选择的值
            for (auto& enumEntry : item.enums)
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
        auto workMode = mWorkModeTreeWidgetMap.key(treeWidget, SystemWorkMode::NotDefine);
        auto& configMacroModeItemMap = configMacroData.configMacroCmdModeMap[workMode];
        configMacroModeItemMap.configMacroCmdMap = cmdMap;
        configMacroModeItemMap.workMode = workMode;

        if (workMode == SystemWorkMode::NotDefine)
        {
            QMessageBox::warning(this, "模式错误", "请检查配置文件后重试", QString("确定"));
            return;
        }
        curModeSet.insert(workMode);
    }

    configMacroData.configMacroName = ConfigMacroMessageHelper::createSearchName(curModeSet, configMacroData.isMaster);
    if (curModeSet.isEmpty())
    {
        QMessageBox::information(this, "提示", "请选择工作模式！", "确定");
        return;
    }
    // 判断是否是编辑模式
    if (mMode == Edit)
    {
        configMacroData.configMacroID = mEditModeConfigMacroData.configMacroID;
    }

    // 是否存在不应该给客户端处理 应该给服务器处理
    emit signalDataReady(configMacroData);

    //    // 已经存在就不添加
    //    ConfigMacroList dataList;
    //    GlobalData::getConfigMacroData(dataList);
    //    for (auto& item : dataList)
    //    {
    //        // 编辑时id才可能相等，编辑时不需要判断和自己之前是否相等(能够保存成功，说明约束条件是唯一的)
    //        if (item.configMacroID == configMacroData.configMacroID)
    //        {
    //            continue;
    //        }

    //        if (item.workModeDesc == configMacroData.workModeDesc && item.isMaster == configMacroData.isMaster)
    //        {
    //            QMessageBox::information(this, QString("提示"), QString("该配置宏已存在"), QString("确定"));
    //            return;
    //        }
    //    }

    //    emit signalDataReady(configMacroData);
}

void ConfigMacroDialog::on_desc_textChanged()
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
