#include "DMUIUtility.h"
#include "DMMessageSerialize.h"
#include "Utility.h"
#include <QComboBox>
#include <QDebug>
#include <QDomDocument>
#include <QDoubleSpinBox>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QTreeWidget>
#include <QUuid>
OptionalNotValue DMUIUtility::parseXmlAttribute(QDomElement& itemElem, TreeList& child)
{
    auto node = itemElem.firstChild();
    while (!node.isNull())
    {
        auto element = node.toElement();
        node = node.nextSibling();
        auto tagName = element.tagName();

        if (tagName == "item")
        {
            auto res = DMUIUtility::parseXmlTree(element, child);
            if (!res)
            {
                return res;
            }
        }
    }
    return OptionalNotValue();
}
OptionalNotValue DMUIUtility::parseXmlConfig(QDomElement& docElem, UIDataParameter& parameter)
{
    auto node = docElem.firstChild();
    while (!node.isNull())
    {
        auto element = node.toElement();
        node = node.nextSibling();
        auto tagName = element.tagName();

        if (tagName == "parameter")
        {
            parameter.fromInfo.name = element.attribute("name").trimmed();
            auto res = DMUIUtility::parseXmlFrom(element, parameter.fromInfo.fromList);
            if (!res)
            {
                return res;
            }
        }
        else if (tagName == "attribute")
        {
            parameter.treeInfo.name = element.attribute("name").trimmed();
            parameter.treeInfo.maxGroup = element.attribute("maxGroup").trimmed().toUInt();
            auto res = parseXmlAttribute(element, parameter.treeInfo.treeList);
            if (!res)
            {
                return res;
            }
        }
    }
    return OptionalNotValue();
}
OptionalNotValue DMUIUtility::parseXmlFrom(QDomElement& itemElem, QList<CommonToWidgetInfo>& parameterList)
{
    auto node = itemElem.firstChild();
    while (!node.isNull())
    {
        auto element = node.toElement();
        node = node.nextSibling();
        auto tagName = element.tagName();

        if (tagName == "item")
        {
            auto parseItemResult = DynamicConfigToWidgetUtility::parseXmlItem(element);
            if (!parseItemResult)
            {
                return OptionalNotValue(parseItemResult.errorCode(), parseItemResult.msg());
            }

            parameterList.append(parseItemResult.value());
        }
    }
    return OptionalNotValue();
}
OptionalNotValue DMUIUtility::parseXmlTree(QDomElement& itemElem, TreeList& child)
{
    auto parseItemResult = DynamicConfigToWidgetUtility::parseXmlItem(itemElem);
    if (!parseItemResult)
    {
        return OptionalNotValue(parseItemResult.errorCode(), parseItemResult.msg());
    }
    child.commonToWidgetInfo = parseItemResult.value();
    if (child.commonToWidgetInfo.id.isEmpty())
    {
        return OptionalNotValue(ErrorCode::XmlParseError, "ID?????????");
    }

    auto node = itemElem.firstChild();
    while (!node.isNull())
    {
        auto element = node.toElement();
        node = node.nextSibling();
        auto tagName = element.tagName();

        if (tagName == "item")
        {
            TreeList tempTreeList;
            auto res = parseXmlTree(element, tempTreeList);
            if (!res)
            {
                return res;
            }
            child.child.append(tempTreeList);
        }
    }
    return OptionalNotValue();
}
Optional<UIDataParameter> DMUIUtility::loadConfigXml(const QString& path)
{
    using ResType = Optional<UIDataParameter>;
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        auto msg = QString("UI????????????????????????: %1").arg(file.errorString());
        return ResType(ErrorCode::OpenFileError, msg);
    }
    QDomDocument doc;
    QString errorMsg;
    int errorLine{ 0 };
    int errorColumn{ 0 };
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn))
    {
        auto msg = QString("????????????????????????????????????: %1:???%2???,???%3???").arg(errorMsg).arg(errorLine).arg(errorColumn);
        return ResType(ErrorCode::XmlParseError, msg);
    }

    UIDataParameter parameter;
    auto root = doc.documentElement();
    parameter.type = root.tagName();
    parameter.name = root.attribute("name").trimmed();
    auto parseResult = parseXmlConfig(root, parameter);
    return ResType(parseResult.errorCode(), parseResult.msg(), parameter);
}

Optional<DMFromWidgetInfo> DMUIUtility::createFromWidget(const FromInfo& param)
{
    using ResType = Optional<DMFromWidgetInfo>;
    DMFromWidgetInfo info;

    info.widget = new QGroupBox;
    info.widget->setTitle(param.name);

    auto layout = new QGridLayout(info.widget);
    layout->setHorizontalSpacing(10);
    layout->setVerticalSpacing(10);
    layout->setContentsMargins(11, 15, 11, 11);

    int row = 0;
    for (auto& item : param.fromList)
    {
        auto createResult = DynamicConfigToWidgetUtility::createWidget(item);
        if (!createResult)
        {
            /* ???????????????????????? */
            for (auto& item : info.fromWidgetInfo)
            {
                item.widget->setParent(nullptr);
                delete item.widget;
            }
            delete info.widget;
            return ResType(createResult.errorCode(), createResult.msg());
        }
        if (createResult->widget == nullptr)
        {
            continue;
        }

        createResult->widget->setParent(info.widget);
        if (!createResult->widgetInfo.id.isEmpty())
        {
            info.fromWidgetInfo << createResult.value();
        }

        auto label = new QLabel(info.widget);
        label->setText(item.name + ": ");

        layout->addWidget(label, row, 0, Qt::AlignLeft | Qt::AlignVCenter);
        layout->addWidget(createResult->widget, row, 1);

        ++row;
    }

    return ResType(info);
}

template<typename T>
static Optional<QTreeWidgetItem*> createTreeItem(const CommonToWidgetInfo& toWidgetInfo, DynamicWidgetInfo& info, QTreeWidget* itemWidget, T* parent)
{
    using ResType = Optional<QTreeWidgetItem*>;
    auto temp = new QTreeWidgetItem(parent);
    temp->setText(0, toWidgetInfo.name);
    if (!toWidgetInfo.desc.isEmpty())
    {
        temp->setText(2, toWidgetInfo.desc);
    }

    auto result = DynamicConfigToWidgetUtility::createWidget(toWidgetInfo);
    if (!result)
    {
        return ResType(result.errorCode(), result.msg());
    }

    info = result.value();
    if (info.widget != nullptr)
    {
        info.widget->setParent(itemWidget);
        itemWidget->setItemWidget(temp, 1, info.widget);
    }

    return ResType(temp);
}
template<typename T>
static OptionalNotValue createChild(const TreeList& treeList, QMap<QString, DynamicWidgetInfo>& treeWidgetMap, QTreeWidget* itemWidget, T* parent)
{
    DynamicWidgetInfo info;
    auto createTreeItemResult = createTreeItem(treeList.commonToWidgetInfo, info, itemWidget, parent);
    if (!createTreeItemResult)
    {
        return OptionalNotValue(createTreeItemResult.errorCode(), createTreeItemResult.msg());
    }
    if (info.widget != nullptr && !treeList.commonToWidgetInfo.id.isEmpty())
    {
        treeWidgetMap.insert(treeList.commonToWidgetInfo.id, info);
    }

    for (auto& attr : treeList.child)
    {
        auto res = createChild(attr, treeWidgetMap, itemWidget, createTreeItemResult.value());
        if (!res)
        {
            return res;
        }
    }
    return OptionalNotValue();
}

Optional<DMTreeWidgetInfo> DMUIUtility::createTreeWidget(const TreeInfo& attrs, int num)
{
    using ResType = Optional<DMTreeWidgetInfo>;
    DMTreeWidgetInfo info;

    auto& tree = info.widget;
    auto& listWidget = info.treeWidgetInfoList;
    tree = new QTreeWidget;
    QStringList header;
    header << "??????"
           << "???"
           << "??????";

    tree->setColumnCount(header.size());
    tree->setHeaderLabels(header);
    tree->setColumnWidth(0, 200);
    tree->setColumnWidth(1, 200);
    tree->setStyleSheet("QTreeWidget::item {margin:2px};");

    /* ???????????? */
    QList<TreeInfo> listInfo;
    for (int i = 0; i < num; ++i)
    {
        auto tempAttrs = attrs;
        tempAttrs.treeList.commonToWidgetInfo.name = QString("%1%2").arg(tempAttrs.treeList.commonToWidgetInfo.name).arg(i + 1);
        listInfo.append(tempAttrs);
    }
    /* ?????????????????? */
    for (auto& item : listInfo)
    {
        QMap<QString, DynamicWidgetInfo> treeWidgetMap;
        auto createResult = createChild(item.treeList, treeWidgetMap, tree, tree);
        if (!createResult)
        {
            return ResType(createResult.errorCode(), createResult.msg());
        }
        listWidget << treeWidgetMap;
    }

    tree->expandAll();

    return ResType(info);
}

OptionalNotValue DMUIUtility::refreshTreeWidget(quint32 channelNum, QTabWidget* treeUI, DMTreeWidgetInfo& attr, const TreeInfo& treeInfo)
{
    if (channelNum <= 0)
    {
        return OptionalNotValue(ErrorCode::InvalidArgument, "??????????????????");
    }
    if (treeInfo.treeList.child.isEmpty())
    {
        return OptionalNotValue();
    }
    /* ?????????????????????????????? */
    auto bak = attr;
    attr = DMTreeWidgetInfo();

    QList<QVariantMap> treeDataMap;
    auto widget = treeUI->widget(0);
    if (widget != nullptr)
    {
        /* ???????????????????????? */
        treeDataMap = DMUIUtility::getTreeWidgetDataNotCheck(bak.treeWidgetInfoList);
        /* ???????????? */
        treeUI->removeTab(0);
        widget->setParent(nullptr);
        delete widget;
    }
    auto len = channelNum > treeInfo.maxGroup ? treeInfo.maxGroup : channelNum;

    /* ???????????? */
    auto createTreeWidgetResult = createTreeWidget(treeInfo, len);
    if (!createTreeWidgetResult)
    {
        return OptionalNotValue(createTreeWidgetResult.errorCode(), createTreeWidgetResult.msg());
    }
    widget = new QWidget(treeUI);
    createTreeWidgetResult->widget->setParent(widget);

    auto layout = new QVBoxLayout(widget);
    layout->addWidget(createTreeWidgetResult->widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    treeUI->addTab(widget, treeInfo.name);
    treeUI->setCurrentWidget(widget);

    /* ????????? */
    setTreeWidgetData(createTreeWidgetResult->treeWidgetInfoList, treeDataMap);
    attr = createTreeWidgetResult.value();
    return OptionalNotValue();
}

void DMUIUtility::setTreeWidgetData(QList<QMap<QString, DynamicWidgetInfo>>& treeListWidget, const QList<QVariantMap>& objDataMap)
{
    auto size = qMin(treeListWidget.size(), objDataMap.size());
    for (int i = 0; i < size; ++i)
    {
        auto& widgetMap = treeListWidget[i];
        auto& objMap = objDataMap[i];
        for (auto iter = widgetMap.begin(); iter != widgetMap.end(); ++iter)
        {
            auto find = objMap.find(iter.key());
            if (find == objMap.end())
            {
                DynamicConfigToWidgetUtility::resetValue(iter.value());
            }
            else
            {
                DynamicConfigToWidgetUtility::setWidgetValue(iter.value(), find.value());
            }
        }
    }
}
void DMUIUtility::setTreeWidgetData(QTabWidget* treeUI, const TreeInfo& treeInfo, DMTreeWidgetInfo& attr, const QList<QVariantMap>& objDataMap)
{
    /* ?????????????????????????????? */
    attr = DMTreeWidgetInfo();

    /* ???????????????????????? */
    auto widget = treeUI->widget(0);
    if (widget != nullptr)
    {
        treeUI->removeTab(0);
        widget->setParent(nullptr);
        delete widget;
    }
    /* ?????????????????? */
    auto size = objDataMap.size();
    auto createTreeWidgetResult = createTreeWidget(treeInfo, size);
    if (!createTreeWidgetResult)
    {
        return;
    }

    attr = createTreeWidgetResult.value();
    widget = new QWidget(treeUI);
    createTreeWidgetResult->widget->setParent(widget);

    auto layout = new QVBoxLayout(widget);
    layout->addWidget(createTreeWidgetResult->widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    treeUI->addTab(widget, treeInfo.name);
    treeUI->setCurrentWidget(widget);

    for (int i = 0; i < size; ++i)
    {
        auto& widgetMap = (attr.treeWidgetInfoList)[i];
        auto& objMap = objDataMap[i];
        for (auto iter = widgetMap.begin(); iter != widgetMap.end(); ++iter)
        {
            auto find = objMap.find(iter.key());
            if (find == objMap.end())
            {
                DynamicConfigToWidgetUtility::resetValue(iter.value());
            }
            else
            {
                DynamicConfigToWidgetUtility::setWidgetValue(iter.value(), find.value());
            }
        }
    }
}

Optional<QList<QVariantMap>> DMUIUtility::getTreeWidgetData(const QList<QMap<QString, DynamicWidgetInfo>>& treeListWidget)
{
    using ResType = Optional<QList<QVariantMap>>;
    QList<QVariantMap> resDataMap;
    QList<std::tuple<QString, QVariant>> onlyOneList;
    for (auto& widgetMap : treeListWidget)
    {
        QVariantMap tempMap;
        for (auto& item : widgetMap)
        {
            auto value = DynamicConfigToWidgetUtility::getWidgetValue(item);
            if (!value)
            {
                return ResType(value.errorCode(), value.msg());
            }
            if (item.widgetInfo.onlyOne)
            {
                onlyOneList << std::make_tuple(item.widgetInfo.id, value.value());
            }
            tempMap[item.widgetInfo.id] = value.value();
        }
        resDataMap << tempMap;
    }
    /* ??????  onlyOne */
    QMap<QString, QSet<QString>> checkMap;
    for (auto& item : onlyOneList)
    {
        checkMap[std::get<0>(item)].insert(std::get<1>(item).toString());
    }
    int count = 0;
    for (auto& item1 : checkMap)
    {
        count += item1.size();
    }
    if (count != onlyOneList.size())
    {
        return ResType(ErrorCode::InvalidData, "??????????????????????????????,???????????????");
    }
    return ResType(resDataMap);
}
QList<QVariantMap> DMUIUtility::getTreeWidgetDataNotCheck(const QList<QMap<QString, DynamicWidgetInfo>>& treeListWidget)
{
    QList<QVariantMap> resDataMap;
    for (auto& widgetMap : treeListWidget)
    {
        QVariantMap tempMap;
        for (auto& item : widgetMap)
        {
            auto value = DynamicConfigToWidgetUtility::getWidgetValue(item);
            if (!value)
            {
                continue;
            }
            tempMap[item.widgetInfo.id] = value.value();
        }
        resDataMap << tempMap;
    }

    return resDataMap;
}

void DMUIUtility::setFromWidgetData(QList<DynamicWidgetInfo>& listWidget, const QVariantMap& objDataMap)
{
    for (auto& item : listWidget)
    {
        auto find = objDataMap.find(item.widgetInfo.id);
        if (find == objDataMap.end())
        {
            DynamicConfigToWidgetUtility::resetValue(item);
        }
        else
        {
            DynamicConfigToWidgetUtility::setWidgetValue(item, find.value());
        }
    }
}
Optional<QVariantMap> DMUIUtility::getFromWidgetData(const QList<DynamicWidgetInfo>& listWidget)
{
    using ResType = Optional<QVariantMap>;
    QVariantMap result;
    for (auto& item : listWidget)
    {
        auto value = DynamicConfigToWidgetUtility::getWidgetValue(item);
        if (!value)
        {
            return ResType(value.errorCode(), value.msg());
        }

        result.insert(item.widgetInfo.id, value.value());
    }
    return ResType(result);
}
