#include "AddSatelliteDialog.h"

#include "ControlFactory.h"
#include "DataTransmissionCenterDefine.h"
#include "DataTransmissionCenterSerialize.h"
#include "ExtendedConfig.h"
#include "RedisHelper.h"
#include "SatelliteManagementSerialize.h"
#include "SatelliteXMLReader.h"
#include "ServiceCheck.h"
#include "StationResManagementDefine.h"
#include "StationResManagementSerialize.h"
#include "TaskCenterDefine.h"
#include "TaskCenterSerialize.h"
#include "WComboBox.h"
#include "XComboBox.h"
#include "ui_AddSatelliteDialog.h"
#include <QMessageBox>
#include <QSpinBox>
static void RefreshComboxValue(QComboBox* combobox, QVariant data)
{
    for (int i = 0; i < combobox->count(); i++)
    {
        if (combobox->itemData(i) == data)
        {
            combobox->setCurrentIndex(i);
            return;
        }
    }
}

AddSatelliteDialog::AddSatelliteDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AddSatelliteDialog)
    , m_currentProject(ExtendedConfig::curProjectID())
{
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);  // 只显示关闭按钮
    ui->setupUi(this);

    QWidget::setTabOrder(ui->satelliteCode, ui->satelliteIdentification);
    QWidget::setTabOrder(ui->satelliteIdentification, ui->satelliteName);
    QWidget::setTabOrder(ui->satelliteName, ui->desc);

    m_sczxbz = new XComboBox();
    ui->sczxbz->layout()->addWidget(m_sczxbz);
}

AddSatelliteDialog::~AddSatelliteDialog() { delete ui; }

void AddSatelliteDialog::initUI()
{
    if (m_workmodeBox == nullptr)
    {
        m_workmodeBox = new XComboBox();
    }
    m_workmodeBox->Clear();

    for (auto item : m_satelliteModeInfoList)
    {
        m_workmodeBox->AddItem(item.m_modeName, false, item.m_modeID);
    }
    connect(m_workmodeBox, &XComboBox::signalValueChanged, this, &AddSatelliteDialog::onWorkModeChanged);
    ui->workModeWidget->layout()->addWidget(m_workmodeBox);

    QString json;
    if (!RedisHelper::getInstance().getData("TaskCenter", json))
    {
        qWarning() << json;
        QMessageBox::information(this, "提示", "获取任务中心数据失败", "确定");
        return;
    }

    TaskCenterMap taskCenterResultMap;
    json >> taskCenterResultMap;
    for (auto item : taskCenterResultMap.values())
    {
        ui->taskCenterName->addItem(item.centerName, item.centerName);
    }
    ui->taskCenterName->setCurrentIndex(0);

    if (!RedisHelper::getInstance().getData("DataTransmissionCenter", json))
    {
        qWarning() << json;
        QMessageBox::information(this, "提示", "获取数传中心数据失败", "确定");
        return;
    }
    DataTransmissionCenterMap dataTransmissionCenterDataMap;
    json >> dataTransmissionCenterDataMap;

    for (auto item : dataTransmissionCenterDataMap.values())
    {
        m_sczxbz->AddItem(item.centerName, false);
    }

    if (!RedisHelper::getInstance().getData("StationResManagement", json))
    {
        qWarning() << json;
        QMessageBox::information(this, "提示", "获取测控站数据失败", "确定");
        return;
    }
    StationResManagementMap stationResResultMap;
    json >> stationResResultMap;

    for (auto item : stationResResultMap.values())
    {
        ui->ckstation->addItem(item.stationName, item.stationIdentifying);
    }
    ui->ckstation->setCurrentIndex(0);

    /* 轨道类型 */
    if (m_stationBz == TK4424)
    {
        ui->gdlx->addItem("低轨卫星", int(SatelliteRailType::LowOrbit));
        ui->gdlx->addItem("高轨卫星", int(SatelliteRailType::HighOrbit));
    }
    else
    {
        ui->gdlx->addItem("高轨卫星", int(SatelliteRailType::HighOrbit));
        ui->gdlx->addItem("低轨卫星", int(SatelliteRailType::LowOrbit));
    }

    ui->m_isCarrierType->addItem("无", int(CarrierType::Unknown));
    ui->m_isCarrierType->addItem("共载波", int(CarrierType::GZB));
    ui->m_isCarrierType->addItem("不共载波", int(CarrierType::BGZB));
    ui->m_isCarrierType->setEnabled(false);
    ui->m_isCarrierType->setVisible(false);
    ui->label_11->setVisible(false);
    /* S校相 */
    ui->sphase->addItem("是", 1);
    ui->sphase->addItem("否", 0);

    /* Ka校相 */
    ui->kaphase->addItem("是", 1);
    ui->kaphase->addItem("否", 0);

    /* 任务代号、任务标识做限制*/
    QRegExp reg(R"(^([0-9a-fA-F][\da-fA-F]*|0)$)");
    ui->satelliteIdentification->setValidator(new QRegExpValidator(reg, this));
    ui->satelliteIdentification->setMaxLength(8);

    //    QRegExp reg1("[a-zA-Z0-9]{6}");
    QRegExp reg1(R"([0-9a-zA-Z_]+)");
    ui->satelliteCode->setValidator(new QRegExpValidator(reg1, this));
    ui->satelliteCode->setMaxLength(6);

    connect(ui->satelliteIdentification, &QLineEdit::textChanged, [=](const QString& text) { ui->satelliteIdentification->setText(text.toUpper()); });
    connect(ui->satelliteCode, &QLineEdit::textChanged, [=](const QString& text) { ui->satelliteCode->setText(text.toUpper()); });
}

void AddSatelliteDialog::dpChangeValue(QString) {}

void AddSatelliteDialog::onWorkModeChanged(int idx)
{
    /* 换一下 先判断再清空,不然一出错就没数据了 */
    auto itemList = m_workmodeBox->GetSelItemsInfo();
    if (itemList.size() > m_maxWorkMode)
    {
        QMessageBox::information(nullptr, "提示", QString("工作模式组合不能超过%1个").arg(m_maxWorkMode), QString("确定"));
        m_workmodeBox->SetItemChecked(idx, false);
        return;
    }

    ui->gztx_tabwidget->clear();

    //记录更改模式前每个模式下每个点频的数据
    m_currentModeControlValueMap.clear();
    for (auto sysWorkMode : m_currentModeControlMap.keys())
    {
        auto pointControlMap = m_currentModeControlMap[sysWorkMode];
        for (auto point : pointControlMap.keys())
        {
            auto controlMap = pointControlMap[point];
            for (auto controlName : controlMap.keys())
            {
                auto wWidget = qobject_cast<WWidget*>(controlMap[controlName]);
                auto value = wWidget->value();
                m_currentModeControlValueMap[sysWorkMode][point][controlName] = value;
            }
        }
    }

    m_currentModeControlMap.clear();
    QMap<int, QTreeWidget*>::const_iterator it = m_worksystemTreeMap.begin();
    while (it != m_worksystemTreeMap.end())
    {
        delete it.value();
        ++it;
    }
    m_worksystemTreeMap.clear();

    if (m_currentProject == "4424")
    {
        QMap<int, QString> modeMap;
        QMap<int, bool> SMap;
        QMap<int, bool> KaMap;
        QMap<int, bool> KaDSMap;
        QMap<int, bool> KaGSMap;
        QMap<int, bool> XGSMap;

        for (auto item : itemList)
        {
            int workMode = (SystemWorkMode)item.userData.toInt();
            if (workMode == STTC || workMode == Skuo2 || workMode == SYTHSMJK2BGZB || workMode == SYTHSMJ || workMode == SYTHWX ||
                workMode == SYTHGML || workMode == SYTHSMJK2GZB || workMode == SYTHWXSK2 || workMode == SYTHGMLSK2 || workMode == SKT)
            {
                SMap[workMode] = true;
            }
            if (workMode == KaKuo2)
            {
                KaMap[workMode] = true;
            }
            if (workMode == KaDS)
            {
                KaDSMap[workMode] = true;
            }
            if (workMode == KaGS)
            {
                KaGSMap[workMode] = true;
            }
            if (workMode == XGS)
            {
                XGSMap[workMode] = true;
            }
        }

        if (SMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4424_S] = "S";
        }
        if (KaMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4424_KaCk] = "Ka遥测";
        }
        if (KaDSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4424_KaDSDT] = "Ka低速";
        }
        if (KaGSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4424_KaGSDT] = "X/Ka高速";
        }
        if (XGSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4424_KaGSDT] = "X/Ka高速";
        }

        if (SMap.size() > 0 && KaMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4424_S_KaCk] = "S+Ka遥测";
        }
        if (SMap.size() > 0 && KaDSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4424_S_KaDSDT] = "S+Ka低速";
        }
        if (SMap.size() > 0 && KaGSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4424_S_KaGSDT] = "S+X/Ka高速";
        }
        if (SMap.size() > 0 && XGSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4424_S_KaGSDT] = "S+X/Ka高速";
        }

        if (SMap.size() > 0 && KaMap.size() > 0 && KaDSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4424_S_KaCk_KaDSDT] = "S+Ka遥测+Ka低速";
        }
        if (SMap.size() > 0 && KaMap.size() > 0 && KaGSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4424_S_KaCK_KaGSDT] = "S+Ka遥测+X/Ka高速";
        }
        if (SMap.size() > 0 && KaMap.size() > 0 && XGSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4424_S_KaCK_KaGSDT] = "S+Ka遥测+X/Ka高速";
        }
        ui->trackMode->clear();
        for (auto type : modeMap.keys())
        {
            ui->trackMode->addItem(modeMap[type], int(type));
        }
    }
    else
    {
        QMap<int, QString> modeMap;
        QMap<int, bool> STTCMap;
        QMap<int, bool> SK2Map;
        QMap<int, bool> SKTMap;
        QMap<int, bool> KaMap;
        QMap<int, bool> KaDSMap;
        QMap<int, bool> KaGSMap;

        for (auto item : itemList)
        {
            int workMode = (SystemWorkMode)item.userData.toInt();
            if (workMode == STTC)
            {
                STTCMap[workMode] = true;
            }
            if (workMode == Skuo2)
            {
                SK2Map[workMode] = true;
            }
            if (workMode == SKT)
            {
                SKTMap[workMode] = true;
            }
            if (workMode == KaKuo2)
            {
                KaMap[workMode] = true;
            }
            if (workMode == KaDS)
            {
                KaDSMap[workMode] = true;
            }
            if (workMode == KaGS)
            {
                KaGSMap[workMode] = true;
            }
        }

        if (STTCMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_STTC] = "STTC";
        }
        if (SK2Map.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_SK2] = "S扩二";
        }
        if (SKTMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_SKT] = "S扩跳";
        }
        if (KaMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_KaCk] = "Ka遥测";
        }
        if (KaDSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_KaDSDT] = "Ka低速数传";
        }
        if (KaGSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_KaGSDT] = "Ka高速数传";
        }

        if (STTCMap.size() > 0 && KaMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_STTC_KaCk] = "STTC+Ka遥测";
        }
        if (STTCMap.size() > 0 && KaDSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_STTC_KaDSDT] = "STTC+Ka低速数传";
        }
        if (STTCMap.size() > 0 && KaGSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_STTC_KaGSDT] = "STTC+Ka高速数传";
        }
        if (STTCMap.size() > 0 && KaMap.size() > 0 && KaDSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_STTC_KaCk_KaDSDT] = "STTC+Ka遥测+Ka低速数传";
        }
        if (STTCMap.size() > 0 && KaMap.size() > 0 && KaGSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_STTC_KaCK_KaGSDT] = "STTC+Ka遥测+Ka高速数传";
        }

        if (SK2Map.size() > 0 && KaMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_SK2_KaCk] = "S扩二+Ka遥测";
        }
        if (SK2Map.size() > 0 && KaDSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_SK2_KaDSDT] = "S扩二+Ka低速数传";
        }
        if (SK2Map.size() > 0 && KaGSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_SK2_KaGSDT] = "S扩二+Ka高速数传";
        }
        if (SK2Map.size() > 0 && KaMap.size() > 0 && KaDSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_SK2_KaCk_KaDSDT] = "S扩二+Ka遥测+Ka低速数传";
        }
        if (SK2Map.size() > 0 && KaMap.size() > 0 && KaGSMap.size() > 0)
        {
            modeMap[SatelliteTrackingMode::_4426_SK2_KaCK_KaGSDT] = "S扩二+Ka遥测+Ka高速数传";
        }

        // 截止20211023 扩跳不能快速校相
        // if (SKTMap.size() > 0 && KaMap.size() > 0)
        // {
        //     modeMap[SatelliteTrackingMode::_4426_SKT_KaCk] = "S扩跳+Ka遥测";
        // }
        // if (SKTMap.size() > 0 && KaDSMap.size() > 0)
        // {
        //     modeMap[SatelliteTrackingMode::_4426_SKT_KaDSDT] = "S扩跳+Ka低速数传";
        // }
        // if (SKTMap.size() > 0 && KaGSMap.size() > 0)
        // {
        //     modeMap[SatelliteTrackingMode::_4426_SKT_KaGSDT] = "S扩跳+Ka高速数传";
        // }
        // if (SKTMap.size() > 0 && KaMap.size() > 0 && KaDSMap.size() > 0)
        // {
        //     modeMap[SatelliteTrackingMode::_4426_SKT_KaCk_KaDSDT] = "S扩跳+Ka遥测+Ka低速数传";
        // }
        // if (SKTMap.size() > 0 && KaMap.size() > 0 && KaGSMap.size() > 0)
        // {
        //     modeMap[SatelliteTrackingMode::_4426_SKT_KaCK_KaGSDT] = "S扩跳+Ka遥测+Ka高速数传";
        // }
        ui->trackMode->clear();
        for (auto type : modeMap.keys())
        {
            ui->trackMode->addItem(modeMap[type], int(type));
        }
    }

    for (auto item : itemList)
    {
        auto workMode = (SystemWorkMode)item.userData.toInt();
        QString worksystem = m_workmodeBox->GetItemTextByData(workMode);

        QTreeWidget* tree = new QTreeWidget();
        tree->setColumnCount(3);
        tree->setColumnWidth(0, 200);
        tree->setColumnWidth(1, 200);
        tree->setHeaderLabels(QStringList() << QString("名称") << QString("值") << QString("描述"));

        QTreeWidgetItem* dpItem = new QTreeWidgetItem;
        dpItem->setText(0, "点频数量");
        QSpinBox* spinbox = new QSpinBox;
        spinbox->setMaximum(10);
        spinbox->setMinimum(1);
        tree->addTopLevelItem(dpItem);  //添加分组节点
        tree->setItemWidget(dpItem, 1, spinbox);

        QTreeWidgetItem* defaultDpItem = new QTreeWidgetItem;
        defaultDpItem->setText(0, "默认点频");
        QComboBox* combobox = new QComboBox;

        tree->addTopLevelItem(defaultDpItem);  //添加分组节点
        tree->setItemWidget(defaultDpItem, 1, combobox);
        m_defaultDpControlMap[workMode] = combobox;

        //初始化创建1个点频
        //        connect(spinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
        //            [=](int i){ /* ... */ });

        connect(spinbox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int i) {
            combobox->clear();
            for (auto ii = 0; ii < i; ii++)
            {
                combobox->addItem(QString("点频%1").arg(ii + 1), ii + 1);
            }
            //这里是更改点频时的处理赋值，上面是更改工作模式时的赋值
            //            if (!m_currentModeControlMap.isEmpty())
            //            {
            //                m_currentModeControlValueMap.clear();
            //                for (auto sysWorkMode : m_currentModeControlMap.keys())
            //                {
            //                    auto pointControlMap = m_currentModeControlMap[sysWorkMode];
            //                    for (auto point : pointControlMap.keys())
            //                    {
            //                        auto controlMap = pointControlMap[point];
            //                        for (auto controlName : controlMap.keys())
            //                        {
            //                            auto wWidget = qobject_cast<WWidget*>(controlMap[controlName]);
            //                            auto value = wWidget->value();
            //                            m_currentModeControlValueMap[sysWorkMode][point][controlName] = value;
            //                        }
            //                    }
            //                }
            //            }
            m_currentModeControlMap[workMode].clear();
            while (defaultDpItem->childCount() > 0)
            {
                defaultDpItem->removeChild(defaultDpItem->child(0));
            }
            //            auto senderWidget = qobject_cast<QSpinBox*>(sender());
            //            if (!senderWidget)
            //            {
            //                return;
            //            }

            auto size = i;
            for (int dpNum = 1; dpNum <= size; dpNum++)
            {
                for (auto satellite_item : m_satelliteModeInfoList)
                {
                    if (satellite_item.m_modeID == workMode)
                    {
                        QTreeWidgetItem* subDpItem = new QTreeWidgetItem;
                        subDpItem->setText(0, QString("点频%1").arg(dpNum));
                        for (auto groupNodeItem : satellite_item.m_modeParamList)
                        {
                            for (auto subNodeItem : groupNodeItem.m_paramList)
                            {
                                QTreeWidgetItem* attrItem = new QTreeWidgetItem;

                                //更新界面的枚举值
                                QMap<QString, QVariant> enumMap;
                                if (subNodeItem.displayFormat == DisplayFormat_Enum)
                                {
                                    if (satellite_item.m_enumMap.contains(subNodeItem.enumType))
                                    {
                                        auto paramEnumList = satellite_item.m_enumMap.value(subNodeItem.enumType);
                                        for (auto paramEnum : paramEnumList.emumEntryList)
                                        {
                                            enumMap[paramEnum.desc] = paramEnum.uValue;
                                        }
                                    }
                                }

                                auto control = ControlFactory::createSetControl(subNodeItem, enumMap);
                                if (control)
                                {
                                    subDpItem->addChild(attrItem);
                                    // groupItem->addChild(attrItem);
                                    tree->setItemWidget(attrItem, 0, control);
                                    control->setObjectName(QString::number(groupNodeItem.m_unitID) + "_" + subNodeItem.id);
                                    //保存当前模式的工作模式的控件
                                    m_currentModeControlMap[workMode][dpNum][QString::number(groupNodeItem.m_unitID) + "_" + subNodeItem.id] =
                                        control;
                                    if (enumMap.size() > 0 && qobject_cast<WComboBox*>(control))
                                    {
                                        qobject_cast<WComboBox*>(control)->resetValue();
                                    }

                                    if (m_currentModeControlValueMap.contains(workMode) && m_currentModeControlValueMap[workMode].contains(dpNum))
                                    {
                                        auto value = m_currentModeControlValueMap[workMode][dpNum]
                                                                                 [QString::number(groupNodeItem.m_unitID) + "_" + subNodeItem.id];
                                        control->setValue(value);
                                    }
                                }
                            }
                            // subDpItem->addChild(groupItem);
                        }
                        defaultDpItem->addChild(subDpItem);
                    }
                }
            }
            tree->expandAll();
        });
        if ((m_currentUIMode == STATUS::SHOW || m_currentUIMode == STATUS::EDIT) && m_workModeMap.contains(workMode))
        {
            if (m_workModeMap[workMode] == 1)
            {
                emit spinbox->valueChanged(1);
            }
            else
            {
                spinbox->setValue(m_workModeMap[workMode]);
            }
        }
        else
        {
            emit spinbox->valueChanged(1);
        }
        tree->expandAll();
        m_worksystemTreeMap.insert(workMode, tree);
        ui->gztx_tabwidget->addTab(tree, worksystem);
    }

    //如果包含扩二和一体化模式就可以设置是否共载波控件
    QMap<SystemWorkMode, int> m_cpMap;
    for (auto item : itemList)
    {
        auto workMode = (SystemWorkMode)item.userData.toInt();
        m_cpMap[workMode] = 1;
    }
    if (m_cpMap.contains(Skuo2) && m_cpMap.contains(SYTHSMJ))
    {
        ui->m_isCarrierType->setEnabled(true);
        ui->m_isCarrierType->setVisible(true);
        ui->label_11->setVisible(true);
    }
    else
    {
        RefreshComboxValue(ui->m_isCarrierType, int(CarrierType::Unknown));
        ui->m_isCarrierType->setEnabled(false);
        ui->m_isCarrierType->setVisible(false);
        ui->label_11->setVisible(false);
    }
    if (m_currentUIMode == STATUS::SHOW)
    {
        ui->m_isCarrierType->setEnabled(false);
    }
}

void AddSatelliteDialog::setCurrentUIMode(const STATUS& currentUIMode)
{
    m_currentUIMode = currentUIMode;
    if (m_currentUIMode == STATUS::ADD)
    {
        setWindowTitle("添加卫星数据");
    }
    else if (m_currentUIMode == STATUS::EDIT)
    {
        setWindowTitle("编辑卫星数据");
    }
    else if (m_currentUIMode == STATUS::SHOW)
    {
        setWindowTitle("查看卫星数据");
    }
}

SatelliteManagementData AddSatelliteDialog::currentsatelliteData() const { return m_currentSalliteData; }

void AddSatelliteDialog::setSatelliteDataList(const QList<SalliteModeInfo>& salliteModeInfoList) { m_satelliteModeInfoList = salliteModeInfoList; }

void AddSatelliteDialog::updateUI(SatelliteManagementData request)
{
    if (m_currentUIMode == EDIT)
    {
        ui->satelliteCode->setEnabled(false);
    }
    if (m_currentUIMode == SHOW)
    {
        m_workmodeBox->setEnabled(false);
        ui->satelliteName->setEnabled(false);
        ui->satelliteCode->setEnabled(false);
        ui->satelliteIdentification->setEnabled(false);
        ui->ckstation->setEnabled(false);
        ui->taskCenterName->setEnabled(false);
        ui->sczxbz->setEnabled(false);
        ui->gdlx->setEnabled(false);
        ui->trackMode->setEnabled(false);
        ui->sphase->setEnabled(false);
        ui->kaphase->setEnabled(false);
        ui->desc->setEnabled(false);
        //        ui->gztx_tabwidget->setEnabled(false);
        for (auto i = 0; i < ui->gztx_tabwidget->count(); i++)
        {
            ui->gztx_tabwidget->widget(i)->setEnabled(false);
        }
        ui->save->setEnabled(false);
    }

    ui->satelliteName->setText(request.m_satelliteName);
    ui->satelliteCode->setText(request.m_satelliteCode);

    ui->satelliteIdentification->setText(request.m_satelliteIdentification);
    RefreshComboxValue(ui->ckstation, request.m_stationID);
    RefreshComboxValue(ui->taskCenterName, request.m_taskCenterID);

    auto itemList = request.m_digitalCenterID.split("+");
    for (auto item : itemList)
    {
        m_sczxbz->SetItemChecked(item, true);
    }

    // RefreshComboxValue(ui->dataTransmissionCenterName, request.m_digitalCenterID);
    RefreshComboxValue(ui->sphase, request.m_sPhasing);
    RefreshComboxValue(ui->kaphase, request.m_kaPhasing);

    ui->desc->setPlainText(request.m_desc);
    for (auto item : request.workMode())
    {
        auto workMode = (SystemWorkMode)item.toInt();
        if (request.m_workModeParamMap.contains(workMode))
        {
            m_workModeMap[workMode] = request.m_workModeParamMap[workMode].m_dpInfoMap.size();
        }
    }
    for (auto item : request.workMode())
    {
        m_workmodeBox->SetItemCheckByData(item.toInt(), true);
    }

    //更新控件的值
    for (auto workMode : request.m_workModeParamMap.keys())
    {
        //更新默认点频的配置
        if (request.m_workModeParamMap.contains(workMode) && m_defaultDpControlMap.contains(workMode))
        {
            auto combox = m_defaultDpControlMap.value(workMode);
            auto val = request.m_workModeParamMap.value(workMode).defalutNum;
            if (combox)
            {
                RefreshComboxValue(combox, val);
            }
        }
        for (auto dpNum : m_currentModeControlMap[workMode].keys())
        {
            for (auto control : m_currentModeControlMap[workMode][dpNum].values())
            {
                auto objName = control->objectName();
                if (request.m_workModeParamMap[workMode].m_dpInfoMap[dpNum].m_itemMap.contains(objName))
                {
                    auto widget = qobject_cast<WWidget*>(control);
                    auto val = request.m_workModeParamMap[workMode].m_dpInfoMap[dpNum].m_itemMap[objName];
                    if (widget)
                    {
                        widget->setValue(val);
                    }
                }
            }
        }
    }

    RefreshComboxValue(ui->trackMode, uint(request.m_trackingMode));
    RefreshComboxValue(ui->gdlx, uint(request.m_railType));

    //刷新是否共载波的值
    RefreshComboxValue(ui->m_isCarrierType, uint(request.m_isCarrierType));
}

void AddSatelliteDialog::addConfigMacroDialogSuccess() {}

void AddSatelliteDialog::slotAddSatelliteResponce(bool flag, SatelliteManagementData request)
{
    Q_UNUSED(request);
    if (flag)
    {
        QDialog::accept();
    }
    else
    {
        QMessageBox::information(nullptr, "提示", "添加卫星信息失败", "确定");
    }
}

void AddSatelliteDialog::slotEditSatelliteResponce(bool flag, SatelliteManagementData request)
{
    Q_UNUSED(request);
    if (flag)
    {
        QDialog::accept();
    }
    else
    {
        QMessageBox::information(nullptr, "提示", "编辑卫星信息失败", "确定");
    }
}

void AddSatelliteDialog::on_save_clicked()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    int count = ui->gztx_tabwidget->count();
    if (count <= 0)
    {
        QMessageBox::information(this, "提示", "请至少添加一个工作体制", QString("确定"));
        return;
    }
    if (ui->satelliteCode->text().isEmpty())
    {
        QMessageBox::information(this, "提示", "任务代号不能为空", QString("确定"));
        return;
    }

    if (ui->satelliteIdentification->text().isEmpty())
    {
        QMessageBox::information(this, "提示", "卫星标识不能为空", QString("确定"));
        return;
    }
    if (ui->satelliteName->text().isEmpty())
    {
        QMessageBox::information(this, "提示", "卫星名称不能为空", QString("确定"));
        return;
    }

    SatelliteManagementData request;

    request.m_satelliteName = ui->satelliteName->text();  //卫星名称
    request.m_satelliteCode = ui->satelliteCode->text();  //任务代号
    request.m_satelliteIdentification = ui->satelliteIdentification->text();
    request.m_stationID = ui->ckstation->currentData().toString();
    request.m_taskCenterID = ui->taskCenterName->currentData().toString();

    request.m_digitalCenterID = m_sczxbz->GetSelItemsText().join("+");  //数传中心标识 name

    if (ui->taskCenterName->currentData().toString().isEmpty())
    {
        QMessageBox::information(this, "提示", "任务中心不能为空", QString("确定"));
        return;
    }
    if (!m_sczxbz->GetSelItemsText().size())
    {
        QMessageBox::information(this, QString("提示"), QString("数传中心标识为空，请去数传中心下添加数据后\n再进行卫星管理数据的添加"),
                                 QString("确定"));
        return;
    }
    if (m_currentUIMode == STATUS::ADD)
    {
        SatelliteManagementDataList satelliteDataList;
        if (!GlobalData::getSatelliteManagementData(satelliteDataList))
        {
            QMessageBox::information(this, "提示", "获取卫星数据失败", QString("确定"));
            return;
        }
        for (auto item : satelliteDataList)
        {
            if (item.m_satelliteCode == request.m_satelliteCode)
            {
                QMessageBox::information(this, "提示", "当前任务代号已存在", QString("确定"));
                return;
            }
        }
    }

    request.m_railType = SatelliteRailType(ui->gdlx->currentData().toUInt());
    request.m_trackingMode = SatelliteTrackingMode(ui->trackMode->currentData().toInt());
    request.m_sPhasing = ui->sphase->currentData().toBool();
    request.m_kaPhasing = ui->kaphase->currentData().toBool();
    auto workModeList = m_workmodeBox->GetSelItemsInfo();
    QStringList modelist;
    for (auto item : workModeList)
    {
        modelist.append(item.userData.toString());
    }
    request.m_workMode = modelist.join("+");

    for (auto key : m_currentModeControlMap.keys())
    {
        for (auto dpNum : m_currentModeControlMap[key].keys())
        {
            auto valueMap = m_currentModeControlMap[key].value(dpNum);
            SatelliteManagementParamItem satelliteItem;
            for (auto itemKey : valueMap.keys())
            {
                auto control = qobject_cast<WWidget*>(valueMap.value(itemKey));
                if (control)
                {
                    bool flag = true;
                    auto value = control->value(flag);

                    if (!flag)
                    {
                        QMessageBox::information(this, "提示", "参数数据非法,参数名称：" + control->paramAttr.desc);
                        return;
                    }
                    else
                    {
                        satelliteItem.m_itemMap[itemKey] = value;
                    }
                }
            }
            request.m_workModeParamMap[key].m_dpInfoMap[dpNum] = satelliteItem;
        }
    }
    //放后面才能得到默认点频数据
    for (auto key : m_defaultDpControlMap.keys())
    {
        auto combox = m_defaultDpControlMap[key];
        if (combox)
        {
            //防止选中了工作体制后再取消,导致程序奔溃的bug
            if (request.m_workModeParamMap.contains(key))
                request.m_workModeParamMap[key].defalutNum = combox->currentData().toInt();
        }
    }

    request.m_desc = ui->desc->toPlainText();
    request.m_isCarrierType = (CarrierType)ui->m_isCarrierType->currentData().toInt();
    QByteArray json;
    json << request;

    qWarning() << json;
    if (m_currentUIMode == STATUS::ADD)
    {
        if (!(QMessageBox::information(this, "提示", "是否添加当前选择的卫星", "确定", "取消")))
        {
            emit signalAddSatellite(json);
        }
    }
    else if (m_currentUIMode == STATUS::EDIT)
    {
        if (!(QMessageBox::information(this, "提示", "是否保存当前选择的卫星", "确定", "取消")))
        {
            emit signalEditSatellite(json);
        }
    }

    m_currentSalliteData = request;
}
void AddSatelliteDialog::on_cancel_clicked() { QDialog::close(); }

void AddSatelliteDialog::setMaxWorkMode(int maxWorkMode) { m_maxWorkMode = maxWorkMode; }

void AddSatelliteDialog::on_desc_textChanged()
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

void AddSatelliteDialog::setStationBz(int stationBz) { m_stationBz = stationBz; /* 轨道类型 */ }
