#include "AutomateTestUtility.h"
#include "GlobalData.h"
#include <QDomDocument>
#include <QFile>
#include <QLabel>
#include <QVBoxLayout>

NoneParam::NoneParam(QWidget* parent)
    : QWidget(parent)
    , m_text(new QLabel(this))
{
    m_text->setAlignment(Qt::AlignCenter);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addStretch();
    mainLayout->addWidget(m_text);
    mainLayout->addStretch();
}
NoneParam::~NoneParam() {}
void NoneParam::setText(const QString& text) { m_text->setText(text); }

/* 查询参数绑定的数据 */
AutomateTestParameterBindMap AutomateTestUtility::queryParameterBind()
{
    AutomateTestParameterBindMap result;
    GlobalData::getAutomateTestParameterBindMap(result);
    return result;
}
OptionalNotValue AutomateTestUtility::parseParameterBindingConfigImpl(const QStringList& valueList, QDomElement& root,
                                                                      ParameterBindingConfigRelation& info)
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
        if (name == "relation")
        {
            auto tempList = element.attribute("value").trimmed().split("|", QString::SkipEmptyParts);
            auto parseItemResult = DynamicConfigToWidgetUtility::parseXmlItem(element);
            if (!parseItemResult)
            {
                return OptionalNotValue(parseItemResult.errorCode(), parseItemResult.msg());
            }
            for (auto& item : tempList)
            {
                info.configMap[item].testItemParameter << parseItemResult.value();
            }
        }
        else if (name == "item")
        {
            auto parseItemResult = DynamicConfigToWidgetUtility::parseXmlItem(element);
            if (!parseItemResult)
            {
                return OptionalNotValue(parseItemResult.errorCode(), parseItemResult.msg());
            }
            for (auto& item : valueList)
            {
                info.configMap[item].testItemParameter << parseItemResult.value();
            }
        }
        else if (name == "head")
        {
            QList<std::tuple<QString, QString>> tempHeadList;
            auto tempList = element.attribute("value").split("|", QString::SkipEmptyParts);
            for (auto& item : tempList)
            {
                auto kvList = item.split("=", QString::SkipEmptyParts);
                if (kvList.size() != 2)
                {
                    return OptionalNotValue(ErrorCode::InvalidData, "表头解析错误");
                }
                tempHeadList << std::make_tuple(kvList.at(0), kvList.at(1));
            }
            for (auto& item : valueList)
            {
                info.configMap[item].headList << tempHeadList;
            }
        }
        parseParameterBindingConfigImpl(valueList, element, info);
    }
    return OptionalNotValue();
}
Optional<ParameterBindingConfigRelation> AutomateTestUtility::parseParameterBindingConfig(const AutomateTestItem& testItem)
{
    using ResType = Optional<ParameterBindingConfigRelation>;
    QFile file(testItem.bind);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        auto errMsg = QString("打开%1参数绑定配置文件错误:%2").arg(testItem.name, file.errorString());
        return ResType(ErrorCode::OpenFileError, errMsg);
    }

    QDomDocument doc;
    QString errorMsg;
    int errorLine{ 0 };
    int errorColumn{ 0 };
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn))
    {
        auto msg = QString("加载%1参数绑定配置文件错误: %2:第%3行,第%4列").arg(testItem.name).arg(errorMsg).arg(errorLine).arg(errorColumn);
        return ResType(ErrorCode::XmlParseError, msg);
    }

    auto root = doc.documentElement();
    ParameterBindingConfigRelation parameterBindingConfig;
    parameterBindingConfig.relation = root.attribute("relation").trimmed();
    auto valueList = root.attribute("allValue").split("|", QString::SkipEmptyParts);
    if (valueList.isEmpty())
    {
        valueList << "all";
    }

    auto result = parseParameterBindingConfigImpl(valueList, root, parameterBindingConfig);
    if (!result)
    {
        return ResType(result.errorCode(), result.msg());
    }
    return ResType(parameterBindingConfig);
}
