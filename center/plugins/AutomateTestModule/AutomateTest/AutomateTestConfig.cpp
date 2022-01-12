#include "AutomateTestConfig.h"
#include "PlatformInterface.h"
#include <QDomDocument>
#include <QFile>
void AutomateTestConfig::parseTestItemConfigImpl(QDomElement& root, AutomateTestItemInfo& testItemInfo, int& index)
{
    auto node = root.firstChild();
    while (!node.isNull())
    {
        auto isComment = node.isComment();
        auto element = node.toElement();
        auto name = node.nodeName();
        node = node.nextSibling();
        /* 跳过注释 */
        if (isComment)
        {
            continue;
        }
        if (name == "params")
        {
            AutomateTestGroup autoTestGroup;
            autoTestGroup.groupName = element.attribute("name").trimmed();
            testItemInfo.groupList << autoTestGroup;
            index = testItemInfo.groupList.size() - 1;
        }
        else if (name == "filed")
        {
            AutomateTestItem autoTestItem;
            autoTestItem.id = element.attribute("id").trimmed();
            autoTestItem.name = element.attribute("name").trimmed();
            autoTestItem.bind = element.attribute("bind").trimmed();
            if (!autoTestItem.bind.isEmpty())
            {
                autoTestItem.bind = PlatformConfigTools::configBusiness(QString("AutomateTestConfigure/%1.xml").arg(autoTestItem.bind));
            }

            auto modeList = element.attribute("mode").trimmed().split("|", QString::SkipEmptyParts);
            for (auto& item : modeList)
            {
                auto modeItem = item.split("=", QString::SkipEmptyParts);
                if (modeItem.size() != 2)
                {
                    continue;
                }
                auto modeEnum = SystemWorkModeHelper::systemWorkModeFromString(modeItem.at(0));
                /* 参数错误 */
                if (modeEnum == NotDefine)
                {
                    continue;
                }
                auto filePath = PlatformConfigTools::configBusiness(QString("AutomateTestConfigure/%1.xml").arg(modeItem.at(1)));
                autoTestItem.workModeMapToParam.insert(modeEnum, filePath);
            }
            testItemInfo.itemMap.insert(autoTestItem.id, autoTestItem);
            /* 这里必须判断,因为配置文件出错也可能导致越界 */
            if (index >= 0 && index < testItemInfo.groupList.size())
            {
                (testItemInfo.groupList)[index].itemList << autoTestItem;
            }
        }
        parseTestItemConfigImpl(element, testItemInfo, index);
    }
}
Optional<AutomateTestItemInfo> AutomateTestConfig::parseTestItemConfig()
{
    using ResType = Optional<AutomateTestItemInfo>;
    /* 此数据是对以上数据的分组 */
    auto configPath = PlatformConfigTools::configBusiness("AutomateTestConfigure/AutomateTestConfigure.xml");

    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        auto errMsg = QString("打开自动化测试配置文件错误:%1").arg(file.errorString());
        return ResType(ErrorCode::OpenFileError, errMsg);
    }

    QDomDocument doc;
    QString errorMsg;
    int errorLine{ 0 };
    int errorColumn{ 0 };
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn))
    {
        auto msg = QString("加载自动化测试配置文件错误: %1:第%2行,第%3列").arg(errorMsg).arg(errorLine).arg(errorColumn);
        return ResType(ErrorCode::XmlParseError, msg);
    }

    auto root = doc.documentElement();

    AutomateTestItemInfo testItemInfo;
    int index = -1;
    parseTestItemConfigImpl(root, testItemInfo, index);
    return ResType(testItemInfo);
}

Optional<TestSingleParameter> AutomateTestConfig::parseSingleTestItem(const QString& filePath)
{
    using ResType = Optional<TestSingleParameter>;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        auto msg = QString("打开测试项配置错误:%1").arg(file.errorString());
        return ResType(ErrorCode::OpenFileError, msg);
    }

    QDomDocument doc;
    QString error;
    int errorRow = -1;
    int errorCol = -1;
    if (!doc.setContent(&file, &error, &errorRow, &errorCol))
    {
        auto msg = QString("加载测试项配置错误：: %1:第%2行,第%3列").arg(error).arg(errorRow).arg(errorCol);
        return ResType(ErrorCode::XmlParseError, msg);
    }

    TestSingleParameter testSingleParameter;
    int index = -1;
    auto root = doc.documentElement();

    auto result = parseSingleTestItemImpl(root, testSingleParameter, index);
    if (!result)
    {
        return ResType(result.errorCode(), result.msg());
    }
    return ResType(testSingleParameter);
}
OptionalNotValue AutomateTestConfig::parseSingleTestItemImpl(QDomElement& root, TestSingleParameter& testItemParameter, int& index)
{
    auto node = root.firstChild();
    while (!node.isNull())
    {
        auto isComment = node.isComment();
        auto element = node.toElement();
        auto name = node.nodeName();
        node = node.nextSibling();
        /* 跳过注释 */
        if (isComment)
        {
            continue;
        }
        if (name == "group")
        {
            TestSingleParameterGroup group;
            group.groupName = element.attribute("name").trimmed();
            if (group.groupName.isEmpty())
            {
                continue;
            }
            testItemParameter.testSingleParameterGroup << group;
            index = testItemParameter.testSingleParameterGroup.size() - 1;
        }
        else if (name == "item")
        {
            auto parseItemResult = DynamicConfigToWidgetUtility::parseXmlItem(element);
            if (!parseItemResult)
            {
                return OptionalNotValue(parseItemResult.errorCode(), parseItemResult.msg());
            }
            if (index >= 0 && index < testItemParameter.testSingleParameterGroup.size())
            {
                (testItemParameter.testSingleParameterGroup)[index].testItemParameter.append(parseItemResult.value());
            }
        }
        parseSingleTestItemImpl(element, testItemParameter, index);
    }

    return OptionalNotValue();
}
