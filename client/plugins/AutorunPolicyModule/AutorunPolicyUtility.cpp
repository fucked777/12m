#include "AutorunPolicyUtility.h"
#include "Utility.h"
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegExp>
#include <QSpinBox>
#include <QTextEdit>
#include <QTreeWidget>
#include <QUuid>
#include <tuple>

Optional<TreeInfo> AutorunPolicyUtility::loadItemConfigXml(const QString& path)
{
    using ResType = Optional<TreeInfo>;
    TreeInfo treeInfo;
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        auto errMsg = QString("自动化运行策略配置加载错误: %1").arg(file.errorString());
        return ResType(ErrorCode::OpenFileError, errMsg);
    }
    QDomDocument doc;
    QString errorMsg;
    int errorLine{ 0 };
    int errorColumn{ 0 };
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn))
    {
        auto errMsg = QString("自动化运行策略配置加载错误: %1:第%2行,第%3列").arg(errorMsg).arg(errorLine).arg(errorColumn);
        return ResType(ErrorCode::OpenFileError, errMsg);
    }

    auto root = doc.documentElement();
    treeInfo.name = root.attribute("name").trimmed();
    auto ret = parseXmlConfig(root, treeInfo.treeList);
    return ResType(ret.errorCode(), ret.msg(), treeInfo);
}

OptionalNotValue AutorunPolicyUtility::parseXmlConfig(QDomElement& element, TreeList& child)
{
    auto parseItemResult = DynamicConfigToWidgetUtility::parseXmlItem(element);
    if (!parseItemResult)
    {
        return OptionalNotValue(parseItemResult.errorCode(), parseItemResult.msg());
    }

    child.commonToWidgetInfo = parseItemResult.value();
    if (child.commonToWidgetInfo.name.isEmpty())
    {
        return OptionalNotValue(ErrorCode::InvalidData, "节点名不能为空");
    }
    if (child.commonToWidgetInfo.id.isEmpty())
    {
        child.commonToWidgetInfo.type = DynamicConfigToWidgetType::Unknown;
    }

    auto node = element.firstChild();
    while (!node.isNull())
    {
        auto childElement = node.toElement();
        node = node.nextSibling();
        auto tagName = childElement.tagName();

        if (tagName == "item")
        {
            TreeList tempTreeList;
            auto res = parseXmlConfig(childElement, tempTreeList);
            if (!res)
            {
                return res;
            }
            child.child.append(tempTreeList);
        }
    }
    return OptionalNotValue();
}

Optional<QList<TreeInfo>> AutorunPolicyUtility::loadConfig(const QString& configDir)
{
    using ResType = Optional<QList<TreeInfo>>;
    QList<TreeInfo> treeInfoList;
    QDir dir(configDir);
    if (!dir.exists())
    {
        auto errMsg = QString("未找到自动化运行策略配置目录:%1").arg(configDir);
        return ResType(ErrorCode::NotFound, errMsg);
    }

    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    auto dirInfoList = dir.entryInfoList();

    for (auto& fileInfoItem : dirInfoList)
    {
        auto ret = loadItemConfigXml(fileInfoItem.filePath());
        if (!ret)
        {
            return ResType(ret.errorCode(), ret.msg());
        }
        treeInfoList << ret.value();
    }
    return ResType(treeInfoList);
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

Optional<ATTreeWidgetInfo> AutorunPolicyUtility::createTreeWidget(const TreeList& treeInfo)
{
    using ResType = Optional<ATTreeWidgetInfo>;
    ATTreeWidgetInfo info;

    auto tree = new QTreeWidget;
    GetValueHelper<QTreeWidget> helper(tree);
    QStringList header;
    header << "名称"
           << "值"
           << "描述";
    tree->setColumnCount(header.size());
    tree->setHeaderLabels(header);

    DynamicWidgetInfo widgetInfo;
    auto topTempWidget = createTreeItem(treeInfo.commonToWidgetInfo, widgetInfo, tree, tree);
    if (!topTempWidget)
    {
        return ResType(topTempWidget.errorCode(), topTempWidget.msg());
    }
    if (widgetInfo.widget != nullptr && !treeInfo.commonToWidgetInfo.id.isEmpty())
    {
        info.treeWidgetMap.insert(treeInfo.commonToWidgetInfo.id, widgetInfo);
    }

    for (auto& item : treeInfo.child)
    {
        auto result = createChild(item, info.treeWidgetMap, tree, topTempWidget.value());
        if (!result)
        {
            return ResType(result.errorCode(), result.msg());
        }
    }
    info.widget = helper.get();
    return ResType(info);
}
