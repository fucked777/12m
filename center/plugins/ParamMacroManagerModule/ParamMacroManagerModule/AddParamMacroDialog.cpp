#include "ui_AddParamMacroDialog.h"

#include "AddParamMacroDialog.h"
#include "ControlFactory.h"
#include "CopyPointFreqDialog.h"
#include "DynamicParamDialog.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "MMDynamicParamDialog.h"
#include "ParamMacroMessageSerialize.h"
#include "ParamMacroXMLReader.h"
#include "ProtocolXmlTypeDefine.h"
#include "SatelliteManagementDefine.h"
#include "ServiceCheck.h"
#include "SystemWorkMode.h"
#include "WNumber.h"
#include <QAction>
#include <QCloseEvent>
#include <QGroupBox>
#include <QMenu>
#include <QMessageBox>
ParamMacroRelationManager AddParamMacroDialog::m_paraRelation;
static DeviceID ds_BBEID = DeviceID(4, 4, 1);

AddParamMacroDialog::AddParamMacroDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AddParamMacroDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);  // 只显示关闭按钮
    ui->paramMacroModeTreeWidget->setHeaderLabels(QStringList() << "参数宏");

    ui->paramMacroModeTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
}

AddParamMacroDialog::~AddParamMacroDialog() { delete ui; }

void AddParamMacroDialog::initUI()
{
    // 获取该参数宏任务代号对应的卫星数据
    SatelliteManagementData currentSatelliteData;
    // 如果没有查询到该参数宏对应的卫星(编辑、查看参数宏的时候可能出现该问题)
    if (!GlobalData::getSatelliteManagementData(mParamMacroData.taskCode, currentSatelliteData))
    {
        auto msg = QString("该参数宏对应的卫星可能被删除，该参数宏点频数据出现错误，请删除该参数宏，重新添加卫星后再进行参数宏操作");
        QMessageBox::information(this, "提示", msg, "确定");
    }

    syncParamData(currentSatelliteData);
    //根据工作模式生成工作模式的节点
    for (auto& item : mParamMacroModeInfoList)
    {
        // 没有当前工作模式就直接跳过不创建
        if (!mParamMacroData.modeParamMap.contains(item.m_modeID))
        {
            continue;
        }
        auto currentWorkMode = item.m_modeID;
        QTreeWidgetItem* workModeWidgetItem = new QTreeWidgetItem;
        workModeWidgetItem->setText(0, item.m_modeName);
        ui->paramMacroModeTreeWidget->addTopLevelItem(workModeWidgetItem);

        // 根据卫星里面的点频数量创建点频
        auto totalDpSize = currentSatelliteData.getDpNumByWorkMode(currentWorkMode);

        //根据配置的点频参数进行动态界面生成
        for (auto dpNum = 1; dpNum <= totalDpSize; dpNum++)
        {
            QTreeWidgetItem* frequencyTreeWidgetItem = new QTreeWidgetItem;
            frequencyTreeWidgetItem->setText(0, QString("点频") + QString::number(dpNum));
            workModeWidgetItem->addChild(frequencyTreeWidgetItem);

            QTreeWidget* tree = new QTreeWidget();
            tree->setColumnCount(3);
            tree->setColumnWidth(0, 200);
            tree->setColumnWidth(1, 200);
            tree->setHeaderLabels(QStringList() << QString("名称") << QString("值") << QString("描述"));

            //每个分组下的参数ID
            for (auto groupfrequencyItem : item.m_frequencyList)
            {
                // 不是可变参数
                if (!groupfrequencyItem.isDynamicParam)
                {
                    QTreeWidgetItem* groupItem = new QTreeWidgetItem;
                    groupItem->setText(0, groupfrequencyItem.m_groupName);

                    int hideUINum = 0;  // 当前分组是否有显示的字段 有可能是当前分组有值但是全部固定隐藏的
                    auto enumMap = item.m_frequencyEnumMap[groupfrequencyItem.enumGroupId];
                    for (auto attr : groupfrequencyItem.m_paramList)
                    {
                        hideUINum += static_cast<int>(!attr.isShow);
                        if (attr.isShow || attr.fixValue.isValid())
                        {
                            createPointFreqControl(tree, groupItem, currentWorkMode, dpNum, groupfrequencyItem.m_deviceID, attr, enumMap);
                            continue;
                        }
                    }

                    tree->addTopLevelItem(groupItem);
                    if (hideUINum == groupfrequencyItem.m_paramList.size())
                    {
                        tree->setItemHidden(groupItem, true);
                    }
                    mPointFreqUnitParamMap[currentWorkMode][dpNum][groupfrequencyItem.m_groupName] = groupItem;
                }
                // 是动态参数
                else
                {
                    QTreeWidgetItem* groupItem = new QTreeWidgetItem;
                    groupItem->setText(0, groupfrequencyItem.m_groupName);
                    QTreeWidgetItem* subPtnItem = new QTreeWidgetItem;
                    groupItem->addChild(subPtnItem);
                    auto dynamicParamBtn = new QPushButton(groupfrequencyItem.m_groupName);
                    dynamicParamBtn->setProperty("DynamicGroupId", groupfrequencyItem.dynamicGroupId);

                    if (groupfrequencyItem.dynamicGroupId == "YaoC_MZ")
                    {
                        connect(dynamicParamBtn, &QPushButton::pressed, this, [=]() {
                            MMDynamicParamDialog dynamicDialog;
                            dynamicDialog.setWidgetMode((MMDynamicParamDialog::WidgetMode)(int)mCurrentMode);
                            dynamicDialog.setWindowTitle(dynamicParamBtn->text());

                            //防止没设置Property导致转换失败,给错默认值
                            bool isEnable = true;
                            QVariant varEnable = dynamicParamBtn->property("enable");
                            if (!varEnable.isNull())
                            {
                                isEnable = varEnable.toBool();
                                dynamicDialog.setButtonEnable(isEnable);
                            }

                            QVariant varMinValue = dynamicParamBtn->property("minValue").toInt();
                            if (!varMinValue.isNull())
                            {
                                int minValue = dynamicParamBtn->property("minValue").toInt();
                                int maxValue = dynamicParamBtn->property("maxValue").toInt();
                                dynamicDialog.setNumRange(minValue, maxValue);
                            }

                            dynamicDialog.setRowParamAttribute(groupfrequencyItem.m_paramList);  // 设置界面
                            auto enumMap = item.m_frequencyEnumMap[groupfrequencyItem.enumGroupId];
                            dynamicDialog.setEnumMap(enumMap);

                            auto data = dynamicParamBtn->property("ChangeValue").value<QMap<int, QMap<QString, QVariant>>>();
                            dynamicDialog.setParamValueMap(data);  // 设置值
                            if (dynamicDialog.exec() == QDialog::Accepted)
                            {
                                QMap<int, QMap<QString, QVariant>> data = dynamicDialog.getParamValueMap();
                                dynamicParamBtn->setProperty("ChangeValue", QVariant::fromValue(data));
                            }
                        });
                    }
                    else
                    {
                        connect(dynamicParamBtn, &QPushButton::pressed, this, [=]() {
                            DynamicParamDialog dynamicDialog;
                            dynamicDialog.setWidgetMode((DynamicParamDialog::WidgetMode)(int)mCurrentMode);
                            dynamicDialog.setWindowTitle(dynamicParamBtn->text());

                            //防止没设置Property导致转换失败,给错默认值
                            bool isEnable = true;
                            QVariant varEnable = dynamicParamBtn->property("enable");
                            if (!varEnable.isNull())
                            {
                                isEnable = varEnable.toBool();
                                dynamicDialog.setButtonEnable(isEnable);
                            }

                            QVariant varMinValue = dynamicParamBtn->property("minValue").toInt();
                            if (!varMinValue.isNull())
                            {
                                int minValue = dynamicParamBtn->property("minValue").toInt();
                                int maxValue = dynamicParamBtn->property("maxValue").toInt();
                                dynamicDialog.setNumRange(minValue, maxValue);
                            }

                            dynamicDialog.setRowParamAttribute(groupfrequencyItem.m_paramList);  // 设置界面
                            auto enumMap = item.m_frequencyEnumMap[groupfrequencyItem.enumGroupId];
                            dynamicDialog.setEnumMap(enumMap);

                            auto data = dynamicParamBtn->property("ChangeValue").value<QMap<int, QMap<QString, QVariant>>>();
                            dynamicDialog.setParamValueMap(data);  // 设置值
                            if (dynamicDialog.exec() == QDialog::Accepted)
                            {
                                QMap<int, QMap<QString, QVariant>> data = dynamicDialog.getParamValueMap();
                                dynamicParamBtn->setProperty("ChangeValue", QVariant::fromValue(data));
                            }
                        });
                    }

                    mModeDynamicBtnMap[currentWorkMode][dpNum][groupfrequencyItem.m_deviceID] = dynamicParamBtn;

                    tree->setItemWidget(subPtnItem, 0, dynamicParamBtn);
                    tree->addTopLevelItem(groupItem);
                }
            }

            //添加单元关联关系
            m_paraRelation.addUnit(currentWorkMode, mModePointFreqControlMap[currentWorkMode][dpNum]);
            tree->expandAll();
            ui->contentStackWidget->addWidget(tree);
            mTreeWidgetMap[frequencyTreeWidgetItem] = tree;
        }
        QTreeWidgetItem* deviceWidgetItemRoot = new QTreeWidgetItem;
        deviceWidgetItemRoot->setText(0, "设备参数");
        workModeWidgetItem->addChild(deviceWidgetItemRoot);

        //默认不生成点频的参数节点，根据输入的数据来设置点频的节点
        for (auto deviceItem : item.m_deviceList)
        {
            QTreeWidgetItem* deviceWidgetItem = new QTreeWidgetItem;
            deviceWidgetItem->setText(0, deviceItem.m_devName);
            deviceWidgetItemRoot->addChild(deviceWidgetItem);

            QTreeWidget* tree = new QTreeWidget();
            tree->setColumnCount(3);
            tree->setColumnWidth(0, 200);
            tree->setColumnWidth(1, 200);
            tree->setHeaderLabels(QStringList() << QString("名称") << QString("值") << QString("描述"));
            for (auto unitID : deviceItem.m_unitNameMap.keys())
            {
                if (!deviceItem.m_unitConfigMap.contains(unitID))
                {
                    continue;
                }
                auto unitData = deviceItem.m_unitConfigMap.value(unitID);
                QTreeWidgetItem* unitItem = new QTreeWidgetItem;
                unitItem->setText(0, deviceItem.m_unitNameMap.value(unitID));

                //如果公共单元有数据
                int hideUINum = 0;  // 当前分组是否有显示的字段 有可能是当前分组有值但是全部固定隐藏的
                for (auto attr : unitData)
                {
                    hideUINum += static_cast<int>(!attr.isShow);
                    if (attr.isShow || attr.fixValue.isValid())
                    {
                        createDeviceParamControl(tree, unitItem, deviceWidgetItem, currentWorkMode, deviceItem.m_deviceID, unitID, attr,
                                                 deviceItem.m_unitEnumMap);
                    }
                }
                tree->addTopLevelItem(unitItem);
                if (hideUINum == unitData.size())
                {
                    tree->setItemHidden(unitItem, true);
                }
            }
            tree->expandAll();
            ui->contentStackWidget->addWidget(tree);
            mTreeWidgetMap[deviceWidgetItem] = tree;
        }
    }
    ui->paramMacroModeTreeWidget->expandAll();

    // 更新点频参数值
    for (auto workMode : mModePointFreqControlMap.keys())
    {
        auto pointFreqContrlMap = mModePointFreqControlMap.value(workMode);

        for (auto pointFreqNo : pointFreqContrlMap.keys())
        {
            auto deviceContrlMap = pointFreqContrlMap.value(pointFreqNo);
            for (auto deviceIDStr : deviceContrlMap.keys())
            {
                auto contrlMap = deviceContrlMap.value(deviceIDStr);

                QStringList deviceIDList = deviceIDStr.split("/", QString::SkipEmptyParts);
                if (deviceIDList.isEmpty())
                {
                    continue;
                }

                DeviceID deviceID;
                int unitId;
                QStringList list2 = deviceIDList.at(0).split("_", QString::SkipEmptyParts);
                if (list2.size() != 2)
                {
                    continue;
                }

                deviceID = DeviceID::fromHex(list2.at(0));
                unitId = list2.at(1).toInt();

                auto paramValueMap = mParamMacroData.modeParamMap[workMode].pointFreqParamMap[pointFreqNo][deviceID].unitParamMap[unitId];

                for (auto paramId : contrlMap.keys())
                {
                    if (paramValueMap.contains(paramId))
                    {
                        auto control = contrlMap.value(paramId);
                        if (control != nullptr)
                        {
                            control->setValue(paramValueMap.value(paramId));
                        }
                    }
                }
            }
        }
    }

    // 更新设备参数值
    for (auto workMode : mModeDeviceControlMap.keys())
    {
        auto deviceControlMap = mModeDeviceControlMap.value(workMode);
        for (auto deviceIDStr : deviceControlMap.keys())
        {
            auto unitContolMap = deviceControlMap.value(deviceIDStr);

            QStringList deviceIDList = deviceIDStr.split("/", QString::SkipEmptyParts);
            if (deviceIDList.isEmpty())
            {
                continue;
            }

            auto deviceID = DeviceID::fromHex(deviceIDList.at(0));

            for (auto unitId : unitContolMap.keys())
            {
                auto contrlMap = unitContolMap.value(unitId);

                auto paramValueMap = mParamMacroData.modeParamMap[workMode].deviceParamMap[deviceID].unitParamMap[unitId];
                for (auto paramId : contrlMap.keys())
                {
                    if (paramValueMap.contains(paramId))
                    {
                        auto control = contrlMap.value(paramId);
                        if (control != nullptr)
                        {
                            control->setValue(paramValueMap.value(paramId));
                        }
                    }
                }
            }
        }
    }

    // 更新动态参数值
    for (auto workMode : mModeDynamicBtnMap.keys())
    {
        auto pointFreqBtnMap = mModeDynamicBtnMap.value(workMode);
        for (auto pointFreqNo : pointFreqBtnMap.keys())
        {
            auto deviceBtnMap = pointFreqBtnMap.value(pointFreqNo);
            for (auto deviceIDStr : deviceBtnMap.keys())
            {
                QStringList deviceIDList = deviceIDStr.split("/", QString::SkipEmptyParts);
                if (deviceIDList.isEmpty())
                {
                    continue;
                }

                DeviceID deviceID;
                int unitId;
                QStringList list2 = deviceIDList.at(0).split("_", QString::SkipEmptyParts);
                if (list2.size() != 2)
                {
                    continue;
                }
                deviceID = DeviceID::fromHex(list2.at(0));
                unitId = list2.at(1).toInt();

                // 设置控件值
                auto btn = deviceBtnMap.value(deviceIDStr);
                if (btn == nullptr)
                {
                    continue;
                }
                auto dynamicGroupId = btn->property("DynamicGroupId").toString();

                auto dynamicParamValueMap =
                    mParamMacroData.modeParamMap[workMode].pointFreqParamMap[pointFreqNo][deviceID].dynamicParamMap[dynamicGroupId][unitId];

                btn->setProperty("ChangeValue", QVariant::fromValue(dynamicParamValueMap));
            }
        }
    }
    //特殊处理约束关系
    if (ExtendedConfig::curProjectID() == "4426")
    {
        if (mModeDeviceControlMap.contains(STTC))
        {
            processRelation_4426_STTC();
        }
        if (mModeDeviceControlMap.contains(Skuo2))
        {
            processRelation_4426_SKuo2();
        }
        if (mModeDeviceControlMap.contains(SKT))
        {
            processRelation_4426_SKT();
        }
        if (mModeDeviceControlMap.contains(KaDS))
        {
            processRelation_4426_KaDS();
        }
        if (mModeDeviceControlMap.contains(KaGS))
        {
            processRelation_4426_KaGS();
        }
        if (mModeDeviceControlMap.contains(XDS))
        {
            processRelation_4426_XDS();
        }
    }

    // 第二次更新点频参数值,防止出现约束关系覆盖刷新的情况
    for (auto workMode : mModePointFreqControlMap.keys())
    {
        auto pointFreqContrlMap = mModePointFreqControlMap.value(workMode);

        for (auto pointFreqNo : pointFreqContrlMap.keys())
        {
            auto deviceContrlMap = pointFreqContrlMap.value(pointFreqNo);
            for (auto deviceIDStr : deviceContrlMap.keys())
            {
                auto contrlMap = deviceContrlMap.value(deviceIDStr);

                QStringList deviceIDList = deviceIDStr.split("/", QString::SkipEmptyParts);
                if (deviceIDList.isEmpty())
                {
                    continue;
                }

                DeviceID deviceID;
                int unitId;
                QStringList list2 = deviceIDList.at(0).split("_", QString::SkipEmptyParts);
                if (list2.size() != 2)
                {
                    continue;
                }

                deviceID = DeviceID::fromHex(list2.at(0));
                unitId = list2.at(1).toInt();

                auto paramValueMap = mParamMacroData.modeParamMap[workMode].pointFreqParamMap[pointFreqNo][deviceID].unitParamMap[unitId];

                for (auto paramId : contrlMap.keys())
                {
                    if (paramValueMap.contains(paramId))
                    {
                        auto control = contrlMap.value(paramId);
                        if (control != nullptr)
                        {
                            if (control->value() != paramValueMap.value(paramId))
                                control->setValue(paramValueMap.value(paramId));
                        }
                    }
                }
            }
        }
    }

    // 第二次更新更新设备参数值,防止出现约束关系覆盖刷新的情况
    for (auto workMode : mModeDeviceControlMap.keys())
    {
        auto deviceControlMap = mModeDeviceControlMap.value(workMode);
        for (auto deviceIDStr : deviceControlMap.keys())
        {
            auto unitContolMap = deviceControlMap.value(deviceIDStr);

            QStringList deviceIDList = deviceIDStr.split("/", QString::SkipEmptyParts);
            if (deviceIDList.isEmpty())
            {
                continue;
            }

            auto deviceID = DeviceID::fromHex(deviceIDList.at(0));

            for (auto unitId : unitContolMap.keys())
            {
                auto contrlMap = unitContolMap.value(unitId);

                auto paramValueMap = mParamMacroData.modeParamMap[workMode].deviceParamMap[deviceID].unitParamMap[unitId];
                for (auto paramId : contrlMap.keys())
                {
                    if (paramValueMap.contains(paramId))
                    {
                        auto control = contrlMap.value(paramId);
                        if (control != nullptr)
                        {
                            if (control->value() != paramValueMap.value(paramId))
                                control->setValue(paramValueMap.value(paramId));
                        }
                    }
                }
            }
        }
    }

    for (auto control : mSReceivPolarList)
    {
        emit control->sig_valueChange(control->value());
    }

    // 查看时隐藏保存按钮和所有控件不可用
    if (mCurrentMode == WidgetMode::Look)
    {
        ui->okBtn->hide();

        for (auto treeWidget : mTreeWidgetMap)
        {
            auto widgets = treeWidget->findChildren<WWidget*>();
            for (auto widget : widgets)
            {
                widget->setEnabled(false);
            }
        }
    }

    // 初始化信号槽连接
    initSlot();
}

void AddParamMacroDialog::syncParamData(const SatelliteManagementData& currentSatelliteData)
{
    /* 查看数据不进行处理 */
    if (mCurrentMode == WidgetMode::Look)
    {
        return;
    }
    /* 20211015 这里得根据当前的卫星的参数去初始化界面,得根据当前卫星的
     * 1.先得到卫星的数据
     * 2.然后遍历当前任务代号的卫星模式
     * 如果卫星中有宏里面没有则添加否则忽略
     *
     * 3.然后遍历当前的宏如果当前宏有的模式卫星里面没有则删除
     */
    for (auto iter = currentSatelliteData.m_workModeParamMap.begin(); iter != currentSatelliteData.m_workModeParamMap.end(); ++iter)
    {
        auto satelliteMode = iter.key();
        if (mParamMacroData.modeParamMap.contains(satelliteMode))
        {
            continue;
        }
        /* 不存在则添加 */
        auto dpData = iter.value();
        auto size = dpData.m_dpInfoMap.size();
        for (int i = 0; i < size; ++i)
        {
            mParamMacroData.modeParamMap[satelliteMode].pointFreqParamMap[i] = QMap<DeviceID, DeviceData>();
        }
    }
    for (auto iter = mParamMacroData.modeParamMap.begin(); iter != mParamMacroData.modeParamMap.end();)
    {
        if (!currentSatelliteData.m_workModeParamMap.contains(iter.key()))
        {
            iter = mParamMacroData.modeParamMap.erase(iter);
            continue;
        }

        ++iter;
    }
    /* 同步一下工作模式的描述 */
    QStringList workModeDesc;
    for (auto iter = mParamMacroData.modeParamMap.begin(); iter != mParamMacroData.modeParamMap.end(); ++iter)
    {
        workModeDesc << SystemWorkModeHelper::systemWorkModeToDesc(iter.key());
    }
    mParamMacroData.workMode = workModeDesc.join("+");
}

void AddParamMacroDialog::initSlot()
{
    connect(ui->paramMacroModeTreeWidget, &QTreeWidget::itemClicked, this, &AddParamMacroDialog::slotParamMacroModeTreeWidgetClicked);
    connect(ui->paramMacroModeTreeWidget, &QTreeWidget::customContextMenuRequested, this,
            &AddParamMacroDialog::slotParamMacroModeTreeWidgetCustomContextMenuRequested);

    connect(ui->okBtn, &QPushButton::clicked, this, &AddParamMacroDialog::slotOkBtnClicked);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &AddParamMacroDialog::close);
}

void AddParamMacroDialog::setWidgetMode(const AddParamMacroDialog::WidgetMode& currentUIMode)
{
    mCurrentMode = currentUIMode;

    switch (mCurrentMode)
    {
    case WidgetMode::Add:
    {
        mTipsMessage = QString("添加");
    }
    break;
    case WidgetMode::Edit:
    {
        mTipsMessage = QString("编辑");
    }
    break;
    }
}

void AddParamMacroDialog::setParamMacroModeInfoList(const QList<ParamMacroModeInfo>& paramMacroModeInfoList)
{
    mParamMacroModeInfoList = paramMacroModeInfoList;
}

void AddParamMacroDialog::setParamMacroData(const ParamMacroData& paramMacro) { mParamMacroData = paramMacro; }

ParamMacroData AddParamMacroDialog::getParamMacroData() const { return mParamMacroData; }

void AddParamMacroDialog::closeEvent(QCloseEvent* event)
{
    if (!QMessageBox::information(this, "提示", "是否保存参数宏数据后关闭当前界面？", QString("确定"),
                                  QString("取消")))  //点击确定结果是0,取消是1,所以用1
    {
        slotOkBtnClicked();  //点击确定就触发一次保存参数宏的按钮后关闭界面  取消的话就直接关闭界面
    }
    else
    {
        event->accept();
    }
}

void AddParamMacroDialog::createPointFreqControl(QTreeWidget* tree, QTreeWidgetItem* parentItem, SystemWorkMode workMode, int dpNum,
                                                 const QString& deviceID, const ParameterAttribute& attr, const QMap<QString, DevEnum>& devEnumMap)
{
    QTreeWidgetItem* attrItem = new QTreeWidgetItem;

    QMap<QString, QVariant> enumMap;
    //更新界面的枚举值
    if (attr.displayFormat == DisplayFormat_Enum)
    {
        auto paramEnumList = devEnumMap.value(attr.enumType);
        for (auto paramEnum : paramEnumList.emumEntryList)
        {
            enumMap[paramEnum.desc] = paramEnum.uValue;
        }
    }

    auto control = ControlFactory::createSetControl(attr, enumMap);

    if (control == nullptr)
    {
        return;
    }

    // 这里设置为固定值时 是可以隐藏控件的
    if (attr.isShow)
    {
        parentItem->addChild(attrItem);
        tree->setItemWidget(attrItem, 0, control);
    }
    if (attr.fixValue.isValid())
    {
        control->setValue(attr.fixValue);
    }
    QString objName = QString("") + QString::number(workMode) + "&&" + QString::number(dpNum) + "&&" + deviceID + "&&" + attr.id;
    control->setObjectName(objName);
    mModePointFreqControlMap[workMode][dpNum][deviceID][attr.id] = control;
    auto text = parentItem->text(0);
    control->setProperty("PositionPoint", text);

    // 判断是否是X高速或者Ka高速接收极化控件
    if (attr.id == "SReceivPolar" && (workMode == SystemWorkMode::XGS || workMode == SystemWorkMode::KaGS))
    {
        connect(control, static_cast<void (WWidget::*)(const QVariant& value)>(&WWidget::sig_valueChange), this, [=](const QVariant& value) {
            auto val = value.toInt();
            auto unitItemMap = mPointFreqUnitParamMap[workMode][dpNum];

            if (val == 1 || val == 2)
            {
                for (auto groupName : unitItemMap.keys())
                {
                    if (groupName == "解调单元2" || groupName == "I路译码单元2" || groupName == "Q路译码单元2")
                    {
                        unitItemMap.value(groupName)->setHidden(true);
                    }
                }
            }
            else if (val == 3)
            {
                for (auto groupName : unitItemMap.keys())
                {
                    if (groupName == "解调单元2" || groupName == "I路译码单元2" || groupName == "Q路译码单元2")
                    {
                        unitItemMap.value(groupName)->setHidden(false);
                    }
                }
            }
        });

        mSReceivPolarList << control;
    }
    if (attr.id == "SReceivPolar")
    {
        m_currentReceivPolarMap[workMode][dpNum] = control;
        //        connect(control, static_cast<void (WWidget::*)(const QVariant& value)>(&WWidget::sig_valueChange), this, [=](const QVariant& value)
        //        {
        //            auto val = value.toInt();
        //            for (auto _key : mModePointFreqControlMap[workMode][dpNum].keys())
        //            {
        //                if (mModePointFreqControlMap[workMode][dpNum][_key].contains("STrackPolar"))
        //                {
        //                    auto STrackPolarControl = mModePointFreqControlMap[workMode][dpNum][_key]["STrackPolar"];
        //                    STrackPolarControl->blockSignals(true);
        //                    //当值为左右旋同时的时候，就取卫星管理里的数据，这是为了避免极化关联关系判断时的死循环
        //                    if (val == 3)
        //                    {
        //                        // 获取该参数宏任务代号对应的卫星数据
        //                        SatelliteManagementData currentSatelliteData;
        //                        // 如果没有查询到该参数宏对应的卫星(编辑、查看参数宏的时候可能出现该问题)
        //                        GlobalData::getSatelliteManagementData(mParamMacroData.taskCode, currentSatelliteData);
        //                        SystemOrientation orientation;
        //                        currentSatelliteData.getSTrackPolar(workMode, dpNum, orientation);
        //                        STrackPolarControl->setValue((int)orientation);
        //                    }
        //                    else
        //                    {
        //                        STrackPolarControl->setValue(val);
        //                    }
        //                    STrackPolarControl->blockSignals(false);
        //                    return;
        //                }
        //            }
        //        });
    }

    if (attr.id == "STrackPolar")
    {
        m_currentTrackPolarMap[workMode][dpNum] = control;
        //        connect(control, static_cast<void (WWidget::*)(const QVariant& value)>(&WWidget::sig_valueChange), this, [=](const QVariant& value)
        //        {
        //            auto val = value.toInt();
        //            for (auto _key : mModePointFreqControlMap[workMode][dpNum].keys())
        //            {
        //                if (mModePointFreqControlMap[workMode][dpNum][_key].contains("SReceivPolar"))
        //                {
        //                    auto SReceivPolarControl = mModePointFreqControlMap[workMode][dpNum][_key]["SReceivPolar"];
        //                    SReceivPolarControl->blockSignals(true);
        //                    SReceivPolarControl->setValue(val);
        //                    SReceivPolarControl->blockSignals(false);
        //                    return;
        //                }
        //            }
        //        });
    }

    if (attr.id == "SEmissPolar")
    {
        m_currentSendPolarMap[workMode][dpNum] = control;
    }
}

void AddParamMacroDialog::createDeviceParamControl(QTreeWidget* tree, QTreeWidgetItem* parentItem, QTreeWidgetItem* deviceWidgetItem,
                                                   SystemWorkMode workMode, const QString& deviceID, int unitID, const ParameterAttribute& attr,
                                                   const QMap<QString, DevEnum>& devEnumMap)
{
    QTreeWidgetItem* attrItem = new QTreeWidgetItem;

    QMap<QString, QVariant> enumMap;
    //更新界面的枚举值
    if (attr.displayFormat == DisplayFormat_Enum)
    {
        auto paramEnumList = devEnumMap.value(attr.enumType);
        for (auto paramEnum : paramEnumList.emumEntryList)
        {
            enumMap[paramEnum.desc] = paramEnum.uValue;
        }
    }

    auto control = ControlFactory::createSetControl(attr, enumMap);

    if (control == nullptr)
    {
        return;
    }
    // 这里设置为固定值时 是可以隐藏控件的
    if (attr.isShow)
    {
        parentItem->addChild(attrItem);
        tree->setItemWidget(attrItem, 0, control);
    }
    if (attr.fixValue.isValid())
    {
        control->setValue(attr.fixValue);
    }

    QString objName = QString("%1&&%2&&%3&&%4").arg(QString::number(workMode)).arg(deviceID).arg(QString::number(unitID)).arg(attr.id);
    control->setObjectName(objName);

    mModeDeviceControlMap[workMode][deviceID][unitID][attr.id] = control;
    auto groupName = parentItem->text(0);         //控件所在单元
    auto deviceName = deviceWidgetItem->text(0);  //控件所在设备
    control->setProperty("PositionDevice", deviceName + "，" + groupName);
}

void AddParamMacroDialog::processRelation_4426_STTC()
{
    QMap<QString, QMap<int, QMap<QString, WWidget*>>> devControls = mModeDeviceControlMap[STTC];
    QString deviceIds = "0x4001/0x4003";
    if (devControls.contains(deviceIds))
    {
        // 参数宏-设备参数-TTC左旋接收、TTC右旋接收单元-载波环路带宽
        QMap<int, QMap<QString, WWidget*>> controls = devControls[deviceIds];
        WWidget* carrierLoopBandWidth_LeftWidget = controls[2]["CarrierLoopBandWidth"];
        WWidget* carrierLoopBandWidth_RightWidget = controls[3]["CarrierLoopBandWidth"];
        carrierLoopBandWidth_RightWidget->setEnabled(false);
        connect(carrierLoopBandWidth_LeftWidget, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this,
                [=](const QVariant& value) { carrierLoopBandWidth_RightWidget->setValue(value); });

        //参数宏-设备参数-左旋测距、右旋测距-距离预报
        WWidget* distMethodWay_LeftWidget = controls[5]["DistMethodWay"];
        WWidget* distMethodWay_RightWidget = controls[6]["DistMethodWay"];
        distMethodWay_RightWidget->setEnabled(false);
        connect(distMethodWay_LeftWidget, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this,
                [=](const QVariant& value) { distMethodWay_RightWidget->setValue(value); });

        //参数宏-设备参数-左旋测距、右旋测距-距离预报值
        WDouble* distPredMethod_LeftWidget = static_cast<WDouble*>(controls[5]["DistPredMethod"]);
        WDouble* distPredMethod_RightWidget = static_cast<WDouble*>(controls[6]["DistPredMethod"]);
        distPredMethod_RightWidget->setEnabled(false);
        connect(distPredMethod_LeftWidget, static_cast<void (WDouble::*)(const QString&)>(&WDouble::sig_valueChange), this,
                [=](const QString& value) { distPredMethod_RightWidget->setValue(value); });
    }
}

void AddParamMacroDialog::processRelation_4426_SKuo2()
{
    QMap<int, QMap<QString, QMap<QString, WWidget*>>> pointFreqControls = mModePointFreqControlMap[Skuo2];
    for (auto pointFreq : pointFreqControls.keys())
    {
        QString deviceIds = "4001_1/4003_1/4001_4/4003_4";
        if (!pointFreqControls[pointFreq].contains(deviceIds))
            continue;
        QMap<QString, WWidget*> controls = pointFreqControls[pointFreq][deviceIds];
        WWidget* workStation = controls["K2_WorkStation"];  //工作站数目
        WWidget* speedMeaPer = controls["K2_SpeedMeaPer"];  //测速采样周期
        workStation->setValue(1);
        connect(workStation, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this, [=](const QVariant& value) {
            if (value == 1)
            {
                QMap<QString, QVariant> enums;
                enums["1Hz"] = QVariant(5);
                enums["2Hz"] = QVariant(4);
                enums["4Hz"] = QVariant(3);
                enums["10Hz"] = QVariant(2);
                enums["20Hz"] = QVariant(1);
                speedMeaPer->setItem(enums);
            }
            else if (value == 2 || value == 3)
            {
                QMap<QString, QVariant> enums;
                enums["1Hz"] = QVariant(5);
                enums["2Hz"] = QVariant(4);
                enums["4Hz"] = QVariant(3);
                speedMeaPer->setItem(enums);
            }
            else
            {
                QMap<QString, QVariant> enums;
                enums["1Hz"] = QVariant(5);
                enums["2Hz"] = QVariant(4);
                speedMeaPer->setItem(enums);
            }
        });
    }
}

void AddParamMacroDialog::processRelation_4426_SKT()
{
    QMap<QString, QMap<int, QMap<QString, WWidget*>>> devControls = mModeDeviceControlMap[SKT];
    QString deviceIds = "0x4001/0x4003/0xAAAA";
    if (devControls.contains(deviceIds))
    {
        // 参数宏-设备参数-测控基带-固定数
        QMap<int, QMap<QString, WWidget*>> controls = devControls[deviceIds];
        WWidget* DataType = controls[6]["DataType"];
        WWidget* FixNumSet = controls[6]["FixNumSet"];
        // WWidget* DataSource = controls[6]["DataSource"];
        connect(DataType, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this, [=](const QVariant& value) {
            if (value.toString() == "1")
            {
                FixNumSet->setEnabled(true);
                //                DataSource->setEnabled(true);
            }
            else
            {
                FixNumSet->setEnabled(false);
                //                DataSource->setEnabled(false);
            }
        });
        DataType->setValue("1");
    }
}

void AddParamMacroDialog::processRelation_4426_KaDS()
{
    QMap<int, QMap<QString, QMap<QString, WWidget*>>> pointFreqControls = mModePointFreqControlMap[KaDS];
    QMap<int, QMap<QString, QPushButton*>> dynamicBtns = mModeDynamicBtnMap[KaDS];
    QMap<int, QMap<DeviceID, DeviceData>> pointFreqParamMap = mParamMacroData.modeParamMap[KaDS].pointFreqParamMap;
    for (auto pointFreq : pointFreqControls.keys())
    {
        WWidget* CarrierModulation = pointFreqControls[pointFreq]["4401_2/4402_2"]["CarrierModulation"];
        //参数宏-点频参数-I路码速率-I路码速率
        QString devids_I = "4401_4/4402_4";
        WWidget* CodeWay_I = pointFreqControls[pointFreq][devids_I]["CodeWay"];
        WWidget* CodeType_I = pointFreqControls[pointFreq][devids_I]["CodeType"];
        WWidget* ViterbiDecoding_I = pointFreqControls[pointFreq][devids_I]["ViterbiDecoding"];
        WWidget* LDPCCodeRate_I = pointFreqControls[pointFreq][devids_I]["LDPCCodeRate"];
        WWidget* TurboCodeRate_I = pointFreqControls[pointFreq][devids_I]["TurboCodeRate"];
        WWidget* CodeRateSett_I = pointFreqControls[pointFreq][devids_I]["CodeRateSett"];
        QPushButton* pushButton_I = dynamicBtns[pointFreq][devids_I];
        QList<WWidget*> iList;
        iList << CodeWay_I << CodeType_I << ViterbiDecoding_I << LDPCCodeRate_I << TurboCodeRate_I << CodeRateSett_I;
        for (auto widget : iList)
        {
            connect(widget, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this, [=](const QVariant& /*value*/) {
                QMap<QString, QVariant> setParamJSMap, setParamJTMap;
                int min = 0, max = 0;
                DeviceData deviceData = pointFreqParamMap[pointFreq][ds_BBEID];
                setParamJSMap.insert("CarrierModulation", CarrierModulation->value());
                setParamJTMap.insert("CodeWay", CodeWay_I->value());
                setParamJTMap.insert("CodeType", CodeType_I->value());
                setParamJTMap.insert("ViterbiDecoding", ViterbiDecoding_I->value());
                setParamJTMap.insert("LDPCCodeRate", LDPCCodeRate_I->value());
                setParamJTMap.insert("TurboCodeRate", TurboCodeRate_I->value());
                setParamJTMap.insert("CodeRateSett", CodeRateSett_I->value());
                dealDSWidgetMSLNumRangeJT(setParamJSMap, setParamJTMap, max, min);
                pushButton_I->setProperty("minValue", min);
                pushButton_I->setProperty("maxValue", max);
            });
        }
        QMap<QString, QVariant> setParamJSMap_I, setParamJTMap_I;
        int min_I = 0, max_I = 0;
        DeviceData deviceData = pointFreqParamMap[pointFreq][ds_BBEID];
        setParamJSMap_I = deviceData.unitParamMap[2];
        setParamJTMap_I = deviceData.unitParamMap[4];
        dealDSWidgetMSLNumRangeJT(setParamJSMap_I, setParamJTMap_I, max_I, min_I);
        pushButton_I->setProperty("minValue", min_I);
        pushButton_I->setProperty("maxValue", max_I);

        //参数宏-点频参数-Q路码速率-Q路码速率
        QString devids_Q = "4401_5/4402_5";
        WWidget* CodeWay_Q = pointFreqControls[pointFreq][devids_Q]["CodeWay"];
        WWidget* CodeType_Q = pointFreqControls[pointFreq][devids_Q]["CodeType"];
        WWidget* ViterbiDecoding_Q = pointFreqControls[pointFreq][devids_Q]["ViterbiDecoding"];
        WWidget* LDPCCodeRate_Q = pointFreqControls[pointFreq][devids_Q]["LDPCCodeRate"];
        WWidget* TurboCodeRate_Q = pointFreqControls[pointFreq][devids_Q]["TurboCodeRate"];
        WWidget* CodeRateSett_Q = pointFreqControls[pointFreq][devids_Q]["CodeRateSett"];
        QPushButton* pushButton_Q = dynamicBtns[pointFreq][devids_Q];
        QList<WWidget*> qList;
        qList << CodeWay_Q << CodeType_Q << ViterbiDecoding_Q << LDPCCodeRate_Q << TurboCodeRate_Q << CodeRateSett_Q;
        for (auto widget : qList)
        {
            connect(widget, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this, [=](const QVariant& /*value*/) {
                QMap<QString, QVariant> setParamJSMap, setParamJTMap;
                int min = 0, max = 0;
                DeviceData deviceData = pointFreqParamMap[pointFreq][ds_BBEID];
                setParamJSMap.insert("CarrierModulation", CarrierModulation->value());
                setParamJTMap.insert("CodeWay", CodeWay_Q->value());
                setParamJTMap.insert("CodeType", CodeType_Q->value());
                setParamJTMap.insert("ViterbiDecoding", ViterbiDecoding_Q->value());
                setParamJTMap.insert("LDPCCodeRate", LDPCCodeRate_Q->value());
                setParamJTMap.insert("TurboCodeRate", TurboCodeRate_Q->value());
                setParamJTMap.insert("CodeRateSett", CodeRateSett_Q->value());
                dealDSWidgetMSLNumRangeJT(setParamJSMap, setParamJTMap, max, min);
                pushButton_Q->setProperty("minValue", min);
                pushButton_Q->setProperty("maxValue", max);
            });
        }
        QMap<QString, QVariant> setParamJSMap_Q, setParamJTMap_Q;
        int min_Q = 0, max_Q = 0;
        DeviceData deviceData_Q = pointFreqParamMap[pointFreq][ds_BBEID];
        setParamJSMap_Q = deviceData.unitParamMap[2];
        setParamJTMap_Q = deviceData.unitParamMap[5];
        dealDSWidgetMSLNumRangeJT(setParamJSMap_Q, setParamJTMap_Q, max_Q, min_Q);
        pushButton_Q->setProperty("minValue", min_Q);
        pushButton_Q->setProperty("maxValue", max_Q);

        connect(CarrierModulation, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this, [=](const QVariant& /*value*/) {
            QMap<QString, QVariant> setParamJSMap, setParamJTMap;
            int min = 0, max = 0;
            DeviceData deviceData = pointFreqParamMap[pointFreq][ds_BBEID];

            setParamJSMap.insert("CarrierModulation", CarrierModulation->value());
            setParamJTMap.insert("CodeWay", CodeWay_I->value());
            setParamJTMap.insert("CodeType", CodeType_I->value());
            setParamJTMap.insert("ViterbiDecoding", ViterbiDecoding_I->value());
            setParamJTMap.insert("LDPCCodeRate", LDPCCodeRate_I->value());
            setParamJTMap.insert("TurboCodeRate", TurboCodeRate_I->value());
            setParamJTMap.insert("CodeRateSett", CodeRateSett_I->value());
            dealDSWidgetMSLNumRangeJT(setParamJSMap, setParamJTMap, max, min);
            pushButton_I->setProperty("minValue", min);
            pushButton_I->setProperty("maxValue", max);

            setParamJSMap.clear();
            setParamJTMap.clear();
            setParamJSMap.insert("CarrierModulation", CarrierModulation->value());
            setParamJTMap.insert("CodeWay", CodeWay_Q->value());
            setParamJTMap.insert("CodeType", CodeType_Q->value());
            setParamJTMap.insert("ViterbiDecoding", ViterbiDecoding_Q->value());
            setParamJTMap.insert("LDPCCodeRate", LDPCCodeRate_Q->value());
            setParamJTMap.insert("TurboCodeRate", TurboCodeRate_Q->value());
            setParamJTMap.insert("CodeRateSett", CodeRateSett_Q->value());
            dealDSWidgetMSLNumRangeJT(setParamJSMap, setParamJTMap, max, min);
            pushButton_Q->setProperty("minValue", min);
            pushButton_Q->setProperty("maxValue", max);
        });

        //参数宏-点频参数-上行数传单元-上行数传码速率
        QString devids_SC = "4401_11/4402_11";
        WWidget* DecodMode_SC = pointFreqControls[pointFreq][devids_SC]["DecodMode"];
        WWidget* CodeType_SC = pointFreqControls[pointFreq][devids_SC]["CodeType"];
        WWidget* ILDPCCodeWay_SC = pointFreqControls[pointFreq][devids_SC]["ILDPCCodeRate"];
        WWidget* CodeRateChoose_SC = pointFreqControls[pointFreq][devids_SC]["CodeRateChoose"];
        QPushButton* pushButton_SC = dynamicBtns[pointFreq][devids_SC];
        QList<WWidget*> scList;
        scList << DecodMode_SC << CodeType_SC << ILDPCCodeWay_SC << CodeRateChoose_SC;
        for (auto widget : scList)
        {
            connect(widget, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this, [=](const QVariant& /*value*/) {
                QMap<QString, QVariant> setParamSCMap;
                int min = 0, max = 0;
                DeviceData deviceData = pointFreqParamMap[pointFreq][ds_BBEID];
                setParamSCMap.insert("DecodMode", DecodMode_SC->value());
                setParamSCMap.insert("CodeType", CodeType_SC->value());
                setParamSCMap.insert("ILDPCCodeRate", ILDPCCodeWay_SC->value());
                setParamSCMap.insert("CodeRateChoose", CodeRateChoose_SC->value());
                dealDSWidgetMSLNumRangeSC(setParamSCMap, max, min);

                pushButton_SC->setProperty("minValue", min);
                pushButton_SC->setProperty("maxValue", max);
            });
        }
        QMap<QString, QVariant> setParamSCMap;
        int min_SC = 0, max_SC = 0;
        DeviceData deviceData_SC = pointFreqParamMap[pointFreq][ds_BBEID];
        setParamSCMap = deviceData.unitParamMap[11];
        dealDSWidgetMSLNumRangeSC(setParamSCMap, max_SC, min_SC);
        pushButton_SC->setProperty("minValue", min_SC);
        pushButton_SC->setProperty("maxValue", max_SC);
    }
}

void AddParamMacroDialog::processRelation_4426_KaGS()
{
    //    //参数宏-设备参数-Ka低速数传及跟踪高频箱-工作频率3,5
    //    QMap<int, QMap<QString, WWidget*>> devControls = mModeDeviceControlMap[KaGS]["0x3102"];
    //    WWidget* SBandDownFreq_3 = devControls[7]["SBandDownFreq"];
    //    WWidget* SBandDownFreq_5 = devControls[9]["SBandDownFreq"];
    //    SBandDownFreq_5->setEnabled(false);
    //    connect(SBandDownFreq_3, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this,
    //            [=](const QVariant& value) { SBandDownFreq_5->setValue(value); });

    //    //参数宏-设备参数-Ka低速数传及跟踪高频箱-工作频率4,6
    //    WWidget* SBandDownFreq_4 = devControls[8]["SBandDownFreq"];
    //    WWidget* SBandDownFreq_6 = devControls[10]["SBandDownFreq"];
    //    SBandDownFreq_6->setEnabled(false);
    //    connect(SBandDownFreq_4, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this,
    //            [=](const QVariant& value) { SBandDownFreq_6->setValue(value); });

    QMap<int, QMap<QString, QMap<QString, WWidget*>>> pointFreqControlMap = mModePointFreqControlMap[KaGS];
    QMap<int, QMap<QString, QPushButton*>> modeDynamicBtnMap = mModeDynamicBtnMap[KaGS];
    for (auto point : pointFreqControlMap.keys())
    {
        WWidget* ModulatSystem = pointFreqControlMap[point]["4101_2/4102_2/4103_2"]["ModulatSystem"];
        QPushButton* pushButton = modeDynamicBtnMap[point]["4101_02/4102_02/4103_02/4101_06/4102_06/4103_06"];
        connect(ModulatSystem, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this, [=](const QVariant& value) {
            switch (value.toInt())
            {
            case 3:
            {
                pushButton->setProperty("enable", false);
                break;
            }
            default:
            {
                pushButton->setProperty("enable", true);
                break;
            }
            }
        });
    }
}

void AddParamMacroDialog::processRelation_4426_XDS()
{
    QMap<int, QMap<QString, QMap<QString, WWidget*>>> pointFreqControls = mModePointFreqControlMap[XDS];
    QMap<int, QMap<QString, QPushButton*>> dynamicBtns = mModeDynamicBtnMap[XDS];
    QMap<int, QMap<DeviceID, DeviceData>> pointFreqParamMap = mParamMacroData.modeParamMap[XDS].pointFreqParamMap;
    for (auto pointFreq : pointFreqControls.keys())
    {
        WWidget* CarrierModulation = pointFreqControls[pointFreq]["4401_3/4402_3"]["CarrierModulation"];
        //参数宏-点频参数-I路码速率-I路码速率
        QString devids_I = "4401_6/4402_6";
        WWidget* CodeWay_I = pointFreqControls[pointFreq][devids_I]["CodeWay"];
        WWidget* CodeType_I = pointFreqControls[pointFreq][devids_I]["CodeType"];
        WWidget* ViterbiDecoding_I = pointFreqControls[pointFreq][devids_I]["ViterbiDecoding"];
        WWidget* LDPCCodeRate_I = pointFreqControls[pointFreq][devids_I]["LDPCCodeRate"];
        WWidget* TurboCodeRate_I = pointFreqControls[pointFreq][devids_I]["TurboCodeRate"];
        WWidget* CodeRateSett_I = pointFreqControls[pointFreq][devids_I]["CodeRateSett"];
        QPushButton* pushButton_I = dynamicBtns[pointFreq][devids_I];
        QList<WWidget*> iList;
        iList << CarrierModulation << CodeWay_I << CodeType_I << ViterbiDecoding_I << LDPCCodeRate_I << TurboCodeRate_I << CodeRateSett_I;
        for (auto widget : iList)
        {
            connect(widget, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this, [=](const QVariant& /*value*/) {
                QMap<QString, QVariant> setParamJSMap, setParamJTMap;
                int min = 0, max = 0;
                DeviceData deviceData = pointFreqParamMap[pointFreq][ds_BBEID];
                setParamJSMap.insert("CarrierModulation", CarrierModulation->value());
                setParamJTMap.insert("CodeWay", CodeWay_I->value());
                setParamJTMap.insert("CodeType", CodeType_I->value());
                setParamJTMap.insert("ViterbiDecoding", ViterbiDecoding_I->value());
                setParamJTMap.insert("LDPCCodeRate", LDPCCodeRate_I->value());
                setParamJTMap.insert("TurboCodeRate", TurboCodeRate_I->value());
                setParamJTMap.insert("CodeRateSett", CodeRateSett_I->value());
                dealDSWidgetMSLNumRangeJT(setParamJSMap, setParamJTMap, max, min);
                pushButton_I->setProperty("minValue", min);
                pushButton_I->setProperty("maxValue", max);
            });
        }
        QMap<QString, QVariant> setParamJSMap_I, setParamJTMap_I;
        int min_I = 0, max_I = 0;
        DeviceData deviceData = pointFreqParamMap[pointFreq][ds_BBEID];
        setParamJSMap_I = deviceData.unitParamMap[3];
        setParamJTMap_I = deviceData.unitParamMap[6];
        dealDSWidgetMSLNumRangeJT(setParamJSMap_I, setParamJTMap_I, max_I, min_I);
        pushButton_I->setProperty("minValue", min_I);
        pushButton_I->setProperty("maxValue", max_I);

        //参数宏-点频参数-Q路码速率-Q路码速率
        QString devids_Q = "4401_7/4402_7";
        WWidget* CodeWay_Q = pointFreqControls[pointFreq][devids_Q]["CodeWay"];
        WWidget* CodeType_Q = pointFreqControls[pointFreq][devids_Q]["CodeType"];
        WWidget* ViterbiDecoding_Q = pointFreqControls[pointFreq][devids_Q]["ViterbiDecoding"];
        WWidget* LDPCCodeRate_Q = pointFreqControls[pointFreq][devids_Q]["LDPCCodeRate"];
        WWidget* TurboCodeRate_Q = pointFreqControls[pointFreq][devids_Q]["TurboCodeRate"];
        WWidget* CodeRateSett_Q = pointFreqControls[pointFreq][devids_Q]["CodeRateSett"];
        QPushButton* pushButton_Q = dynamicBtns[pointFreq][devids_Q];
        QList<WWidget*> qList;
        qList << CarrierModulation << CodeWay_Q << CodeType_Q << ViterbiDecoding_Q << LDPCCodeRate_Q << TurboCodeRate_Q << CodeRateSett_Q;
        for (auto widget : qList)
        {
            connect(widget, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this, [=](const QVariant& /*value*/) {
                QMap<QString, QVariant> setParamJSMap, setParamJTMap;
                int min = 0, max = 0;
                DeviceData deviceData = pointFreqParamMap[pointFreq][ds_BBEID];
                setParamJSMap.insert("CarrierModulation", CarrierModulation->value());
                setParamJTMap.insert("CodeWay", CodeWay_Q->value());
                setParamJTMap.insert("CodeType", CodeType_Q->value());
                setParamJTMap.insert("ViterbiDecoding", ViterbiDecoding_Q->value());
                setParamJTMap.insert("LDPCCodeRate", LDPCCodeRate_Q->value());
                setParamJTMap.insert("TurboCodeRate", TurboCodeRate_Q->value());
                setParamJTMap.insert("CodeRateSett", CodeRateSett_Q->value());
                dealDSWidgetMSLNumRangeJT(setParamJSMap, setParamJTMap, max, min);
                pushButton_Q->setProperty("minValue", min);
                pushButton_Q->setProperty("maxValue", max);
            });
        }
        QMap<QString, QVariant> setParamJSMap_Q, setParamJTMap_Q;
        int min_Q = 0, max_Q = 0;
        DeviceData deviceData_Q = pointFreqParamMap[pointFreq][ds_BBEID];
        setParamJSMap_Q = deviceData.unitParamMap[3];
        setParamJTMap_Q = deviceData.unitParamMap[7];
        dealDSWidgetMSLNumRangeJT(setParamJSMap_Q, setParamJTMap_Q, max_Q, min_Q);
        pushButton_Q->setProperty("minValue", min_Q);
        pushButton_Q->setProperty("maxValue", max_Q);
    }

    QMap<int, QMap<QString, WWidget*>> devControls = mModeDeviceControlMap[XDS]["0x3104"];
    WWidget* RadioFrequDecrement_A_1 = devControls[6]["RadioFrequDecrement_A"];
    WWidget* RadioFrequDecrement_A_2 = devControls[7]["RadioFrequDecrement_A"];
    //  RadioFrequDecrement_A_2->setEnabled(false);
    connect(RadioFrequDecrement_A_1, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this,
            [=](const QVariant& value) { RadioFrequDecrement_A_2->setValue(value); });
}

void AddParamMacroDialog::dealDSWidgetMSLNumRangeJT(WWidget* control, const QMap<QString, QVariant>& setParamJSMap,
                                                    const QMap<QString, QVariant>& setParamJTMap)
{
    if (control == nullptr || setParamJSMap.isEmpty() || setParamJTMap.isEmpty())
        return;

    int tztz = setParamJSMap.value("CarrierModulation").toInt();          //调制体制
    auto codeWay = setParamJTMap.value("CodeWay").toInt();                //译码方式
    auto codeType = setParamJTMap.value("CodeType").toInt();              //码型
    auto viterbiCode = setParamJTMap.value("ViterbiDecoding").toInt();    // Viterbi译码
    auto ldpcCode = setParamJTMap.value("LDPCCodeRate").toInt();          // Ldpc码率
    auto turboCode = setParamJTMap.value("TurboCodeRate").toInt();        // Turbo码率
    auto codeSpeedType = setParamJTMap.value("CodeRateSett").toDouble();  //码速率设置方式

    int initMinValue;  //这个是译码方式为关各个调制体制的初始最小值
    int initMaxValue;  //这个是译码方式为关各个调制体制的初始最大值
    double bValue;
    QString cValue;

    int minValue = 0;
    int maxValue = 0;

    if (tztz == 1)  // BPSK
    {
        initMinValue = 1000;
        initMaxValue = 15000000;
    }
    else if (tztz == 2 || tztz == 5)  // QPSK OQPSK
    {
        initMinValue = 20000;
        initMaxValue = 30000000;
    }
    else if (tztz == 4)  // UQPSK时必为IQ分路
    {
        initMinValue = 10000;
        initMaxValue = 15000000;
    }

    if (codeType == 1 || codeType == 2 || codeType == 3)  // NRZ-L、M、S
    {
        bValue = 1;
    }
    else if (codeType == 4 || codeType == 5 || codeType == 6)  // Biφ-L、M、S
    {
        bValue = 0.5;
    }

    if (codeSpeedType == 1)  //编码前
    {
        if (codeWay == 2 || codeWay == 4)  // Viterbi或者R-S和Viterbi级联译码
        {
            if (viterbiCode == 1)  //(7,(1/2))
            {
                cValue = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                control->setNumRange(minValue, maxValue);
            }
            else if (viterbiCode == 2)
            {
                cValue = QString("2/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                control->setNumRange(minValue, maxValue);
            }
            else if (viterbiCode == 3)
            {
                cValue = QString("3/4");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                control->setNumRange(minValue, maxValue);
            }
            else if (viterbiCode == 4)
            {
                cValue = QString("5/6");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                control->setNumRange(minValue, maxValue);
            }
            else if (viterbiCode == 5)
            {
                cValue = QString("7/8");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                control->setNumRange(minValue, maxValue);
            }
        }
        else if (codeWay == 5)  // LDPC
        {
            if (ldpcCode == 1 || ldpcCode == 2)  // 1/2
            {
                cValue = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                control->setNumRange(minValue, maxValue);
            }
            else if (ldpcCode == 3 || ldpcCode == 4)  // 2/3
            {
                cValue = QString("2/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                control->setNumRange(minValue, maxValue);
            }
            else if (ldpcCode == 5 || ldpcCode == 6)  // 4/5
            {
                cValue = QString("4/5");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                control->setNumRange(minValue, maxValue);
            }
            if (ldpcCode == 7)  // 7/8
            {
                cValue = QString("7/8");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                control->setNumRange(minValue, maxValue);
            }
        }
        else if (codeWay == 6)  // Turbo
        {
            if (turboCode == 1 || turboCode == 2 || turboCode == 3 || turboCode == 4)  // 1/2
            {
                cValue = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                control->setNumRange(minValue, maxValue);
            }
            else if (turboCode == 5 || turboCode == 6 || turboCode == 7 || turboCode == 8)  // 1/3
            {
                cValue = QString("1/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                control->setNumRange(minValue, maxValue);
            }
            else if (turboCode == 9 || turboCode == 10 || turboCode == 11 || turboCode == 12)  // 1/4
            {
                cValue = QString("1/4");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                control->setNumRange(minValue, maxValue);
            }
            if (turboCode == 13 || turboCode == 14 || turboCode == 15 || turboCode == 16)  // 1/6
            {
                cValue = QString("1/6");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                control->setNumRange(minValue, maxValue);
            }
        }
        else if (codeWay == 1 || codeWay == 3)
        {
            cValue = QString("1/1");

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            control->setNumRange(minValue, maxValue);
        }
    }
    else if (codeSpeedType == 2)  //编码后  码速率值不再受到译码方式参数的影响
    {
        cValue = QString("1/1");

        calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

        control->setNumRange(minValue, maxValue);
    }

    control->setValue(minValue + 5000);
}

void AddParamMacroDialog::dealDSWidgetMSLNumRangeJT(const QMap<QString, QVariant>& setParamJSMap, const QMap<QString, QVariant>& setParamJTMap,
                                                    int& maxValue, int& minValue)
{
    if (setParamJSMap.isEmpty() || setParamJTMap.isEmpty())
        return;

    int tztz = setParamJSMap.value("CarrierModulation").toInt();          //调制体制
    auto codeWay = setParamJTMap.value("CodeWay").toInt();                //译码方式
    auto codeType = setParamJTMap.value("CodeType").toInt();              //码型
    auto viterbiCode = setParamJTMap.value("ViterbiDecoding").toInt();    // Viterbi译码
    auto ldpcCode = setParamJTMap.value("LDPCCodeRate").toInt();          // Ldpc码率
    auto turboCode = setParamJTMap.value("TurboCodeRate").toInt();        // Turbo码率
    auto codeSpeedType = setParamJTMap.value("CodeRateSett").toDouble();  //码速率设置方式

    int initMinValue;  //这个是译码方式为关各个调制体制的初始最小值
    int initMaxValue;  //这个是译码方式为关各个调制体制的初始最大值
    double bValue;
    QString cValue;

    if (tztz == 1)  // BPSK
    {
        initMinValue = 1000;
        initMaxValue = 15000000;
    }
    else if (tztz == 2 || tztz == 5)  // QPSK OQPSK
    {
        initMinValue = 20000;
        initMaxValue = 30000000;
    }
    else if (tztz == 4)  // UQPSK时必为IQ分路
    {
        initMinValue = 10000;
        initMaxValue = 15000000;
    }

    if (codeType == 1 || codeType == 2 || codeType == 3)  // NRZ-L、M、S
    {
        bValue = 1;
    }
    else if (codeType == 4 || codeType == 5 || codeType == 6)  // Biφ-L、M、S
    {
        bValue = 0.5;
    }

    if (codeSpeedType == 1)  //编码前
    {
        if (codeWay == 2 || codeWay == 4)  // Viterbi或者R-S和Viterbi级联译码
        {
            if (viterbiCode == 1)  //(7,(1/2))
            {
                cValue = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
            }
            else if (viterbiCode == 2)
            {
                cValue = QString("2/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
            }
            else if (viterbiCode == 3)
            {
                cValue = QString("3/4");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
            }
            else if (viterbiCode == 4)
            {
                cValue = QString("5/6");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
            }
            else if (viterbiCode == 5)
            {
                cValue = QString("7/8");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
            }
        }
        else if (codeWay == 5)  // LDPC
        {
            if (ldpcCode == 1 || ldpcCode == 2)  // 1/2
            {
                cValue = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
            }
            else if (ldpcCode == 3 || ldpcCode == 4)  // 2/3
            {
                cValue = QString("2/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
            }
            else if (ldpcCode == 5 || ldpcCode == 6)  // 4/5
            {
                cValue = QString("4/5");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
            }
            if (ldpcCode == 7)  // 7/8
            {
                cValue = QString("7/8");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
            }
        }
        else if (codeWay == 6)  // Turbo
        {
            if (turboCode == 1 || turboCode == 2 || turboCode == 3 || turboCode == 4)  // 1/2
            {
                cValue = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
            }
            else if (turboCode == 5 || turboCode == 6 || turboCode == 7 || turboCode == 8)  // 1/3
            {
                cValue = QString("1/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
            }
            else if (turboCode == 9 || turboCode == 10 || turboCode == 11 || turboCode == 12)  // 1/4
            {
                cValue = QString("1/4");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
            }
            if (turboCode == 13 || turboCode == 14 || turboCode == 15 || turboCode == 16)  // 1/6
            {
                cValue = QString("1/6");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
            }
        }
        else if (codeWay == 1 || codeWay == 3)
        {
            cValue = QString("1/1");

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
        }
    }
    else if (codeSpeedType == 2)  //编码后  码速率值不再受到译码方式参数的影响
    {
        cValue = QString("1/1");

        calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);
    }
}
void AddParamMacroDialog::dealDSWidgetMSLNumRangeSC(WWidget* control, const QMap<QString, QVariant>& setParamSCMap)
{
    if (control == nullptr || setParamSCMap.isEmpty())
        return;

    auto codeWay = setParamSCMap.value("DecodMode").toInt();  //译码方式
    auto codeType = setParamSCMap.value("CodeType").toInt();  //码型
    // auto viterbiCode = setParamSCMap.value("ViterbiDecoding").toInt();    // Viterbi译码
    // auto ldpcCode = setParamSCMap.value("ILDPCCodeRate").toInt();  // Ldpc码率
    // auto turboCode = setParamSCMap.value("TurboCodeRate").toInt();        // Turbo码率
    auto codeSpeedType = setParamSCMap.value("CodeRateChoose").toDouble();  //码速率设置方式

    int minValue = 0;
    int maxValue = 0;
    //编码后 编码方式为LDPC  码型为NRZ3个
    if (codeSpeedType == 2 && codeWay == 6 && (codeType == 1 || codeType == 2 || codeType == 3))
    {
        minValue = 1000;
        maxValue = 20000000;
    }
    //编码后 编码方式为LDPC  码型为Bi3个
    else if (codeSpeedType == 2 && codeWay == 6 && (codeType == 4 || codeType == 5 || codeType == 6))
    {
        minValue = 1000;
        maxValue = 10000000;
    }
    //编码后 编码方式为关 码型为Bi3个
    else if (codeSpeedType == 2 && codeWay == 1)
    {
        minValue = 1000;
        maxValue = 10000000;
    }
    //编码前 编码方式为关 码型为NRZ3个
    else if (codeSpeedType == 1 && codeWay == 1 && (codeType == 1 || codeType == 2 || codeType == 3))
    {
        minValue = 1000;
        maxValue = 10000000;
    }
    //编码前 编码方式为关 码型为Bi3个
    else if (codeSpeedType == 1 && codeWay == 1 && (codeType == 4 || codeType == 5 || codeType == 6))
    {
        minValue = 1000;
        maxValue = 5000000;
    }
    //编码前 编码方式为LDPC 码型为NRZ3个
    else if (codeSpeedType == 1 && codeWay == 6 && (codeType == 1 || codeType == 2 || codeType == 3))
    {
        minValue = 1000;
        maxValue = 8750000;
    }
    //编码前 编码方式为LDPC 码型为Bi3个
    else if (codeSpeedType == 1 && codeWay == 6 && (codeType == 4 || codeType == 5 || codeType == 6))
    {
        minValue = 1000;
        maxValue = 4375000;
    }

    control->setNumRange(minValue, maxValue);
    control->setValue(minValue + 5000);
}

void AddParamMacroDialog::dealDSWidgetMSLNumRangeSC(const QMap<QString, QVariant>& setParamSCMap, int& maxValue, int& minValue)
{
    if (setParamSCMap.isEmpty())
        return;

    auto codeWay = setParamSCMap.value("DecodMode").toInt();  //译码方式
    auto codeType = setParamSCMap.value("CodeType").toInt();  //码型
    // auto viterbiCode = setParamSCMap.value("ViterbiDecoding").toInt();    // Viterbi译码
    // auto ldpcCode = setParamSCMap.value("ILDPCCodeRate").toInt();  // Ldpc码率
    // auto turboCode = setParamSCMap.value("TurboCodeRate").toInt();        // Turbo码率
    auto codeSpeedType = setParamSCMap.value("CodeRateChoose").toDouble();  //码速率设置方式

    //编码后 编码方式为LDPC  码型为NRZ3个
    if (codeSpeedType == 2 && codeWay == 6 && (codeType == 1 || codeType == 2 || codeType == 3))
    {
        minValue = 1000;
        maxValue = 20000000;
    }
    //编码后 编码方式为LDPC  码型为Bi3个
    else if (codeSpeedType == 2 && codeWay == 6 && (codeType == 4 || codeType == 5 || codeType == 6))
    {
        minValue = 1000;
        maxValue = 10000000;
    }

    //编码后 编码方式为关 码型为Bi3个
    else if (codeSpeedType == 2 && codeWay == 1)
    {
        minValue = 1000;
        maxValue = 10000000;
    }
    //编码前 编码方式为关 码型为NRZ3个
    else if (codeSpeedType == 1 && codeWay == 1 && (codeType == 1 || codeType == 2 || codeType == 3))
    {
        minValue = 1000;
        maxValue = 10000000;
    }
    //编码前 编码方式为关 码型为Bi3个
    else if (codeSpeedType == 1 && codeWay == 1 && (codeType == 4 || codeType == 5 || codeType == 6))
    {
        minValue = 1000;
        maxValue = 5000000;
    }
    //编码前 编码方式为LDPC 码型为NRZ3个
    else if (codeSpeedType == 1 && codeWay == 6 && (codeType == 1 || codeType == 2 || codeType == 3))
    {
        minValue = 1000;
        maxValue = 8750000;
    }
    //编码前 编码方式为LDPC 码型为Bi3个
    else if (codeSpeedType == 1 && codeWay == 6 && (codeType == 4 || codeType == 5 || codeType == 6))
    {
        minValue = 1000;
        maxValue = 4375000;
    }
}

void AddParamMacroDialog::calcuCodeRateMinMaxValue(int initMinValue, int initMaxValue, double bValue, QString mark, int& minValue, int& maxValue)
{
    QString frontStep = mark.section('/', 0, 0);
    QString endStep = mark.section('/', 1, 1);
    if (endStep.toInt() >= 1 && frontStep.toInt() >= 1)
    {
        minValue = (int)(((initMinValue * bValue) * frontStep.toInt()) / endStep.toInt());
        maxValue = (int)(((initMaxValue * bValue) * frontStep.toInt()) / endStep.toInt());
    }
}

bool AddParamMacroDialog::checkPolarRelation(QString& msg)
{
    // 获取该参数宏任务代号对应的卫星数据
    SatelliteManagementData currentSatelliteData;
    // 如果没有查询到该参数宏对应的卫星(编辑、查看参数宏的时候可能出现该问题)
    if (!GlobalData::getSatelliteManagementData(mParamMacroData.taskCode, currentSatelliteData))
    {
        auto msg = QString("接收极化值更改,判断与卫星极化是否相等,但未找到对应的卫星,请确认卫星是否存在！");
        return false;
    }
    //发射极化判断
    if (!m_currentSendPolarMap.isEmpty())
    {
        for (auto workMode : m_currentSendPolarMap.keys())
        {
            //只有X低速和Ka高速没有发射极化
            if (workMode != XDS && workMode != KaGS)
            {
                for (auto dpNum : m_currentSendPolarMap[workMode].keys())
                {
                    auto control = m_currentSendPolarMap[workMode][dpNum];
                    SystemOrientation orientation;
                    currentSatelliteData.getLaunchPolar(workMode, dpNum, orientation);
                    auto controlValue = control->value();
                    if (controlValue != (int)orientation)
                    {
                        auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(workMode);
                        msg = QString("工作模式:%1,点频%2，当前发射极化与卫星所设置极化不一致!").arg(workModeDesc).arg(dpNum);
                        return false;
                    }
                }
            }
        }
    }
    //接收极化判断
    if (!m_currentReceivPolarMap.isEmpty())
    {
        for (auto workMode : m_currentReceivPolarMap.keys())
        {
            for (auto dpNum : m_currentReceivPolarMap[workMode].keys())
            {
                auto control = m_currentReceivPolarMap[workMode][dpNum];
                SystemOrientation orientation;
                currentSatelliteData.getSReceivPolar(workMode, dpNum, orientation);
                auto controlValue = control->value();
                if (controlValue != (int)orientation)
                {
                    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(workMode);
                    msg = QString("工作模式:%1,点频%2，当前接收极化与卫星所设置极化不一致!").arg(workModeDesc).arg(dpNum);
                    return false;
                }
            }
        }
    }
    //跟踪极化判断
    if (!m_currentTrackPolarMap.isEmpty())
    {
        for (auto workMode : m_currentTrackPolarMap.keys())
        {
            for (auto dpNum : m_currentTrackPolarMap[workMode].keys())
            {
                auto control = m_currentTrackPolarMap[workMode][dpNum];
                SystemOrientation orientation;
                currentSatelliteData.getSTrackPolar(workMode, dpNum, orientation);
                auto controlValue = control->value();
                if (controlValue != (int)orientation)
                {
                    auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(workMode);
                    msg = QString("工作模式:%1,点频%2，当前跟踪极化与卫星所设置极化不一致!").arg(workModeDesc).arg(dpNum);
                    return false;
                }
            }
        }
    }

    return true;
}

void AddParamMacroDialog::slotOperationResult(bool result, const QString& msg)
{
    if (result)
    {
        QMessageBox::information(nullptr, "提示", mTipsMessage + "参数宏成功", "确定");
        if (mCurrentMode == WidgetMode::Add)  //第一次添加参数宏成功后就使其变为编辑状态 可以继续更改数据
        {
            mCurrentMode = WidgetMode::Edit;
            mTipsMessage = QString("编辑");
        }
    }
    else
    {
        QMessageBox::critical(nullptr, mTipsMessage + "参数宏失败", msg, "确定");
    }
}

void AddParamMacroDialog::slotParamMacroModeTreeWidgetClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    if (mTreeWidgetMap.contains(item))
    {
        ui->contentStackWidget->setCurrentWidget(mTreeWidgetMap.value(item));
    }
}

void AddParamMacroDialog::slotOkBtnClicked()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    QString erroMsg;
    if (!checkPolarRelation(erroMsg))
    {
        QMessageBox::information(this, "提示", erroMsg, "确定");
        return;
    }

    mParamMacroData.modeParamMap.clear();

    // 点频参数
    for (auto workMode : mModePointFreqControlMap.keys())
    {
        auto pointFreqContrMap = mModePointFreqControlMap.value(workMode);
        auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(workMode);
        for (auto pointFreqNo : pointFreqContrMap.keys())
        {
            auto deviceControlMap = pointFreqContrMap.value(pointFreqNo);
            for (auto deviceIDStr : deviceControlMap.keys())
            {
                auto controlMap = deviceControlMap.value(deviceIDStr);

                QStringList deviceIdList = deviceIDStr.split("/", QString::SkipEmptyParts);
                for (auto item : deviceIdList)
                {
                    QStringList list2 = item.split("_", QString::SkipEmptyParts);
                    if (list2.size() == 2)
                    {
                        auto deviceID = DeviceID::fromHex(list2.at(0));
                        auto unitId = list2.at(1).toInt();

                        for (auto paramId : controlMap.keys())
                        {
                            auto control = controlMap.value(paramId);
                            if (control != nullptr)
                            {
                                bool flag = false;
                                auto value = control->value(flag);
                                if (!flag)
                                {
                                    auto text = control->property("PositionPoint").toString();
                                    QString errMsg = QString("点频参数数据非法，参数所在体制和点频：%1，点频%2，单元：%3，参数名称：%4")
                                                         .arg(workModeDesc)
                                                         .arg(pointFreqNo)
                                                         .arg(text)
                                                         .arg(control->paramAttr.desc);
                                    QMessageBox::information(this, "提示", errMsg, QString("确定"));
                                    return;
                                }

                                mParamMacroData.modeParamMap[workMode].pointFreqParamMap[pointFreqNo][deviceID].unitParamMap[unitId][paramId] = value;
                            }
                        }
                    }
                }
            }
        }
    }

    // 动态参数
    for (auto workMode : mModeDynamicBtnMap.keys())
    {
        auto pointFreqBtnMap = mModeDynamicBtnMap.value(workMode);
        for (auto pointFreqNo : pointFreqBtnMap.keys())
        {
            auto deviceBtnMap = pointFreqBtnMap.value(pointFreqNo);
            for (auto deviceIDStr : deviceBtnMap.keys())
            {
                auto btn = deviceBtnMap.value(deviceIDStr);
                auto dynamicParamMap = btn->property("ChangeValue").value<QMap<int, QMap<QString, QVariant>>>();
                auto dynamicGroupId = btn->property("DynamicGroupId").toString();

                QStringList deviceIdList = deviceIDStr.split("/", QString::SkipEmptyParts);
                for (auto item : deviceIdList)
                {
                    QStringList list2 = item.split("_", QString::SkipEmptyParts);
                    if (list2.size() == 2)
                    {
                        auto deviceID = DeviceID::fromHex(list2.at(0));
                        auto unitId = list2.at(1).toInt();

                        mParamMacroData.modeParamMap[workMode].pointFreqParamMap[pointFreqNo][deviceID].dynamicParamMap[dynamicGroupId][unitId] =
                            dynamicParamMap;
                    }
                }
            }
        }
    }

    // 设备参数
    for (auto workMode : mModeDeviceControlMap.keys())
    {
        auto deviceControlMap = mModeDeviceControlMap.value(workMode);
        auto workModeDesc = SystemWorkModeHelper::systemWorkModeToDesc(workMode);
        for (auto deviceIDStr : deviceControlMap.keys())
        {
            auto unitControlMap = deviceControlMap.value(deviceIDStr);

            QStringList list = deviceIDStr.split("/", QString::SkipEmptyParts);
            for (auto item : list)
            {
                auto deviceID = DeviceID(item.toInt(nullptr, 16));

                for (auto unitId : unitControlMap.keys())
                {
                    auto controlMap = unitControlMap.value(unitId);
                    for (auto paramId : controlMap.keys())
                    {
                        auto control = controlMap.value(paramId);
                        if (control != nullptr)
                        {
                            bool flag = false;
                            auto value = control->value(flag);
                            if (!flag)
                            {
                                auto text = control->property("PositionDevice").toString();
                                QString errMsg = QString("设备参数数据非法，参数所在体制：%1，设备名称及单元：%2，参数名称：%3")
                                                     .arg(workModeDesc)
                                                     .arg(text)
                                                     .arg(control->paramAttr.desc);
                                QMessageBox::information(this, "提示", errMsg, QString("确定"));
                                return;
                            }

                            mParamMacroData.modeParamMap[workMode].deviceParamMap[deviceID].unitParamMap[unitId][paramId] = value;
                        }
                    }
                }
            }
        }
    }

    QByteArray json;
    json << mParamMacroData;

    if (mCurrentMode == WidgetMode::Add)
    {
        //        if (!QMessageBox::information(this, "提示", "是否添加当前的参数宏", QString("确定"), QString("取消")))
        //        //点击确定结果是0,取消是1,所以用1
        //        {
        emit signalAddParamMacro(json);
        // }
    }
    else if (mCurrentMode == WidgetMode::Edit)
    {
        //        if (!QMessageBox::information(this, "提示", "是否保存当前的参数宏", QString("确定"), QString("取消")))
        //        //点击确定结果是0,取消是1,所以用1
        //        {
        emit signalEditParamMacro(json);
        // }
    }
}

void AddParamMacroDialog::slotParamMacroModeTreeWidgetCustomContextMenuRequested(const QPoint& pos)
{
    mPointFreqGroupParamItem = nullptr;
    auto treeItem = ui->paramMacroModeTreeWidget->itemAt(pos);
    if (treeItem != nullptr && treeItem->text(0).contains("点频"))
    {
        mPointFreqGroupParamItem = treeItem->parent();  // 记住点击节点的父节点

        QAction copyAction(QString("复制"), this);
        connect(&copyAction, &QAction::triggered, this, &AddParamMacroDialog::slotCopyPointFreqClicked);

        QMenu menu;
        menu.addAction(&copyAction);
        menu.exec(QCursor::pos());
    }
}

void AddParamMacroDialog::slotCopyPointFreqClicked()
{
    if (mPointFreqGroupParamItem == nullptr)
    {
        return;
    }

    QMap<QString, QTreeWidget*> pointFreqMap;
    for (auto i = 0; i < mPointFreqGroupParamItem->childCount(); i++)
    {
        auto item = mPointFreqGroupParamItem->child(i);
        if (!item->text(0).contains("点频"))  // 只保存点频的界面
        {
            continue;
        }

        auto pointFreqTreeWidget = mTreeWidgetMap.value(item);
        if (pointFreqTreeWidget != nullptr)
        {
            pointFreqMap.insert(item->text(0), pointFreqTreeWidget);
        }
    }

    // 进行复制点频
    CopyPointFreqDialog dialog;
    dialog.setPointFreqMap(pointFreqMap);
    dialog.exec();
}
