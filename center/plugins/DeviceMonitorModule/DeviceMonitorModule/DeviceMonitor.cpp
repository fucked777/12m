#include "DeviceMonitor.h"
#include "ControlFactory.h"
#include "DevProtocolSerialize.h"
#include "DeviceAutoMapDealWith.h"
#include "DeviceProcessMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "QFlowLayout.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "WMatrixWidget.h"
#include "WNumber.h"
#include "WPlainTextEdit.h"
#include "ui_DeviceMonitor.h"

#include <QComboBox>
#include <QMessageBox>

DeviceMonitor::DeviceMonitor(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DeviceMonitor)
{
    ui->setupUi(this);

    startTimer(1000);

    connect(ui->unitTabWidget, SIGNAL(currentChanged(int)), this, SLOT(slot_unitChange(int)));
    ui->macroBtn->hide();
    ui->isOnlineLabel->setStyleSheet("font-size:30px;");

    //初始化qss
    QString fileName = "DeviceMonitor.qss";
    QssCommonHelper::setWidgetStyle(this, fileName);

    m_currentProject = ExtendedConfig::curProjectID();
}

DeviceMonitor::~DeviceMonitor() { delete ui; }

void DeviceMonitor::setDeviceID(DeviceID deviceID) { m_currentDeviceID = deviceID; }

const DeviceID& DeviceMonitor::getDeviceID() const { return m_currentDeviceID; }

void DeviceMonitor::setModeCtrl(ModeCtrl mode)
{
    m_currentMode = mode;
    if (m_currentMode.desc.trimmed() == "")
    {
        ui->modesLabel->setText(QString("无模式"));
    }
    else
    {
        ui->modesLabel->setText(m_currentMode.desc);
    }
}

ModeCtrl DeviceMonitor::getModeCtrl() const { return m_currentMode; }

QMap<int, QWidget*> DeviceMonitor::getUnitIDWidgetMap() { return m_unitIDWidgetMap; }

QMap<int, QWidget*> DeviceMonitor::getCmdIDWidgetMap() { return m_cmdIDWidgetMap; }
QMap<int, QMap<QString, WWidget*>> DeviceMonitor::getAllWidgetMap() { return m_widgetMap; }
QMap<QString, WWidget*> DeviceMonitor::getUnitWidgetMap(int unitID)
{
    auto find = m_widgetMap.find(unitID);
    return find == m_widgetMap.end() ? QMap<QString, WWidget*>() : find.value();
}
int DeviceMonitor::currentModeID() { return m_currentMode.ID; }

void DeviceMonitor::initWidget()
{
    m_deviceID = createDeviceID(m_currentDeviceID.sysID, m_currentDeviceID.devID, m_currentDeviceID.extenID);
    m_widgetDeal.setMode(m_currentMode);
    m_widgetDeal.setDeviceID(m_currentDeviceID);
    for (auto unitID : m_currentMode.unitMap.keys())
    {
        Unit unit = m_currentMode.unitMap[unitID];

        if (!unit.isShow)
        {
            continue;
        }

        //判断是否配置了布局
        auto haveLayout = false;
        haveLayout = unit.unitLayoutMap.isEmpty() ? false : true;
        QWidget* currentWidget = new QWidget;
        QVBoxLayout* currentLayout = new QVBoxLayout;

        QWidget* statusWidget = nullptr;
        QWidget* setsWidget = nullptr;

        if (haveLayout)  //有分组
        {
            //添加状态面板
            statusWidget = initUnitStatusGroupWidget(unit);
            if (statusWidget != nullptr)
            {
                currentLayout->addWidget(statusWidget);
            }

            //存储转发多中心数界面
            if (m_currentDeviceID.sysID == 8 && m_currentDeviceID.devID == 1 && unit.unitType == "FU")
            {
                auto centerStatusMultiWidget = m_widgetDeal.initDataStatusMultiWidget(unit, 1);
                if (centerStatusMultiWidget)
                    currentLayout->addWidget(centerStatusMultiWidget);
            }

            //添加设置面板
            setsWidget = initUnitSetsGroupWidget(unit, unitID);
            if (setsWidget != nullptr)
            {
                currentLayout->addWidget(setsWidget);
            }
        }
        else  //无分组
        {
            //添加状态面板
            statusWidget = initUnitStatusWidget(unit);
            if (statusWidget != nullptr)
            {
                currentLayout->addWidget(statusWidget);
            }

            //存储转发多中心数界面
            if (m_currentDeviceID.sysID == 8 && m_currentDeviceID.devID == 1 && unit.unitType == "FU")
            {
                auto centerStatusMultiWidget = m_widgetDeal.initDataStatusMultiWidget(unit, 1);
                if (centerStatusMultiWidget)
                    currentLayout->addWidget(centerStatusMultiWidget);
            }

            //添加设置面板
            setsWidget = initUnitSetsWidget(unit, unitID);
            if (setsWidget != nullptr)
            {
                currentLayout->addWidget(setsWidget);
            }
        }

        //添加两个按钮
        QPushButton* cmdUpdateCmdBtn = new QPushButton("修改");
        QPushButton* cmdSettingCmdBtn = new QPushButton("设置");

        QPushButton* autoMapBtn = nullptr;
        if (unit.isAutoMap)
        {
            autoMapBtn = new QPushButton("自动映射");
            autoMapBtn->setEnabled(false);

            connect(autoMapBtn, &QPushButton::clicked, this, [=]() {
                SERVICEONLINECHECK();  //判断服务器是否离线

                QString errorMsg;
                if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
                {
                    QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
                    return;
                }

                cmdUpdateCmdBtn->setText(QString("修改"));
                cmdUpdateCmdBtn->setEnabled(true);
                cmdSettingCmdBtn->setEnabled(false);
                autoMapBtn->setEnabled(false);
                //设置面板不可用
                setSettingPanel(false, setsWidget);

                packAutoMapUnitParamSet(unitID, unit, setsWidget);
            });
        }

        cmdSettingCmdBtn->setEnabled(false);
        m_unitChangeStatus[unitID] = true;

        m_changeBtnMap[unitID] = cmdUpdateCmdBtn;
        m_settingBtnMap[unitID] = cmdSettingCmdBtn;

        connect(cmdUpdateCmdBtn, &QPushButton::clicked, this, [=]() {
            for (QWidget* control : m_needUpdateColorControlCommon.keys())
            {
                if (control)
                {
                    control->setStyleSheet("");  // 清空样式
                }
            }
            for (auto recycle : m_needUpdateColorControlMulti.keys())
            {
                auto controlMap = m_needUpdateColorControlMulti[recycle];
                for (auto controlMulti : controlMap.keys())
                {
                    if (controlMulti)
                    {
                        controlMulti->setStyleSheet("");  // 清空样式
                    }
                }
            }
            m_needUpdateColorControlCommon.clear();
            m_needUpdateColorControlMulti.clear();
            m_commonMapBefore.clear();
            m_commonMapAfter.clear();
            m_multiMapBefore.clear();
            m_multiMapAfter.clear();

            if (cmdUpdateCmdBtn->text() == "修改")
            {
                cmdUpdateCmdBtn->setText(QString("取消"));
                cmdSettingCmdBtn->setEnabled(true);

                if (autoMapBtn)
                    autoMapBtn->setEnabled(true);

                //设置面板可用
                setSettingPanel(true, setsWidget);

                m_unitChangeStatus[unitID] = false;

                m_click = true;

                getSetParaAllMap(currentWidget);  //修改的时候获取界面上值
            }
            else
            {
                cmdUpdateCmdBtn->setText(QString("修改"));
                cmdSettingCmdBtn->setEnabled(false);

                if (autoMapBtn)
                    autoMapBtn->setEnabled(false);

                //设置面板不可用
                setSettingPanel(false, setsWidget);

                m_unitChangeStatus[unitID] = true;

                m_click = false;
            }
        });

        connect(cmdSettingCmdBtn, &QPushButton::clicked, this, [=]() {
            SERVICEONLINECHECK();  //判断服务器是否离线

            QString errorMsg;
            if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
            {
                QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
                return;
            }

            auto packResult = packUnitParamSet(currentWidget, unitID);
            //正常的下发失败和成功才会进入这里  范围和关联关系的失败就不进入，保持现状，直到值对了才进入这里
            if (packResult == PackResult::Failed || packResult == PackResult::Success)
            {
                cmdUpdateCmdBtn->setText(QString("修改"));
                cmdUpdateCmdBtn->setEnabled(true);
                cmdSettingCmdBtn->setEnabled(false);
                if (autoMapBtn)
                    autoMapBtn->setEnabled(false);
                //设置面板不可用
                setSettingPanel(false, setsWidget);

                m_click = false;

                getSetParaAllMap(currentWidget);  //设置之后获取界面上的值

                comPareSetParaValue();  //然后比较修改前及修改后的值

                m_unitChangeStatus[unitID] = true;
            }
        });

        //初始化设置面板不可用
        setSettingPanel(false, setsWidget);
        //有些单元不需要修改设置按钮  做一个判断
        if (!unit.isModifySetBtn)
        {
            cmdUpdateCmdBtn->hide();
            cmdSettingCmdBtn->hide();
        }
        else
        {
            auto btnLayout = new QHBoxLayout();
            btnLayout->setSpacing(6);
            btnLayout->setContentsMargins(0, 0, 0, 0);
            auto spacerItem = new QSpacerItem(197, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
            btnLayout->addItem(spacerItem);
            if (autoMapBtn)
                btnLayout->addWidget(autoMapBtn);
            btnLayout->addWidget(cmdUpdateCmdBtn);
            btnLayout->addWidget(cmdSettingCmdBtn);
            currentLayout->addLayout(btnLayout);
        }

        currentWidget->setLayout(currentLayout);

        QScrollArea* scrollArea = new QScrollArea;
        scrollArea->setWidget(currentWidget);
        scrollArea->setWidgetResizable(true);
        m_unitWidgetIDMap[scrollArea] = unitID;
        m_unitIDWidgetMap[unitID] = scrollArea;

        ui->unitTabWidget->insertTab(unitID, scrollArea, unit.desc);
    }
    ui->stackedWidget->setCurrentWidget(ui->unitPages);  //默认显示单元界面
}

QWidget* DeviceMonitor::initUnitStatusWidget(Unit& unit)
{
    QGroupBox* groupBox = new QGroupBox(STATUS_TITLE);
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    bool haveControlCommon = false;  // 如果没有公共参数就不显示公共参数的groupBox
    bool haveControl = false;        //如果没有多目标或者一体化参数就不显示对应的groupBox
    /* 下面是公共单元的参数
     * **********************************************************/
    for (auto groupKey : unit.statusGroupMap.keys())
    {
        QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
        auto commonAttrList = unit.statusGroupMap[groupKey];
        for (auto attr : commonAttrList)
        {
            if (attr.isShow)
            {
                QMap<QString, QVariant> enumMap;
                if (attr.displayFormat == DisplayFormat_Enum)
                {
                    m_widgetDeal.getEnumData(attr.enumType, unit, enumMap);
                }
                else if (attr.displayFormat == DisplayFormat_LED)
                {
                    m_widgetDeal.getEnumData(attr.enumType, unit, enumMap);
                }
                auto control = ControlFactory::createStatusControl(attr, enumMap);
                if (control)
                {
                    commonLayout->addWidget(control);
                    haveControlCommon = true;
                }
            }
        }
        vboxLayout->addItem(commonLayout);
    }

    // 创建LED矩阵
    QWidget* widget = createMatrix(unit);
    if (widget)
    {
        vboxLayout->addWidget(widget);
        vboxLayout->addStretch(1);
    }

    /********多目标********/
    QGroupBox* multiBox = new QGroupBox(MULTI_TARGET);
    QTabWidget* multiTabWidget = new QTabWidget;
    QHBoxLayout* hboxLayout = new QHBoxLayout;
    for (auto targetKey : unit.multiStatusGroupMap.keys())
    {
        auto multiAttrMap = unit.multiStatusGroupMap[targetKey];
        auto recycleNum = unit.targetData[targetKey].recycleNum.toInt();
        auto desc = unit.targetData[targetKey].desc;

        if (unit.targetData[targetKey].isShow)
        {
            for (auto i = 1; i <= recycleNum; ++i)
            {
                QWidget* multiWidget = new QWidget;
                QFlowLayout* multiLayout = new QFlowLayout(6, 0, 10);

                for (auto groupKey : multiAttrMap.keys())
                {
                    auto multiAttrLists = multiAttrMap[groupKey];
                    for (auto attr : multiAttrLists)
                    {
                        if (attr.isShow)
                        {
                            QMap<QString, QVariant> enumMap;
                            if (attr.displayFormat == DisplayFormat_Enum)
                            {
                                m_widgetDeal.getEnumData(attr.enumType, unit, enumMap);
                            }
                            else if (attr.displayFormat == DisplayFormat_LED)
                            {
                                m_widgetDeal.getEnumData(attr.enumType, unit, enumMap);
                            }
                            auto control = ControlFactory::createStatusControl(attr, enumMap);
                            if (control)
                            {
                                multiLayout->addWidget(control);
                                haveControl = true;
                            }
                        }
                    }
                }
                multiWidget->setLayout(multiLayout);
                multiWidget->setProperty("TargetKey", targetKey + i - 1);
                if (recycleNum == 1)
                {
                    multiTabWidget->addTab(multiWidget, QString("%1").arg(desc));
                }
                else
                {
                    multiTabWidget->addTab(multiWidget, QString("目标%1").arg(i));
                }
            }
        }
    }

    if (multiTabWidget->count() > 0)
    {
        hboxLayout->addWidget(multiTabWidget);
        multiBox->setLayout(hboxLayout);
        vboxLayout->addWidget(multiBox);
    }

    groupBox->setLayout(vboxLayout);

    //如果没有控件就删除界面，不创建
    if (!haveControl && !haveControlCommon)
    {
        delete groupBox;
        groupBox = nullptr;
    }

    return groupBox;
}

QWidget* DeviceMonitor::initUnitSetsWidget(Unit& unit, int unitID)
{
    QGroupBox* groupBox = new QGroupBox(SET_TITLE);

    QVBoxLayout* vboxLayout = new QVBoxLayout();

    bool haveControlCommon = false;  // 如果没有公共参数就不显示公共参数的groupBox
    bool haveControl = false;        //如果没有多目标或者一体化参数就不显示对应的groupBox

    if (!unit.kpValid.isEmpty())
    {
        m_widgetDeal.setTargetID(unit.kpValid);
    }

    /* 下面是公共单元的参数
     * **********************************************************/
    for (auto groupKey : unit.settingGroupMap.keys())
    {
        QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
        auto commonAttrList = unit.settingGroupMap[groupKey];

        for (auto attr : commonAttrList)
        {
            if (attr.isShow)
            {
                WWidget* control = nullptr;
                if (attr.displayFormat == DisplayFormat_Enum)
                {
                    QMap<int, QPair<QString, QVariant>> enumMap;
                    m_widgetDeal.getEnumDataOrder(attr.enumType, unit, enumMap);
                    control = ControlFactory::createSetControlOrder(attr, enumMap);
                }
                else if (attr.displayFormat == DisplayFormat_LED)
                {
                    QMap<int, QPair<QString, QVariant>> enumMap;
                    m_widgetDeal.getEnumDataOrder(attr.enumType, unit, enumMap);
                    control = ControlFactory::createSetControlOrder(attr, enumMap);
                }
                else
                {
                    QMap<QString, QVariant> enumMap;
                    control = ControlFactory::createSetControl(attr, enumMap);
                }

                if (!attr.isEnable)
                {
                    control->setEnabled(false);
                }

                if (control)
                {
                    commonLayout->addWidget(control);
                    haveControlCommon = true;

                    if (control->objectName() == unit.kpValid)  //存储能控制多目标数量的控件
                    {
                        m_targetNumControlList.append(control);
                    }
                    m_widgetMap[unit.unitCode][attr.id] = control;  //保存所有控件

                    connect(control, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this, [=](const QVariant& value) {
                        auto objName = control->objectName();
                        m_widgetDeal.dealWidgetRelation(unitID, objName, value, groupBox);
                        if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 4 && m_currentProject == "4426")
                        {
                            dealWithDSMSLNumRange(unitID, objName);  //处理低速基带解调和模拟源单元码速率控件的参数范围
                        }
                        else if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && (m_currentMode.ID == 4 || m_currentMode.ID == 5) &&
                                 objName == unit.kpValid)  //单目标的时候 接收通道有三个选项
                        {
                            auto controlMap = getDeviceMultiWidgetParamControl(1, "SReceivPolar");
                            m_widgetDeal.dealWidgetControlMultiRelation(0, "SReceivPolar", value, controlMap);
                        }
                        else if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && m_currentMode.ID == 1 && unitID == 7)
                        {
                            //测控基带标准TTC遥控单元码速率范围动态更改
                            dealWithCKTTCRcuNumRange(unitID, objName);
                        }
                        //跟踪基带标准+低速  扩频+低速码速率关联关系
                        else if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 2 && (m_currentMode.ID == 10 || m_currentMode.ID == 11))
                        {
                            dealWithGZMSLNumRange(unitID, objName);
                        }
                    });
                }
            }
        }

        vboxLayout->addItem(commonLayout);
    }

    if (!unit.settingGroupMap.isEmpty())
    {
        //这个判断就能判定这个设备必定是4424的测控基带的一体化模式   测控基带只有一体化才不需要初始化时的修改设置按钮
        if (m_currentProject == "4424" && m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && !unit.isModifySetBtn)
        {
            auto btnLayout = getCommonYTHAndKPBtnLayout(unitID, unit, groupBox);
            vboxLayout->addLayout(btnLayout);
        }
    }

    /********多目标********/
    QGroupBox* multiBox = new QGroupBox(MULTI_TARGET);
    QTabWidget* multiTabWidget = new QTabWidget;
    QHBoxLayout* hboxLayout = new QHBoxLayout;
    for (auto targetKey : unit.multiSettingGroupMap.keys())
    {
        auto multiAttrMap = unit.multiSettingGroupMap[targetKey];
        auto recycleNum = unit.targetData[targetKey].recycleNum.toInt();
        auto desc = unit.targetData[targetKey].desc;

        if (unit.targetData[targetKey].isShow)
        {
            for (auto i = 1; i <= recycleNum; ++i)
            {
                QWidget* multiWidget = new QWidget;
                QFlowLayout* multiLayout = new QFlowLayout(6, 0, 10);

                for (auto groupKey : multiAttrMap.keys())
                {
                    auto multiAttrLists = multiAttrMap[groupKey];
                    for (auto attr : multiAttrLists)
                    {
                        if (attr.isShow)
                        {
                            WWidget* control = nullptr;
                            if (attr.displayFormat == DisplayFormat_Enum)
                            {
                                QMap<int, QPair<QString, QVariant>> enumMap;
                                m_widgetDeal.getEnumDataOrder(attr.enumType, unit, enumMap);
                                control = ControlFactory::createSetControlOrder(attr, enumMap);
                            }
                            else if (attr.displayFormat == DisplayFormat_LED)
                            {
                                QMap<int, QPair<QString, QVariant>> enumMap;
                                m_widgetDeal.getEnumDataOrder(attr.enumType, unit, enumMap);
                                control = ControlFactory::createSetControlOrder(attr, enumMap);
                            }
                            else
                            {
                                QMap<QString, QVariant> enumMap;
                                control = ControlFactory::createSetControl(attr, enumMap);
                            }

                            if (!attr.isEnable)
                            {
                                control->setEnabled(false);
                            }

                            if (control)
                            {
                                multiLayout->addWidget(control);
                                haveControl = true;
                                control->setProperty("targetNum", i);
                                // 测控基带扩频模式接收单元：只对目标一的接收通道设置为可用(因为设备只取了第一个目标的接收通道来用)
                                //跟踪基带扩频+数传/扩频+低速（4426），公共单元-S设备组合号：目标1能设置，其它三个不能设
                                if ((m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && m_currentMode.ID == 4 &&
                                     unit.unitType == "TIRU" && attr.id == "SReceivPolar" && i > 1) ||
                                    (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 2 &&
                                     (m_currentMode.ID == 6 || m_currentMode.ID == 11) && unit.unitType == "SK2TBPU" && attr.id == "SEquipCombNum" &&
                                     i > 1))
                                    control->setEnabled(false);

                                connect(control, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this,
                                        [=](const QVariant& value) {
                                            auto objName = control->objectName();
                                            if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0)
                                            {
                                                if (objName == "K2_WorkStation")  //包含扩频目标的模式里的工作站数目
                                                {
                                                    auto controlMap = getDeviceMultiWidgetParamControl(0, "K2_SpeedMeaPer");
                                                    m_widgetDeal.dealWidgetControlMultiRelation(i, "K2_SpeedMeaPer", value, controlMap);
                                                }
                                                //处理扩频/一体化码速率的控件范围
                                                dealWithCKMultiRcuNumRange(i, unitID, m_currentMode.ID, objName);
                                            }
                                        });
                            }
                        }
                    }
                }
                multiWidget->setLayout(multiLayout);
                multiWidget->setProperty("TargetKey", targetKey + i - 1);  // 用于有效标识
                if (recycleNum == 1)
                {
                    multiTabWidget->addTab(multiWidget, QString("%1").arg(desc));
                }
                else
                {
                    multiTabWidget->addTab(multiWidget, QString("目标%1").arg(i));
                }
            }
        }
    }

    if (multiTabWidget->count() > 0)
    {
        hboxLayout->addWidget(multiTabWidget);
        multiBox->setLayout(hboxLayout);
        vboxLayout->addWidget(multiBox);
        //这个判断就能判定这个设备必定是4424的测控基带的一体化模式   测控基带只有一体化才不需要初始化时的修改设置按钮
        if (m_currentProject == "4424" && m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && !unit.isModifySetBtn)
        {
            auto btnLayout = getMultiYTHAndKPBtnLayout(unitID, unit, groupBox);
            vboxLayout->addLayout(btnLayout);
        }
    }

    groupBox->setLayout(vboxLayout);

    //如果没有控件就删除界面，不创建
    if (!haveControl && !haveControlCommon)
    {
        delete groupBox;
        groupBox = nullptr;
    }

    // 如果设置多目标数量的控件不为空
    if (!m_targetNumControlList.isEmpty())
    {
        for (auto control : m_targetNumControlList)
        {
            connect(control, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this, [=](const QVariant& value) {
                auto size = multiTabWidget->count();
                for (int i = 0; i < size; ++i)
                {
                    if (i < value.toInt())
                    {
                        multiTabWidget->setTabEnabled(i, true);

                        if (m_click)  //当点击修改或者设置按钮后为true
                        {
                            if (unitID != 1)
                                multiTabWidget->widget(i)->setEnabled(false);
                        }
                        else
                        {
                            multiTabWidget->widget(i)->setEnabled(false);
                        }
                    }
                    else
                    {
                        multiTabWidget->setTabEnabled(i, false);
                    }
                }
                auto widget = groupBox->parentWidget();
                if (widget == nullptr)
                    return;
                QList<QGroupBox*> allgroup = widget->findChildren<QGroupBox*>();
                for (auto child : allgroup)
                {
                    QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
                    if (STATUS_TITLE == groupBox->title().trimmed())
                    {
                        QList<QTabWidget*> multiTabList = groupBox->findChildren<QTabWidget*>();
                        for (auto tabWidget : multiTabList)
                        {
                            auto size = tabWidget->count();
                            for (int i = 0; i < size; ++i)
                            {
                                if (i < value.toInt())
                                {
                                    tabWidget->setTabEnabled(i, true);
                                }
                                else
                                {
                                    tabWidget->setTabEnabled(i, false);
                                }
                            }
                        }
                    }
                }
            });

            emit control->sig_valueChange(control->value());
        }
    }
    return groupBox;
}

QWidget* DeviceMonitor::initUnitStatusGroupWidget(Unit& unit)
{
    QGroupBox* groupBox = new QGroupBox(STATUS_TITLE);
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    bool haveControlCommon = false;  // 如果没有公共参数就不显示公共参数的groupBox
    bool haveGrouboxCommon = false;  //如果一个groubox里面的数据都被隐藏掉了就不显示这个groupbox（普通参数）
    bool haveControl = false;        //如果没有多目标或者一体化参数就不显示对应的groupBox
    bool haveGrouboxMulti = false;   //如果一个groubox里面的数据都被隐藏掉了就不显示这个groupbox(多目标/一体化)
    /* 下面是公共单元的参数
     * **********************************************************/
    QGridLayout* commonGridLayout = new QGridLayout();
    for (auto groupKey : unit.statusGroupMap.keys())
    {
        UnitLayout unitLayout;
        if (unit.unitLayoutMap.contains(groupKey))
        {
            unitLayout = unit.unitLayoutMap.value(groupKey);
        }
        QGroupBox* commomGroupBox = new QGroupBox(unitLayout.desc);
        QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
        auto commonAttrList = unit.statusGroupMap[groupKey];
        int groupBoxEmpty = 0;
        for (auto attr : commonAttrList)
        {
            if (attr.isShow)
            {
                QMap<QString, QVariant> enumMap;
                if (attr.displayFormat == DisplayFormat_Enum)
                {
                    m_widgetDeal.getEnumData(attr.enumType, unit, enumMap);
                }
                else if (attr.displayFormat == DisplayFormat_LED)
                {
                    m_widgetDeal.getEnumData(attr.enumType, unit, enumMap);
                }
                auto control = ControlFactory::createStatusControl(attr, enumMap);
                if (control)
                {
                    commonLayout->addWidget(control);
                    haveControlCommon = true;
                    haveGrouboxCommon = true;
                }
            }

            if (attr.groupId == "undetermined")
                groupBoxEmpty++;
        }

        if (groupBoxEmpty == commonAttrList.size())
        {
            vboxLayout->addItem(commonLayout);
        }
        else
        {
            commomGroupBox->setLayout(commonLayout);
            if (haveGrouboxCommon)
            {
                commonGridLayout->addWidget(commomGroupBox, unitLayout.row, unitLayout.col, unitLayout.rowspan, unitLayout.colspan);
            }
            haveGrouboxCommon = false;  //每次加完一组参数后，即一个groupbox组完后重置为false
        }
    }
    if (!haveGrouboxCommon)  //这里的意思是如果haveGrouboxCommon为false，代表参数有分组，因此取反值进入下面，如果为true，代表参数没有分组，就不进去
    {
        vboxLayout->addItem(commonGridLayout);
    }

    // 创建LED矩阵
    QWidget* widget = createMatrix(unit);
    if (widget)
    {
        vboxLayout->addWidget(widget);
        vboxLayout->addStretch(1);
    }

    //存储转发多中心数界面
    if (m_currentDeviceID.sysID == 8 && m_currentDeviceID.devID == 1 && unit.unitType == "FU")
    {
        auto centerStatusMultiWidget = m_widgetDeal.initDataStatusMultiWidget(unit, 1);
        if (centerStatusMultiWidget)
            vboxLayout->addWidget(centerStatusMultiWidget);
    }

    /********多目标********/
    QGroupBox* multiBox = new QGroupBox(MULTI_TARGET);
    QTabWidget* multiTabWidget = new QTabWidget;
    QHBoxLayout* hboxLayout = new QHBoxLayout;
    for (auto targetKey : unit.multiStatusGroupMap.keys())
    {
        auto multiAttrMap = unit.multiStatusGroupMap[targetKey];
        auto targetType = unit.targetData[targetKey];
        auto recycleNum = unit.targetData[targetKey].recycleNum.toInt();
        auto desc = unit.targetData[targetKey].desc;

        if (unit.targetData[targetKey].isShow)
        {
            for (auto i = 1; i <= recycleNum; ++i)
            {
                QWidget* multiWidget = new QWidget;
                QGridLayout* multiGridLayout = new QGridLayout();

                for (auto groupKey : multiAttrMap.keys())
                {
                    //目标信息
                    UnitLayout unitLayout;
                    if (unit.unitLayoutMap.contains(groupKey))
                    {
                        unitLayout = unit.unitLayoutMap.value(groupKey);
                    }
                    QGroupBox* commomGroupBox = new QGroupBox(unitLayout.desc);
                    QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
                    commonLayout->setAlignment(Qt::AlignTop);
                    auto multiAttrLists = multiAttrMap[groupKey];
                    int groupBoxEmpty = 0;
                    for (auto attr : multiAttrLists)
                    {
                        if (attr.isShow)
                        {
                            QMap<QString, QVariant> enumMap;
                            if (attr.displayFormat == DisplayFormat_Enum)
                            {
                                m_widgetDeal.getEnumData(attr.enumType, unit, enumMap);
                            }
                            else if (attr.displayFormat == DisplayFormat_LED)
                            {
                                m_widgetDeal.getEnumData(attr.enumType, unit, enumMap);
                            }
                            auto control = ControlFactory::createStatusControl(attr, enumMap);
                            if (control)
                            {
                                commonLayout->addWidget(control);
                                haveControl = true;
                                haveGrouboxMulti = true;
                            }
                        }
                        if (attr.groupId == "undetermined")
                            groupBoxEmpty++;
                    }
                    if (groupBoxEmpty == multiAttrLists.size())
                    {
                        multiWidget->setLayout(commonLayout);
                    }
                    else
                    {
                        commomGroupBox->setLayout(commonLayout);
                        if (haveGrouboxMulti)
                        {
                            multiGridLayout->addWidget(commomGroupBox, unitLayout.row, unitLayout.col, unitLayout.rowspan, unitLayout.colspan);
                        }
                        haveGrouboxMulti = false;
                    }
                }

                if (!haveGrouboxMulti)  //这里的意思是如果haveGrouboxCommon为false，代表参数有分组，因此取反值进入下面，如果为true，代表参数没有分组，就不进去
                {
                    multiWidget->setLayout(multiGridLayout);
                }
                multiWidget->setProperty("TargetKey", targetKey + i - 1);
                if (recycleNum == 1)
                {
                    multiTabWidget->addTab(multiWidget, QString("%1").arg(desc));
                }
                else
                {
                    multiTabWidget->addTab(multiWidget, QString("目标%1").arg(i));
                }
            }
        }
    }

    if (multiTabWidget->count() > 0)
    {
        hboxLayout->addWidget(multiTabWidget);
        multiBox->setLayout(hboxLayout);
        vboxLayout->addWidget(multiBox);
    }

    groupBox->setLayout(vboxLayout);

    //如果没有控件就删除界面，不创建
    if (!haveControl && !haveControlCommon)
    {
        delete groupBox;
        groupBox = nullptr;
    }

    return groupBox;
}

QWidget* DeviceMonitor::initUnitSetsGroupWidget(Unit& unit, int unitID)
{
    QGroupBox* groupBox = new QGroupBox(SET_TITLE);
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    bool haveControlCommon = false;  // 如果没有公共参数就不显示公共参数的groupBox
    bool haveGrouboxCommon = false;  //如果一个groubox里面的数据都被隐藏掉了就不显示这个groupbox（普通参数）
    bool haveControl = false;        //如果没有多目标或者一体化参数就不显示对应的groupBox
    bool haveGrouboxMulti = false;   //如果一个groubox里面的数据都被隐藏掉了就不显示这个groupbox(多目标/一体化)

    if (!unit.kpValid.isEmpty())
    {
        m_widgetDeal.setTargetID(unit.kpValid);
    }

    /* 下面是公共单元的参数
     * **********************************************************/
    QGridLayout* commonGridLayout = new QGridLayout();
    for (auto groupKey : unit.settingGroupMap.keys())
    {
        UnitLayout unitLayout;
        if (unit.unitLayoutMap.contains(groupKey))
        {
            unitLayout = unit.unitLayoutMap.value(groupKey);
        }
        QGroupBox* commomGroupBox = new QGroupBox(unitLayout.desc);
        QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
        auto commonAttrList = unit.settingGroupMap[groupKey];
        for (auto attr : commonAttrList)
        {
            if (attr.isShow)
            {
                WWidget* control = nullptr;
                if (attr.displayFormat == DisplayFormat_Enum)
                {
                    QMap<int, QPair<QString, QVariant>> enumMap;
                    m_widgetDeal.getEnumDataOrder(attr.enumType, unit, enumMap);
                    control = ControlFactory::createSetControlOrder(attr, enumMap);
                }
                else if (attr.displayFormat == DisplayFormat_LED)
                {
                    QMap<int, QPair<QString, QVariant>> enumMap;
                    m_widgetDeal.getEnumDataOrder(attr.enumType, unit, enumMap);
                    control = ControlFactory::createSetControlOrder(attr, enumMap);
                }
                else
                {
                    QMap<QString, QVariant> enumMap;
                    control = ControlFactory::createSetControl(attr, enumMap);
                }

                if (!attr.isEnable)
                {
                    control->setEnabled(false);
                }

                if (control)
                {
                    commonLayout->addWidget(control);
                    haveControlCommon = true;
                    haveGrouboxCommon = true;
                    if (control->objectName() == unit.kpValid)  //存储能控制多目标数量的控件
                    {
                        m_targetNumControlList.append(control);
                    }
                    m_widgetMap[unit.unitCode][attr.id] = control;  //保存所有控件

                    connect(control, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this, [=](const QVariant& value) {
                        auto objName = control->objectName();
                        m_widgetDeal.dealWidgetRelation(unitID, objName, value, groupBox);
                        if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 4 && m_currentProject == "4426")
                        {
                            dealWithDSMSLNumRange(unitID, objName);  //处理低速基带解调和模拟源单元码速率控件的参数范围
                        }
                        else if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && (m_currentMode.ID == 4 || m_currentMode.ID == 5) &&
                                 objName == unit.kpValid)  //单目标的时候 接收通道有三个选项
                        {
                            auto controlMap = getDeviceMultiWidgetParamControl(1, "SReceivPolar");
                            m_widgetDeal.dealWidgetControlMultiRelation(0, "SReceivPolar", value, controlMap);
                        }
                        else if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && m_currentMode.ID == 1 && unitID == 7)
                        {
                            //测控基带标准TTC遥控单元码速率范围动态更改
                            dealWithCKTTCRcuNumRange(unitID, objName);
                        }
                        //跟踪基带标准+低速  扩频+低速码速率关联关系
                        else if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 2 && (m_currentMode.ID == 10 || m_currentMode.ID == 11))
                        {
                            dealWithGZMSLNumRange(unitID, objName);
                        }
                    });
                }
            }
        }

        commomGroupBox->setLayout(commonLayout);
        commomGroupBox->setProperty("isCommon", 1);

        if (haveGrouboxCommon)
        {
            commonGridLayout->addWidget(commomGroupBox, unitLayout.row, unitLayout.col, unitLayout.rowspan, unitLayout.colspan);
        }
        haveGrouboxCommon = false;  //每次加完一组参数后，即一个groupbox组完后重置为false
    }

    vboxLayout->addItem(commonGridLayout);

    if (!unit.settingGroupMap.isEmpty())
    {
        //这个判断就能判定这个设备必定是4424的测控基带的一体化模式   测控基带只有一体化才不需要初始化时的修改设置按钮
        if (m_currentProject == "4424" && m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && !unit.isModifySetBtn)
        {
            auto btnLayout = getCommonYTHAndKPBtnLayout(unitID, unit, groupBox);
            vboxLayout->addLayout(btnLayout);
        }
    }

    /********多目标********/
    QGroupBox* multiBox = new QGroupBox(MULTI_TARGET);
    QTabWidget* multiTabWidget = new QTabWidget;
    QHBoxLayout* hboxLayout = new QHBoxLayout;
    for (auto targetKey : unit.multiSettingGroupMap.keys())
    {
        auto multiAttrMap = unit.multiSettingGroupMap[targetKey];
        auto targetType = unit.targetData[targetKey];
        auto recycleNum = unit.targetData[targetKey].recycleNum.toInt();
        auto desc = unit.targetData[targetKey].desc;

        if (unit.targetData[targetKey].isShow)
        {
            for (auto i = 1; i <= recycleNum; ++i)
            {
                QWidget* multiWidget = new QWidget;
                QGridLayout* multiGridLayout = new QGridLayout();

                for (auto groupKey : multiAttrMap.keys())
                {
                    //目标信息
                    UnitLayout unitLayout;
                    if (unit.unitLayoutMap.contains(groupKey))
                    {
                        unitLayout = unit.unitLayoutMap.value(groupKey);
                    }
                    QGroupBox* commomGroupBox = new QGroupBox(unitLayout.desc);
                    QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
                    commonLayout->setAlignment(Qt::AlignTop);
                    auto multiAttrLists = multiAttrMap[groupKey];
                    for (auto attr : multiAttrLists)
                    {
                        if (attr.isShow)
                        {
                            WWidget* control = nullptr;
                            if (attr.displayFormat == DisplayFormat_Enum)
                            {
                                QMap<int, QPair<QString, QVariant>> enumMap;
                                m_widgetDeal.getEnumDataOrder(attr.enumType, unit, enumMap);
                                control = ControlFactory::createSetControlOrder(attr, enumMap);
                            }
                            else if (attr.displayFormat == DisplayFormat_LED)
                            {
                                QMap<int, QPair<QString, QVariant>> enumMap;
                                m_widgetDeal.getEnumDataOrder(attr.enumType, unit, enumMap);
                                control = ControlFactory::createSetControlOrder(attr, enumMap);
                            }
                            else
                            {
                                QMap<QString, QVariant> enumMap;
                                control = ControlFactory::createSetControl(attr, enumMap);
                            }

                            if (!attr.isEnable)
                            {
                                control->setEnabled(false);
                            }

                            if (control)
                            {
                                commonLayout->addWidget(control);
                                haveControl = true;
                                haveGrouboxMulti = true;
                                control->setProperty("targetNum", i);
                                // 测控基带扩频模式接收单元：只对目标一的接收通道设置为可用(因为设备只取了第一个目标的接收通道来用)
                                //跟踪基带扩频+数传，扩频跟踪接收单元-AGC时间常数：目标1能设置，其它三个不能设
                                //跟踪基带扩频+数传/扩频+低速（4426），公共单元-S设备组合号：目标1能设置，其它三个不能设
                                if ((m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && m_currentMode.ID == 4 &&
                                     unit.unitType == "TIRU" && attr.id == "SReceivPolar" && i > 1) ||
                                    (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 2 &&
                                     (m_currentMode.ID == 2 || m_currentMode.ID == 6 || m_currentMode.ID == 11) && unit.unitType == "SK2TBPU" &&
                                     attr.id == "SEquipCombNum" && i > 1))
                                    control->setEnabled(false);

                                connect(control, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this,
                                        [=](const QVariant& value) {
                                            auto objName = control->objectName();
                                            if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0)
                                            {
                                                if (objName == "K2_WorkStation")  //包含扩频目标的模式里的工作站数目
                                                {
                                                    auto controlMap = getDeviceMultiWidgetParamControl(0, "K2_SpeedMeaPer");
                                                    m_widgetDeal.dealWidgetControlMultiRelation(i, "K2_SpeedMeaPer", value, controlMap);
                                                }
                                                //处理扩频/一体化码速率的控件范围
                                                dealWithCKMultiRcuNumRange(i, unitID, m_currentMode.ID, objName);
                                            }
                                        });
                            }
                        }
                    }

                    commomGroupBox->setLayout(commonLayout);
                    if (haveGrouboxMulti)
                    {
                        multiGridLayout->addWidget(commomGroupBox, unitLayout.row, unitLayout.col, unitLayout.rowspan, unitLayout.colspan);
                    }
                    haveGrouboxMulti = false;
                }

                multiWidget->setLayout(multiGridLayout);

                multiWidget->setProperty("TargetKey", targetKey + i - 1);  // 用于有效标识
                if (recycleNum == 1)
                {
                    multiTabWidget->addTab(multiWidget, QString("%1").arg(desc));
                }
                else
                {
                    multiTabWidget->addTab(multiWidget, QString("目标%1").arg(i));
                }
            }
        }
    }

    if (multiTabWidget->count() > 0)
    {
        hboxLayout->addWidget(multiTabWidget);
        multiBox->setLayout(hboxLayout);
        vboxLayout->addWidget(multiBox);
        //这个判断就能判定这个设备必定是4424的测控基带的一体化模式   测控基带只有一体化才不需要初始化时的修改设置按钮
        if (m_currentProject == "4424" && m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && !unit.isModifySetBtn)
        {
            auto btnLayout = getMultiYTHAndKPBtnLayout(unitID, unit, groupBox);
            vboxLayout->addLayout(btnLayout);
        }
    }

    groupBox->setLayout(vboxLayout);

    //如果没有控件就删除界面，不创建
    if (!haveControl && !haveControlCommon)
    {
        delete groupBox;
        groupBox = nullptr;
    }

    // 如果设置多目标数量的控件不为空
    if (!m_targetNumControlList.isEmpty())
    {
        for (auto control : m_targetNumControlList)
        {
            connect(control, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange), this, [=](const QVariant& value) {
                auto size = multiTabWidget->count();
                for (int i = 0; i < size; ++i)
                {
                    if (i < value.toInt())
                    {
                        multiTabWidget->setTabEnabled(i, true);

                        if (m_click)  //当点击修改或者设置按钮后为true
                        {
                            if (unitID != 1)
                                multiTabWidget->widget(i)->setEnabled(false);
                        }
                        else
                        {
                            multiTabWidget->widget(i)->setEnabled(false);
                        }
                    }
                    else
                    {
                        multiTabWidget->setTabEnabled(i, false);
                    }
                }
                auto widget = groupBox->parentWidget();
                if (widget == nullptr)
                    return;
                QList<QGroupBox*> allgroup = widget->findChildren<QGroupBox*>();
                for (auto child : allgroup)
                {
                    QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
                    if (STATUS_TITLE == groupBox->title().trimmed())
                    {
                        QList<QTabWidget*> multiTabList = groupBox->findChildren<QTabWidget*>();
                        for (auto tabWidget : multiTabList)
                        {
                            auto size = tabWidget->count();
                            for (int i = 0; i < size; ++i)
                            {
                                if (i < value.toInt())
                                {
                                    tabWidget->setTabEnabled(i, true);
                                }
                                else
                                {
                                    tabWidget->setTabEnabled(i, false);
                                }
                            }
                        }
                    }
                }
            });
            emit control->sig_valueChange(control->value());
        }
    }

    return groupBox;
}

QWidget* DeviceMonitor::createMatrix(Unit& unit)
{
    int sysID = m_currentDeviceID.sysID;
    int devID = m_currentDeviceID.devID;
    int extentID = m_currentDeviceID.extenID;

    ArrowDirection dire = Right;
    // 4424 S频段射频开关矩阵
    if (sysID == 3 && devID == 1 && extentID == 1 && unit.unitCode == 1 && m_currentProject == "4424")
    {
        dire = Right;
    }
    else if (sysID == 3 && devID == 5 && extentID == 1 && unit.unitCode == 1 && m_currentProject == "4424")  // 4424 X频段8×8射频开关矩阵
    {
        dire = Right;
    }
    else if (sysID == 3 && devID == 12 && extentID == 1 && unit.unitCode == 1 && m_currentProject == "4424")  // 4424 1.2GHz中频开关矩阵
    {
        dire = Right;
    }
    else if (sysID == 2 && devID == 5 && extentID == 1 && unit.unitCode == 1 && m_currentProject == "4426")  // 4426 S射频发射矩阵
    {
        dire = Right;
    }
    else if (sysID == 3 && devID == 3 && extentID == 1 && unit.unitCode == 1 && m_currentProject == "4426")  // 4426 L数传中频矩阵
    {
        dire = Right;
    }
    else if (sysID == 3 && devID == 6 && extentID == 1 && unit.unitCode == 1 && m_currentProject == "4426")  // 4426 S频段射频矩阵
    {
        dire = Right;
    }
    else
    {
        return nullptr;
    }

    // 参数对应的枚举值
    QList<QPair<ParameterAttribute, QMap<int, QString>>> attrEnumList;
    for (auto attr : unit.parameterSetList)
    {
        if (attr.isShow && attr.displayFormat == DisplayFormat_Enum)
        {
            auto groupDesc = findAttrGroupDesc(unit, attr.id);
            if (!groupDesc.isEmpty())
            {
                attr.desc = QString("%1:%2").arg(groupDesc).arg(attr.desc);
            }

            QMap<QString, QVariant> tempDescValMap;
            m_widgetDeal.getEnumData(attr.enumType, unit, tempDescValMap);

            QMap<int, QString> tempValDescMap;
            // 反转一下，使用uValue作为键，确保map中只有一个相同uValue
            for (auto& desc : tempDescValMap.keys())
            {
                tempValDescMap.insert(tempDescValMap.value(desc).toInt(), desc);
            }

            attrEnumList.append(QPair<ParameterAttribute, QMap<int, QString>>(attr, tempValDescMap));
        }
    }

    QWidget* widget = ControlFactory::createMatrix(unit.unitCode, attrEnumList);
    if (widget)
    {
        auto matrixWidget = qobject_cast<WMatrixWidget*>(widget);
        matrixWidget->setDiagramArrowDirection(dire);
        connect(matrixWidget, &WMatrixWidget::actionTriggered, this,
                [=](const QString& attrId, const int& uValue) { matrixDiagramMenuTriggered(unit.unitCode, attrId, uValue); });
    }
    return widget;
}

QHBoxLayout* DeviceMonitor::getCommonYTHAndKPBtnLayout(int unitID, Unit unit, QGroupBox* groupBox)
{
    QPushButton* commonUpdataBtn = new QPushButton("修改");
    QPushButton* commonSettingBtn = new QPushButton("设置");

    QPushButton* autoMapBtn = nullptr;
    if (unit.isAutoMap)
    {
        autoMapBtn = new QPushButton("自动映射");
        autoMapBtn->setEnabled(false);

        connect(autoMapBtn, &QPushButton::clicked, this, [=]() {
            SERVICEONLINECHECK();  //判断服务器是否离线

            QString errorMsg;
            if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
            {
                QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
                return;
            }

            commonUpdataBtn->setText(QString("修改"));
            commonUpdataBtn->setEnabled(true);
            commonSettingBtn->setEnabled(false);
            autoMapBtn->setEnabled(false);
            //设置面板不可用
            setCommonSettingPanel(false, groupBox);

            packAutoMapUnitParamSet(unitID, unit, groupBox);
        });
    }

    m_changeBtnMapYTHCommon[unitID] = commonUpdataBtn;
    m_settingBtnMapYTHCommon[unitID] = commonSettingBtn;

    commonSettingBtn->setEnabled(false);

    connect(commonUpdataBtn, &QPushButton::clicked, this, [=]() {
        for (QWidget* control : m_needUpdateColorControlCommon.keys())
        {
            if (control)
            {
                control->setStyleSheet("");  // 清空样式
            }
        }
        for (auto recycle : m_needUpdateColorControlMulti.keys())
        {
            auto controlMap = m_needUpdateColorControlMulti[recycle];
            for (auto controlMulti : controlMap.keys())
            {
                if (controlMulti)
                {
                    controlMulti->setStyleSheet("");  // 清空样式
                }
            }
        }
        m_needUpdateColorControlCommon.clear();
        m_needUpdateColorControlMulti.clear();
        m_commonMapBefore.clear();
        m_commonMapAfter.clear();
        m_multiMapBefore.clear();
        m_multiMapAfter.clear();

        if (commonUpdataBtn->text() == "修改")
        {
            commonUpdataBtn->setText(QString("取消"));
            commonSettingBtn->setEnabled(true);

            if (autoMapBtn)
                autoMapBtn->setEnabled(true);

            //设置面板可用
            setCommonSettingPanel(true, groupBox);

            m_unitChangeStatus[unitID] = false;

            m_click = true;

            getSetParaAllMap(groupBox->parentWidget());  //修改的时候获取界面上值
        }
        else
        {
            commonUpdataBtn->setText(QString("修改"));
            commonSettingBtn->setEnabled(false);

            if (autoMapBtn)
                autoMapBtn->setEnabled(false);

            //设置面板不可用
            setCommonSettingPanel(false, groupBox);

            m_unitChangeStatus[unitID] = true;

            m_click = false;
        }
    });

    connect(commonSettingBtn, &QPushButton::clicked, this, [=]() {
        SERVICEONLINECHECK();  //判断服务器是否离线

        QString errorMsg;
        if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
        {
            QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
            return;
        }

        auto packResult = packUnitCommonParamSet(groupBox, unitID);
        if (packResult == PackResult::Failed || packResult == PackResult::Success)
        {
            commonUpdataBtn->setText(QString("修改"));
            commonUpdataBtn->setEnabled(true);
            commonSettingBtn->setEnabled(false);

            if (autoMapBtn)
                autoMapBtn->setEnabled(false);

            m_click = false;
            getSetParaAllMap(groupBox->parentWidget());  //设置之后获取界面上的值

            comPareSetParaValue();  //然后比较修改前及修改后的值

            //设置面板不可用
            setCommonSettingPanel(false, groupBox);
            m_unitChangeStatus[unitID] = true;
        }
    });

    auto btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(6);
    btnLayout->setContentsMargins(0, 0, 0, 0);

    auto spacerItem = new QSpacerItem(197, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    btnLayout->addItem(spacerItem);
    if (autoMapBtn)
        btnLayout->addWidget(autoMapBtn);
    btnLayout->addWidget(commonUpdataBtn);
    btnLayout->addWidget(commonSettingBtn);

    return btnLayout;
}

QHBoxLayout* DeviceMonitor::getMultiYTHAndKPBtnLayout(int unitID, Unit unit, QGroupBox* groupBox)
{
    QPushButton* multiUpdataBtn = new QPushButton("修改");
    QPushButton* multiSettingBtn = new QPushButton("设置");

    QPushButton* autoMapBtn = nullptr;
    if (unit.isAutoMap)
    {
        autoMapBtn = new QPushButton("自动映射");
        autoMapBtn->setEnabled(false);

        connect(autoMapBtn, &QPushButton::clicked, this, [=]() {
            SERVICEONLINECHECK();  //判断服务器是否离线

            QString errorMsg;
            if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
            {
                QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
                return;
            }

            multiUpdataBtn->setText(QString("修改"));
            multiUpdataBtn->setEnabled(true);
            multiSettingBtn->setEnabled(false);
            autoMapBtn->setEnabled(false);
            //设置面板不可用
            setMultiSettingPanel(false, groupBox);

            packAutoMapUnitParamSet(unitID, unit, groupBox);
        });
    }

    m_changeBtnMapYTHMulti[unitID] = multiUpdataBtn;
    m_settingBtnMapYTHMulti[unitID] = multiSettingBtn;

    multiSettingBtn->setEnabled(false);

    connect(multiUpdataBtn, &QPushButton::clicked, this, [=]() {
        for (QWidget* control : m_needUpdateColorControlCommon.keys())
        {
            if (control)
            {
                control->setStyleSheet("");  // 清空样式
            }
        }
        for (auto recycle : m_needUpdateColorControlMulti.keys())
        {
            auto controlMap = m_needUpdateColorControlMulti[recycle];
            for (auto controlMulti : controlMap.keys())
            {
                if (controlMulti)
                {
                    controlMulti->setStyleSheet("");  // 清空样式
                }
            }
        }
        m_needUpdateColorControlCommon.clear();
        m_needUpdateColorControlMulti.clear();
        m_commonMapBefore.clear();
        m_commonMapAfter.clear();
        m_multiMapBefore.clear();
        m_multiMapAfter.clear();

        if (multiUpdataBtn->text() == "修改")
        {
            multiUpdataBtn->setText(QString("取消"));
            multiSettingBtn->setEnabled(true);

            if (autoMapBtn)
                autoMapBtn->setEnabled(true);

            //设置面板可用
            setMultiSettingPanel(true, groupBox);

            m_unitChangeStatus[unitID] = false;

            m_click = true;

            getSetParaAllMap(groupBox->parentWidget());  //修改的时候获取界面上值
        }
        else
        {
            multiUpdataBtn->setText(QString("修改"));
            multiSettingBtn->setEnabled(false);

            if (autoMapBtn)
                autoMapBtn->setEnabled(false);

            //设置面板不可用
            setMultiSettingPanel(false, groupBox);

            m_unitChangeStatus[unitID] = true;

            m_click = false;
        }
    });

    connect(multiSettingBtn, &QPushButton::clicked, this, [=]() {
        SERVICEONLINECHECK();  //判断服务器是否离线

        QString errorMsg;
        if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
        {
            QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
            return;
        }

        auto packResult = packUnitMultiParamSet(groupBox, unitID);
        if (packResult == PackResult::Failed || packResult == PackResult::Success)
        {
            multiUpdataBtn->setText(QString("修改"));
            multiUpdataBtn->setEnabled(true);
            multiSettingBtn->setEnabled(false);
            if (autoMapBtn)
                autoMapBtn->setEnabled(false);

            m_click = false;
            getSetParaAllMap(groupBox->parentWidget());  //设置之后获取界面上的值

            comPareSetParaValue();  //然后比较修改前及修改后的值
            //设置面板不可用
            setMultiSettingPanel(false, groupBox);
        }

        m_unitChangeStatus[unitID] = true;
    });

    auto btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(6);
    btnLayout->setContentsMargins(0, 0, 0, 0);

    auto spacerItem = new QSpacerItem(197, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    btnLayout->addItem(spacerItem);
    if (autoMapBtn)
        btnLayout->addWidget(autoMapBtn);
    btnLayout->addWidget(multiUpdataBtn);
    btnLayout->addWidget(multiSettingBtn);

    return btnLayout;
}

void DeviceMonitor::initCmdWidget()
{
    if (!initCmdUI)
    {
        return;
    }
    initCmdUI = false;
    auto comboboxListLayout = new QHBoxLayout();
    comboboxListLayout->setSpacing(6);
    comboboxListLayout->setContentsMargins(6, 6, 6, 6);
    comboboxListLayout->addWidget(new QLabel("过程控制命令选择："));
    QComboBox* cmdListCombox = new QComboBox();
    connect(cmdListCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(cmdCurrentIndexChanged(int)));
    comboboxListLayout->addWidget(cmdListCombox);
    auto comboboxListspacerItem = new QSpacerItem(197, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    comboboxListLayout->addItem(comboboxListspacerItem);

    m_cmdscrollArea = new QScrollArea;

    QWidget* scrollContentWidget = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout;
    for (auto cmdID : m_currentMode.cmdMap.keys())
    {
        if (cmdID >= 100)  //先添加公共命令
        {
            CmdAttribute cmdAttribute = m_currentMode.cmdMap.value(cmdID);

            if (!cmdAttribute.isShow)
            {
                continue;
            }

            //判断是否配置了布局
            auto haveLayout = false;
            haveLayout = cmdAttribute.unitLayoutMap.isEmpty() ? false : true;

            QGroupBox* currentWidget = new QGroupBox(cmdAttribute.desc);

            QVBoxLayout* currentLayout = new QVBoxLayout;

            QWidget* statusWidget = nullptr;
            QWidget* setWidget = nullptr;
            if (haveLayout)  //有分组
            {
                //添加状态面板
                statusWidget = initCmdStatusGroupWidget(cmdAttribute);
                if (statusWidget != nullptr)
                {
                    currentLayout->addWidget(statusWidget);
                }

                //为需要的命令添加QPlainTextEdit
                QWidget* widget = m_widgetDeal.initCmdStatusPlaintextWidget(cmdID, m_currentMode.ID, cmdAttribute, m_currentDeviceID);
                if (widget)
                {
                    currentLayout->addWidget(widget);
                }

                //添加命令设置面板
                setWidget = initCmdSetGroupWidget(cmdAttribute);
                if (setWidget != nullptr)
                {
                    currentLayout->addWidget(setWidget);
                }
            }
            else  //无分组
            {
                //添加状态面板
                statusWidget = initCmdStatusWidget(cmdAttribute);
                if (statusWidget != nullptr)
                {
                    currentLayout->addWidget(statusWidget);
                }

                //为需要的命令添加QPlainTextEdit
                QWidget* widget = m_widgetDeal.initCmdStatusPlaintextWidget(cmdID, m_currentMode.ID, cmdAttribute, m_currentDeviceID);
                if (widget)
                {
                    currentLayout->addWidget(widget);
                }

                //添加命令设置面板
                setWidget = initCmdSetWidget(cmdAttribute);
                if (setWidget != nullptr)
                {
                    currentLayout->addWidget(setWidget);
                }
            }

            auto btnLayout = new QHBoxLayout();
            btnLayout->setSpacing(6);
            btnLayout->setContentsMargins(0, 0, 0, 0);
            auto spacerItem = new QSpacerItem(197, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
            btnLayout->addItem(spacerItem);

            QPushButton* cmdSettingCmdBtn = new QPushButton("命令设置");
            connect(cmdSettingCmdBtn, &QPushButton::clicked, this, &DeviceMonitor::packCmdParamSet);
            connect(cmdSettingCmdBtn, &QPushButton::clicked, this, &DeviceMonitor::initPlainTextEdit);

            btnLayout->addWidget(cmdSettingCmdBtn);
            currentLayout->addLayout(btnLayout);

            currentWidget->setLayout(currentLayout);

            m_cmdIDWidgetMap[cmdID] = currentWidget;
            m_cmdWidgetIDMap[currentWidget] = cmdID;
            scrollLayout->addWidget(currentWidget);
            cmdListCombox->addItem(cmdAttribute.desc, cmdID);
        }
    }

    for (auto cmdID : m_currentMode.cmdMap.keys())
    {
        if (cmdID < 100)  //再添加普通命令
        {
            CmdAttribute cmdAttribute = m_currentMode.cmdMap.value(cmdID);

            if (!cmdAttribute.isShow)
            {
                continue;
            }

            //判断是否配置了布局
            auto haveLayout = false;
            haveLayout = cmdAttribute.unitLayoutMap.isEmpty() ? false : true;

            QGroupBox* currentWidget = new QGroupBox(cmdAttribute.desc);

            QVBoxLayout* currentLayout = new QVBoxLayout;

            QWidget* statusWidget = nullptr;
            QWidget* setWidget = nullptr;
            if (haveLayout)  //有分组
            {
                //添加状态面板
                statusWidget = initCmdStatusGroupWidget(cmdAttribute);
                if (statusWidget != nullptr)
                {
                    currentLayout->addWidget(statusWidget);
                }

                //为需要的命令添加QPlainTextEdit
                QWidget* widget = m_widgetDeal.initCmdStatusPlaintextWidget(cmdID, m_currentMode.ID, cmdAttribute, m_currentDeviceID);
                if (widget)
                {
                    currentLayout->addWidget(widget);
                }

                //添加命令设置面板
                setWidget = initCmdSetGroupWidget(cmdAttribute);
                if (setWidget != nullptr)
                {
                    currentLayout->addWidget(setWidget);
                }
            }
            else  //无分组
            {
                //添加状态面板
                statusWidget = initCmdStatusWidget(cmdAttribute);
                if (statusWidget != nullptr)
                {
                    currentLayout->addWidget(statusWidget);
                }

                //为需要的命令添加QPlainTextEdit
                QWidget* widget = m_widgetDeal.initCmdStatusPlaintextWidget(cmdID, m_currentMode.ID, cmdAttribute, m_currentDeviceID);
                if (widget)
                {
                    currentLayout->addWidget(widget);
                }

                //添加命令设置面板
                setWidget = initCmdSetWidget(cmdAttribute);
                if (setWidget != nullptr)
                {
                    currentLayout->addWidget(setWidget);
                }
            }

            auto btnLayout = new QHBoxLayout();
            btnLayout->setSpacing(6);
            btnLayout->setContentsMargins(0, 0, 0, 0);
            auto spacerItem = new QSpacerItem(197, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
            btnLayout->addItem(spacerItem);

            QPushButton* cmdSettingCmdBtn = new QPushButton("命令设置");
            connect(cmdSettingCmdBtn, &QPushButton::clicked, this, &DeviceMonitor::packCmdParamSet);
            connect(cmdSettingCmdBtn, &QPushButton::clicked, this, &DeviceMonitor::initPlainTextEdit);

            btnLayout->addWidget(cmdSettingCmdBtn);
            currentLayout->addLayout(btnLayout);

            currentWidget->setLayout(currentLayout);

            m_cmdIDWidgetMap[cmdID] = currentWidget;
            m_cmdWidgetIDMap[currentWidget] = cmdID;
            scrollLayout->addWidget(currentWidget);
            cmdListCombox->addItem(cmdAttribute.desc, cmdID);
        }
    }

    scrollContentWidget->setLayout(scrollLayout);
    m_cmdscrollArea->setWidget(scrollContentWidget);
    m_cmdscrollArea->setWidgetResizable(true);

    QVBoxLayout* allVLayout = new QVBoxLayout;
    allVLayout->addLayout(comboboxListLayout);
    allVLayout->addWidget(m_cmdscrollArea);
    ui->cmdPages->setLayout(allVLayout);

    signalInitCmdFinished(this);
}

QWidget* DeviceMonitor::initCmdStatusWidget(CmdAttribute cmdAttribute)
{
    QGroupBox* groupBox = new QGroupBox(STATUS_TITLE);
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    bool haveControl = false;  // 如果没有控件需要显示就不创建界面

    for (auto groupKey : cmdAttribute.statusGroupMap.keys())
    {
        QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
        auto commonAttrList = cmdAttribute.statusGroupMap[groupKey];
        for (auto attr : commonAttrList)
        {
            if (attr.isShow)
            {
                QMap<QString, QVariant> enumMap;
                if (attr.displayFormat == DisplayFormat_Enum)
                {
                    m_widgetDeal.getCmdEnumData(attr.enumType, enumMap);
                }
                else if (attr.displayFormat == DisplayFormat_LED)
                {
                    m_widgetDeal.getCmdEnumData(attr.enumType, enumMap);
                }
                auto control = ControlFactory::createStatusControl(attr, enumMap);
                if (control)
                {
                    commonLayout->addWidget(control);
                    haveControl = true;
                }
            }
        }
        vboxLayout->addItem(commonLayout);
    }

    //创建动态界面
    auto hLayout = m_widgetDeal.initMultiWidgetStatusHLayout(cmdAttribute);

    if (hLayout != nullptr)
        vboxLayout->addItem(hLayout);

    groupBox->setLayout(vboxLayout);

    //如果没有控件就删除界面，不创建
    if (!haveControl)
    {
        delete groupBox;
        groupBox = nullptr;
    }

    return groupBox;
}

QWidget* DeviceMonitor::initCmdSetWidget(CmdAttribute cmdAttribute)
{
    QGroupBox* groupBox = new QGroupBox(SET_TITLE);

    QVBoxLayout* vboxLayout = new QVBoxLayout();

    bool haveControl = false;  // 如果没有控件需要显示就不创建界面

    for (auto groupKey : cmdAttribute.settingGroupMap.keys())
    {
        QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
        auto commonAttrList = cmdAttribute.settingGroupMap[groupKey];

        for (auto attr : commonAttrList)
        {
            if (attr.isShow)
            {
                WWidget* control = nullptr;
                if (attr.displayFormat == DisplayFormat_Enum)
                {
                    QMap<int, QPair<QString, QVariant>> enumMap;
                    m_widgetDeal.getCmdEnumDataOrder(attr.enumType, enumMap);
                    control = ControlFactory::createSetControlOrder(attr, enumMap);
                }
                else if (attr.displayFormat == DisplayFormat_LED)
                {
                    QMap<int, QPair<QString, QVariant>> enumMap;
                    m_widgetDeal.getCmdEnumDataOrder(attr.enumType, enumMap);
                    control = ControlFactory::createSetControlOrder(attr, enumMap);
                }
                else
                {
                    QMap<QString, QVariant> enumMap;
                    control = ControlFactory::createSetControl(attr, enumMap);
                }

                if (!attr.isEnable)
                {
                    control->setEnabled(false);
                }

                if (control)
                {
                    commonLayout->addWidget(control);
                    haveControl = true;

                    void (WUNumber::*signal_valueChange)(const QVariant&) = &WUNumber::sig_valueChanged;
                    auto unumberControl = qobject_cast<WUNumber*>(control);
                    if (!unumberControl)
                        continue;

                    connect(unumberControl, signal_valueChange, this, [=](QVariant value) {
                        auto objName = control->objectName();
                        m_widgetDeal.dealCmdMulitiSetsWidgets(objName, value, cmdAttribute, m_cmdIDWidgetMap);
                    });
                }
            }
        }

        vboxLayout->addItem(commonLayout);
    }

    //创建动态界面
    auto hLayout = m_widgetDeal.initMultiWidgetSetsHLayout(cmdAttribute);

    if (hLayout != nullptr)
        vboxLayout->addItem(hLayout);

    groupBox->setLayout(vboxLayout);

    //如果没有控件就删除界面，不创建
    if (!haveControl)
    {
        delete groupBox;
        groupBox = nullptr;
    }

    return groupBox;
}

QWidget* DeviceMonitor::initCmdStatusGroupWidget(CmdAttribute cmdAttribute)
{
    QGroupBox* groupBox = new QGroupBox(STATUS_TITLE);
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    bool haveControl = false;  // 如果没有控件需要显示就不创建界面
    QGridLayout* commonGridLayout = new QGridLayout();
    for (auto groupKey : cmdAttribute.statusGroupMap.keys())
    {
        UnitLayout unitLayout;
        if (cmdAttribute.unitLayoutMap.contains(groupKey))
        {
            unitLayout = cmdAttribute.unitLayoutMap.value(groupKey);
        }
        QGroupBox* commomGroupBox = new QGroupBox(unitLayout.desc);
        QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
        auto commonAttrList = cmdAttribute.statusGroupMap[groupKey];
        for (auto attr : commonAttrList)
        {
            if (attr.isShow)
            {
                QMap<QString, QVariant> enumMap;
                if (attr.displayFormat == DisplayFormat_Enum)
                {
                    m_widgetDeal.getCmdEnumData(attr.enumType, enumMap);
                }
                else if (attr.displayFormat == DisplayFormat_LED)
                {
                    m_widgetDeal.getCmdEnumData(attr.enumType, enumMap);
                }
                auto control = ControlFactory::createStatusControl(attr, enumMap);
                if (control)
                {
                    commonLayout->addWidget(control);
                    haveControl = true;
                }
            }
        }
        commomGroupBox->setLayout(commonLayout);
        commonGridLayout->addWidget(commomGroupBox, unitLayout.row, unitLayout.col, unitLayout.rowspan, unitLayout.colspan);
    }
    vboxLayout->addItem(commonGridLayout);

    //创建动态界面
    auto hLayout = m_widgetDeal.initMultiWidgetStatusHLayout(cmdAttribute);

    if (hLayout != nullptr)
        vboxLayout->addItem(hLayout);

    groupBox->setLayout(vboxLayout);

    //如果没有控件就删除界面，不创建
    if (!haveControl)
    {
        delete groupBox;
        groupBox = nullptr;
    }

    return groupBox;
}

QWidget* DeviceMonitor::initCmdSetGroupWidget(CmdAttribute cmdAttribute)
{
    QGroupBox* groupBox = new QGroupBox(SET_TITLE);
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    bool haveControl = false;  // 如果没有控件需要显示就不创建界面
    QGridLayout* commonGridLayout = new QGridLayout();
    for (auto groupKey : cmdAttribute.settingGroupMap.keys())
    {
        UnitLayout unitLayout;
        if (cmdAttribute.unitLayoutMap.contains(groupKey))
        {
            unitLayout = cmdAttribute.unitLayoutMap.value(groupKey);
        }
        QGroupBox* commomGroupBox = new QGroupBox(unitLayout.desc);
        QFlowLayout* commonLayout = new QFlowLayout(6, 0, 10);
        auto commonAttrList = cmdAttribute.settingGroupMap[groupKey];
        for (auto attr : commonAttrList)
        {
            if (attr.isShow)
            {
                WWidget* control = nullptr;
                if (attr.displayFormat == DisplayFormat_Enum)
                {
                    QMap<int, QPair<QString, QVariant>> enumMap;
                    m_widgetDeal.getCmdEnumDataOrder(attr.enumType, enumMap);
                    control = ControlFactory::createSetControlOrder(attr, enumMap);
                }
                else if (attr.displayFormat == DisplayFormat_LED)
                {
                    QMap<int, QPair<QString, QVariant>> enumMap;
                    m_widgetDeal.getCmdEnumDataOrder(attr.enumType, enumMap);
                    control = ControlFactory::createSetControlOrder(attr, enumMap);
                }
                else
                {
                    QMap<QString, QVariant> enumMap;
                    control = ControlFactory::createSetControl(attr, enumMap);
                }

                if (!attr.isEnable)
                {
                    control->setEnabled(false);
                }

                if (control)
                {
                    commonLayout->addWidget(control);
                    haveControl = true;

                    void (WUNumber::*signal_valueChange)(const QVariant&) = &WUNumber::sig_valueChanged;
                    auto unumberControl = qobject_cast<WUNumber*>(control);
                    if (!unumberControl)
                        continue;

                    connect(unumberControl, signal_valueChange, this, [=](QVariant value) {
                        auto objName = control->objectName();
                        m_widgetDeal.dealCmdMulitiSetsWidgets(objName, value, cmdAttribute, m_cmdIDWidgetMap);
                    });
                }
            }
        }
        commomGroupBox->setLayout(commonLayout);
        commonGridLayout->addWidget(commomGroupBox, unitLayout.row, unitLayout.col, unitLayout.rowspan, unitLayout.colspan);
    }
    vboxLayout->addItem(commonGridLayout);

    //创建动态界面
    auto hLayout = m_widgetDeal.initMultiWidgetSetsHLayout(cmdAttribute);

    if (hLayout != nullptr)
        vboxLayout->addItem(hLayout);

    groupBox->setLayout(vboxLayout);

    //如果没有控件就删除界面，不创建
    if (!haveControl)
    {
        delete groupBox;
        groupBox = nullptr;
    }

    return groupBox;
}

DeviceMonitor::PackResult DeviceMonitor::packUnitCommonParamSet(QWidget* widget, int unitID)
{
    auto currentWidget = widget;

    //    //设置参数
    if (!m_currentMode.unitMap.contains(unitID))
    {
        return PackResult::Failed;
    }
    QList<ParameterAttribute> setParamList = m_currentMode.unitMap.value(unitID).parameterSetList;

    QMap<QString, QVariant> setParamData;
    if (!m_widgetDeal.getSetParamMapYTH(setParamData, currentWidget))
    {
        QMessageBox::warning(this, "提示", "参数值不在合法范围", QString("确定"));
        m_setTime = GlobalData::currentTime();
        m_currentScroArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->currentWidget());
        m_colorFlag = true;
        return PackResult::RangeFailed;
    }

    int systemId = m_currentDeviceID.sysID;
    int deviceId = m_currentDeviceID.devID;
    int extionId = m_currentDeviceID.extenID;
    int modeId = m_currentMode.ID;

    UnitParamRequest unitParamRequest;
    unitParamRequest.m_systemNumb = systemId;
    unitParamRequest.m_deviceNumb = deviceId;
    unitParamRequest.m_extenNumb = extionId;

    unitParamRequest.m_modeID = modeId;
    unitParamRequest.m_unitID = unitID;

    //获取有效标识
    unitParamRequest.m_validIdent = 0;

    if (!setParamList.isEmpty())
    {
        for (auto attr : setParamList)
        {
            if (setParamData.contains(attr.id))
            {
                unitParamRequest.m_paramdataMap[attr.id] = setParamData[attr.id];
            }
        }
    }

    JsonWriter writer;
    writer& unitParamRequest;

    RedisHelper::getInstance().setData("单元发送数据（一体化公共参数）", writer.GetQString());

    emit signalsUnitDeviceJson(writer.GetString());
    m_setTime = GlobalData::currentTime();
    m_currentScroArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->currentWidget());
    m_colorFlag = true;

    return PackResult::Success;
}

DeviceMonitor::PackResult DeviceMonitor::packUnitMultiParamSet(QWidget* widget, int unitID)
{
    auto currentWidget = widget;  // 从滚动区域取

    //    //设置参数
    if (!m_currentMode.unitMap.contains(unitID))
    {
        return PackResult::Failed;
    }

    int systemId = m_currentDeviceID.sysID;
    int deviceId = m_currentDeviceID.devID;
    int extionId = m_currentDeviceID.extenID;
    int modeId = m_currentMode.ID;

    UnitParamRequest unitParamRequest;
    unitParamRequest.m_systemNumb = systemId;
    unitParamRequest.m_deviceNumb = deviceId;
    unitParamRequest.m_extenNumb = extionId;

    unitParamRequest.m_modeID = modeId;
    unitParamRequest.m_unitID = unitID;

    QMap<int, QList<ParameterAttribute>> setMultiParaRecycleMap = m_currentMode.unitMap.value(unitID).multiSettingMap;

    ////获取有效标识和界面值
    QList<QMap<QString, QVariant>> paraTargerMap;
    if (!m_widgetDeal.getSetMultiTargetParamMapYTH(paraTargerMap, unitParamRequest, currentWidget))
    {
        QMessageBox::warning(this, "提示", "参数值不在合法范围", QString("确定"));
        m_setTime = GlobalData::currentTime();
        m_currentScroArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->currentWidget());
        m_colorFlag = true;
        return PackResult::RangeFailed;
    }

    // 这里检查一些特殊的关联关系（需要参数满足特殊公式的） 限扩频模式
    QString errorMsg2;
    bool flag2 = checkParamRelation2(paraTargerMap, errorMsg2);
    if (!flag2)
    {
        QMessageBox::warning(this, "提示", errorMsg2, QString("确定"));
        m_setTime = GlobalData::currentTime();
        m_currentScroArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->currentWidget());
        m_colorFlag = true;
        return PackResult::RelationFailed;
    }

    if (!setMultiParaRecycleMap.isEmpty())
    {
        QList<ParameterAttribute> multiParaRecycleList = setMultiParaRecycleMap.begin().value();

        for (auto key : setMultiParaRecycleMap.keys())
        {
            for (auto currentData : paraTargerMap)
            {
                for (auto attr : setMultiParaRecycleMap.value(key))
                {
                    if (currentData.contains(attr.id))
                    {
                        unitParamRequest.m_multiTargetParamMap[key].m_paramdataMap[attr.id] = currentData[attr.id];
                    }
                }
            }
        }
    }

    JsonWriter writer;
    writer& unitParamRequest;

    RedisHelper::getInstance().setData("单元发送数据（一体化所有模式）", writer.GetQString());

    emit signalsUnitDeviceJson(writer.GetString());
    m_setTime = GlobalData::currentTime();
    m_currentScroArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->currentWidget());
    m_colorFlag = true;

    return PackResult::Success;
}

DeviceMonitor::PackResult DeviceMonitor::packUnitParamSet(QWidget* widget, int unitID)
{
    auto currentWidget = widget;  // 从滚动区域取

    //    //设置参数
    if (!m_currentMode.unitMap.contains(unitID))
    {
        return PackResult::Failed;
    }
    QList<ParameterAttribute> setParamList = m_currentMode.unitMap.value(unitID).parameterSetList;

    QMap<QString, QVariant> setParamData;
    if (!m_widgetDeal.getSetParamMapOther(setParamData, currentWidget))
    {
        QMessageBox::warning(this, "提示", "参数值不在合法范围", QString("确定"));
        m_setTime = GlobalData::currentTime();
        m_currentScroArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->currentWidget());
        m_colorFlag = true;
        return PackResult::RangeFailed;
    }

    //检查一些需要计算的关联关系
    QString errorMsg;
    bool flag = checkParamRelation(setParamData, errorMsg);
    if (!flag)
    {
        QMessageBox::warning(this, "提示", errorMsg, QString("确定"));
        m_setTime = GlobalData::currentTime();
        m_currentScroArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->currentWidget());
        m_colorFlag = true;
        return PackResult::RelationFailed;
    }
    int systemId = m_currentDeviceID.sysID;
    int deviceId = m_currentDeviceID.devID;
    int extionId = m_currentDeviceID.extenID;
    int modeId = m_currentMode.ID;

    UnitParamRequest unitParamRequest;
    unitParamRequest.m_systemNumb = systemId;
    unitParamRequest.m_deviceNumb = deviceId;
    unitParamRequest.m_extenNumb = extionId;

    unitParamRequest.m_modeID = modeId;
    unitParamRequest.m_unitID = unitID;

    if (!setParamList.isEmpty())
    {
        for (auto attr : setParamList)
        {
            if (setParamData.contains(attr.id))
            {
                unitParamRequest.m_paramdataMap[attr.id] = setParamData[attr.id];
            }
        }
    }

    QMap<int, QList<ParameterAttribute>> setMultiParaRecycleMap = m_currentMode.unitMap.value(unitID).multiSettingMap;

    QList<QMap<QString, QVariant>> paraTargerMap;
    if (!m_widgetDeal.getSetMultiTargetParamMapOther(paraTargerMap, currentWidget))
    {
        QMessageBox::warning(this, "提示", "参数值不在合法范围", QString("确定"));
        m_setTime = GlobalData::currentTime();
        m_currentScroArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->currentWidget());
        m_colorFlag = true;
        return PackResult::RangeFailed;
    }

    // 这里检查一些特殊的关联关系（需要参数满足特殊公式的） 限扩频模式
    QString errorMsg2;
    bool flag2 = checkParamRelation2(paraTargerMap, errorMsg2);
    if (!flag2)
    {
        QMessageBox::warning(this, "提示", errorMsg2, QString("确定"));
        m_setTime = GlobalData::currentTime();
        m_currentScroArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->currentWidget());
        m_colorFlag = true;
        return PackResult::RelationFailed;
    }

    Unit unit = m_currentMode.unitMap[unitID];
    //这里是获取扩频4目标时的有效标识
    if (!unit.kpValid.isEmpty())
    {
        unitParamRequest.m_validIdent = getDeviceWidgetParamData(0, unit.kpValid).toInt() + 1;
    }

    //这里的有效标识是因为跟踪基带一体+扩频模式又没有多目标又需要通道标识的情况下，基带数目固定为1，故有效标识设置为0
    switch (m_deviceID)
    {
    case 0x4201:
    {
        switch (m_currentMode.ID)
        {
        case 0x8: unitParamRequest.m_validIdent = 0; break;
        }
    }
    default: break;
    }

    if (!setMultiParaRecycleMap.isEmpty())
    {
        int i = 1;
        QList<ParameterAttribute> multiParaRecycleList = setMultiParaRecycleMap.begin().value();
        for (auto currentData : paraTargerMap)
        {
            for (auto attr : multiParaRecycleList)
            {
                if (currentData.contains(attr.id))
                {
                    unitParamRequest.m_multiTargetParamMap[i].m_paramdataMap[attr.id] = currentData[attr.id];
                }
            }
            i++;
        }
    }

    JsonWriter writer;
    writer& unitParamRequest;

    RedisHelper::getInstance().setData("单元发送数据", writer.GetQString());
    m_setTime = GlobalData::currentTime();
    m_currentScroArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->currentWidget());
    m_colorFlag = true;

    //   qDebug() << writer.GetString();

    emit signalsUnitDeviceJson(writer.GetString());

    return PackResult::Success;
}

void DeviceMonitor::packCmdParamSet()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    QPushButton* settingBtn = qobject_cast<QPushButton*>(sender());
    if (settingBtn)
    {
        QGroupBox* currentWidget = qobject_cast<QGroupBox*>(settingBtn->parent());
        if (currentWidget && m_cmdWidgetIDMap.contains(currentWidget))
        {
            auto cmdID = m_cmdWidgetIDMap[currentWidget];
            int systemId = m_currentDeviceID.sysID;
            int deviceId = m_currentDeviceID.devID;
            int extionId = m_currentDeviceID.extenID;
            int modeId = m_currentMode.ID;

            CmdRequest cmdRequest;
            cmdRequest.m_systemNumb = systemId;
            cmdRequest.m_deviceNumb = deviceId;
            cmdRequest.m_extenNumb = extionId;

            cmdRequest.m_modeID = modeId;
            cmdRequest.m_cmdID = cmdID;

            QList<ParameterAttribute> cmdSetParamList = m_currentMode.cmdMap.value(cmdID).requestList;

            QMap<QString, QVariant> setParamData;
            if (!m_widgetDeal.getCmdSetParamMap(setParamData, currentWidget))
            {
                QMessageBox::warning(this, "提示", "参数值不在合法范围", QString("确定"));
                return;
            }

            //检查一些关联关系（如时间判断等）
            QString errorMsg;
            bool flag = checkParamCmdRelation(setParamData, cmdID, errorMsg);
            if (!flag)
            {
                QMessageBox::warning(this, "提示", errorMsg, QString("确定"));
                return;
            }

            if (!cmdSetParamList.isEmpty())
            {
                cmdRequest.m_paramdataMap = setParamData;
            }
            //留着有recycle标签的命令使用
            QMap<int, QList<ParameterAttribute>> setMultiParaRecycleMap = m_currentMode.cmdMap.value(cmdID).requestRecycle;

            QMap<int, QList<QPair<QString, QVariant>>> paraTargerMap;
            if (!m_widgetDeal.getCmdMultiParamMap(paraTargerMap, currentWidget))
            {
                QMessageBox::warning(this, "提示", "参数值不在合法范围", QString("确定"));
                return;
            }

            if (!setMultiParaRecycleMap.isEmpty())
            {
                cmdRequest.multiParamMap = paraTargerMap;
            }

            JsonWriter writer;
            writer& cmdRequest;
            RedisHelper::getInstance().setData("过程控制命令发送数据", writer.GetQString());

            emit signalsCmdDeviceJson(writer.GetString());
            qDebug() << writer.GetString();
        }
    }
}

quint64 DeviceMonitor::createDeviceID(int sysID, int devID, int extenID)
{
    quint32 deviceID = ((static_cast<quint32>(sysID) & 0xF) << 12);
    deviceID = deviceID | ((static_cast<quint32>(devID) & 0xF) << 8);
    deviceID = deviceID | (static_cast<quint32>(extenID) & 0xFF);
    return deviceID;
}

void DeviceMonitor::timerEvent(QTimerEvent* event)
{
    if (m_isOnline > -1)
    {
        m_isOnline--;
    }
    if (m_isOnline == 0)
    {
        ui->isOnlineLabel->setText(QString("设备离线"));
        ui->isOnlineLabel->setStyleSheet("color:red;font-size:30px;");

        setDeviceReduction();
    }

    auto currentTime = GlobalData::currentTime();
    if (qAbs(m_setTime.secsTo(currentTime)) >= 6)
    {
        if (m_colorFlag)
        {
            comPareWidgetSetValue();
            m_colorFlag = false;
        }
    }
}

void DeviceMonitor::showEvent(QShowEvent*) { m_isShow = true; }

void DeviceMonitor::hideEvent(QHideEvent*)
{
    m_isShow = false;
    //只要切换了设备，按钮全部置为初始状态
    for (auto unitID : m_changeBtnMap.keys())
    {
        auto changeBtn = m_changeBtnMap[unitID];
        if (changeBtn->text() == QString("取消"))
            changeBtn->click();
    }

    for (auto unitID : m_settingBtnMap.keys())
    {
        auto settingBtn = m_settingBtnMap[unitID];
        settingBtn->setEnabled(false);
    }

    for (auto unitID : m_changeBtnMapYTHCommon.keys())
    {
        auto changeBtn = m_changeBtnMapYTHCommon[unitID];
        if (changeBtn->text() == QString("取消"))
            changeBtn->click();
    }

    for (auto unitID : m_settingBtnMapYTHCommon.keys())
    {
        auto settingBtn = m_settingBtnMapYTHCommon[unitID];
        settingBtn->setEnabled(false);
    }

    for (auto unitID : m_changeBtnMapYTHMulti.keys())
    {
        auto changeBtn = m_changeBtnMapYTHMulti[unitID];
        if (changeBtn->text() == QString("取消"))
            changeBtn->click();
    }

    for (auto unitID : m_settingBtnMapYTHMulti.keys())
    {
        auto settingBtn = m_settingBtnMapYTHMulti[unitID];
        settingBtn->setEnabled(false);
    }

    for (auto item : m_unitChangeStatus.keys())
    {
        m_unitChangeStatus[item] = true;
    }
}

void DeviceMonitor::setCommonSettingPanel(bool state, QGroupBox* groupBox)
{
    if (groupBox == nullptr)
    {
        return;
    }

    // 查找出设置面板
    QGroupBox* setGroupBox = nullptr;
    if (SET_TITLE == groupBox->title().trimmed())  // 本身就是设置面板
    {
        setGroupBox = groupBox;
    }
    else
    {
        auto allgroup = groupBox->findChildren<QGroupBox*>();
        for (auto groupBox : allgroup)
        {
            if (SET_TITLE == groupBox->title().trimmed())
            {
                setGroupBox = groupBox;
            }
        }
    }

    if (setGroupBox)
    {
        auto allGroupBox = setGroupBox->findChildren<QGroupBox*>();
        //公共参数状态控件更改
        for (auto box : allGroupBox)
        {
            if (box->title().trimmed() != MULTI_TARGET && box->property("isCommon") == 1)
            {
                box->setEnabled(state);
            }
        }
    }
}

void DeviceMonitor::setMultiSettingPanel(bool state, QGroupBox* groupBox)
{
    if (groupBox)
    {
        // 查找出设置面板
        QGroupBox* setGroupBox = nullptr;
        if (groupBox != nullptr && SET_TITLE == groupBox->title().trimmed())  // 本身就是设置面板
        {
            setGroupBox = groupBox;
        }
        else
        {
            auto allgroup = groupBox->findChildren<QGroupBox*>();
            for (auto groupBox : allgroup)
            {
                if (SET_TITLE == groupBox->title().trimmed())
                {
                    setGroupBox = groupBox;
                }
            }
        }

        if (setGroupBox)
        {
            auto allGroupBox = setGroupBox->findChildren<QGroupBox*>();
            //一体化状态控件更改
            for (auto box : allGroupBox)
            {
                if (box->title().trimmed() == MULTI_TARGET)
                {
                    QList<QTabWidget*> multiTabList = box->findChildren<QTabWidget*>();
                    for (auto& tabWidget : multiTabList)
                    {
                        for (int i = 0; i < tabWidget->count(); ++i)
                        {
                            auto widget = tabWidget->widget(i);
                            widget->setEnabled(state);
                        }
                    }
                }
            }
        }
    }
}

void DeviceMonitor::setSettingPanel(bool state, QWidget* widget)
{
    if (widget)
    {
        // 查找出设置面板
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(widget);
        QGroupBox* setGroupBox = nullptr;
        if (groupBox != nullptr && SET_TITLE == groupBox->title().trimmed())  // 本身就是设置面板
        {
            setGroupBox = groupBox;
        }
        else
        {
            auto allgroup = widget->findChildren<QGroupBox*>();
            for (auto groupBox : allgroup)
            {
                if (SET_TITLE == groupBox->title().trimmed())
                {
                    setGroupBox = groupBox;
                }
            }
        }

        if (setGroupBox)
        {
            bool isHasTargetSelect = false;
            auto allGroupBox = setGroupBox->findChildren<QGroupBox*>();
            //公共参数状态控件更改
            for (auto& box : allGroupBox)
            {
                if (box->title().trimmed() == MULTI_TARGET)
                {
                    isHasTargetSelect = true;
                }
            }
            if (!isHasTargetSelect)  //这里就是没有多目标的如高频接收分系统，标准TTC这种界面
            {
                setGroupBox->setEnabled(state);
            }
            else  //有多目标的需要分情况
            {
                //多目标状态控件更改
                for (auto box : allGroupBox)
                {
                    if (box->title().trimmed() == MULTI_TARGET)
                    {
                        QList<QTabWidget*> multiTabList = box->findChildren<QTabWidget*>();
                        for (auto& tabWidget : multiTabList)
                        {
                            for (int i = 0; i < tabWidget->count(); ++i)
                            {
                                auto widget = tabWidget->widget(i);
                                widget->setEnabled(state);
                            }
                        }
                    }  //除开多目标的其他必须分组才能设置当前状态
                    else if (box->property("isCommon") == 1)
                    {
                        box->setEnabled(state);
                    }
                }
            }
        }
    }
}

void DeviceMonitor::getSetParaAllMap(QWidget* widget)
{
    bool isCommonValid = false;
    bool isMultiValid = false;

    for (auto child : widget->children())
    {
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
        if (!groupBox)
        {
            continue;
        }

        if (groupBox->title().trimmed() == SET_TITLE)
        {
            QList<QGroupBox*> multiGrouList = groupBox->findChildren<QGroupBox*>();
            QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();

            for (auto& control : allPButtons)
            {
                auto controlName = control->objectName();

                WWidget* wWidget = qobject_cast<WWidget*>(control);
                if (wWidget)
                {
                    if (m_click)
                    {
                        m_commonMapBefore[control] = wWidget->value(isCommonValid);
                    }
                    else
                    {
                        m_commonMapAfter[control] = wWidget->value(isCommonValid);
                    }

                    if (!isCommonValid)
                    {
                        return;
                    }
                }
            }
            for (auto& grouBoxChild : multiGrouList)
            {
                //多目标参数状态更新
                if (grouBoxChild->title().trimmed() == MULTI_TARGET)
                {
                    QList<QTabWidget*> multiTabList = grouBoxChild->findChildren<QTabWidget*>();
                    for (auto& tabWidget : multiTabList)
                    {
                        //更新多目标参数
                        for (int j = 0; j < tabWidget->count(); j++)
                        {
                            QMap<WWidget*, QVariant> map;

                            //如果不可选中，就不更新数据
                            bool enable = tabWidget->isTabEnabled(j);
                            if (!enable)
                                continue;

                            auto multiWidget = tabWidget->widget(j);

                            auto widgetTargetKey = multiWidget->property("TargetKey").toInt();

                            QList<WWidget*> subAllControl = multiWidget->findChildren<WWidget*>();
                            for (auto control : subAllControl)
                            {
                                auto objName = control->objectName();

                                WWidget* wWidget = qobject_cast<WWidget*>(control);
                                if (wWidget)
                                {
                                    map[control] = wWidget->value(isMultiValid);
                                    if (!isMultiValid)
                                    {
                                        return;
                                    }
                                }
                            }
                            if (m_click)
                            {
                                m_multiMapBefore[widgetTargetKey] = map;
                            }
                            else
                            {
                                m_multiMapAfter[widgetTargetKey] = map;
                            }
                        }
                    }
                }
            }
        }
    }
}

void DeviceMonitor::comPareSetParaValue()
{
    //比较普通参数的值
    if (!m_commonMapBefore.isEmpty())
    {
        for (auto beforeKey : m_commonMapBefore.keys())
        {
            auto beforeValue = m_commonMapBefore[beforeKey];
            for (auto afterKey : m_commonMapAfter.keys())
            {
                auto afterValue = m_commonMapAfter[afterKey];
                if (beforeKey == afterKey)
                {
                    if (beforeValue != afterValue)
                    {
                        m_needUpdateColorControlCommon[afterKey] = afterValue;
                        setSettingPanelColor(afterKey, QColor(229, 153, 0));
                    }
                }
            }
        }
    }
    //比较一体化/扩频多目标的值
    if (!m_multiMapBefore.isEmpty())
    {
        for (auto recycleKey : m_multiMapBefore.keys())
        {
            auto beforeMap = m_multiMapBefore[recycleKey];
            if (m_multiMapAfter.contains(recycleKey))
            {
                auto afterMap = m_multiMapAfter[recycleKey];
                for (auto beforKey : beforeMap.keys())
                {
                    auto beforeValue = beforeMap[beforKey];
                    for (auto afterKey : afterMap.keys())
                    {
                        auto afterValue = afterMap[afterKey];
                        if (beforKey == afterKey)
                        {
                            if (beforeValue != afterValue)
                            {
                                m_needUpdateColorControlMulti[recycleKey][afterKey] = afterValue;
                                setSettingPanelColor(afterKey, QColor(229, 153, 0));
                            }
                        }
                    }
                }
            }
        }
    }
}

void DeviceMonitor::comPareWidgetSetValue()
{
    QColor color(0, 170, 0);
    if (m_unitWidgetIDMap.contains(m_currentScroArea))
    {
        auto unitID = m_unitWidgetIDMap[m_currentScroArea];
        for (auto statusUnitID : m_currentStatusReport.unitReportMsgMap.keys())
        {
            if (unitID == statusUnitID)
            {
                auto statusUnitData = m_currentStatusReport.unitReportMsgMap[statusUnitID];

                for (auto statusControlName : statusUnitData.paramMap.keys())
                {
                    auto value = statusUnitData.paramMap[statusControlName];
                    for (auto afterControl : m_needUpdateColorControlCommon.keys())
                    {
                        if (statusControlName == afterControl->objectName())
                        {
                            auto afterValue = m_needUpdateColorControlCommon[afterControl];
                            auto valueString = value.toString().toUpper();
                            auto afterValueString = afterValue.toString().toUpper();
                            valueString.remove(QRegExp(R"(^0+)"));       //去除前面多余的0
                            afterValueString.remove(QRegExp(R"(^0+)"));  //去除前面多余的0
                            if (value != afterValue)
                            {
                                if (valueString == afterValueString)
                                {
                                    setSettingPanelColor(afterControl, color);
                                }
                                else
                                {
                                    setSettingPanelColor(afterControl, QColor(Qt::red));
                                }
                            }
                            else
                            {
                                setSettingPanelColor(afterControl, color);
                            }
                        }
                    }
                }

                for (auto statusRecycle : statusUnitData.multiTargetParamMap.keys())
                {
                    if (m_needUpdateColorControlMulti.contains(statusRecycle))
                    {
                        auto statusControlMultiMap = statusUnitData.multiTargetParamMap[statusRecycle];
                        auto needUpdataControlMultiMap = m_needUpdateColorControlMulti[statusRecycle];
                        for (auto statusControlName : statusControlMultiMap.keys())
                        {
                            auto value = statusControlMultiMap[statusControlName];
                            for (auto afterControl : needUpdataControlMultiMap.keys())
                            {
                                if (statusControlName == afterControl->objectName())
                                {
                                    auto afterValue = needUpdataControlMultiMap[afterControl];
                                    auto valueString = value.toString().toUpper();
                                    auto afterValueString = afterValue.toString().toUpper();
                                    valueString.remove(QRegExp(R"(^0+)"));       //去除前面多余的0
                                    afterValueString.remove(QRegExp(R"(^0+)"));  //去除前面多余的0
                                    if (value != afterValue)
                                    {
                                        if (valueString == afterValueString)
                                        {
                                            setSettingPanelColor(afterControl, color);
                                        }
                                        else
                                        {
                                            setSettingPanelColor(afterControl, QColor(Qt::red));
                                        }
                                    }
                                    else
                                    {
                                        setSettingPanelColor(afterControl, color);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void DeviceMonitor::setSettingPanelColor(QWidget* control, QColor color)
{
    if (control != nullptr)
    {
        if (color.isValid())
        {
            control->setStyleSheet(
                QString(".QLabel[nameLabel=\"true\"] {color: rgb(%1, %2, %3);}").arg(color.red()).arg(color.green()).arg(color.blue()));
        }
    }
}

QString DeviceMonitor::findAttrGroupDesc(Unit unit, const QString& attrId)
{
    for (auto groupKey : unit.settingGroupMap.keys())
    {
        auto attrList = unit.settingGroupMap[groupKey];
        for (auto attr : attrList)
        {
            if (attr.id == attrId)
            {
                if (unit.unitLayoutMap.contains(groupKey))
                {
                    return unit.unitLayoutMap.value(groupKey).desc;
                }
            }
        }
    }
    return QString("");
}

void DeviceMonitor::matrixDiagramMenuTriggered(const int& unitCode, const QString& attrId, const int& uValue)
{
    if (attrId.isEmpty())
        return;

    QScrollArea* unitScrollWidget;

    unitScrollWidget = qobject_cast<QScrollArea*>(ui->unitTabWidget->currentWidget());

    if (!unitScrollWidget)
        return;

    auto allgroup = unitScrollWidget->findChildren<QGroupBox*>();
    for (auto child : allgroup)
    {
        auto groupBox = qobject_cast<QGroupBox*>(child);
        if (!groupBox)
            continue;

        if (SET_TITLE == groupBox->title().trimmed())
        {
            auto allPButtons = groupBox->findChildren<WWidget*>();
            for (auto& control : allPButtons)
            {
                // 在参数设置面板中找到该控件，设置值
                if (control->objectName() == attrId)
                {
                    control->setValue(uValue);
                    break;
                }
            }
        }
    }

    // 发送设置按钮点击信号
    emit m_settingBtnMap[unitCode]->clicked();
}

bool DeviceMonitor::checkParamRelation(QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    int systemId = m_currentDeviceID.sysID;
    int deviceId = m_currentDeviceID.devID;
    int extionId = m_currentDeviceID.extenID;
    int modeId = m_currentMode.ID;

    auto scrollArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->currentWidget());
    auto unitID = m_unitWidgetIDMap[scrollArea];

    bool ret = true;
    // 测控基带 标准TTC遥测模拟源单元
    if (systemId == 4 && deviceId == 0 && (extionId == 1 || extionId == 2 || extionId == 3 || extionId == 4) && unitID == 12 && modeId == 1)
    {
        ret = m_paramOther.checkParamRelationCkTtcTAS(setParamData, errorMsg);
    }
    // 测控基带 标准TTC遥测解调单元
    else if (systemId == 4 && deviceId == 0 && (extionId == 1 || extionId == 2 || extionId == 3 || extionId == 4) && unitID == 8 && modeId == 1)
    {
        ret = m_paramOther.checkParamRelationCkTtcTU(setParamData, errorMsg);
    }
    // 测控基带 标准TTC遥控单元
    else if (systemId == 4 && deviceId == 0 && (extionId == 1 || extionId == 2 || extionId == 3 || extionId == 4) && unitID == 7 && modeId == 1)
    {
        ret = m_paramOther.checkParamRelationCkTtcRCU(setParamData, errorMsg);
    }
    // 高速基带 I/Q路译码单元
    else if (systemId == 4 && deviceId == 1 && (extionId == 1 || extionId == 2) && (unitID == 3 || unitID == 4 || unitID == 7 || unitID == 8))
    {
        ret = m_paramOther.checkParamRelationGSRDU(setParamData, errorMsg);
    }
    // 高速基带 模拟源单元
    else if (systemId == 4 && deviceId == 1 && (extionId == 1 || extionId == 2) && unitID == 10)
    {
        ret = m_paramOther.checkParamRelationGSMNY(setParamData, errorMsg);
    }
    // 低速基带 Ka解调单元
    else if (systemId == 4 && deviceId == 4 && (extionId == 1 || extionId == 2) && (unitID == 4 || unitID == 5))
    {
        //这里是跨单元取值
        setParamData["tztz"] = getDeviceWidgetParamData(1, "CarrierModulation").toInt();   //调制体制
        setParamData["dataType"] = getDeviceWidgetParamData(1, "DataOutWay").toInt();      // IQ分路选择
        setParamData["frmLenI"] = getDeviceWidgetParamData(3, "FrameLenght").toInt();      // I路帧长
        setParamData["frmLenQ"] = getDeviceWidgetParamData(4, "FrameLenght").toInt();      // Q路帧长
        setParamData["bcZhfas"] = getDeviceWidgetParamData(1, "ParallelSCMode").toInt();   // 并串转换方式
        setParamData["powerIQ"] = getDeviceWidgetParamData(1, "IOPowerDB").toDouble();     // IQ功率比
        setParamData["codeSpeedI"] = getDeviceWidgetParamData(3, "CodeSpeed").toDouble();  // I路码速率
        setParamData["codeSpeedQ"] = getDeviceWidgetParamData(4, "CodeSpeed").toDouble();  // Q路码速率
        ret = m_paramOther.checkParamRelationDSJT(setParamData, errorMsg);
    }
    // 低速基带 X解调单元
    else if (systemId == 4 && deviceId == 4 && (extionId == 1 || extionId == 2) && (unitID == 6 || unitID == 7))
    {
        //这里是跨单元取值
        setParamData["tztz"] = getDeviceWidgetParamData(2, "CarrierModulation").toInt();   //调制体制
        setParamData["dataType"] = getDeviceWidgetParamData(2, "DataOutWay").toInt();      // IQ分路选择
        setParamData["frmLenI"] = getDeviceWidgetParamData(5, "FrameLenght").toInt();      // I路帧长
        setParamData["frmLenQ"] = getDeviceWidgetParamData(6, "FrameLenght").toInt();      // Q路帧长
        setParamData["bcZhfas"] = getDeviceWidgetParamData(2, "ParallelSCMode").toInt();   // 并串转换方式
        setParamData["powerIQ"] = getDeviceWidgetParamData(2, "IOPowerDB").toDouble();     // IQ功率比
        setParamData["codeSpeedI"] = getDeviceWidgetParamData(3, "CodeSpeed").toDouble();  // I路码速率
        setParamData["codeSpeedQ"] = getDeviceWidgetParamData(4, "CodeSpeed").toDouble();  // Q路码速率
        ret = m_paramOther.checkParamRelationDSJT(setParamData, errorMsg);
    }
    // 跟踪基带 标准+低速 扩频+低速  解调单元
    else if (systemId == 4 && deviceId == 2 && (unitID == 5 || unitID == 6) && (modeId == 10 || modeId == 11))
    {
        setParamData["tztz"] = getDeviceWidgetParamData(3, "CarrierModulation").toInt();  //调制体制
        ret = m_paramOther.checkParamRelationGZJT(setParamData, errorMsg);
    }
    // 低速基带 模拟源单元
    else if (systemId == 4 && deviceId == 4 && (extionId == 1 || extionId == 2) && (unitID == 8 || unitID == 9))
    {
        ret = m_paramOther.checkParamRelationDSMNY(setParamData, errorMsg);
    }
    // 联试应答机 标准TTC 遥测模拟源单元
    else if (systemId == 6 && deviceId == 0 && extionId == 1 && modeId == 1 && unitID == 4)
    {
        ret = m_paramOther.checkParamRelationLSTtcTAS(setParamData, errorMsg);
    }
    // 联试应答机  一体化上面级+扩二共载波\扩二不共载波、卫星+扩2 遥测模拟源单元
    else if ((systemId == 6 && deviceId == 0 && extionId == 1 && (modeId == 5 || modeId == 6) && unitID == 6) ||
             (systemId == 6 && deviceId == 0 && extionId == 1 && modeId == 7 && unitID == 6))
    {
        ret = m_paramOther.checkParamRelationYTHSMJK2GTtcTAS(setParamData, errorMsg);
    }
    // 测控基带 扩跳频 接收单元或者调制单元
    else if ((systemId == 4 && deviceId == 0 && (extionId == 1 || extionId == 2) && (unitID == 2 || unitID == 3) && modeId == 33) ||
             (systemId == 4 && deviceId == 2 && (extionId == 1 || extionId == 2) && unitID == 2 && modeId == 9) ||
             (systemId == 6 && deviceId == 0 && extionId == 2 && (unitID == 2 || unitID == 4) && modeId == 9))
    {
        ret = m_paramOther.checkParamRelationKTJSAndTZ(setParamData, errorMsg);
    }
    // 前端测试开关网络
    else if (systemId == 6 && deviceId == 3 && extionId == 3)
    {
        ret = m_paramOther.checkParamRelationBeforeTest(setParamData, errorMsg);
    }
    // 存储转发存储单元
    else if (systemId == 8 && deviceId == 1 && (unitID == 2 || unitID == 3 || unitID == 4 || unitID == 5))
    {
        //        ret = m_paramOther.checkParamCCZFRelationSU(setParamData, errorMsg);
    }
    // 时频终端
    else if (systemId == 5 && deviceId == 0 && unitID == 1)
    {
        ret = m_paramOther.checkParamSPZDRelationTFTU(setParamData, errorMsg);
    }
    return ret;
}

bool DeviceMonitor::checkParamRelation2(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg)
{
    int systemId = m_currentDeviceID.sysID;
    int deviceId = m_currentDeviceID.devID;
    int extionId = m_currentDeviceID.extenID;
    int modeId = m_currentMode.ID;

    auto scrollArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->currentWidget());
    auto unitID = m_unitWidgetIDMap[scrollArea];

    bool ret = true;

    // 测控基带 遥测解调单元
    if (systemId == 4 && deviceId == 0 && (extionId == 1 || extionId == 2 || extionId == 3 || extionId == 4) && unitID == 6 &&
        (modeId == 4 || modeId == 5))
    {
        ret = m_paramOther.checkParamRelationCkSKpTU(paraTargerMap, errorMsg);
    }
    // 测控基带 扩频遥测模拟源单元
    else if (systemId == 4 && deviceId == 0 && (extionId == 1 || extionId == 2 || extionId == 3 || extionId == 4) && unitID == 8 &&
             (modeId == 4 || modeId == 5))
    {
        ret = m_paramOther.checkParamRelationCkSKpTAS(paraTargerMap, errorMsg);
    }
    // 测控基带 扩频遥控单元
    else if (systemId == 4 && deviceId == 0 && (extionId == 1 || extionId == 2 || extionId == 3 || extionId == 4) && unitID == 5 &&
             (modeId == 4 || modeId == 5))
    {
        ret = m_paramOther.checkParamRelationCkSKpRCU(paraTargerMap, errorMsg);
    }
    // 测控基带 一体化上面级+扩二不共载波、一体化上面级+扩二共载波、上行码率+扩二、卫星+扩二 遥测模拟源
    else if (systemId == 4 && deviceId == 0 && (extionId == 1 || extionId == 2 || extionId == 3 || extionId == 4) && (unitID == 6 || unitID == 8) &&
             (modeId == 0x13 || modeId == 0x15 || modeId == 0x17 || modeId == 0x19))
    {
        ret = m_paramOther.checkParamRelationYTHSKpTUAndTAS(paraTargerMap, errorMsg);
    }
    // 测控基带 一体化卫星/卫星+扩二 遥控单元
    else if (systemId == 4 && deviceId == 0 && (extionId == 1 || extionId == 2 || extionId == 3 || extionId == 4) && unitID == 5 &&
             (modeId == 0x11 || modeId == 0x17))
    {
        ret = m_paramOther.checkParamRelationYTHWXRCU(paraTargerMap, errorMsg);
    }
    // 测控基带 一体化上面级+扩二不共载波、一体化上面级+扩二共载波、上行码率+扩二 遥控单元
    else if (systemId == 4 && deviceId == 0 && (extionId == 1 || extionId == 2 || extionId == 3 || extionId == 4) && unitID == 5 &&
             (modeId == 0x13 || modeId == 0x15 || modeId == 0x19))
    {
        ret = m_paramOther.checkParamRelationYTHRCU(paraTargerMap, errorMsg);
    }
    // 联试应答机终端 扩频模式 模拟源单元
    else if (systemId == 6 && deviceId == 0 && extionId == 1 && modeId == 2 && unitID == 6)
    {
        ret = m_paramOther.checkParamRelationLSKuoTAS(paraTargerMap, errorMsg);
    }
    return ret;
}

void DeviceMonitor::dealWithDSMSLNumRange(int unitID, const QString& objName)
{
    if (objName == "CarrierModulation" || objName == "CodeWay" || objName == "CodeType" || objName == "ViterbiDecoding" ||
        objName == "LDPCCodeRate" || objName == "TurboCodeRate" || objName == "CodeRateSett")
    {
        if (unitID == 2)  // Ka中频接收
        {
            auto tztzControlKa = getDeviceWidgetParamControl(1, "CarrierModulation");
            auto codeSpeedKaI = getDeviceWidgetParamControl(3, "CodeSpeed");
            auto codeSpeedKaQ = getDeviceWidgetParamControl(4, "CodeSpeed");
            QMap<QString, QVariant> setParamDataJS;
            QMap<QString, QVariant> setParamDataJTI;
            QMap<QString, QVariant> setParamDataJTQ;

            if (m_unitIDWidgetMap.contains(2))
            {
                auto scroArea = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[2]);  // Ka接收单元界面
                auto widget = scroArea->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJS, widget);
            }

            if (m_unitIDWidgetMap.contains(4) && m_unitIDWidgetMap.contains(5))
            {
                auto scroArea2 = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[4]);  // KaI解调单元界面
                auto widget2 = scroArea2->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJTI, widget2);

                auto scroArea3 = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[5]);  // KaQ解调单元界面
                auto widget3 = scroArea3->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJTQ, widget3);
            }

            m_widgetDeal.dealWidgetControlRelation(tztzControlKa, codeSpeedKaI, codeSpeedKaQ);
            m_widgetDeal.dealDSWidgetMSLNumRangeJT(codeSpeedKaI, setParamDataJS,
                                                   setParamDataJTI);  // KaI解调单元码速率控件设置范围
            m_widgetDeal.dealDSWidgetMSLNumRangeJT(codeSpeedKaQ, setParamDataJS,
                                                   setParamDataJTQ);  // KaQ解调单元码速率控件设置范围
        }
        else if (unitID == 3)  // X中频接收
        {
            auto tztzControlX = getDeviceWidgetParamControl(2, "CarrierModulation");
            auto codeSpeedKaI = getDeviceWidgetParamControl(5, "CodeSpeed");
            auto codeSpeedKaQ = getDeviceWidgetParamControl(6, "CodeSpeed");
            QMap<QString, QVariant> setParamDataJS;
            QMap<QString, QVariant> setParamDataJTI;
            QMap<QString, QVariant> setParamDataJTQ;

            if (m_unitIDWidgetMap.contains(3))
            {
                auto scroArea = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[3]);  // X接收单元界面
                auto widget = scroArea->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJS, widget);
            }

            if (m_unitIDWidgetMap.contains(6) && m_unitIDWidgetMap.contains(7))
            {
                auto scroArea2 = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[6]);  // XI解调单元界面
                auto widget2 = scroArea2->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJTI, widget2);

                auto scroArea3 = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[7]);  // XQ解调单元界面
                auto widget3 = scroArea3->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJTQ, widget3);
            }

            m_widgetDeal.dealWidgetControlRelation(tztzControlX, codeSpeedKaI, codeSpeedKaQ);
            m_widgetDeal.dealDSWidgetMSLNumRangeJT(codeSpeedKaI, setParamDataJS,
                                                   setParamDataJTI);  // KaI解调单元码速率控件设置范围
            m_widgetDeal.dealDSWidgetMSLNumRangeJT(codeSpeedKaQ, setParamDataJS,
                                                   setParamDataJTQ);  // KaQ解调单元码速率控件设置范围
        }
        else if (unitID == 4)  // KaI路解调
        {
            auto codeSpeedKaI = getDeviceWidgetParamControl(3, "CodeSpeed");  //注意这里的3是当前widget的index
            QMap<QString, QVariant> setParamDataJS;
            QMap<QString, QVariant> setParamDataJTI;

            if (m_unitIDWidgetMap.contains(2))
            {
                auto scroArea = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[2]);  // Ka接收单元界面
                auto widget = scroArea->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJS, widget);
            }

            if (m_unitIDWidgetMap.contains(4))
            {
                auto scroArea2 = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[4]);  // KaI解调单元界面
                auto widget2 = scroArea2->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJTI, widget2);
            }
            m_widgetDeal.dealDSWidgetMSLNumRangeJT(codeSpeedKaI, setParamDataJS,
                                                   setParamDataJTI);  // KaI解调单元码速率控件设置范围
        }
        else if (unitID == 5)  // KaQ路解调
        {
            auto codeSpeedKaQ = getDeviceWidgetParamControl(4, "CodeSpeed");  //注意这里的4是当前widget的index
            QMap<QString, QVariant> setParamDataJS;
            QMap<QString, QVariant> setParamDataJTQ;

            if (m_unitIDWidgetMap.contains(2))
            {
                auto scroArea = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[2]);  // Ka接收单元界面
                auto widget = scroArea->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJS, widget);
            }

            if (m_unitIDWidgetMap.contains(5))
            {
                auto scroArea3 = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[5]);  // KaQ解调单元界面
                auto widget3 = scroArea3->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJTQ, widget3);
            }
            m_widgetDeal.dealDSWidgetMSLNumRangeJT(codeSpeedKaQ, setParamDataJS,
                                                   setParamDataJTQ);  // KaQ解调单元码速率控件设置范围
        }
        else if (unitID == 6)  // XI路解调
        {
            auto codeSpeedKaI = getDeviceWidgetParamControl(5, "CodeSpeed");  //注意这里的5是当前widget的index
            QMap<QString, QVariant> setParamDataJS;
            QMap<QString, QVariant> setParamDataJTI;

            if (m_unitIDWidgetMap.contains(3))
            {
                auto scroArea = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[3]);  // X接收单元界面
                auto widget = scroArea->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJS, widget);
            }

            if (m_unitIDWidgetMap.contains(6))
            {
                auto scroArea3 = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[6]);  // KaQ解调单元界面
                auto widget3 = scroArea3->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJTI, widget3);
            }
            m_widgetDeal.dealDSWidgetMSLNumRangeJT(codeSpeedKaI, setParamDataJS,
                                                   setParamDataJTI);  // KaQ解调单元码速率控件设置范围
        }
        else if (unitID == 7)  // XQ路解调
        {
            auto codeSpeedKaQ = getDeviceWidgetParamControl(6, "CodeSpeed");  //注意这里的6是当前widget的index
            QMap<QString, QVariant> setParamDataJS;
            QMap<QString, QVariant> setParamDataJTQ;

            if (m_unitIDWidgetMap.contains(3))
            {
                auto scroArea = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[3]);  // X接收单元界面
                auto widget = scroArea->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJS, widget);
            }

            if (m_unitIDWidgetMap.contains(7))
            {
                auto scroArea3 = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[7]);  // XQ解调单元界面
                auto widget3 = scroArea3->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJTQ, widget3);
            }
            m_widgetDeal.dealDSWidgetMSLNumRangeJT(codeSpeedKaQ, setParamDataJS,
                                                   setParamDataJTQ);  // KaQ解调单元码速率控件设置范围
        }
    }
    else if (objName == "CarrierModulation" || objName == "ICodeWay" || objName == "ICodeType" || objName == "IViterbiDecoding" ||
             objName == "ILDPCCodeRate" || objName == "ITurboCodeRate" || objName == "QCodeWay" || objName == "QCodeType" ||
             objName == "QViterbiDecoding" || objName == "QLDPCCodeRate" || objName == "QTurboCodeRate")
    {
        if (unitID == 8 || unitID == 9)  // Ka/X模拟源单元
        {
            auto codeSpeedI = getDeviceWidgetParamControl(unitID - 1, "ICodeSpeed");
            auto codeSpeedQ = getDeviceWidgetParamControl(unitID - 1, "QCodeSpeed");
            QMap<QString, QVariant> setParamData;

            if (m_unitIDWidgetMap.contains(unitID))
            {
                auto scroArea = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[unitID]);  // Ka接收单元界面
                auto widget = scroArea->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamData, widget);
            }

            m_widgetDeal.dealDSWidgetMSLNumRangeMNY(codeSpeedI, codeSpeedQ,
                                                    setParamData);  // 模拟源单元码速率控件设置范围
        }
    }

    if (unitID == 11)  // 11是上行数传单元
    {
        if (objName == "CodeType" || objName == "DecodMode" || objName == "CodeRateChoose")
        {
            auto codeSpeed = getDeviceWidgetParamControl(10, "IChannRate");  //获得上行数传单元的码速率控件
            QMap<QString, QVariant> setParamDataSC;

            if (m_unitIDWidgetMap.contains(11))
            {
                auto scroArea3 = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[11]);  // 上行数传单元
                auto widget3 = scroArea3->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataSC, widget3);
            }

            m_widgetDeal.dealDSWidgetMSLNumRangeSC(codeSpeed,
                                                   setParamDataSC);  // 上行数传码速率控件设置范围
        }
    }
}

void DeviceMonitor::dealWithCKTTCRcuNumRange(int unitID, const QString& objName)
{
    if (objName == "SettBefafter" || objName == "ConvCodType")
    {
        auto codeSpeed = getDeviceWidgetParamControl(6, "CodeRate");  //获取码速率控件
        QMap<QString, QVariant> setParamData;

        if (m_unitIDWidgetMap.contains(unitID))
        {
            auto scroArea = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[unitID]);
            auto widget = scroArea->widget();
            m_widgetDeal.getSetParamMapOtherRelation(setParamData, widget);
        }

        m_widgetDeal.dealCkWidgetMSLNumRangeTTCRCU(codeSpeed, setParamData);
    }
}

void DeviceMonitor::dealWithGZMSLNumRange(int unitID, const QString& objName)
{
    if (objName == "CarrierModulation" || objName == "CodeType" || objName == "ViterbiDecoding" || objName == "LDPCDecoding" ||
        objName == "LDPCCodeRate" || objName == "TurboDecoding" || objName == "TurboCodeRate")
    {
        if (unitID == 4)
        {
            auto codeSpeedI = getDeviceWidgetParamControl(4, "CodeSpeed");
            auto codeSpeedQ = getDeviceWidgetParamControl(5, "CodeSpeed");
            QMap<QString, QVariant> setParamDataJS;
            QMap<QString, QVariant> setParamDataJTI;
            QMap<QString, QVariant> setParamDataJTQ;

            if (m_unitIDWidgetMap.contains(4))
            {
                auto scroArea = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[4]);  // 接收单元界面
                auto widget = scroArea->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJS, widget);
            }

            if (m_unitIDWidgetMap.contains(5) && m_unitIDWidgetMap.contains(6))
            {
                auto scroArea2 = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[5]);  // I解调单元界面
                auto widget2 = scroArea2->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJTI, widget2);

                auto scroArea3 = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[6]);  // Q解调单元界面
                auto widget3 = scroArea3->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJTQ, widget3);
            }

            m_widgetDeal.dealGZWidgetMSLNumRangeJT(codeSpeedI, setParamDataJS,
                                                   setParamDataJTI);  // I解调单元码速率控件设置范围
            m_widgetDeal.dealGZWidgetMSLNumRangeJT(codeSpeedQ, setParamDataJS,
                                                   setParamDataJTQ);  // Q解调单元码速率控件设置范围
        }
        else if (unitID == 5)
        {
            auto codeSpeedI = getDeviceWidgetParamControl(4, "CodeSpeed");  //注意这里的5是当前widget的index
            QMap<QString, QVariant> setParamDataJS;
            QMap<QString, QVariant> setParamDataJTI;

            if (m_unitIDWidgetMap.contains(4))
            {
                auto scroArea = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[4]);  // 接收单元界面
                auto widget = scroArea->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJS, widget);
            }

            if (m_unitIDWidgetMap.contains(5))
            {
                auto scroArea3 = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[5]);  // I解调单元界面
                auto widget3 = scroArea3->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJTI, widget3);
            }
            m_widgetDeal.dealGZWidgetMSLNumRangeJT(codeSpeedI, setParamDataJS,
                                                   setParamDataJTI);  // KaQ解调单元码速率控件设置范围
        }
        else if (unitID == 6)
        {
            auto codeSpeedQ = getDeviceWidgetParamControl(5, "CodeSpeed");  //注意这里的5是当前widget的index
            QMap<QString, QVariant> setParamDataJS;
            QMap<QString, QVariant> setParamDataJTQ;

            if (m_unitIDWidgetMap.contains(4))
            {
                auto scroArea = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[4]);  // 接收单元界面
                auto widget = scroArea->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJS, widget);
            }

            if (m_unitIDWidgetMap.contains(6))
            {
                auto scroArea3 = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[6]);  // I解调单元界面
                auto widget3 = scroArea3->widget();
                m_widgetDeal.getSetParamMapOtherRelation(setParamDataJTQ, widget3);
            }
            m_widgetDeal.dealGZWidgetMSLNumRangeJT(codeSpeedQ, setParamDataJS,
                                                   setParamDataJTQ);  // KaQ解调单元码速率控件设置范围
        }
    }
}

void DeviceMonitor::dealWithCKMultiRcuNumRange(int recycle, int unitID, int modeID, const QString& objName)
{
    if ((modeID != 1 && modeID != 0x11 && modeID != 0x21) && unitID == 5)  //扩频 卫星+扩二  上面级+扩二 等
    {
        if (objName == "K2_CodeSelect" || objName == "K2_ConvCodType")
        {
            auto controlMap = getDeviceMultiWidgetParamControl(4, "K2_CodeRate");
            QString trigerParam1 = "K2_CodeSelect";
            QString trigerParam2 = "K2_ConvCodType";
            QList<QMap<QString, QVariant>> setParamDataList;

            if (m_unitIDWidgetMap.contains(unitID))
            {
                auto scroArea = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[unitID]);
                auto widget = scroArea->widget();
                m_widgetDeal.getSetMultiTargetParamMapOther(setParamDataList, widget);
            }

            m_widgetDeal.dealCkWidgetMSLMultiNumRangeRCU(recycle, trigerParam1, trigerParam2, controlMap, setParamDataList);
        }
    }
    else if (modeID == 0x11 && unitID == 5)
    {
        if (objName == "WX_CodeSelect" || objName == "WX_ConvCodType")
        {
            auto controlMap = getDeviceMultiWidgetParamControl(4, "WX_CodeRate");
            QString trigerParam1 = "WX_CodeSelect";
            QString trigerParam2 = "WX_ConvCodType";
            QList<QMap<QString, QVariant>> setParamDataList;

            if (m_unitIDWidgetMap.contains(unitID))
            {
                auto scroArea = qobject_cast<QScrollArea*>(m_unitIDWidgetMap[unitID]);
                auto widget = scroArea->widget();
                m_widgetDeal.getSetMultiTargetParamMapOther(setParamDataList, widget);
            }

            m_widgetDeal.dealCkWidgetMSLMultiNumRangeRCU(recycle, trigerParam1, trigerParam2, controlMap, setParamDataList);
        }
    }
}

bool DeviceMonitor::checkParamCmdRelation(const QMap<QString, QVariant>& setParamData, int cmdID, QString& errorMsg)
{
    int systemId = m_currentDeviceID.sysID;
    int deviceId = m_currentDeviceID.devID;
    int extionId = m_currentDeviceID.extenID;

    bool ret = true;
    if (systemId == 8 && deviceId == 1 && (extionId == 1 || extionId == 2 || extionId == 3))
    {
        ret = m_paramOther.checkCmdParamCCZFRelation(setParamData, errorMsg, cmdID, m_currentDeviceID);
    }

    return ret;
}

void DeviceMonitor::on_switchsBtn_clicked()
{
    if (m_currentMode.cmdMap.isEmpty())
    {
        if (m_currentMode.ID != 65535 && m_currentMode.ID > 0)
        {
            QMessageBox::information(this, QString("提示"), QString("该设备该模式下没有命令"), QString("确定"));
        }
        else
        {
            QMessageBox::information(this, QString("提示"), QString("该设备没有命令"), QString("确定"));
        }
        return;
    }

    m_switchFlag = !m_switchFlag;
    if (m_switchFlag)
    {
        ui->stackedWidget->setCurrentWidget(ui->unitPages);
        ui->switchsBtn->setText("命令设置");
    }
    else
    {
        initCmdWidget();
        ui->stackedWidget->setCurrentWidget(ui->cmdPages);
        ui->switchsBtn->setText("状态监视");
    }
}

void DeviceMonitor::cmdCurrentIndexChanged(int index)
{
    Q_UNUSED(index);
    QComboBox* combobox = qobject_cast<QComboBox*>(sender());
    if (combobox)
    {
        auto cmdID = combobox->currentData().toInt();
        if (m_cmdIDWidgetMap.contains(cmdID))
        {
            if (m_cmdCurrGroupBox)
            {
                m_cmdCurrGroupBox->setStyleSheet(".QGroupBox,.MyGroupBox{font: 12pt;font-weight:bold; border:2px solid "
                                                 "#C0DCF2;border-radius:5px;margin-top:10px;margin-left:5px;}");  //还原上一个被选中的groupBox的样式表
            }
            QGroupBox* groupBox = qobject_cast<QGroupBox*>(m_cmdIDWidgetMap[cmdID]);
            if (groupBox)
            {
                m_cmdCurrGroupBox = groupBox;
                auto cmdscrollAreaBar = m_cmdscrollArea->verticalScrollBar();
                cmdscrollAreaBar->setValue(groupBox->y());
                groupBox->setStyleSheet(".QGroupBox,.MyGroupBox{font: 12pt;font-weight:bold; border:2px solid "
                                        "#000000;border-radius:5px;margin-top:10px;margin-left:5px;}");
            }
        }
    }
}

void DeviceMonitor::setDeviceReduction()
{
    for (auto i = 0; i < ui->unitTabWidget->count(); i++)
    {
        auto scrollArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->widget(i));
        if (m_unitWidgetIDMap.contains(scrollArea))
        {
            auto unitID = m_unitWidgetIDMap[scrollArea];
            auto unit = m_currentMode.unitMap.value(unitID);
            QList<QGroupBox*> allgroup = scrollArea->findChildren<QGroupBox*>();
            for (auto child : allgroup)
            {
                QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
                if (STATUS_TITLE == groupBox->title().trimmed() || SET_TITLE == groupBox->title().trimmed())
                {
                    QList<QGroupBox*> multiGrouList = groupBox->findChildren<QGroupBox*>();
                    //公共单元设置参数状态更新
                    QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();

                    for (auto& control : allPButtons)
                    {
                        auto controlName = control->objectName();
                        for (auto attr : unit.queryResultList)
                        {
                            QVariant value;
                            if (attr.id == controlName)
                            {
                                if (attr.minValue.indexOf("|") >= 0)  //双范围处理
                                {
                                    auto minValueList = attr.minValue.split("|");
                                    if (minValueList.size() > 0)
                                    {
                                        auto firstMinValue = minValueList.at(0);
                                        value = firstMinValue;  //全部设为最小值
                                    }
                                }
                                else if (attr.attributeType == AttributeType_Char ||
                                         (attr.minValue.isEmpty() && SET_TITLE == groupBox->title().trimmed()))  //字符
                                {
                                    value = "1";
                                }
                                else
                                {
                                    value = attr.minValue;  //全部设为最小值
                                }
                                WWidget* wWidget = qobject_cast<WWidget*>(control);
                                if (wWidget)
                                {
                                    wWidget->setValue(value);
                                }
                            }
                        }
                    }
                    for (auto& grouBoxChild : multiGrouList)
                    {
                        //多目标参数状态更新
                        if (grouBoxChild->title().trimmed() == MULTI_TARGET)
                        {
                            QList<QTabWidget*> multiTabList = grouBoxChild->findChildren<QTabWidget*>();
                            for (auto& tabWidget : multiTabList)
                            {
                                //更新多目标参数
                                for (int j = 0; j < tabWidget->count(); j++)
                                {
                                    auto multiWidget = tabWidget->widget(j);

                                    QList<WWidget*> subAllControl = multiWidget->findChildren<WWidget*>();
                                    for (auto control : subAllControl)
                                    {
                                        auto controlName = control->objectName();
                                        auto widgetTargetKey = multiWidget->property("TargetKey").toInt();

                                        auto data = unit.queryRecycleMap[widgetTargetKey];

                                        if (data.isEmpty())
                                            continue;

                                        for (auto attr : data)
                                        {
                                            QVariant value;
                                            if (attr.id == controlName)
                                            {
                                                if (attr.minValue.indexOf("|") >= 0)  //双范围处理
                                                {
                                                    auto minValueList = attr.minValue.split("|");
                                                    if (minValueList.size() > 0)
                                                    {
                                                        auto firstMinValue = minValueList.at(0);
                                                        value = firstMinValue;  //全部设为最小值
                                                    }
                                                }
                                                else if (attr.attributeType == AttributeType_Char ||
                                                         (attr.minValue.isEmpty() && SET_TITLE == groupBox->title().trimmed()))  //字符
                                                {
                                                    value = "1";
                                                }
                                                else
                                                {
                                                    value = attr.minValue;  //全部设为最小值
                                                }

                                                WWidget* wWidget = qobject_cast<WWidget*>(control);
                                                if (wWidget)
                                                {
                                                    wWidget->setValue(value);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

QVariant DeviceMonitor::getDeviceWidgetParamData(int widgetIndex, QString attrName)
{
    QScrollArea* scrollArea = nullptr;
    if (ui->unitTabWidget->widget(widgetIndex))
    {
        scrollArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->widget(widgetIndex));
    }

    if (m_unitWidgetIDMap.contains(scrollArea))
    {
        QVariant value;
        m_widgetDeal.getWidgetParamValue(attrName, value, scrollArea);
        return value;
    }

    return QVariant();
}

WWidget* DeviceMonitor::getDeviceWidgetParamControl(int widgetIndex, QString attrName)
{
    QScrollArea* scrollArea = nullptr;
    if (ui->unitTabWidget->widget(widgetIndex))
    {
        scrollArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->widget(widgetIndex));
    }

    if (m_unitWidgetIDMap.contains(scrollArea))
    {
        WWidget* control = m_widgetDeal.getWidgetParamControl(attrName, scrollArea);
        return control;
    }

    return nullptr;
}

QMap<int, WWidget*> DeviceMonitor::getDeviceMultiWidgetParamControl(int widgetIndex, QString attrName)
{
    QScrollArea* scrollArea = nullptr;
    QMap<int, WWidget*> map;
    if (ui->unitTabWidget->widget(widgetIndex))
    {
        scrollArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->widget(widgetIndex));
    }

    if (m_unitWidgetIDMap.contains(scrollArea))
    {
        map = m_widgetDeal.getMultiWidgetParamControl(attrName, scrollArea);
    }

    return map;
}

bool DeviceMonitor::packMacroUnitParamSet(QWidget* widget, int unitID, UnitParamRequest& unitParamRequest)
{
    auto currentWidget = widget;  // 从滚动区域取

    //    //设置参数
    if (!m_currentMode.unitMap.contains(unitID))
    {
        return false;
    }
    QList<ParameterAttribute> setParamList = m_currentMode.unitMap.value(unitID).parameterSetList;

    QMap<QString, QVariant> setParamData;
    if (!m_widgetDeal.getSetParamMapOther(setParamData, currentWidget))
    {
        QMessageBox::warning(this, "提示", "参数值不在合法范围", QString("确定"));
        return false;
    }

    int systemId = m_currentDeviceID.sysID;
    int deviceId = m_currentDeviceID.devID;
    int extionId = m_currentDeviceID.extenID;
    int modeId = m_currentMode.ID;

    unitParamRequest.m_systemNumb = systemId;
    unitParamRequest.m_deviceNumb = deviceId;
    unitParamRequest.m_extenNumb = extionId;

    unitParamRequest.m_modeID = modeId;
    unitParamRequest.m_unitID = unitID;

    if (!setParamList.isEmpty())
    {
        for (auto attr : setParamList)
        {
            if (setParamData.contains(attr.id))
            {
                attr.initValue = setParamData[attr.id];
            }
            unitParamRequest.m_paramdataMap[attr.id] = attr.initValue;  //不管是初始值还是界面值，都得发出去
        }
    }

    QMap<int, QList<ParameterAttribute>> setMultiParaRecycleMap = m_currentMode.unitMap.value(unitID).multiSettingMap;

    QList<QMap<QString, QVariant>> paraTargerMap;
    if (!m_widgetDeal.getSetMultiTargetParamMapOther(paraTargerMap, currentWidget))
    {
        QMessageBox::warning(this, "提示", "参数值不在合法范围", QString("确定"));
        return false;
    }

    Unit unit = m_currentMode.unitMap[unitID];

    //这里是获取扩频4目标时的有效标识
    if (!unit.kpValid.isEmpty())
    {
        unitParamRequest.m_validIdent = getDeviceWidgetParamData(0, unit.kpValid).toInt() + 1;
    }

    //这里的有效标识是因为跟踪基带一体+扩频模式又没有多目标又需要通道标识的情况下，基带数目固定为1，故有效标识设置为1
    switch (m_deviceID)
    {
    case 0x4201:
    {
        switch (m_currentMode.ID)
        {
        case 0x8: unitParamRequest.m_validIdent = 1; break;
        }
    }
    default: break;
    }

    if (!setMultiParaRecycleMap.isEmpty())
    {
        int i = 1;
        QList<ParameterAttribute> multiParaRecycleList = setMultiParaRecycleMap.begin().value();

        switch (m_deviceID)
        {
        case 0x4001:
        case 0x4002:
        {
            switch (m_currentMode.ID)
            {
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
            case 0x15:
            case 0x17:
            case 0x19:
            {
                //一体化组包(主要是针对有一体化+扩频模式的，其他一体化可以在这组包，也可以用下面的标准扩频4目标组包)
                for (auto key : setMultiParaRecycleMap.keys())
                {
                    auto data = setMultiParaRecycleMap[key];
                    for (auto currentData : paraTargerMap)
                    {
                        for (auto attr : data)
                        {
                            if (currentData.contains(attr.id))
                            {
                                attr.initValue = currentData[attr.id];
                                unitParamRequest.m_multiTargetParamMap[key].m_paramdataMap[attr.id] = attr.initValue;
                            }
                        }
                    }
                }
            }
            break;
            default:
            {
                //标准扩频4目标组包
                for (auto currentData : paraTargerMap)
                {
                    for (auto attr : multiParaRecycleList)
                    {
                        if (currentData.contains(attr.id))
                        {
                            attr.initValue = currentData[attr.id];
                        }
                        unitParamRequest.m_multiTargetParamMap[i].m_paramdataMap[attr.id] = attr.initValue;
                    }
                    i++;
                }
            }
            break;
            }
            break;
        }
        default:
        {
            //标准扩频4目标组包
            for (auto currentData : paraTargerMap)
            {
                for (auto attr : multiParaRecycleList)
                {
                    if (currentData.contains(attr.id))
                    {
                        attr.initValue = currentData[attr.id];
                    }
                    unitParamRequest.m_multiTargetParamMap[i].m_paramdataMap[attr.id] = attr.initValue;
                }
                i++;
            }
        }
        break;
        }
    }

    return true;
}

void DeviceMonitor::packAutoMapUnitParamSet(int unitID, Unit unit, QWidget* widget)
{
    DeviceAutoMapDealWith::instance().decideDeviceAutoMap(m_currentDeviceID, m_currentMode, m_currentStatusReport);
    int systemId = m_currentDeviceID.sysID;
    int deviceId = m_currentDeviceID.devID;
    int extionId = m_currentDeviceID.extenID;
    int modeId = m_currentMode.ID;

    UnitParamRequest unitParamRequest;
    unitParamRequest.m_systemNumb = systemId;
    unitParamRequest.m_deviceNumb = deviceId;
    unitParamRequest.m_extenNumb = extionId;

    unitParamRequest.m_modeID = modeId;
    unitParamRequest.m_unitID = unitID;

    if (modeId == 0x4)
    {
        //这里是获取扩频4目标时的有效标识
        if (!unit.kpValid.isEmpty())
        {
            unitParamRequest.m_validIdent = getDeviceWidgetParamData(0, unit.kpValid).toInt() + 1;
        }
    }
    else if (modeId == 0x10 || modeId == 0x11 || modeId == 0x12 || modeId == 0x13 || modeId == 0x15 || modeId == 0x17 ||
             modeId == 0x19)  //一体化所有模式
    {
        ////获取有效标识
        QList<QMap<QString, QVariant>> paraTargerMap;
        if (!m_widgetDeal.getSetMultiTargetParamMapYTH(paraTargerMap, unitParamRequest, widget))
        {
            QMessageBox::warning(this, "提示", "参数值不在合法范围", QString("确定"));
            return;
        }
    }

    unitParamRequest.m_paramdataMap = m_currentStatusReport.unitReportMsgMap[unitID].paramMap;
    for (auto recycle : m_currentStatusReport.unitReportMsgMap[unitID].multiTargetParamMap.keys())
    {
        unitParamRequest.m_multiTargetParamMap[recycle].m_paramdataMap = m_currentStatusReport.unitReportMsgMap[unitID].multiTargetParamMap[recycle];
    }

    JsonWriter writer;
    writer& unitParamRequest;

    emit signalsUnitDeviceJson(writer.GetString());

    m_unitChangeStatus[unitID] = true;
}

void DeviceMonitor::slotsExtenStatusReport(const ExtensionStatusReportMessage& statusReport)
{
    m_isOnline = 5;
    ui->isOnlineLabel->setText(QString("设备在线"));
    ui->isOnlineLabel->setStyleSheet("color:green;font-size:30px;");
    if (m_isShow)
    {
        m_currentStatusReport = statusReport;
        for (auto i = 0; i < ui->unitTabWidget->count(); ++i)
        {
            auto scrollArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->widget(i));
            if (m_unitWidgetIDMap.contains(scrollArea))
            {
                auto unitID = m_unitWidgetIDMap[scrollArea];
                QList<QGroupBox*> allgroup = scrollArea->findChildren<QGroupBox*>();
                for (auto child : allgroup)
                {
                    QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
                    //存储转发多中心数界面
                    if (m_currentDeviceID.sysID == 8 && m_currentDeviceID.devID == 1)
                    {
                        Unit unit = m_currentMode.unitMap.value(unitID);
                        auto statusUnitData = statusReport.unitReportMsgMap[unitID];
                        m_widgetDeal.dealWithDataStatusMultiWidgets(groupBox, unit, statusUnitData);
                    }
                    if (STATUS_TITLE == groupBox->title().trimmed() || SET_TITLE == groupBox->title().trimmed())
                    {
                        //在修改的时候状态也对应改变
                        if (!m_unitChangeStatus[unitID])
                        {
                            continue;
                        }
                        QList<QGroupBox*> multiGrouList = groupBox->findChildren<QGroupBox*>();

                        //无模式单元设置参数状态更新
                        QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();
                        for (auto statusUnitID : statusReport.unitReportMsgMap.keys())
                        {
                            if (unitID == statusUnitID)
                            {
                                auto statusUnitData = statusReport.unitReportMsgMap[statusUnitID];
                                //低速基带工作模式处理 只取第一个单元
                                if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 4 && m_currentProject == "4426" && unitID == 1)
                                {
                                    Unit unit = m_currentMode.unitMap.value(unitID);
                                    QString modeText = m_widgetDeal.getDSWidgetWorkModeText(unit, statusUnitData);
                                    ui->modesLabel->setText(modeText);
                                }
                                for (auto statusControlName : statusUnitData.paramMap.keys())
                                {
                                    for (auto& control : allPButtons)
                                    {
                                        auto controlName = control->objectName();

                                        // 矩阵特殊处理
                                        if (controlName == "WMatrixWidget")
                                        {
                                            auto matrix = qobject_cast<WMatrixWidget*>(control);
                                            if (matrix)
                                                matrix->setValue(statusUnitData.paramMap);
                                            continue;
                                        }

                                        if (statusControlName == controlName)
                                        {
                                            WWidget* wWidget = qobject_cast<WWidget*>(control);
                                            auto value = statusUnitData.paramMap[statusControlName];
                                            if (wWidget)
                                            {
                                                wWidget->setValue(value);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        for (auto& grouBoxChild : multiGrouList)
                        {
                            //多目标参数状态更新
                            if (grouBoxChild->title().trimmed() == MULTI_TARGET)
                            {
                                QList<QTabWidget*> multiTabList = grouBoxChild->findChildren<QTabWidget*>();
                                for (auto& tabWidget : multiTabList)
                                {
                                    auto recyleSize = statusReport.unitReportMsgMap[unitID].multiTargetParamMap.size();
                                    //更新多目标参数
                                    for (int j = 0; j < tabWidget->count() && j < recyleSize; j++)
                                    {
                                        //如果不可选中，就不更新数据
                                        bool enable = tabWidget->isTabEnabled(j);
                                        if (!enable)
                                            continue;

                                        auto multiWidget = tabWidget->widget(j);

                                        QList<WWidget*> subAllControl = multiWidget->findChildren<WWidget*>();
                                        for (auto control : subAllControl)
                                        {
                                            auto objName = control->objectName();
                                            auto widgetTargetKey = multiWidget->property("TargetKey").toInt();

                                            if (statusReport.unitReportMsgMap[unitID].multiTargetParamMap[widgetTargetKey].contains(objName))
                                            {
                                                auto value = statusReport.unitReportMsgMap[unitID].multiTargetParamMap[widgetTargetKey][objName];
                                                WWidget* wWidget = qobject_cast<WWidget*>(control);
                                                if (wWidget)
                                                {
                                                    wWidget->setValue(value);
                                                }
                                                else
                                                {
                                                    qDebug() << QString("多目标状态控件类型错误 ID ： ") << objName;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void DeviceMonitor::slotsControlCmdResponse(const ControlCmdResponse& cmdResponce)
{
    if (m_isShow)
    {
        if (cmdResponce.deviceID == m_currentDeviceID)
        {
            auto result = DevProtocolEnumHelper::controlCmdResponseToDescStr(cmdResponce.responseResult);
            QMessageBox::warning(this, QString("提示"), QString(result), QString("确定"));
        }

        if (cmdResponce.responseResult != ControlCmdResponseType::Success)
        {
            for (auto control : m_needUpdateColorControlCommon.keys())
            {
                setSettingPanelColor(control, QColor(Qt::red));
            }

            for (auto recycle : m_needUpdateColorControlMulti.keys())
            {
                auto controlMap = m_needUpdateColorControlMulti[recycle];
                for (auto controlMulti : controlMap.keys())
                {
                    setSettingPanelColor(controlMulti, QColor(Qt::red));
                }
            }
        }
    }
}

void DeviceMonitor::slotsCmdResult(const CmdResult& cmdResponce)
{
    auto cmdID = cmdResponce.m_cmdID;
    if (m_cmdIDWidgetMap.contains(cmdID))
    {
        auto widget = m_cmdIDWidgetMap[cmdID];
        CmdAttribute cmdAttribute = m_currentMode.cmdMap.value(cmdID);
        for (auto child : widget->children())
        {
            QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
            if (!groupBox)
            {
                continue;
            }
            //这里进行类型判断
            if (groupBox->title().trimmed() == STATUS_TITLE || groupBox->title().trimmed() == CMD_TEST || groupBox->title().trimmed() == CMD_FILENAME)
            {
                // 测控基带AGC曲线装订
                if ((m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && m_currentMode.ID == 1 && cmdAttribute.cmdNumb == 59) ||
                    (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && m_currentMode.ID != 1 && cmdAttribute.cmdNumb == 33))
                {
                    m_widgetDeal.dealWithAGCMultiWidgets(groupBox, cmdAttribute, cmdResponce);
                }
                QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();
                for (auto& control : allPButtons)
                {
                    auto objName = control->objectName();
                    if (objName == "WPlainTextEdit")
                    {
                        WPlainTextEdit* widget = qobject_cast<WPlainTextEdit*>(control);
                        if (widget)
                        {
                            int paramType = widget->getParamType();  //判断是动态参数还是普通参数的类型
                            if (paramType)                           // 0：普通参数   1：动态参数
                            {
                                widget->dealCmdMultiValue(cmdID, m_currentMode.ID, cmdResponce.m_multiParamMap, m_currentDeviceID);
                            }
                            else
                            {
                                widget->dealCmdValue(cmdID, m_currentMode.ID, cmdResponce.m_paramdataMap, m_currentDeviceID);
                            }
                        }
                    }
                    if (cmdResponce.m_paramdataMap.contains(objName))
                    {
                        auto value = cmdResponce.m_paramdataMap.value(objName);
                        WWidget* wWidget = qobject_cast<WWidget*>(control);
                        if (wWidget)
                        {
                            wWidget->setValue(value);
                        }
                    }
                }
            }
        }
    }
}

void DeviceMonitor::on_macroBtn_clicked()
{
    ParamMacroRequest paramMacro;
    paramMacro.m_systemNumb = m_currentDeviceID.sysID;
    paramMacro.m_deviceNumb = m_currentDeviceID.devID;
    paramMacro.m_extenNumb = m_currentDeviceID.extenID;
    paramMacro.m_modeID = m_currentMode.ID;
    for (int i = 0; i < ui->unitTabWidget->count(); ++i)
    {
        auto scrollArea = qobject_cast<QScrollArea*>(ui->unitTabWidget->widget(i));
        if (m_unitWidgetIDMap.contains(scrollArea))
        {
            auto unitID = m_unitWidgetIDMap[scrollArea];
            QList<QGroupBox*> allgroup = scrollArea->findChildren<QGroupBox*>();
            for (auto child : allgroup)
            {
                QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
                if (SET_TITLE == groupBox->title().trimmed())
                {
                    UnitParamRequest unitParamRequest;
                    auto widget = scrollArea->widget();
                    bool ret = packMacroUnitParamSet(widget, unitID, unitParamRequest);
                    if (ret)
                    {
                        paramMacro.m_validIdent = unitParamRequest.m_validIdent;
                        paramMacro.m_paramMacroInfoMap[unitID] = unitParamRequest;
                    }
                }
            }
        }
    }

    JsonWriter writer;
    writer& paramMacro;

    RedisHelper::getInstance().setData("参数宏监控发送数据", writer.GetQString());

    emit signalsParaMacroDeviceJson(writer.GetString());
}

void DeviceMonitor::slot_unitChange(int index)
{
    Q_UNUSED(index);
    //只要切换了单元，就将所有单元的状态上报flag置为true刷新界面，免得切换另一个单元时界面上的数据还没刷新
    //并且要把按钮全部置为初始状态
    for (auto unitID : m_changeBtnMap.keys())
    {
        auto changeBtn = m_changeBtnMap[unitID];
        if (changeBtn->text() == QString("取消"))
            changeBtn->click();
    }

    for (auto unitID : m_settingBtnMap.keys())
    {
        auto settingBtn = m_settingBtnMap[unitID];
        settingBtn->setEnabled(false);
    }

    for (auto unitID : m_changeBtnMapYTHCommon.keys())
    {
        auto changeBtn = m_changeBtnMapYTHCommon[unitID];
        if (changeBtn->text() == QString("取消"))
            changeBtn->click();
    }

    for (auto unitID : m_settingBtnMapYTHCommon.keys())
    {
        auto settingBtn = m_settingBtnMapYTHCommon[unitID];
        settingBtn->setEnabled(false);
    }

    for (auto unitID : m_changeBtnMapYTHMulti.keys())
    {
        auto changeBtn = m_changeBtnMapYTHMulti[unitID];
        if (changeBtn->text() == QString("取消"))
            changeBtn->click();
    }

    for (auto unitID : m_settingBtnMapYTHMulti.keys())
    {
        auto settingBtn = m_settingBtnMapYTHMulti[unitID];
        settingBtn->setEnabled(false);
    }

    for (auto item : m_unitChangeStatus.keys())
    {
        m_unitChangeStatus[item] = true;
    }
}

void DeviceMonitor::initPlainTextEdit()
{
    QPushButton* settingBtn = qobject_cast<QPushButton*>(sender());
    if (settingBtn)
    {
        QGroupBox* currentWidget = qobject_cast<QGroupBox*>(settingBtn->parent());
        if (currentWidget == nullptr)
            return;
        for (auto child : currentWidget->children())
        {
            QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
            if (!groupBox)
            {
                continue;
            }
            //这里进行类型判断
            if (groupBox->title().trimmed() == CMD_TEST || groupBox->title().trimmed() == CMD_FILENAME)
            {
                QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();
                for (auto& control : allPButtons)
                {
                    auto objName = control->objectName();
                    if (objName == "WPlainTextEdit")
                    {
                        WPlainTextEdit* widget = qobject_cast<WPlainTextEdit*>(control);
                        if (widget)
                            widget->initUI();
                        break;  //清除数据后立马结束循环
                    }
                }
            }
        }
    }
}
