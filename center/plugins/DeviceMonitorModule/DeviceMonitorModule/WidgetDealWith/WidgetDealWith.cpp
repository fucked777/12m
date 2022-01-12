#include "WidgetDealWith.h"

#include "ControlFactory.h"
#include <QFlowLayout.h>
#include <QGroupBox>
#include <QTabWidget>
#include <QVBoxLayout>

WidgetDealWith::WidgetDealWith(QWidget* parent)
    : QWidget(parent)
{
}

void WidgetDealWith::setTargetID(const QString& targetID) { m_targetID = targetID; }

QWidget* WidgetDealWith::initCmdStatusPlaintextWidget(int cmdID, int modeID, CmdAttribute cmdAttribute, DeviceID deviceID)
{
    int sysID = deviceID.sysID;
    int devID = deviceID.devID;
    int extentID = deviceID.extenID;

    QGroupBox* groupBox = new QGroupBox(CMD_TEST);
    QVBoxLayout* vLayout = new QVBoxLayout;
    QVector<QString> name;

    if (sysID == 4 && devID == 0 && modeID == 1 && cmdID == 51)
    {
        name.push_back(QString("(当前时间)"));
        name.push_back(QString("(存盘文件名)"));
        QWidget* control = ControlFactory::createPlainText(name, cmdAttribute);
        if (control)
        {
            vLayout->addWidget(control);
        }

        groupBox->setLayout(vLayout);
        return groupBox;
    }
    if (sysID == 4 && devID == 0 && modeID == 33 && cmdID == 8)
    {
        name.push_back(QString("(当前时间)"));
        name.push_back(QString("(存盘文件名)"));
        QWidget* control = ControlFactory::createPlainText(name, cmdAttribute);
        if (control)
        {
            vLayout->addWidget(control);
        }

        groupBox->setLayout(vLayout);
        return groupBox;
    }
    else if (sysID == 4 && devID == 0 && modeID != 1 && cmdID == 27)
    {
        name.push_back(QString("(当前时间)"));
        name.push_back(QString("(通道/目标)"));
        name.push_back(QString("(存盘文件名)"));
        QWidget* control = ControlFactory::createPlainText(name, cmdAttribute);
        if (control)
        {
            vLayout->addWidget(control);
        }

        groupBox->setLayout(vLayout);
        return groupBox;
    }
    else if ((sysID == 7 && devID == 0 && cmdID == 9) || (sysID == 7 && devID == 1 && cmdID == 9))
    {
        name.push_back(QString("(当前时间)"));
        name.push_back(QString("(AGC电平)"));
        QWidget* control = ControlFactory::createPlainText(name, cmdAttribute);
        if (control)
        {
            vLayout->addWidget(control);
        }

        groupBox->setLayout(vLayout);
        return groupBox;
    }
    else if ((sysID == 7 && devID == 0 && cmdID == 10) || (sysID == 7 && devID == 1 && cmdID == 10))
    {
        name.push_back(QString("(当前时间)"));
        name.push_back(QString("(时常数1ms)"));
        name.push_back(QString("(时常数10ms)"));
        name.push_back(QString("(时常数100ms)"));
        name.push_back(QString("(时常数1000ms)"));
        QWidget* control = ControlFactory::createPlainText(name, cmdAttribute);
        if (control)
        {
            vLayout->addWidget(control);
        }

        groupBox->setLayout(vLayout);
        return groupBox;
    }
    else if (((sysID == 6 && devID == 0 && extentID == 1) || (sysID == 6 && devID == 0 && extentID == 2)) && cmdID == 3)
    {
        QGroupBox* groupBoxMulti = new QGroupBox(CMD_FILENAME);
        QWidget* control = ControlFactory::createPlainText(name, cmdAttribute);
        if (control)
        {
            vLayout->addWidget(control);
        }

        groupBoxMulti->setLayout(vLayout);
        return groupBoxMulti;
    }
    else
    {
        return nullptr;
    }
}

QWidget* WidgetDealWith::initDataStatusMultiWidget(Unit unit, int centerNum)
{
    QGroupBox* groupBox = new QGroupBox(CENTERNUM_TITLE);
    QVBoxLayout* groupBoxLayout = new QVBoxLayout;
    QTabWidget* tableWidget = new QTabWidget;

    for (auto queryResultAttr : unit.queryResultList)
    {
        auto recycleKey = queryResultAttr.id;
        auto targetType = unit.targetData;
        QList<ParameterAttribute> queryRecycleAttrList;
        for (auto key : targetType.keys())
        {
            auto targetData = targetType[key];
            if (targetData.recycleNum == recycleKey)
            {
                queryRecycleAttrList = unit.queryRecycleMap[key];
            }
        }

        if (queryRecycleAttrList.isEmpty())
            continue;

        if (recycleKey == "CenterNum")
        {
            //循环当前目标,默认创建1个中心数
            for (int i = 0; i < centerNum; i++)
            {
                QWidget* currentWidget = new QWidget();
                QVBoxLayout* currentLayout = new QVBoxLayout;
                QFlowLayout* flowLayout = new QFlowLayout(6, 0, 10);
                QFlowLayout* subExt = new QFlowLayout();
                for (auto attr : queryRecycleAttrList)
                {
                    if (attr.isShow)
                    {
                        QMap<QString, QVariant> enumMap;
                        if (attr.displayFormat == DisplayFormat_Enum)
                        {
                            getEnumData(attr.enumType, unit, enumMap);
                        }
                        else if (attr.displayFormat == DisplayFormat_LED)
                        {
                            getEnumData(attr.enumType, unit, enumMap);
                        }
                        auto control = ControlFactory::createStatusControl(attr, enumMap);
                        if (control)
                        {
                            flowLayout->addWidget(control);
                        }
                    }
                    //添加下面
                    if (attr.id == "CenterKnowVirChannelNum")
                    {
                        int subExtSize = attr.initValue.toInt();
                        QList<ParameterAttribute> centerRecycleAttrList;
                        for (auto key : targetType.keys())
                        {
                            auto targetData = targetType[key];
                            if (targetData.recycleNum == attr.id)
                            {
                                centerRecycleAttrList = unit.queryRecycleMap[key];
                            }
                        }
                        for (auto ii = 0; ii < subExtSize; ii++)
                        {
                            for (auto subExtItem : centerRecycleAttrList)
                            {
                                if (subExtItem.isShow)
                                {
                                    QMap<QString, QVariant> enumMaps;
                                    if (subExtItem.displayFormat == DisplayFormat_Enum)
                                    {
                                        getEnumData(subExtItem.enumType, unit, enumMaps);
                                    }
                                    else if (attr.displayFormat == DisplayFormat_LED)
                                    {
                                        getEnumData(subExtItem.enumType, unit, enumMaps);
                                    }
                                    subExtItem.desc = subExtItem.desc + QString::number(ii + 1);
                                    auto controls = ControlFactory::createStatusControl(subExtItem, enumMaps);
                                    if (controls)
                                    {
                                        subExt->addWidget(controls);
                                    }
                                }
                            }
                        }
                    }
                }
                currentLayout->addItem(flowLayout);
                currentLayout->addItem(subExt);
                currentWidget->setLayout(currentLayout);
                tableWidget->addTab(currentWidget, QString("中心%1").arg(i + 1));
                // qDebug() << QString("目标%1").arg(i);
            }

            groupBoxLayout->addWidget(tableWidget);
            break;
        }
    }
    groupBox->setLayout(groupBoxLayout);
    return groupBox;
}

QHBoxLayout* WidgetDealWith::initMultiWidgetStatusHLayout(CmdAttribute cmdAttribute)
{
    // 测控基带AGC曲线装订
    if ((m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && m_currentMode.ID == 1 && cmdAttribute.cmdNumb == 59) ||
        (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && m_currentMode.ID != 1 && cmdAttribute.cmdNumb == 33) ||
        (m_currentDeviceID.sysID == 1 && m_currentDeviceID.devID == 0 && (cmdAttribute.cmdNumb == 19 || cmdAttribute.cmdNumb == 20)))
    {
        QHBoxLayout* hboxLayout = new QHBoxLayout();
        // AGC
        auto agcSetMultiWidget = initCmdMulitiSetWidgets(cmdAttribute, CMD_SET_MULTI_AGC);
        if (agcSetMultiWidget)
            hboxLayout->addWidget(agcSetMultiWidget);

        return hboxLayout;
    }

    return nullptr;
}

QWidget* WidgetDealWith::initCmdMulitiStatusWidgets(CmdAttribute cmdAttribute, const QString& name)
{
    if (cmdAttribute.resultRecycle.isEmpty())
    {
        return nullptr;
    }
    QGroupBox* groubox = new QGroupBox(name);
    QFlowLayout* flowLayout = new QFlowLayout(4, 0, 5);

    groubox->setLayout(flowLayout);

    return groubox;
}

void WidgetDealWith::dealWithAGCMultiWidgets(QWidget* widget, CmdAttribute cmdAttribute, const CmdResult& cmdResponce)
{
    auto pointValue = cmdResponce.m_paramdataMap.value("PointCurve").toInt();
    for (auto child : widget->children())
    {
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
        if (!groupBox)
        {
            continue;
        }
        if (groupBox->title().trimmed() == CMD_SET_MULTI_AGC)
        {
            QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();
            auto controlSize = allPButtons.size();
            auto resultControlPairList = cmdResponce.m_multiParamMap[1];  // AGC只有一个recycle，且id是1 可以直接取
            // pointValue * 2原因是一个点包括两个值AGC和S/Φ，相当于两个控件
            if (pointValue * 2 == controlSize && pointValue * 2 > 0)  //比较上报的所有agc点数数量和界面上控件的数量对比  相等就只更新值
            {
                for (auto i = 0; i < controlSize; ++i)
                {
                    auto control = allPButtons.at(i);
                    WWidget* wWidget = qobject_cast<WWidget*>(control);
                    auto value = resultControlPairList.at(i).second;  //界面上的控件顺序和上报的值顺序是一样的，所以可以直接按顺序赋值
                    if (wWidget)
                        wWidget->setValue(value);
                }
            }
            else if (pointValue * 2 == 0)
            {
                continue;
            }
            else
            {
                QFlowLayout* flowLayout = (QFlowLayout*)groupBox->layout();
                QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();
                for (int i = 0; i < allPButtons.size(); ++i)
                {
                    delete allPButtons[i];
                    allPButtons[i] = nullptr;
                }
                allPButtons.clear();

                QList<ParameterAttribute> cmdValueList = cmdAttribute.requestRecycle.value(1);  //这里的1是recycle的id
                int flag = 0;
                for (int i = 0; i < pointValue; ++i)
                {
                    for (auto cmdValue : cmdValueList)
                    {
                        if (cmdValue.isShow)
                        {
                            QMap<QString, QVariant> enumMap;
                            if (cmdValue.displayFormat == DisplayFormat_Enum)
                            {
                                getCmdEnumData(cmdValue.enumType, enumMap);
                            }
                            else if (cmdValue.displayFormat == DisplayFormat_LED)
                            {
                                getCmdEnumData(cmdValue.enumType, enumMap);
                            }
                            cmdValue.desc = cmdValue.desc + QString::number(i + 1);
                            auto value = resultControlPairList.at(flag++).second;  //界面上的控件顺序和上报的值顺序是一样的，所以可以直接按顺序赋值
                            cmdValue.initValue = value;
                            auto control = ControlFactory::createStatusControl(cmdValue, enumMap);
                            if (control)
                                flowLayout->addWidget(control);
                        }
                    }
                }
            }
        }
    }
}

QHBoxLayout* WidgetDealWith::initMultiWidgetSetsHLayout(CmdAttribute cmdAttribute)
{
    //存储转发虚拟信道和挑帧个数界面
    if (m_currentDeviceID.sysID == 8 && m_currentDeviceID.devID == 1 && cmdAttribute.cmdNumb == 8)
    {
        QHBoxLayout* hboxLayout = new QHBoxLayout();
        //虚拟信道
        auto virSetMultiWidget = initCmdMulitiSetWidgets(cmdAttribute, CMD_SET_MULTI_VIRTITLE);
        if (virSetMultiWidget)
            hboxLayout->addWidget(virSetMultiWidget);
        //挑帧个数
        auto frameSetMultiWidget = initCmdMulitiSetWidgets(cmdAttribute, CMD_SET_MULTI_FRAMETITLE);
        if (frameSetMultiWidget)
            hboxLayout->addWidget(frameSetMultiWidget);

        return hboxLayout;
    }
    // DTE基带数量界面
    else if (m_currentDeviceID.sysID == 8 && m_currentDeviceID.devID == 0 && cmdAttribute.cmdNumb == 9)
    {
        QHBoxLayout* hboxLayout = new QHBoxLayout();
        //基带数量
        auto jdSetMultiWidget = initCmdMulitiSetWidgets(cmdAttribute, CMD_SET_MULTI_JDNUM);
        if (jdSetMultiWidget)
            hboxLayout->addWidget(jdSetMultiWidget);

        return hboxLayout;
    }
    // 测控基带AGC曲线装订
    else if ((m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && m_currentMode.ID == 1 && cmdAttribute.cmdNumb == 59) ||
             (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && m_currentMode.ID != 1 && cmdAttribute.cmdNumb == 33) ||
             (m_currentDeviceID.sysID == 1 && m_currentDeviceID.devID == 0 && cmdAttribute.cmdNumb == 19))
    {
        QHBoxLayout* hboxLayout = new QHBoxLayout();
        // AGC
        auto adcSetMultiWidget = initCmdMulitiSetWidgets(cmdAttribute, CMD_SET_MULTI_AGC);
        if (adcSetMultiWidget)
            hboxLayout->addWidget(adcSetMultiWidget);

        return hboxLayout;
    }
    // 4426ACU的AGC曲线装订  这个比较特殊，要先创建60组(因为下发长度固定60组参数的长度),然后再隐藏掉，再通过修改值显示参数
    else if (m_currentDeviceID.sysID == 1 && m_currentDeviceID.devID == 0 && cmdAttribute.cmdNumb == 20)
    {
        QHBoxLayout* hboxLayout = new QHBoxLayout();
        // AGC
        auto adcSetMultiWidget = initCmdMulitiACUAGCSetWidgets(cmdAttribute, CMD_SET_MULTI_AGC);
        if (adcSetMultiWidget)
            hboxLayout->addWidget(adcSetMultiWidget);

        return hboxLayout;
    }

    return nullptr;
}

QWidget* WidgetDealWith::initCmdMulitiSetWidgets(CmdAttribute cmdAttribute, const QString& name)
{
    if (cmdAttribute.requestRecycle.isEmpty())
    {
        return nullptr;
    }
    QGroupBox* groubox = new QGroupBox(name);
    QFlowLayout* flowLayout = new QFlowLayout(4, 0, 5);

    groubox->setLayout(flowLayout);

    return groubox;
}

QWidget* WidgetDealWith::initCmdMulitiACUAGCSetWidgets(CmdAttribute cmdAttribute, const QString& name)
{
    m_tfsAGCWidget.clear();
    /* 天线里面的agc曲线装订的指令是固定长度60组数据,不管当前使用多少组都是发60组 */
    int len = -1;
    for (auto& item : cmdAttribute.requestList)
    {
        if (item.id == "PointCurve")
        {
            len = item.maxValue.toInt();
            break;
        }
    }
    if (cmdAttribute.requestRecycle.size() == 0 || len <= 0)
    {
        return nullptr;
    }

    QGroupBox* groubox = new QGroupBox(name);
    auto flowLayout = new QFlowLayout(4, 0, 5);

    for (int i = 0; i < len; ++i)
    {
        QList<ParameterAttribute> cmdValueList = cmdAttribute.requestRecycle.value(1);  //这里的1是recycle的id

        for (auto cmdValue : cmdValueList)
        {
            if (cmdValue.isShow)
            {
                QMap<QString, QVariant> enumMap;
                if (cmdValue.displayFormat == DisplayFormat_Enum)
                {
                    getCmdEnumData(cmdValue.enumType, enumMap);
                }
                else if (cmdValue.displayFormat == DisplayFormat_LED)
                {
                    getCmdEnumData(cmdValue.enumType, enumMap);
                }
                cmdValue.desc = cmdValue.desc + QString::number(i + 1);
                auto control = ControlFactory::createSetControl(cmdValue, enumMap);
                if (control)
                {
                    control->setVisible(false);
                    m_tfsAGCWidget.append(control);
                    flowLayout->addWidget(control);
                }
            }
        }
    }

    groubox->setLayout(flowLayout);

    return groubox;
}

void WidgetDealWith::dealCmdMulitiSetsWidgets(const QString& objName, QVariant value, CmdAttribute cmdAttribute, QMap<int, QWidget*> cmdIDWidgetMap)
{
    auto widget = cmdIDWidgetMap[cmdAttribute.cmdNumb];
    if (objName == "KnowVirChannelNum")  //虚拟信道
    {
        dealCmdMulitiSetVirWidgets(objName, value, widget, cmdAttribute);
    }
    else if (objName == "PickFrameNumber")  //挑帧个数
    {
        dealCmdMulitiSetFrameWidgets(objName, value, widget, cmdAttribute);
    }
    else if (objName == "BbeNum")  //基带数量
    {
        dealCmdMulitiSetJDNumWidgets(objName, value, widget, cmdAttribute);
    }
    else if (objName == "PointCurve")  // AGC曲线点数
    {
        // ACU处理AGC  暂只有4426
        if (m_currentDeviceID.sysID == 1 && cmdAttribute.cmdNumb == 20)
        {
            dealCmdMulitiSetACUAGCWidgets(objName, value, widget, cmdAttribute);
        }
        //测控基带处理AGC
        else
        {
            dealCmdMulitiSetAGCWidgets(objName, value, widget, cmdAttribute);
        }
    }
}

void WidgetDealWith::dealCmdMulitiSetVirWidgets(const QString& objName, QVariant value, QWidget* widget, CmdAttribute cmdAttribute)
{
    for (auto child : widget->children())
    {
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
        if (!groupBox)
        {
            continue;
        }
        if (groupBox->title().trimmed() == SET_TITLE)
        {
            for (auto grouBoxChild : groupBox->children())
            {
                QGroupBox* groupBoxs = qobject_cast<QGroupBox*>(grouBoxChild);
                if (!groupBoxs)
                {
                    continue;
                }
                if (groupBoxs->title().trimmed() == CMD_SET_MULTI_VIRTITLE)
                {
                    QFlowLayout* flowVirLayout = (QFlowLayout*)groupBoxs->layout();
                    QList<WWidget*> allPButtons = groupBoxs->findChildren<WWidget*>();
                    for (int i = 0; i < allPButtons.size(); ++i)
                    {
                        delete allPButtons[i];
                        allPButtons[i] = nullptr;
                    }
                    allPButtons.clear();

                    QList<ParameterAttribute> cmdValues = cmdAttribute.requestRecycle.value(1);  //这里的1是recycle的id
                    for (int i = 0; i < value.toInt(); ++i)
                    {
                        for (auto cmdValue : cmdValues)
                        {
                            if (cmdValue.isShow)
                            {
                                QMap<QString, QVariant> enumMap;
                                if (cmdValue.displayFormat == DisplayFormat_Enum)
                                {
                                    getCmdEnumData(cmdValue.enumType, enumMap);
                                }
                                else if (cmdValue.displayFormat == DisplayFormat_LED)
                                {
                                    getCmdEnumData(cmdValue.enumType, enumMap);
                                }
                                cmdValue.desc = cmdValue.desc + QString::number(i + 1);
                                auto control = ControlFactory::createSetControl(cmdValue, enumMap);
                                if (control)
                                    flowVirLayout->addWidget(control);
                            }
                        }
                    }
                }
            }
        }
    }
}

void WidgetDealWith::dealCmdMulitiSetFrameWidgets(const QString& objName, QVariant value, QWidget* widget, CmdAttribute cmdAttribute)
{
    for (auto child : widget->children())
    {
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
        if (!groupBox)
        {
            continue;
        }
        if (groupBox->title().trimmed() == SET_TITLE)
        {
            for (auto grouBoxChild : groupBox->children())
            {
                QGroupBox* groupBoxs = qobject_cast<QGroupBox*>(grouBoxChild);
                if (!groupBoxs)
                {
                    continue;
                }
                if (groupBoxs->title().trimmed() == CMD_SET_MULTI_FRAMETITLE)
                {
                    QFlowLayout* flowVirLayout = (QFlowLayout*)groupBoxs->layout();
                    QList<WWidget*> allPButtons = groupBoxs->findChildren<WWidget*>();
                    for (int i = 0; i < allPButtons.size(); ++i)
                    {
                        delete allPButtons[i];
                        allPButtons[i] = nullptr;
                    }
                    allPButtons.clear();

                    QList<ParameterAttribute> cmdValues = cmdAttribute.requestRecycle.value(2);  //这里的2是recycle的id
                    for (int i = 0; i < value.toInt(); ++i)
                    {
                        for (auto cmdValue : cmdValues)
                        {
                            if (cmdValue.isShow)
                            {
                                QMap<QString, QVariant> enumMap;
                                if (cmdValue.displayFormat == DisplayFormat_Enum)
                                {
                                    getCmdEnumData(cmdValue.enumType, enumMap);
                                }
                                else if (cmdValue.displayFormat == DisplayFormat_LED)
                                {
                                    getCmdEnumData(cmdValue.enumType, enumMap);
                                }
                                cmdValue.desc = cmdValue.desc + QString::number(i + 1);
                                auto control = ControlFactory::createSetControl(cmdValue, enumMap);
                                if (control)
                                    flowVirLayout->addWidget(control);
                            }
                        }
                    }
                }
            }
        }
    }
}

void WidgetDealWith::dealCmdMulitiSetJDNumWidgets(const QString& objName, QVariant value, QWidget* widget, CmdAttribute cmdAttribute)
{
    for (auto child : widget->children())
    {
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
        if (!groupBox)
        {
            continue;
        }
        if (groupBox->title().trimmed() == SET_TITLE)
        {
            for (auto grouBoxChild : groupBox->children())
            {
                QGroupBox* groupBoxs = qobject_cast<QGroupBox*>(grouBoxChild);
                if (!groupBoxs)
                {
                    continue;
                }
                if (groupBoxs->title().trimmed() == CMD_SET_MULTI_JDNUM)
                {
                    QFlowLayout* flowLayout = (QFlowLayout*)groupBoxs->layout();
                    QList<WWidget*> allPButtons = groupBoxs->findChildren<WWidget*>();
                    for (int i = 0; i < allPButtons.size(); ++i)
                    {
                        delete allPButtons[i];
                        allPButtons[i] = nullptr;
                    }
                    allPButtons.clear();

                    QList<ParameterAttribute> cmdValues = cmdAttribute.requestRecycle.value(1);  //这里的1是recycle的id
                    for (int i = 0; i < value.toInt(); ++i)
                    {
                        for (auto cmdValue : cmdValues)
                        {
                            if (cmdValue.isShow)
                            {
                                QMap<QString, QVariant> enumMap;
                                if (cmdValue.displayFormat == DisplayFormat_Enum)
                                {
                                    getCmdEnumData(cmdValue.enumType, enumMap);
                                }
                                else if (cmdValue.displayFormat == DisplayFormat_LED)
                                {
                                    getCmdEnumData(cmdValue.enumType, enumMap);
                                }
                                cmdValue.desc = cmdValue.desc + QString::number(i + 1);
                                auto control = ControlFactory::createSetControl(cmdValue, enumMap);
                                if (control)
                                    flowLayout->addWidget(control);
                            }
                        }
                    }
                }
            }
        }
    }
}

void WidgetDealWith::dealCmdMulitiSetAGCWidgets(const QString& objName, QVariant value, QWidget* widget, CmdAttribute cmdAttribute)
{
    for (auto child : widget->children())
    {
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
        if (!groupBox)
        {
            continue;
        }
        if (groupBox->title().trimmed() == SET_TITLE)
        {
            for (auto grouBoxChild : groupBox->children())
            {
                QGroupBox* groupBoxs = qobject_cast<QGroupBox*>(grouBoxChild);
                if (!groupBoxs)
                {
                    continue;
                }
                if (groupBoxs->title().trimmed() == CMD_SET_MULTI_AGC)
                {
                    QFlowLayout* flowLayout = (QFlowLayout*)groupBoxs->layout();
                    QList<WWidget*> allPButtons = groupBoxs->findChildren<WWidget*>();
                    for (int i = 0; i < allPButtons.size(); ++i)
                    {
                        delete allPButtons[i];
                        allPButtons[i] = nullptr;
                    }
                    allPButtons.clear();

                    QList<ParameterAttribute> cmdValueList = cmdAttribute.requestRecycle.value(1);  //这里的1是recycle的id
                    for (int i = 0; i < value.toInt(); ++i)
                    {
                        for (auto cmdValue : cmdValueList)
                        {
                            if (cmdValue.isShow)
                            {
                                QMap<QString, QVariant> enumMap;
                                if (cmdValue.displayFormat == DisplayFormat_Enum)
                                {
                                    getCmdEnumData(cmdValue.enumType, enumMap);
                                }
                                else if (cmdValue.displayFormat == DisplayFormat_LED)
                                {
                                    getCmdEnumData(cmdValue.enumType, enumMap);
                                }
                                cmdValue.desc = cmdValue.desc + QString::number(i + 1);
                                auto control = ControlFactory::createSetControl(cmdValue, enumMap);
                                if (control)
                                    flowLayout->addWidget(control);
                            }
                        }
                    }
                }
            }
        }
    }
}

void WidgetDealWith::dealCmdMulitiSetACUAGCWidgets(const QString& objName, QVariant value, QWidget* widget, CmdAttribute cmdAttribute)
{
    /* 天线里面的agc曲线装订的指令是固定长度60组数据,不管当前使用多少组都是发60组 */
    int len = -1;
    for (auto& item : cmdAttribute.requestList)
    {
        if (item.id == "PointCurve")
        {
            len = item.maxValue.toInt();
            break;
        }
    }

    auto curLen = value.toInt();
    if (len < 0 || curLen <= 0 || curLen > len)
    {
        return;
    }

    if (m_tfsAGCWidget.isEmpty())
    {
        return;
    }
    //*2是因为m_tfsAGCWidget里面存的控件是一组组存的，每组两个控件
    for (int i = 0; i < curLen * 2; ++i)
    {
        auto widget = m_tfsAGCWidget.at(i);
        if (widget != nullptr)
        {
            widget->setVisible(true);
        }
    }
    for (int i = curLen * 2; i < len * 2; ++i)
    {
        auto widget = m_tfsAGCWidget.at(i);
        if (widget != nullptr)
        {
            widget->setVisible(false);
        }
    }
}

void WidgetDealWith::dealWithDataStatusMultiWidgets(QGroupBox* groupBox, Unit unit, UnitReportMessage reportMessage)
{
    auto centerNum = reportMessage.paramMap.value("CenterNum").toInt();
    if (centerNum == 0)
    {
        return;
    }
    if (groupBox->title().trimmed() == CENTERNUM_TITLE)
    {
        QList<QTabWidget*> multiTabList = groupBox->findChildren<QTabWidget*>();

        for (auto& tabWidget : multiTabList)
        {
            auto targetType = unit.targetData;
            int targetID1;
            int targetID2;
            QList<ParameterAttribute> firstRecycleAttrList;
            QList<ParameterAttribute> secondRecycleAttrList;
            QList<QMap<QString, QVariant>> firstDataList;
            QList<QMap<QString, QVariant>> secondDataList;
            for (auto key : targetType.keys())
            {
                auto targetData = targetType[key];
                if (targetData.recycleNum == "CenterNum")
                {
                    firstRecycleAttrList = unit.queryRecycleMap[key];
                    firstDataList = reportMessage.multiTargetParamMap2[key];
                    targetID1 = key;
                }
                else if (targetData.recycleNum == "CenterKnowVirChannelNum")
                {
                    secondRecycleAttrList = unit.queryRecycleMap[key];
                    targetID2 = key;
                }
            }

            if (reportMessage.multiTargetParamMap3.contains(targetID1))
            {
                if (reportMessage.multiTargetParamMap3[targetID1].contains(targetID2))
                    secondDataList = reportMessage.multiTargetParamMap3[targetID1][targetID2];
            }
            if (centerNum == tabWidget->count() && centerNum > 0)
            {
                for (int i = 0; i < centerNum; ++i)
                {
                    auto widget = tabWidget->widget(i);
                    QList<WWidget*> subControlList = widget->findChildren<WWidget*>();
                    //一个中心数据
                    QMap<QString, QVariant> centerDataMap;
                    if (firstDataList.size() >= i)
                        centerDataMap = firstDataList.at(i);

                    auto virChannelNum = centerDataMap.value("CenterKnowVirChannelNum").toInt();
                    int secondControlNum = 0;
                    for (auto control : subControlList)
                    {
                        auto objName = control->objectName();
                        auto valueControl = control->value();
                        for (auto id : centerDataMap.keys())
                        {
                            if (id == objName)
                            {
                                auto value = centerDataMap.value(id);
                                if (value != valueControl)
                                    control->setValue(value);
                            }
                        }
                        if (objName == "CenterVirOutNumVCID1")
                        {
                            secondControlNum++;  //这里是计数，看有多少个虚拟信道丢帧VCID的控件
                        }
                    }
                    //                    if (virChannelNum && secondControlNum == virChannelNum)
                    //                    //上报的虚拟信道值不为0且值和控件数目相等的话就只需要赋值就可以了
                    //                    {
                    //                        int flag = 0;
                    //                        for (auto control : subControlList)
                    //                        {
                    //                            auto objName = control->objectName();
                    //                            auto valueControl = control->value();
                    //                            if (objName == "CenterVirOutNumVCID1")
                    //                            {
                    //                                QMap<QString, QVariant> secondDataMap;
                    //                                if (secondDataList.size() >= secondControlNum)
                    //                                {
                    //                                    secondDataMap = secondDataList[flag];  //从0开始取值
                    //                                    auto value = secondDataMap.value(objName);
                    //                                    if (value != valueControl)
                    //                                        control->setValue(value);
                    //                                }

                    //                                flag++;
                    //                            }
                    //                        }
                    //                    }
                    //                    else if (virChannelNum == 0)
                    //                    {
                    //                        continue;
                    //                    }
                    //                    else
                    //                    {
                    //                        for (auto j = 0; j < subControlList.size(); ++j)
                    //                        {
                    //                            auto control = subControlList[j];
                    //                            if (control->objectName() == "CenterVirOutNumVCID1")
                    //                            {
                    //                                delete subControlList[j];
                    //                                subControlList[j] = nullptr;
                    //                            }
                    //                        }
                    //                        QVBoxLayout* vBoxLayout = (QVBoxLayout*)widget->layout();
                    //                        QFlowLayout* subLayout = new QFlowLayout(6, 0, 10);
                    //                        for (auto k = 0; k < virChannelNum; ++k)
                    //                        {
                    //                            QMap<QString, QVariant> secondDataMap;
                    //                            if (secondDataList.size() >= k)
                    //                                secondDataMap = secondDataList[k];
                    //                            for (auto subExtItem : secondRecycleAttrList)
                    //                            {
                    //                                if (subExtItem.isShow)
                    //                                {
                    //                                    QMap<QString, QVariant> enumMaps;
                    //                                    if (subExtItem.displayFormat == DisplayFormat_Enum)
                    //                                    {
                    //                                        getEnumData(subExtItem.enumType, unit, enumMaps);
                    //                                    }
                    //                                    else if (subExtItem.displayFormat == DisplayFormat_LED)
                    //                                    {
                    //                                        getEnumData(subExtItem.enumType, unit, enumMaps);
                    //                                    }
                    //                                    if (secondDataMap.contains(subExtItem.id))
                    //                                    {
                    //                                        subExtItem.initValue = secondDataMap.value(subExtItem.id);
                    //                                        subExtItem.desc = subExtItem.desc + QString::number(k + 1);
                    //                                    }
                    //                                    auto control = ControlFactory::createStatusControl(subExtItem, enumMaps);
                    //                                    if (control)
                    //                                        subLayout->addWidget(control);
                    //                                }
                    //                            }
                    //                        }
                    //                        vBoxLayout->addItem(subLayout);
                    //                    }
                }
            }
            else if (centerNum == 0)
            {
                continue;
            }
            else
            {
                for (int i = 0; i < tabWidget->count(); ++i)
                {
                    //                        tabWidget->currentWidget()->hide();
                    //                        tabWidget->removeTab(tabWidget->currentIndex());
                    auto widget = tabWidget->widget(i);
                    tabWidget->removeTab(i);
                    if (widget != nullptr)
                    {
                        widget->setParent(nullptr);
                        delete widget;
                    }
                }
                tabWidget->clear();

                for (int i = 0; i < centerNum; ++i)
                {
                    QWidget* currentWidget = new QWidget();
                    QVBoxLayout* currentLayout = new QVBoxLayout;
                    QFlowLayout* flowLayout = new QFlowLayout(6, 0, 10);
                    QFlowLayout* subExt = new QFlowLayout();
                    //一个中心数据
                    QMap<QString, QVariant> centerDataMap;
                    if (firstDataList.size() >= i)
                        centerDataMap = firstDataList.at(i);
                    auto virChannelNum = centerDataMap.value("CenterKnowVirChannelNum").toInt();
                    for (auto attr : firstRecycleAttrList)
                    {
                        if (attr.isShow)
                        {
                            QMap<QString, QVariant> enumMap;
                            if (attr.displayFormat == DisplayFormat_Enum)
                            {
                                getEnumData(attr.enumType, unit, enumMap);
                            }
                            else if (attr.displayFormat == DisplayFormat_LED)
                            {
                                getEnumData(attr.enumType, unit, enumMap);
                            }
                            if (centerDataMap.contains(attr.id))
                            {
                                attr.initValue = centerDataMap.value(attr.id);  //将初始值转成上报上来的值
                            }
                            auto control = ControlFactory::createStatusControl(attr, enumMap);
                            if (control)
                            {
                                flowLayout->addWidget(control);
                            }
                        }
                        //添加下面
                        //                        if (attr.id == "CenterKnowVirChannelNum")
                        //                        {
                        //                            if (virChannelNum > 0)
                        //                            {
                        //                                for (int ii = 0; ii < virChannelNum; ++ii)
                        //                                {
                        //                                    QMap<QString, QVariant> secondDataMap;
                        //                                    if (secondDataList.size() >= ii)
                        //                                        secondDataMap = secondDataList[ii];
                        //                                    for (auto subExtItem : secondRecycleAttrList)
                        //                                    {
                        //                                        if (subExtItem.isShow)
                        //                                        {
                        //                                            QMap<QString, QVariant> enumMaps;
                        //                                            if (subExtItem.displayFormat == DisplayFormat_Enum)
                        //                                            {
                        //                                                getEnumData(subExtItem.enumType, unit, enumMaps);
                        //                                            }
                        //                                            else if (subExtItem.displayFormat == DisplayFormat_LED)
                        //                                            {
                        //                                                getEnumData(subExtItem.enumType, unit, enumMaps);
                        //                                            }
                        //                                            if (secondDataMap.contains(subExtItem.id))
                        //                                            {
                        //                                                subExtItem.initValue = secondDataMap.value(subExtItem.id);
                        //                                                subExtItem.desc = subExtItem.desc + QString::number(ii + 1);
                        //                                            }
                        //                                            auto control = ControlFactory::createStatusControl(subExtItem, enumMaps);
                        //                                            if (control)
                        //                                                subExt->addWidget(control);
                        //                                        }
                        //                                    }
                        //                                }
                        //                            }
                        //                            else
                        //                            {
                        //                                continue;
                        //                            }
                        //                        }
                    }
                    currentLayout->addItem(flowLayout);
                    currentLayout->addItem(subExt);
                    currentWidget->setLayout(currentLayout);
                    tabWidget->addTab(currentWidget, QString("中心%1").arg(i + 1));
                }
            }
        }
    }
}

void WidgetDealWith::getWidgetParamValue(const QString& paramName, QVariant& paramValue, QScrollArea* scrollArea)
{
    QList<QGroupBox*> allgroup = scrollArea->findChildren<QGroupBox*>();
    for (auto child : allgroup)
    {
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
        if (SET_TITLE == groupBox->title().trimmed())
        {
            QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();
            for (auto& control : allPButtons)
            {
                auto controlName = control->objectName();
                if (controlName == paramName)
                {
                    WWidget* wWidget = qobject_cast<WWidget*>(control);
                    if (wWidget)
                    {
                        auto value = wWidget->value();
                        paramValue = value;
                    }
                }
            }
        }
    }
}

WWidget* WidgetDealWith::getWidgetParamControl(const QString& paramName, QScrollArea* scrollArea)
{
    QList<QGroupBox*> allgroup = scrollArea->findChildren<QGroupBox*>();
    for (auto child : allgroup)
    {
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
        if (SET_TITLE == groupBox->title().trimmed())
        {
            QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();
            for (auto& control : allPButtons)
            {
                auto controlName = control->objectName();
                if (controlName == paramName)
                {
                    WWidget* wWidget = qobject_cast<WWidget*>(control);
                    if (wWidget)
                    {
                        return wWidget;
                    }
                }
            }
        }
    }

    return nullptr;
}

QMap<int, WWidget*> WidgetDealWith::getMultiWidgetParamControl(const QString& paramName, QScrollArea* scrollArea)
{
    QList<QGroupBox*> allgroup = scrollArea->findChildren<QGroupBox*>();
    QMap<int, WWidget*> map;
    for (auto child : allgroup)
    {
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
        if (SET_TITLE == groupBox->title().trimmed())
        {
            QList<QTabWidget*> multiTabList = groupBox->findChildren<QTabWidget*>();
            for (auto& tabWidget : multiTabList)
            {
                for (int i = 0; i < tabWidget->count(); i++)
                {
                    auto multiWidget = tabWidget->widget(i);

                    QList<WWidget*> subAllControl = multiWidget->findChildren<WWidget*>();
                    for (auto control : subAllControl)
                    {
                        auto objName = control->objectName();
                        if (paramName == objName)
                        {
                            map[i + 1] = control;
                        }
                    }
                }
            }
        }
    }

    return map;
}

void WidgetDealWith::dealWidgetControlMultiRelation(int recycle, const QString& objName, QVariant value, QMap<int, WWidget*> controlMap)
{
    if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && (m_currentMode.ID == 4 || m_currentMode.ID == 5) && objName == "SReceivPolar")
    {
        if (value == 1)
        {
            for (auto recycle : controlMap.keys())
            {
                auto control = controlMap[recycle];
                if (control)
                {
                    QMap<QString, QVariant> enums;
                    enums["左旋"] = QVariant(1);
                    enums["右旋"] = QVariant(2);
                    enums["左右旋同时"] = QVariant(3);
                    control->setItem(enums);
                }
            }
        }
        else
        {
            for (auto recycle : controlMap.keys())
            {
                auto control = controlMap[recycle];
                if (control)
                {
                    QMap<QString, QVariant> enums;
                    enums["左旋"] = QVariant(1);
                    enums["右旋"] = QVariant(2);
                    control->setItem(enums);
                }
            }
        }
    }
    else if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && (m_currentMode.ID == 4 || m_currentMode.ID == 5) &&
             objName == "K2_SpeedMeaPer")
    {
        if (value == 1)
        {
            if (controlMap.contains(recycle))
            {
                auto control = controlMap[recycle];
                if (control)
                {
                    QMap<QString, QVariant> enums;
                    enums["1Hz"] = QVariant(5);
                    enums["2Hz"] = QVariant(4);
                    enums["4Hz"] = QVariant(3);
                    enums["10Hz"] = QVariant(2);
                    enums["20Hz"] = QVariant(1);
                    control->setItem(enums);
                }
            }
        }
        else if (value == 2 || value == 3)
        {
            if (controlMap.contains(recycle))
            {
                auto control = controlMap[recycle];
                if (control)
                {
                    QMap<QString, QVariant> enums;
                    enums["1Hz"] = QVariant(5);
                    enums["2Hz"] = QVariant(4);
                    enums["4Hz"] = QVariant(3);
                    control->setItem(enums);
                }
            }
        }
        else
        {
            if (controlMap.contains(recycle))
            {
                auto control = controlMap[recycle];
                if (control)
                {
                    QMap<QString, QVariant> enums;
                    enums["1Hz"] = QVariant(5);
                    enums["2Hz"] = QVariant(4);
                    control->setItem(enums);
                }
            }
        }
    }
    else if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 0 && (m_currentMode.ID != 4 || m_currentMode.ID != 5) &&
             objName == "K2_SpeedMeaPer")
    {
        if (value == 1)
        {
            for (auto recycle : controlMap.keys())
            {
                auto control = controlMap[recycle];
                if (control)
                {
                    QMap<QString, QVariant> enums;
                    enums["1Hz"] = QVariant(4);
                    enums["2Hz"] = QVariant(3);
                    enums["10Hz"] = QVariant(2);
                    enums["20Hz"] = QVariant(1);
                    control->setItem(enums);
                }
            }
        }
        else
        {
            for (auto recycle : controlMap.keys())
            {
                auto control = controlMap[recycle];
                if (control)
                {
                    QMap<QString, QVariant> enums;
                    enums["1Hz"] = QVariant(4);
                    enums["2Hz"] = QVariant(3);
                    control->setItem(enums);
                }
            }
        }
    }
}

QString WidgetDealWith::getDSWidgetWorkModeText(Unit unit, UnitReportMessage reportMessage)
{
    if (reportMessage.paramMap.contains("WorkingMode"))
    {
        auto modeValue = reportMessage.paramMap.value("WorkingMode");

        for (auto attr : unit.parameterStatusList)
        {
            if (attr.id == "WorkingMode")
            {
                QMap<QString, QVariant> enumMap;
                if (attr.displayFormat == DisplayFormat_Enum)
                {
                    getEnumData(attr.enumType, unit, enumMap);
                }
                auto modeName = enumMap.key(modeValue);

                return modeName;
            }
        }
    }

    return QString();
}

void WidgetDealWith::dealWidgetControlRelation(WWidget* trigger, WWidget* receiverI, WWidget* receiverQ)
{
    if (trigger && receiverI && receiverQ)
    {
        int tztz = trigger->value().toInt();
        if (tztz == 1)
        {
            receiverI->setParamDesc("码速率:");
            receiverQ->setEnabled(false);
        }
        else if (tztz == 2 || tztz == 5)  // BPSK QPSK OQPSK
        {
            receiverI->setParamDesc("总码率:");
            receiverQ->setEnabled(false);
        }
        else if (tztz == 4)  // UQPSK
        {
            receiverI->setParamDesc("码速率:");
            receiverQ->setEnabled(true);
        }
    }
}

void WidgetDealWith::dealWidgetRelation(int unitID, const QString& objName, const QVariant value, QGroupBox* groupBox)
{
    if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 2 && (m_currentMode.ID == 10 || m_currentMode.ID == 11))
    {
        // dealWithGZDSRelation(objName, value, groupBox);
    }
    else if (m_currentDeviceID.sysID == 4 && m_currentDeviceID.devID == 4 && (unitID == 8 || unitID == 9) &&
             (objName == "CarrierModulation" || objName == "ICodeSpeed"))  //只有调制体制和I路码速率可以进下面的函数
    {
        dealWithDSRelation(objName, value, groupBox);
    }
}

void WidgetDealWith::dealWithDSRelation(const QString& objName, const QVariant value, QGroupBox* groupBox)
{
    int tztz;                       //调制体制
    WWidget* iCodeSpeed = nullptr;  // I路码速率控件
    WWidget* qCodeSpeed = nullptr;  // q路码速率控件
    if (SET_TITLE == groupBox->title().trimmed())
    {
        QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();
        for (auto& control : allPButtons)
        {
            auto controlName = control->objectName();
            if (controlName == "CarrierModulation")
            {
                tztz = control->value().toInt();
            }
            else if (controlName == "ICodeSpeed")
            {
                iCodeSpeed = control;
            }
            else if (controlName == "QCodeSpeed")
            {
                qCodeSpeed = control;
            }
        }
    }

    if (iCodeSpeed && qCodeSpeed)
    {
        if (tztz == 1)  // BPSK
        {
            iCodeSpeed->setParamDesc("码速率:");
        }
        else if (tztz == 2 || tztz == 5)  // QPSK OQPSK
        {
            iCodeSpeed->setParamDesc("总码率:");
            qCodeSpeed->setValue(value);
        }
        else if (tztz == 4)  // UQPSK
        {
            iCodeSpeed->setParamDesc("码速率:");
        }
    }
}

void WidgetDealWith::dealWithGZDSRelation(const QString& objName, const QVariant value, QGroupBox* groupBox)
{
    if (SET_TITLE == groupBox->title().trimmed())
    {
        QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();
        for (auto& control : allPButtons)
        {
            auto controlName = control->objectName();
            if (objName == "RSDecoding" && value == 0)
            {
                if (controlName == "ViterbiDecoding" || controlName == "TurboDecoding" || controlName == "LDPCDecoding")
                {
                    control->setEnabled(true);
                }
            }
            else if (objName == "RSDecoding" && (value == 1 || value == 2))
            {
                if (controlName == "ViterbiDecoding" || controlName == "TurboDecoding" || controlName == "LDPCDecoding")
                {
                    control->setEnabled(false);
                }
            }

            if (objName == "TurboDecoding" && value == 0)
            {
                if (controlName == "ViterbiDecoding" || controlName == "RSDecoding" || controlName == "LDPCDecoding")
                {
                    control->setEnabled(true);
                }
            }
            else if (objName == "TurboDecoding")
            {
                if (controlName == "ViterbiDecoding" || controlName == "RSDecoding" || controlName == "LDPCDecoding")
                {
                    control->setEnabled(false);
                }
            }

            if (objName == "LDPCDecoding" && value == 0)
            {
                if (controlName == "ViterbiDecoding" || controlName == "RSDecoding" || controlName == "TurboDecoding")
                {
                    control->setEnabled(true);
                }
            }
            else if (objName == "LDPCDecoding")
            {
                if (controlName == "ViterbiDecoding" || controlName == "RSDecoding" || controlName == "TurboDecoding")
                {
                    control->setEnabled(false);
                }
            }

            if (objName == "ViterbiDecoding" && value == 0)
            {
                if (controlName == "LDPCDecoding" || controlName == "RSDecoding" || controlName == "TurboDecoding")
                {
                    control->setEnabled(true);
                }
            }
            else if (objName == "ViterbiDecoding")
            {
                if (controlName == "LDPCDecoding" || controlName == "RSDecoding" || controlName == "TurboDecoding")
                {
                    control->setEnabled(false);
                }
            }
        }
    }
}

void WidgetDealWith::dealDSWidgetMSLNumRangeJT(WWidget* control, const QMap<QString, QVariant>& setParamJSMap,
                                               const QMap<QString, QVariant>& setParamJTMap)
{
    if (control == nullptr || setParamJSMap.isEmpty() || setParamJTMap.isEmpty())
        return;

    auto value = control->value();

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
            else if (ldpcCode == 7)  // 7/8
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
            else if (turboCode == 13 || turboCode == 14 || turboCode == 15 || turboCode == 16)  // 1/6
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
    //还原回之前设置的值
    control->setValue(value);
}

void WidgetDealWith::dealDSWidgetMSLNumRangeMNY(WWidget* controlI, WWidget* controlQ, const QMap<QString, QVariant>& setParamData)
{
    if (controlI == nullptr || controlQ == nullptr || setParamData.isEmpty())
        return;

    auto valueI = controlI->value();
    auto valueQ = controlQ->value();

    auto tztz = setParamData.value("CarrierModulation").toInt();  //调制体制
    // I路
    int codeWayI = setParamData.value("ICodeWay").toInt();              //译码方式
    int codeTypeI = setParamData.value("ICodeType").toInt();            //码型
    int viterbiCodeI = setParamData.value("IViterbiDecoding").toInt();  // Viterbi译码
    int ldpcCodeI = setParamData.value("ILDPCCodeRate").toInt();        // Ldpc码率
    int turboCodeI = setParamData.value("ITurboCodeRate").toInt();      // Turbo码率
    // Q路
    int codeWayQ = setParamData.value("QCodeWay").toInt();              //译码方式
    int codeTypeQ = setParamData.value("QCodeType").toInt();            //码型
    int viterbiCodeQ = setParamData.value("QViterbiDecoding").toInt();  // Viterbi译码
    int ldpcCodeQ = setParamData.value("QLDPCCodeRate").toInt();        // Ldpc码率
    int turboCodeQ = setParamData.value("QTurboCodeRate").toInt();      // Turbo码率

    int initMinValue;  //这个是译码方式为关各个调制体制的初始最小值
    int initMaxValue;  //这个是译码方式为关各个调制体制的初始最大值
    double bValueI;
    double bValueQ;
    QString cValueI;
    QString cValueQ;

    int minValue = 0;
    int maxValue = 0;

    if (tztz == 1)  // BPSK  只有I路
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

    if (tztz == 1 || tztz == 2 || tztz == 5)  // BPSK  QPSK OQPSK
    {
        if (codeTypeI == 1 || codeTypeI == 2 || codeTypeI == 3)  // NRZ-L、M、S
        {
            bValueI = 1;
        }
        else if (codeTypeI == 4 || codeTypeI == 5 || codeTypeI == 6)  // Biφ-L、M、S
        {
            bValueI = 0.5;
        }

        if (codeWayI == 2 || codeWayI == 4)  // Viterbi或者R-S和Viterbi级联译码
        {
            if (viterbiCodeI == 1)  //(7,(1/2))
            {
                cValueI = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (viterbiCodeI == 2)
            {
                cValueI = QString("2/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (viterbiCodeI == 3)
            {
                cValueI = QString("3/4");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (viterbiCodeI == 4)
            {
                cValueI = QString("5/6");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (viterbiCodeI == 5)
            {
                cValueI = QString("7/8");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
        }
        else if (codeWayI == 5)  // Turbo
        {
            if (turboCodeI == 1 || turboCodeI == 2 || turboCodeI == 3 || turboCodeI == 4)  // 1/2
            {
                cValueI = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (turboCodeI == 5 || turboCodeI == 6 || turboCodeI == 7 || turboCodeI == 8)  // 1/3
            {
                cValueI = QString("1/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (turboCodeI == 9 || turboCodeI == 10 || turboCodeI == 11 || turboCodeI == 12)  // 1/4
            {
                cValueI = QString("1/4");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (turboCodeI == 13 || turboCodeI == 14 || turboCodeI == 15 || turboCodeI == 16)  // 1/6
            {
                cValueI = QString("1/6");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
        }
        else if (codeWayI == 6)  // LDPC
        {
            if (ldpcCodeI == 1 || ldpcCodeI == 2)  // 1/2
            {
                cValueI = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (ldpcCodeI == 3 || ldpcCodeI == 4)  // 2/3
            {
                cValueI = QString("2/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (ldpcCodeI == 5 || ldpcCodeI == 6)  // 4/5
            {
                cValueI = QString("4/5");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (ldpcCodeI == 7)  // 7/8
            {
                cValueI = QString("7/8");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
        }
        else if (codeWayI == 1 || codeWayI == 3)
        {
            cValueI = QString("1/1");

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

            controlI->setNumRange(minValue, maxValue);

            controlI->setValue(minValue + 5000);
        }
    }
    else if (tztz == 4)  // UQPSK
    {
        // I路
        if (codeTypeI == 1 || codeTypeI == 2 || codeTypeI == 3)  // NRZ-L、M、S
        {
            bValueI = 1;
        }
        else if (codeTypeI == 4 || codeTypeI == 5 || codeTypeI == 6)  // Biφ-L、M、S
        {
            bValueI = 0.5;
        }

        if (codeWayI == 2 || codeWayI == 4)  // Viterbi或者R-S和Viterbi级联译码
        {
            if (viterbiCodeI == 1)  //(7,(1/2))
            {
                cValueI = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (viterbiCodeI == 2)
            {
                cValueI = QString("2/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (viterbiCodeI == 3)
            {
                cValueI = QString("3/4");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (viterbiCodeI == 4)
            {
                cValueI = QString("5/6");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (viterbiCodeI == 5)
            {
                cValueI = QString("7/8");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
        }
        else if (codeWayI == 5)  // Turbo
        {
            if (turboCodeI == 1 || turboCodeI == 2 || turboCodeI == 3 || turboCodeI == 4)  // 1/2
            {
                cValueI = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (turboCodeI == 5 || turboCodeI == 6 || turboCodeI == 7 || turboCodeI == 8)  // 1/3
            {
                cValueI = QString("1/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (turboCodeI == 9 || turboCodeI == 10 || turboCodeI == 11 || turboCodeI == 12)  // 1/4
            {
                cValueI = QString("1/4");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (turboCodeI == 13 || turboCodeI == 14 || turboCodeI == 15 || turboCodeI == 16)  // 1/6
            {
                cValueI = QString("1/6");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
        }
        else if (codeWayI == 6)  // LDPC
        {
            if (ldpcCodeI == 1 || ldpcCodeI == 2)  // 1/2
            {
                cValueI = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (ldpcCodeI == 3 || ldpcCodeI == 4)  // 2/3
            {
                cValueI = QString("2/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (ldpcCodeI == 5 || ldpcCodeI == 6)  // 4/5
            {
                cValueI = QString("4/5");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
            else if (ldpcCodeI == 7)  // 7/8
            {
                cValueI = QString("7/8");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                controlI->setNumRange(minValue, maxValue);

                controlI->setValue(minValue + 5000);
            }
        }
        else if (codeWayI == 1 || codeWayI == 3)
        {
            cValueI = QString("1/1");

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

            controlI->setNumRange(minValue, maxValue);

            controlI->setValue(minValue + 5000);
        }
        // Q路
        if (codeTypeQ == 1 || codeTypeQ == 2 || codeTypeQ == 3)  // NRZ-L、M、S
        {
            bValueQ = 1;
        }
        else if (codeTypeQ == 4 || codeTypeQ == 5 || codeTypeQ == 6)  // Biφ-L、M、S
        {
            bValueQ = 0.5;
        }

        if (codeWayQ == 2 || codeWayQ == 4)  // Viterbi或者R-S和Viterbi级联译码
        {
            if (viterbiCodeQ == 1)  //(7,(1/2))
            {
                cValueQ = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                controlQ->setNumRange(minValue, maxValue);

                controlQ->setValue(minValue + 5000);
            }
            else if (viterbiCodeQ == 2)
            {
                cValueQ = QString("2/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                controlQ->setNumRange(minValue, maxValue);

                controlQ->setValue(minValue + 5000);
            }
            else if (viterbiCodeQ == 3)
            {
                cValueQ = QString("3/4");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                controlQ->setNumRange(minValue, maxValue);
            }
            else if (viterbiCodeQ == 4)
            {
                cValueQ = QString("5/6");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                controlQ->setNumRange(minValue, maxValue);

                controlQ->setValue(minValue + 5000);
            }
            else if (viterbiCodeQ == 5)
            {
                cValueQ = QString("7/8");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                controlQ->setNumRange(minValue, maxValue);

                controlQ->setValue(minValue + 5000);
            }
        }
        else if (codeWayQ == 5)  // Turbo
        {
            if (turboCodeQ == 1 || turboCodeQ == 2 || turboCodeQ == 3 || turboCodeQ == 4)  // 1/2
            {
                cValueQ = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                controlQ->setNumRange(minValue, maxValue);

                controlQ->setValue(minValue + 5000);
            }
            else if (turboCodeQ == 5 || turboCodeQ == 6 || turboCodeQ == 7 || turboCodeQ == 8)  // 1/3
            {
                cValueQ = QString("1/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                controlQ->setNumRange(minValue, maxValue);

                controlQ->setValue(minValue + 5000);
            }
            else if (turboCodeQ == 9 || turboCodeQ == 10 || turboCodeQ == 11 || turboCodeQ == 12)  // 1/4
            {
                cValueQ = QString("1/4");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                controlQ->setNumRange(minValue, maxValue);

                controlQ->setValue(minValue + 5000);
            }
            else if (turboCodeQ == 13 || turboCodeQ == 14 || turboCodeQ == 15 || turboCodeQ == 16)  // 1/6
            {
                cValueQ = QString("1/6");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                controlQ->setNumRange(minValue, maxValue);

                controlQ->setValue(minValue + 5000);
            }
        }
        else if (codeWayQ == 6)  // LDPC
        {
            if (ldpcCodeQ == 1 || ldpcCodeQ == 2)  // 1/2
            {
                cValueQ = QString("1/2");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                controlQ->setNumRange(minValue, maxValue);

                controlQ->setValue(minValue + 5000);
            }
            else if (ldpcCodeQ == 3 || ldpcCodeQ == 4)  // 2/3
            {
                cValueQ = QString("2/3");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                controlQ->setNumRange(minValue, maxValue);

                controlQ->setValue(minValue + 5000);
            }
            else if (ldpcCodeQ == 5 || ldpcCodeQ == 6)  // 4/5
            {
                cValueQ = QString("4/5");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                controlQ->setNumRange(minValue, maxValue);

                controlQ->setValue(minValue + 5000);
            }
            else if (ldpcCodeQ == 7)  // 7/8
            {
                cValueQ = QString("7/8");

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                controlQ->setNumRange(minValue, maxValue);

                controlQ->setValue(minValue + 5000);
            }
        }
        else if (codeWayQ == 1 || codeWayQ == 3)
        {
            cValueQ = QString("1/1");

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

            controlQ->setNumRange(minValue, maxValue);

            controlQ->setValue(minValue + 5000);
        }
    }
    //还原回之前设置的值
    controlI->setValue(valueI);
    controlQ->setValue(valueQ);
}

void WidgetDealWith::dealDSWidgetMSLNumRangeSC(WWidget* control, const QMap<QString, QVariant>& setParamSCMap)
{
    if (control == nullptr || setParamSCMap.isEmpty())
        return;

    auto value = control->value();

    auto codeWay = setParamSCMap.value("DecodMode").toInt();  //译码方式
    auto codeType = setParamSCMap.value("CodeType").toInt();  //码型
    // auto viterbiCode = setParamSCMap.value("ViterbiDecoding").toInt();    // Viterbi译码
    auto ldpcCode = setParamSCMap.value("ILDPCCodeRate").toInt();  // Ldpc码率
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
    control->setValue(value);
}

void WidgetDealWith::dealGZWidgetMSLNumRangeJT(WWidget* control, const QMap<QString, QVariant>& setParamJSMap,
                                               const QMap<QString, QVariant>& setParamJTMap)
{
    if (control == nullptr || setParamJSMap.isEmpty() || setParamJTMap.isEmpty())
        return;

    auto value = control->value();

    int tztz = setParamJSMap.value("CarrierModulation").toInt();        //调制体制
    auto codeType = setParamJTMap.value("CodeType").toInt();            //码型
    auto viterbiCode = setParamJTMap.value("ViterbiDecoding").toInt();  // Viterbi译码

    auto ldpcType = setParamJTMap.value("LDPCDecoding").toInt();  // Ldpc译码
    auto ldpcCode = setParamJTMap.value("LDPCCodeRate").toInt();  // Ldpc码率

    auto turboType = setParamJTMap.value("TurboDecoding").toInt();  // Turbo译码
    auto turboCode = setParamJTMap.value("TurboCodeRate").toInt();  // Turbo码率

    int initMinValue;  //这个是译码方式为关各个调制体制的初始最小值
    int initMaxValue;  //这个是译码方式为关各个调制体制的初始最大值
    double bValue;
    QString cValue;

    int minValue = 0;
    int maxValue = 0;

    if (tztz == 2)  // BPSK
    {
        initMinValue = 1000;
        initMaxValue = 15000000;
    }
    else if (tztz == 3 || tztz == 4)  // QPSK OQPSK
    {
        initMinValue = 20000;
        initMaxValue = 30000000;
    }
    else if (tztz == 5)  // UQPSK
    {
        initMinValue = 10000;
        initMaxValue = 15000000;
    }

    if (codeType == 0 || codeType == 1 || codeType == 2)  // NRZ-L、M、S
    {
        bValue = 1;
    }
    else if (codeType == 3 || codeType == 4 || codeType == 5)  // Biφ-L、M、S
    {
        bValue = 0.5;
    }

    if (viterbiCode != 0)  // Viterbi不为否的时候
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
        else if (viterbiCode == 6)
        {
            cValue = QString("6/7");

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            control->setNumRange(minValue, maxValue);
        }
    }
    else if (turboType == 1)  // Turbo译码为有
    {
        if (turboCode == 0)  // 1/2
        {
            cValue = QString("1/2");

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            control->setNumRange(minValue, maxValue);
        }
        else if (turboCode == 1)  // 1/3
        {
            cValue = QString("1/3");

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            control->setNumRange(minValue, maxValue);
        }
        else if (turboCode == 2)  // 1/4
        {
            cValue = QString("1/4");

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            control->setNumRange(minValue, maxValue);
        }
        if (turboCode == 3)  // 1/6
        {
            cValue = QString("1/6");

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            control->setNumRange(minValue, maxValue);
        }
    }
    else if (ldpcType == 1)  // LDPC译码为有
    {
        if (ldpcCode == 0)  // 1/2
        {
            cValue = QString("1/2");

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            control->setNumRange(minValue, maxValue);
        }
        else if (ldpcCode == 1)  // 2/3
        {
            cValue = QString("2/3");

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            control->setNumRange(minValue, maxValue);
        }
        else if (ldpcCode == 2)  // 4/5
        {
            cValue = QString("4/5");

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            control->setNumRange(minValue, maxValue);
        }
        if (ldpcCode == 3)  // 7/8
        {
            cValue = QString("7/8");

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            control->setNumRange(minValue, maxValue);
        }
    }
    else  // viterbi ldpc turbo都不选的时候进里面
    {
        cValue = QString("1/1");

        calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

        control->setNumRange(minValue, maxValue);
    }

    control->setValue(value);
}

void WidgetDealWith::dealCkWidgetMSLNumRangeTTCRCU(WWidget* control, const QMap<QString, QVariant>& setParamData)
{
    if (control == nullptr || setParamData.isEmpty())
        return;

    auto value = control->value();

    int SettBefafter = setParamData.value("SettBefafter").toInt();  // 码速率控制
    int convCodType = setParamData.value("ConvCodType").toInt();    // 卷积编码类型 7,1/2  7,3/4

    if (SettBefafter == 1)  //编码前
    {
        int minValue = 100;
        int maxValue = 8000;

        control->setNumRange(minValue, maxValue);
    }
    else if (SettBefafter == 2)  //编码后
    {
        if (convCodType == 1)  // 7,1/2
        {
            int minValue = 200;
            int maxValue = 16000;

            control->setNumRange(minValue, maxValue);
        }
        else if (convCodType == 2)  // 7,3/4
        {
            int minValue = 134;
            int maxValue = 10666;

            control->setNumRange(minValue, maxValue);
        }
    }

    control->setValue(value);
}

void WidgetDealWith::dealCkWidgetMSLMultiNumRangeRCU(int recycle, QString trigParam1, QString trigParam2, QMap<int, WWidget*> controlMap,
                                                     const QList<QMap<QString, QVariant>>& paraTargerMap)
{
    if (paraTargerMap.isEmpty())
        return;

    WWidget* control = nullptr;
    QMap<QString, QVariant> setParamMap;
    if (m_currentMode.ID == 4 || m_currentMode.ID == 5 || m_currentMode.ID == 0x11)
    {
        if (!controlMap.contains(recycle))
            return;

        control = controlMap[recycle];

        setParamMap = paraTargerMap.at(recycle - 1);
    }
    else
    {
        if (!controlMap.contains(recycle + 1))
            return;

        control = controlMap[recycle + 1];

        setParamMap = paraTargerMap.at(recycle);
    }

    auto value = control->value();

    int SettBefafter = setParamMap.value(trigParam1).toInt();  // 码速率控制
    int convCodType = setParamMap.value(trigParam2).toInt();   // 卷积编码类型 7,1/2  7,3/4

    if (SettBefafter == 1)  //编码前
    {
        int minValue = 100;
        int maxValue = 8000;

        control->setNumRange(minValue, maxValue);
        control->setValue(value);
    }
    else if (SettBefafter == 2)  //编码后
    {
        if (convCodType == 1)  // 7,1/2
        {
            int minValue = 200;
            int maxValue = 16000;

            control->setNumRange(minValue, maxValue);
            control->setValue(value);
        }
        else if (convCodType == 2)  // 7,3/4
        {
            int minValue = 134;
            int maxValue = 10666;

            control->setNumRange(minValue, maxValue);
            control->setValue(value);
        }
    }
}

void WidgetDealWith::calcuCodeRateMinMaxValue(int initMinValue, int initMaxValue, double bValue, QString mark, int& minValue, int& maxValue)
{
    QString frontStep = mark.section('/', 0, 0);
    QString endStep = mark.section('/', 1, 1);
    if (endStep.toInt() >= 1 && frontStep.toInt() >= 1)
    {
        minValue = (int)(((initMinValue * bValue) * frontStep.toInt()) / endStep.toInt());
        maxValue = (int)(((initMaxValue * bValue) * frontStep.toInt()) / endStep.toInt());
    }
}

void WidgetDealWith::setMode(ModeCtrl mode) { m_currentMode = mode; }

void WidgetDealWith::setDeviceID(DeviceID deviceID) { m_currentDeviceID = deviceID; }

bool WidgetDealWith::getSetParamMapOther(QMap<QString, QVariant>& map, QWidget* widget)
{
    bool isValid = false;
    auto deviceTargetNum = GlobalData::getReportParamData(m_currentDeviceID, 1, m_targetID).toInt();
    for (auto child : widget->children())
    {
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
        if (!groupBox)
        {
            continue;
        }

        if (groupBox->title().trimmed() == SET_TITLE)
        {
            QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();
            for (auto& control : allPButtons)
            {
                auto objName = control->objectName();
                WWidget* wWidget = qobject_cast<WWidget*>(control);
                auto targetNum = control->property("targetNum").toInt();  //是第几个目标的参数
                if (wWidget)
                {
                    auto value = wWidget->value(isValid);
                    map[objName] = value;
                }

                if (targetNum >= 1)  //代表是多目标控件
                {
                    if (targetNum <= deviceTargetNum)  //代表这个多目标控件处于可设置的目标内 这个时候再来比较是否是存在范围内的值
                    {
                        if (!isValid)
                            return false;
                    }
                }
                else  //否则便是普通控件
                {
                    if (!isValid)
                        return false;
                }
            }
        }
    }

    return true;
}

bool WidgetDealWith::getSetMultiTargetParamMapOther(QList<QMap<QString, QVariant>>& list, QWidget* widget)
{
    bool isValid = false;

    for (auto child : widget->children())
    {
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
        if (!groupBox)
        {
            continue;
        }
        if (groupBox->title().trimmed() == SET_TITLE)
        {
            for (auto childWidget : groupBox->children())
            {
                if (childWidget->metaObject()->className() == QString("QGroupBox"))
                {
                    for (auto tabWidgetChild : childWidget->children())
                    {
                        auto tabWidget = qobject_cast<QTabWidget*>(tabWidgetChild);
                        if (tabWidget)
                        {
                            for (int i = 0; i < tabWidget->count(); i++)
                            {
                                QMap<QString, QVariant> map;
                                //这里的i表示目标几
                                //如果不可选中，就不更新数据
                                bool enable = tabWidget->isTabEnabled(i);
                                if (!enable)
                                    continue;

                                auto multiWidget = tabWidget->widget(i);

                                QList<WWidget*> allPButtons = multiWidget->findChildren<WWidget*>();
                                for (auto control : allPButtons)
                                {
                                    auto objName = control->objectName();
                                    WWidget* wWidget = qobject_cast<WWidget*>(control);
                                    if (wWidget)
                                    {
                                        map[objName] = wWidget->value(isValid);
                                        if (!isValid)
                                        {
                                            return false;
                                        }
                                    }
                                }
                                list.append(map);
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool WidgetDealWith::getSetParamMapYTH(QMap<QString, QVariant>& map, QWidget* widget)
{
    bool isValid = false;
    QGroupBox* groupBox = qobject_cast<QGroupBox*>(widget);
    if (!groupBox)
    {
        return false;
    }

    if (groupBox->title().trimmed() == SET_TITLE)
    {
        QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();
        for (auto& control : allPButtons)
        {
            auto objName = control->objectName();
            WWidget* wWidget = qobject_cast<WWidget*>(control);
            if (wWidget)
            {
                auto value = wWidget->value(isValid);

                map[objName] = value;
            }
            if (!isValid)
            {
                return false;
            }
        }
    }

    return true;
}

bool WidgetDealWith::getSetMultiTargetParamMapYTH(QList<QMap<QString, QVariant>>& list, UnitParamRequest& unitParamRequest, QWidget* widget)
{
    bool isValid = false;

    QGroupBox* groupBox = qobject_cast<QGroupBox*>(widget);
    if (!groupBox)
    {
        return false;
    }
    if (groupBox->title().trimmed() == SET_TITLE)
    {
        for (auto childWidget : groupBox->children())
        {
            if (childWidget->metaObject()->className() == QString("QGroupBox"))
            {
                for (auto tabWidgetChild : childWidget->children())
                {
                    auto tabWidget = qobject_cast<QTabWidget*>(tabWidgetChild);

                    if (tabWidget)
                    {
                        //这是获取有效标识
                        auto currentWidget = tabWidget->currentWidget();
                        unitParamRequest.m_validIdent = currentWidget->property("TargetKey").toInt();

                        //下面是取界面值(一体化是只获取当前目标的值，而不是全部目标的值)
                        QMap<QString, QVariant> map;

                        QList<WWidget*> allPButtons = currentWidget->findChildren<WWidget*>();
                        for (auto control : allPButtons)
                        {
                            auto objName = control->objectName();
                            WWidget* wWidget = qobject_cast<WWidget*>(control);
                            if (wWidget)
                            {
                                map[objName] = wWidget->value(isValid);
                                if (!isValid)
                                {
                                    return false;
                                }
                            }
                        }
                        list.append(map);
                    }
                }
            }
        }
    }

    return true;
}

bool WidgetDealWith::getEnumData(QString id, Unit& unit, QMap<QString, QVariant>& map)
{
    if (unit.enumMap.contains(id))
    {
        auto paramEnumList = unit.enumMap.value(id);
        for (auto paramEnum : paramEnumList.emumEntryList)
        {
            map[paramEnum.desc] = paramEnum.uValue;
        }
    }
    return true;
}

bool WidgetDealWith::getCmdEnumData(QString id, QMap<QString, QVariant>& map)
{
    if (m_currentMode.cmdEnumMap.contains(id))
    {
        auto paramEnumList = m_currentMode.cmdEnumMap.value(id);
        for (auto paramEnum : paramEnumList.emumEntryList)
        {
            map[paramEnum.desc] = paramEnum.uValue;
        }
    }
    return true;
}

bool WidgetDealWith::getEnumDataOrder(QString id, Unit& unit, QMap<int, QPair<QString, QVariant>>& map)
{
    if (unit.enumMap.contains(id))
    {
        auto paramEnumList = unit.enumMap.value(id);
        for (auto paramEnum : paramEnumList.emumEntryList)
        {
            QPair<QString, QVariant> pair;
            pair.first = paramEnum.desc;
            pair.second = paramEnum.uValue;
            map[paramEnum.sValue.toInt()] = pair;
        }
    }
    return true;
}

bool WidgetDealWith::getCmdEnumDataOrder(QString id, QMap<int, QPair<QString, QVariant>>& map)
{
    if (m_currentMode.cmdEnumMap.contains(id))
    {
        auto paramEnumList = m_currentMode.cmdEnumMap.value(id);
        for (auto paramEnum : paramEnumList.emumEntryList)
        {
            QPair<QString, QVariant> pair;
            pair.first = paramEnum.desc;
            pair.second = paramEnum.uValue;
            map[paramEnum.sValue.toInt()] = pair;
        }
    }
    return true;
}

bool WidgetDealWith::getCmdMultiParamMap(QMap<int, QList<QPair<QString, QVariant>>>& map, QWidget* widget)
{
    bool isValid = false;
    for (auto child : widget->children())
    {
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
        if (!groupBox)
        {
            continue;
        }
        if (groupBox->title().trimmed() == SET_TITLE)
        {
            for (auto childWidget : groupBox->children())
            {
                if (childWidget->metaObject()->className() == QString("QGroupBox"))
                {
                    auto groupWidget = qobject_cast<QGroupBox*>(childWidget);
                    if (groupWidget->title().trimmed() == CMD_SET_MULTI_VIRTITLE)
                    {
                        QList<WWidget*> allPButtons = groupWidget->findChildren<WWidget*>();
                        QPair<QString, QVariant> pair;
                        QList<QPair<QString, QVariant>> list;
                        int num = 0;
                        for (auto& control : allPButtons)
                        {
                            bool enable = control->isEnabled();
                            if (!enable)
                                continue;

                            auto objName = control->objectName();
                            WWidget* wWidget = qobject_cast<WWidget*>(control);
                            if (wWidget)
                            {
                                pair.first = objName + QString::number(num++);
                                pair.second = wWidget->value(isValid);
                                if (!isValid)
                                {
                                    return false;
                                }
                            }
                            list.append(pair);
                        }
                        map[1] = list;  //这里的1是recycle的id
                    }
                    if (groupWidget->title().trimmed() == CMD_SET_MULTI_FRAMETITLE)
                    {
                        QList<WWidget*> allPButtons = groupWidget->findChildren<WWidget*>();
                        QPair<QString, QVariant> pair;
                        QList<QPair<QString, QVariant>> list;
                        int num = 0;
                        for (auto& control : allPButtons)
                        {
                            bool enable = control->isEnabled();
                            if (!enable)
                                continue;

                            auto objName = control->objectName();
                            WWidget* wWidget = qobject_cast<WWidget*>(control);
                            if (wWidget)
                            {
                                pair.first = objName + QString::number(num++);
                                pair.second = wWidget->value(isValid);
                                if (!isValid)
                                {
                                    return false;
                                }
                            }
                            list.append(pair);
                        }
                        map[2] = list;  //这里的2是recycle的id
                    }
                    if (groupWidget->title().trimmed() == CMD_SET_MULTI_JDNUM)
                    {
                        QList<WWidget*> allPButtons = groupWidget->findChildren<WWidget*>();
                        QPair<QString, QVariant> pair;
                        QList<QPair<QString, QVariant>> list;
                        int num = 0;
                        for (auto& control : allPButtons)
                        {
                            bool enable = control->isEnabled();
                            if (!enable)
                                continue;

                            auto objName = control->objectName();
                            WWidget* wWidget = qobject_cast<WWidget*>(control);
                            if (wWidget)
                            {
                                pair.first = objName + QString::number(num++);
                                pair.second = wWidget->value(isValid);
                                if (!isValid)
                                {
                                    return false;
                                }
                            }
                            list.append(pair);
                        }
                        map[1] = list;  //这里的1是recycle的id
                    }
                    if (groupWidget->title().trimmed() == CMD_SET_MULTI_AGC)
                    {
                        QList<WWidget*> allPButtons = groupWidget->findChildren<WWidget*>();
                        QPair<QString, QVariant> pair;
                        QList<QPair<QString, QVariant>> list;
                        int num = 0;
                        for (auto& control : allPButtons)
                        {
                            bool enable = control->isEnabled();
                            if (!enable)
                                continue;

                            auto objName = control->objectName();
                            WWidget* wWidget = qobject_cast<WWidget*>(control);
                            if (wWidget)
                            {
                                pair.first = objName + QString::number(num++);
                                pair.second = wWidget->value(isValid);
                                if (!isValid)
                                {
                                    return false;
                                }
                            }
                            list.append(pair);
                        }
                        map[1] = list;  //这里的1是recycle的id
                    }
                }
            }
        }
    }

    return true;
}

bool WidgetDealWith::getCmdSetParamMap(QMap<QString, QVariant>& map, QWidget* widget)
{
    bool isValid = false;
    for (auto child : widget->children())
    {
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
        if (!groupBox)
        {
            continue;
        }
        if (groupBox->title().trimmed() == SET_TITLE)
        {
            QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();
            for (auto& control : allPButtons)
            {
                auto objName = control->objectName();
                WWidget* wWidget = qobject_cast<WWidget*>(control);
                if (wWidget)
                {
                    auto value = wWidget->value(isValid);

                    map[objName] = value;
                }
                if (!isValid)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

void WidgetDealWith::getSetParamMapOtherRelation(QMap<QString, QVariant>& map, QWidget* widget)
{
    for (auto child : widget->children())
    {
        QGroupBox* groupBox = qobject_cast<QGroupBox*>(child);
        if (!groupBox)
        {
            continue;
        }

        if (groupBox->title().trimmed() == SET_TITLE)
        {
            QList<WWidget*> allPButtons = groupBox->findChildren<WWidget*>();
            for (auto& control : allPButtons)
            {
                auto objName = control->objectName();
                WWidget* wWidget = qobject_cast<WWidget*>(control);
                if (wWidget)
                {
                    auto value = wWidget->value();

                    map[objName] = value;
                }
            }
        }
    }
}
