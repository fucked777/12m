#include "ParamMacroRelationManager.h"

#include "PlatformInterface.h"
#include "ProtocolXmlTypeDefine.h"
#include <QApplication>
#include <QDir>
#include <QDomDocument>
#include <QDomNodeList>
#include <QFile>
#include <QScrollArea>
#include <QStringList>
#include <QTabWidget>

#include "CConverter.h"
#include "WWidget.h"

#define CURRENT_DEVICE_ID 0xFFFF  // 表示当前分机
class ParamMacroRelationManagerPrivate
{
public:
    template<typename T>
    static void receiverOperate(const T& values);
    template<typename T>
    static void receiverOperate(T& values, const QMap<QString, WWidget*>&);
};

template<typename T>
void ParamMacroRelationManagerPrivate::receiverOperate(const T& values)
{
    for (auto& receiver : values)
    {
        if (!receiver.control)
            continue;

        // 是否可用
        if (receiver.isEnable != Default)
        {
            receiver.control->setEnabled(receiver.isEnable);
        }

        // 下拉框
        if (receiver.control->metaObject()->className() == QString("WComboBox"))
        {
            if (!receiver.enums.isEmpty())
                receiver.control->setItem(receiver.enums);

            // 设置当前值（目的是触发一次值改变信号）
            auto valueString = receiver.value.toString();
            if (valueString.compare(QString("CurrentValue"), Qt::CaseInsensitive))
            {
                emit receiver.control->sig_valueChange(receiver.control->value());
            }
            //            else if (!valueString.isEmpty())
            //            {
            //                receiver.control->setValue(receiver.value);
            //            }
        }
        // 十六进制输入框
        else if (receiver.control->metaObject()->className() == QString("WHexLineEdit"))
        {
            // 设置最大最小值
            if (!receiver.minValue.toString().isEmpty())
            {
                receiver.control->setMinNum(receiver.minValue);
            }
            if (!receiver.maxValue.toString().isEmpty())
            {
                receiver.control->setMaxNum(receiver.maxValue);
            }
            // 设置值  放到最后来设置
            if (!receiver.value.toString().isEmpty())
            {
                receiver.control->setValue(receiver.value.toString());
            }
        }
        // 无符号数值
        else if (receiver.control->metaObject()->className() == QString("WUNumber"))
        {
            // 设置最大最小值
            if (!receiver.minValue.toString().isEmpty())
            {
                receiver.control->setMinNum(receiver.minValue);
            }
            if (!receiver.maxValue.toString().isEmpty())
            {
                receiver.control->setMaxNum(receiver.maxValue);
            }
            // 设置值
            if (!receiver.value.toString().isEmpty())
            {
                receiver.control->setValue(receiver.value.toString().toULongLong());
            }
        }
        // 有符号数值
        else if (receiver.control->metaObject()->className() == QString("WINumber"))
        {
            // 设置最大最小值
            if (!receiver.minValue.toString().isEmpty())
            {
                receiver.control->setMinNum(receiver.minValue);
            }
            if (!receiver.maxValue.toString().isEmpty())
            {
                receiver.control->setMaxNum(receiver.maxValue);
            }
            // 设置值
            if (!receiver.value.toString().isEmpty())
            {
                receiver.control->setValue(receiver.value.toString().toULongLong());
            }
        }
        // 小数数值
        else if (receiver.control->metaObject()->className() == QString("WDouble"))
        {
            // 设置最大最小值
            if (!receiver.minValue.toString().isEmpty())
            {
                receiver.control->setMinNum(receiver.minValue);
            }
            if (!receiver.maxValue.toString().isEmpty())
            {
                receiver.control->setMaxNum(receiver.maxValue);
            }
            // 设置值
            if (!receiver.value.toString().isEmpty())
            {
                receiver.control->setValue(receiver.value.toString().toULongLong());
            }
        }
        // 字符串
        else if (receiver.control->metaObject()->className() == QString("WStrLineEdit"))
        {
            // 设置最大长度
            if (!receiver.maxValue.toString().isEmpty())
            {
                receiver.control->setMaxLength(receiver.maxValue.toInt());
            }
        }
        else if (receiver.control->metaObject()->className() == QString("WHexNumStr"))
        {
            if (!receiver.value.toString().isEmpty())
            {
                /* 先转换为非16进制的字节流 */
                /* 这里要倒序输出 */
                auto rawText = receiver.value.toString().trimmed();
                // 解决需要多个整型的十六进制（比如UInt8[24], UInt32[4]）
                if (rawText.length() % 2 != 0)
                    rawText.prepend("0");
                QString tempText;
                for (int i = 0; i < rawText.length(); i += 2)
                {
                    tempText.prepend(rawText.mid(i, 2));
                }
                receiver.control->setValue(tempText);
            }
        }

        else
        {
            // 设置最大最小值
            if (!receiver.minValue.toString().isEmpty())
            {
                receiver.control->setMinNum(receiver.minValue);
            }
            if (!receiver.maxValue.toString().isEmpty())
            {
                receiver.control->setMaxNum(receiver.maxValue);
            }
            if (!receiver.value.toString().isEmpty())
            {
                receiver.control->setValue(receiver.value);
            }
        }
    }
}

template<typename T>
void ParamMacroRelationManagerPrivate::receiverOperate(T& values, const QMap<QString, WWidget*>& widgetMap)
{
    for (auto& receiver : values)
    {
        if (nullptr == receiver.control)
        {
            auto find = widgetMap.find(receiver.id);
            if (find == widgetMap.end())
            {
                continue;
            }
            receiver.control = find.value();
        }

        // 是否可用
        if (receiver.isEnable != Default)
        {
            receiver.control->setEnabled(receiver.isEnable);
        }

        // 下拉框
        if (receiver.control->metaObject()->className() == QString("WCombobox"))
        {
            if (!receiver.enums.isEmpty())
                receiver.control->setItem(receiver.enums);

            // 设置当前值（目的是触发一次值改变信号）
            auto valueString = receiver.value.toString();
            if (valueString.compare(QString("CurrentValue"), Qt::CaseInsensitive))
            {
                emit receiver.control->sig_valueChange(receiver.control->value());
            }
            else if (!valueString.isEmpty())
            {
                receiver.control->setValue(receiver.value);
            }
        }
        // 十六进制输入框
        else if (receiver.control->metaObject()->className() == QString("WHexLineEdit"))
        {
            // 设置最大最小值
            if (!receiver.minValue.toString().isEmpty())
            {
                receiver.control->setMinNum(receiver.minValue);
            }
            if (!receiver.maxValue.toString().isEmpty())
            {
                receiver.control->setMaxNum(receiver.maxValue);
            }
            // 设置值
            if (!receiver.value.toString().isEmpty())
            {
                receiver.control->setValue(receiver.value.toString());
            }
        }
        // 无符号数值
        else if (receiver.control->metaObject()->className() == QString("WUNumber"))
        {
            // 设置最大最小值
            if (!receiver.minValue.toString().isEmpty())
            {
                receiver.control->setMinNum(receiver.minValue);
            }
            if (!receiver.maxValue.toString().isEmpty())
            {
                receiver.control->setMaxNum(receiver.maxValue);
            }
            // 设置值
            if (!receiver.value.toString().isEmpty())
            {
                receiver.control->setValue(receiver.value.toString().toULongLong());
            }
        }
        // 有符号数值
        else if (receiver.control->metaObject()->className() == QString("WINumber"))
        {
            // 设置最大最小值
            if (!receiver.minValue.toString().isEmpty())
            {
                receiver.control->setMinNum(receiver.minValue);
            }
            if (!receiver.maxValue.toString().isEmpty())
            {
                receiver.control->setMaxNum(receiver.maxValue);
            }
            // 设置值
            if (!receiver.value.toString().isEmpty())
            {
                receiver.control->setValue(receiver.value.toString().toULongLong());
            }
        }
        // 小数数值
        else if (receiver.control->metaObject()->className() == QString("WDouble"))
        {
            // 设置最大最小值
            if (!receiver.minValue.toString().isEmpty())
            {
                receiver.control->setMinNum(receiver.minValue);
            }
            if (!receiver.maxValue.toString().isEmpty())
            {
                receiver.control->setMaxNum(receiver.maxValue);
            }
            // 设置值
            if (!receiver.value.toString().isEmpty())
            {
                receiver.control->setValue(receiver.value.toString().toULongLong());
            }
        }
        // 字符串
        else if (receiver.control->metaObject()->className() == QString("WStrLineEdit"))
        {
            // 设置最大长度
            if (!receiver.maxValue.toString().isEmpty())
            {
                receiver.control->setMaxLength(receiver.maxValue.toInt());
            }
        }
        else if (receiver.control->metaObject()->className() == QString("WHexNumStr"))
        {
            if (!receiver.value.toString().isEmpty())
            {
                /* 先转换为非16进制的字节流 */
                /* 这里要倒序输出 */
                auto rawText = receiver.value.toString().trimmed();
                // 解决需要多个整型的十六进制（比如UInt8[24], UInt32[4]）
                if (rawText.length() % 2 != 0)
                    rawText.prepend("0");
                QString tempText;
                for (int i = 0; i < rawText.length(); i += 2)
                {
                    tempText.prepend(rawText.mid(i, 2));
                }

                receiver.control->setValue(QByteArray::fromHex(tempText.toUtf8()));
            }
        }
        else
        {
            // 设置最大最小值
            if (!receiver.minValue.toString().isEmpty())
            {
                receiver.control->setMinNum(receiver.minValue);
            }
            if (!receiver.maxValue.toString().isEmpty())
            {
                receiver.control->setMaxNum(receiver.maxValue);
            }
            if (!receiver.value.toString().isEmpty())
            {
                receiver.control->setValue(receiver.value);
            }
        }
    }
}

ParamMacroRelationManager::ParamMacroRelationManager(QObject* parent)
    : QObject(parent)
{
    auto relationDirPath = QString("%1/ParamRelationXML").arg(PlatformConfigTools::configShareDir());
    loadXML(relationDirPath);
}

void ParamMacroRelationManager::addUnit(SystemWorkMode workMode, QMap<QString, QMap<QString, WWidget*>> deviceMap)
{
    int currentWorkMode = conventWorkMode(workMode);
    //单元，单元下的参数
    QMap<int, QMap<QString, WWidget*>> unitDeviceMap;
    QMap<QString, QMap<QString, WWidget*>> newOneDeviceMap;

    for (const auto& deviceName : deviceMap.keys())
    {
        for (auto item : deviceName.split("/"))
        {
            for (auto subItem : deviceMap.value(deviceName).keys())
            {
                newOneDeviceMap[item][subItem] = deviceMap.value(deviceName).value(subItem);
            }
        }
    }
    for (const auto& deviceName : newOneDeviceMap.keys())
    {
        QStringList list = deviceName.split("_");
        QMap<QString, WWidget*> unitWidgets = newOneDeviceMap.value(deviceName);
        if (list.size() != 2)
        {
            continue;
        }
        DeviceID deviceId(list[0].toInt(nullptr, 16));
        int unitID = list[1].toInt();
        unitDeviceMap[unitID] = unitWidgets;

        auto unit = GlobalData::getUnit(deviceId.sysID, deviceId.devID, deviceId.extenID, currentWorkMode, unitID);
        if (unit.unitType.isEmpty())
        {
            continue;
        }
        QString unitType = unit.unitType;
        Relation relation;
        if (!findUnitRelation(deviceId.sysID, deviceId.devID, currentWorkMode, unitType, relation))
        {
            continue;
        }

        relation.extenID = deviceId.extenID;
        relation.unitID = unitID;

        initUnitRelationControl(unitWidgets, relation);

        auto relationKey = QUuid::createUuid().toString();
        mUnitParamRelationMap.insert(relationKey, relation);
        addParamRelation(&mUnitParamRelationMap, relation, relationKey, unitDeviceMap);

        // 初始化状态
        initControlStatus(relation);
    }
}

static bool createAssociationKey(QList<std::tuple<QString, WWidget*>>& controlList, QString& key, const QMap<QString, WWidget*>& widgetMap)
{
    bool isValid = true;
    for (auto iter = controlList.begin(); iter != controlList.end(); ++iter)
    {
        auto& tempKey = std::get<0>(*iter);
        auto& widget = std::get<1>(*iter);
        if (widget == nullptr)
        {
            auto find = widgetMap.find(tempKey);
            if (find == widgetMap.end())
            {
                return false;
            }
            widget = find.value();
        }

        auto tempValue = widget->value(isValid);
        /* 无效的话当 就下一个 */
        if (!isValid)
        {
            return false;
        }
        key += tempValue.toString();
        key += " ";
    }
    key = key.trimmed();
    return true;
}
void ParamMacroRelationManager::onTrigger(QMap<QString, Relation>* relationMap, const QString& relationKey, const QString& triggerID,
                                          const QVariant& value, QMap<int, QMap<QString, WWidget*>> unitDeviceMap)
{
    auto& relation = (*relationMap)[relationKey];
    auto& trigger = relation.triggerIdMap[triggerID];
    auto triggerValueIter = trigger.trigValMap.find(value.toString());
    if (triggerValueIter == trigger.trigValMap.end())
    {
        return;
    }

    /* 一对多 */
    ParamMacroRelationManagerPrivate::receiverOperate(triggerValueIter->receiverMap);

    QMap<QString, WWidget*> widgetMap;
    if (unitDeviceMap.contains(relation.unitID))
    {
        widgetMap = unitDeviceMap[relation.unitID];
    }
    /* 多对多 */
    for (auto& association : triggerValueIter->associationMap)
    {
        /* 先获取指定控件的值 */
        QString key;
        key += value.toString();
        key += " ";
        if (!createAssociationKey(association.controlList, key, widgetMap))
        {
            continue;
        }
        /* 查找key */
        auto find = association.receiverAssociationMap.find(key);
        if (find == association.receiverAssociationMap.end())
        {
            /* 没找到直接忽略 */
            continue;
        }
        /* 找到了,直接赋值就行 */
        ParamMacroRelationManagerPrivate::receiverOperate(find.value(), widgetMap);
    }
}

void ParamMacroRelationManager::loadXML(const QString& relationXmlPath)
{
    if (!QFileInfo::exists(relationXmlPath))
        return;

    QFileInfoList xmlFileInfoList;
    scanXmlFile(xmlFileInfoList, relationXmlPath);

    for (auto& fileInfo : xmlFileInfoList)
    {
        QString triggerID;
        QString trigValue;
        QString associationID;
        QString associationItemID;

        QFile file(fileInfo.absoluteFilePath());
        if (!file.exists())
            continue;

        if (!file.open(QFile::ReadOnly | QFile::Text))
            continue;

        QDomDocument doc;
        if (!doc.setContent(&file))
            continue;

        QDomElement rootEle = doc.documentElement();
        if (rootEle.tagName() != "Relation")
            continue;

        Relation relation;
        relation.sysID = rootEle.attribute("sysID").trimmed().toInt(0, 16);
        relation.devID = rootEle.attribute("deviceID").trimmed().toInt(0, 16);
        relation.modelID = rootEle.attribute("modeID").trimmed().toInt(0, 16);
        relation.desc = rootEle.attribute("desc").trimmed();

        // 判断该配置文件是单元还是命令的关联关系
        QString unitType = rootEle.attribute("unitType").trimmed();
        QString cmdID = rootEle.attribute("cmdID").trimmed();
        if (!unitType.isEmpty())
        {
            relation.unitType = unitType;
        }
        else if (!cmdID.isEmpty())
        {
            relation.cmdID = cmdID.toInt();
        }
        else
        {
            continue;
        }

        parseXML(rootEle, triggerID, trigValue, associationID, associationItemID, relation);

        // 添加到对应的容器中
        if (!unitType.isEmpty())
        {
            tempUnitParamRelationList.append(relation);
        }
        else
        {
            tempCmdParamRelationList.append(relation);
        }
    }
}

void ParamMacroRelationManager::scanXmlFile(QFileInfoList& infoList, const QString& filePath)
{
    QFileInfo fileInfo(filePath);

    if (fileInfo.isDir())
    {
        QDir dir(filePath);
        dir.setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

        for (const QFileInfo& info : dir.entryInfoList())
        {
            scanXmlFile(infoList, info.absoluteFilePath());
        }
    }
    else if (fileInfo.isFile())
    {
        infoList.append(fileInfo);
    }
}

static void parseReceiver(QDomElement& element, Receiver& receiver)
{
    receiver.id = element.attribute("id").trimmed();
    receiver.desc = element.attribute("desc").trimmed();

    QString isEnable = element.attribute("isEnable").trimmed();
    receiver.isEnable = isEnable == "0" ? Unusable : isEnable == "1" ? Usable : Default;

    receiver.minValue = element.attribute("minValue").trimmed();
    receiver.maxValue = element.attribute("maxValue").trimmed();

    receiver.value = element.attribute("values").trimmed();

    // 如果是下拉框值
    auto enumList = element.attribute("values").trimmed().split(";");
    for (const auto& enu : enumList)
    {
        auto entryMap = enu.split(":");
        if (entryMap.size() != 2)
            continue;
        receiver.enums.insert(entryMap[1], QVariant(entryMap[0]));
    }

    // 如果是引用值
    QStringList refList = element.attribute("values").trimmed().split("|");
    if (refList.size() == 4 || refList.size() == 5)  // =4表示没有多目标 =5表示有多目标
    {
        if (refList[0] == "CurrentDeviceID")  // 引用值所的设备ID为当前命令分机
        {
            receiver.refVal.deviceID = CURRENT_DEVICE_ID;
        }
        else
        {
            receiver.refVal.deviceID = refList[0].toInt(0, 16);
        }

        receiver.refVal.modeID = refList[1].toInt(0, 16);
        receiver.refVal.unitID = refList[2].toInt();
        receiver.refVal.refParamId = refList[3];
        if (refList.size() == 5)
            receiver.refVal.targetIdx = refList[4].toInt();
    }
}
void ParamMacroRelationManager::parseXML(const QDomElement& docElement, QString& triggerId, QString& trigValue, QString& associationID,
                                         QString& associationItemID, Relation& relation)
{
    QDomNode node = docElement.firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            QString tagName = element.tagName();
            if (tagName == "Trigger")
            {
                Trigger trigger;
                trigger.id = element.attribute("id").trimmed();
                trigger.desc = element.attribute("desc").trimmed();

                relation.triggerIdMap.insert(trigger.id, trigger);
                triggerId = trigger.id;
            }
            else if (tagName == "TriggerValue")
            {
                TriggerValue triggerValue;
                triggerValue.value = element.attribute("value").trimmed();
                triggerValue.desc = element.attribute("desc").trimmed();

                relation.triggerIdMap[triggerId].trigValMap.insert(triggerValue.value, triggerValue);
                trigValue = triggerValue.value;
            }
            else if (tagName == "Receiver")
            {
                Receiver receiver;
                parseReceiver(element, receiver);

                relation.triggerIdMap[triggerId].trigValMap[trigValue].receiverMap.insert(receiver.id, receiver);
            }
            else if (tagName == "Association")
            {
                associationID = element.attribute("id").trimmed();
                auto tempList = associationID.split(' ', QString::SkipEmptyParts);
                if (!tempList.isEmpty())
                {
                    AssociationTriggered tempTriggered;
                    for (auto& item : tempList)
                    {
                        tempTriggered.controlList << std::make_tuple(item, nullptr);
                        // tempTriggered.controlMap[item] = nullptr;
                    }

                    relation.triggerIdMap[triggerId].trigValMap[trigValue].associationMap.insert(associationID, tempTriggered);
                }
            }
            else if (tagName == "AssociationItem")
            {
                associationItemID = element.attribute("id").trimmed();
            }
            else if (tagName == "AssociationReceiver")
            {
                Receiver receiver;
                parseReceiver(element, receiver);

                relation.triggerIdMap[triggerId].trigValMap[trigValue].associationMap[associationID].receiverAssociationMap[associationItemID]
                    << receiver;
            }
            parseXML(element, triggerId, trigValue, associationID, associationItemID, relation);
        }

        node = node.nextSibling();
    }
}

bool ParamMacroRelationManager::findUnitRelation(int sysId, int deviceId, int modeId, const QString& unitType, Relation& relation)
{
    //  查找是否配置了该单元参数关联关系
    for (auto& tempRelation : tempUnitParamRelationList)
    {
        // 根据unitType判断是否存在关联关系，如果单元使用的同一个配置文件，就只需要配置一个关联关系文件                 65535 无模式
        if (tempRelation.sysID == sysId && tempRelation.devID == deviceId && (tempRelation.modelID == modeId || modeId == 65535) &&
            tempRelation.unitType == unitType)
        {
            relation = tempRelation;
            return true;
        }
    }
    return false;
}

bool ParamMacroRelationManager::findCmdRelation(int sysId, int deviceId, int modeId, int cmdId, Relation& relation)
{
    for (auto tempRelation : tempCmdParamRelationList)
    {
        if (tempRelation.sysID == sysId && tempRelation.devID == deviceId && (tempRelation.modelID == modeId || modeId == 65535) &&
            tempRelation.cmdID == cmdId)
        {
            relation = tempRelation;
            return true;
        }
    }
    return false;
}

void ParamMacroRelationManager::initUnitRelationControl(QMap<QString, WWidget*> unitWidgets, Relation& relation)
{
    QList<WWidget*> allPButtons = unitWidgets.values();
    for (auto& control : allPButtons)
    {
        for (auto& trigger : relation.triggerIdMap)
        {
            auto objNameList = control->objectName().split("&&");
            auto objName =
                objNameList
                    .last();  //取最后一项  参数宏的objName里的最后一项才是放的参数ID
                              //如果用control->objectName().contains的话有些参数它有类似的名称，会导致触发一个控件的关联关系时同时触发了另一个控件的关联关系
            if (objName == trigger.id)
            {
                trigger.control = control;
            }
            else
            {
                for (auto& triggerValue : trigger.trigValMap)
                {
                    for (auto& receiver : triggerValue.receiverMap)
                    {
                        if (objName == receiver.id)
                        {
                            receiver.control = control;
                        }
                    }
                }
            }
        }
    }

    //多目标参数
    //    QList<QTabWidget*> multiTabList = groupBox->findChildren<QTabWidget*>();
    //    for (auto& tabWidget : multiTabList)
    //    {
    //        Relation tempRelation = relation;
    //        QStringList unnecessaryTriggerIdList;  // 如果是多目标的参数，记录下来，移除
    //        QStringList unnecessaryReceiverIdList;

    //        for (int i = 0; i < tabWidget->count(); i++)
    //        {
    //            auto multiWidget = tabWidget->widget(i);

    //            QList<WWidget*> subAllControl = multiWidget->findChildren<WWidget*>();
    //            // 先初始化全部trigger，保证trigger在receive之前初始化
    //            for (auto control : subAllControl)
    //            {
    //                auto objName = control->objectName();
    //                for (auto& trigger : tempRelation.triggerIdMap)
    //                {
    //                    if (trigger.id == objName)
    //                    {
    //                        Trigger multiTrigger = trigger;
    //                        multiTrigger.id = objName + "_" + QString::number(i + 1);
    //                        multiTrigger.control = control;
    //                        relation.triggerIdMap.insert(multiTrigger.id, multiTrigger);

    //                        unnecessaryTriggerIdList.append(trigger.id);
    //                    }
    //                }
    //            }

    //            // 再初始化receiver
    //            for (auto control : subAllControl)
    //            {
    //                auto objName = control->objectName();
    //                for (auto& trigger : tempRelation.triggerIdMap)
    //                {
    //                    for (auto& triggerValue : trigger.trigValMap)
    //                    {
    //                        for (auto& receiver : triggerValue.receiverMap)
    //                        {
    //                            if (receiver.id == objName)
    //                            {
    //                                Receiver multiReceiver = receiver;
    //                                multiReceiver.control = control;
    //                                multiReceiver.id = objName + "_" + QString::number(i + 1);

    //                                QString triggerId = trigger.id;
    //                                relation.triggerIdMap[triggerId.append(QString("_%1").arg(i + 1))]
    //                                    .trigValMap[triggerValue.value]
    //                                    .receiverMap[multiReceiver.id] = multiReceiver;
    //                                unnecessaryReceiverIdList.append(receiver.id);
    //                            }
    //                        }
    //                    }
    //                }
    //            }
    //        }

    //        // 删除多余的
    //        for (const auto& triggerId : unnecessaryTriggerIdList)
    //            relation.triggerIdMap.remove(triggerId);

    //        for (const auto& receiverId : unnecessaryReceiverIdList)
    //        {
    //            for (auto& trigger : relation.triggerIdMap)
    //            {
    //                for (auto& triggerValue : trigger.trigValMap)
    //                {
    //                    triggerValue.receiverMap.remove(receiverId);
    //                }
    //            }
    //        }
    //    }
}

void ParamMacroRelationManager::addParamRelation(QMap<QString, Relation>* relationMap, const Relation& relation, const QString& relationKey,
                                                 QMap<int, QMap<QString, WWidget*>> unitDeviceMap)
{
    for (auto& trigger : relation.triggerIdMap)
    {
        auto triggerControl = trigger.control;
        if (!triggerControl)
            continue;

        auto triggerID = trigger.id;
        connect(triggerControl, static_cast<void (WWidget::*)(const QVariant&)>(&WWidget::sig_valueChange),
                [=](const QVariant& value) { onTrigger(relationMap, relationKey, triggerID, value, unitDeviceMap); });
    }
}

void ParamMacroRelationManager::initControlStatus(const Relation& relation)
{
    for (auto& trigger : relation.triggerIdMap)
    {
        auto triggerControl = trigger.control;
        if (!triggerControl)
            continue;

        emit triggerControl->sig_valueChange(triggerControl->value());
    }
}

int ParamMacroRelationManager::conventWorkMode(SystemWorkMode workMode)
{
    if (workMode == SystemWorkMode::STTC)
        return 0x1;
    if (workMode == SystemWorkMode::Skuo2)
        return 0x4;
    if (workMode == SystemWorkMode::KaKuo2)
        return 0x5;
    if (workMode == SystemWorkMode::XDS)
        return 0xFFFF;
    if (workMode == SystemWorkMode::KaDS)
        return 0xFFFF;
    if (workMode == SystemWorkMode::KaGS)
        return 0xFFFF;
    if (workMode == SystemWorkMode::SYTHSMJ)
        return 0x10;
    if (workMode == SystemWorkMode::SYTHWX)
        return 0x11;
    if (workMode == SystemWorkMode::SYTHGML)
        return 0x12;
    if (workMode == SystemWorkMode::SKT)
        return 0x21;
    if (workMode == SystemWorkMode::XGS)
        return 0xFFFF;

    return 0xFFFF;
}
