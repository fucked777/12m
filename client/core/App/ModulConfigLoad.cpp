#include "ModulConfigLoad.h"
#include "PlatformInterface.h"
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QMap>
#include <QSet>
#include <QStack>
#include <QVariant>

struct ModuleRelationalInfo
{
    QString depend;
    QString on;
};
using RelationalList = QList<ModuleRelationalInfo>;

class ModulConfigLoadImpl
{
public:
    /* 模块配置文件的读取 */
    static void moduleConfigXML(QDomElement& docElem, QMap<QString, ModuleInfo>& coreModule, QMap<QString, ModuleInfo>& generalModule,
                                RelationalList& relationalList, QMap<QString, ModuleInfo>* tempModuleList);
    /* 当一个模块同时存在于核心模块和普通模块时此模块会被认为是普通模块 */
    static void moduleContainsAnalysis(QMap<QString, ModuleInfo>& coreModule, QMap<QString, ModuleInfo>& generalModule);
    /* 解析模块的依赖关系 */
    static bool moduleRelationalAnalysis(QMap<QString, ModuleInfo>& generalModule, RelationalList& relationalList, QList<ModuleInfo>&);
};
void ModulConfigLoadImpl::moduleConfigXML(QDomElement& docElem, QMap<QString, ModuleInfo>& coreModule, QMap<QString, ModuleInfo>& generalModule,
                                          RelationalList& relationalList, QMap<QString, ModuleInfo>* tempModule)
{
    auto node = docElem.firstChild();
    while (!node.isNull())
    {
        auto name = node.nodeName();
        auto isComment = node.isComment();
        auto element = node.toElement();
        node = node.nextSibling();
        /* 跳过注释 */
        if (isComment)
        {
            continue;
        }

        if (name == "core")
        {
            tempModule = &coreModule;
        }
        else if (name == "general")
        {
            tempModule = &generalModule;
        }
        else if (name == "module")
        {
            ModuleInfo info;
            info.name = element.attribute("name").trimmed();
            info.use = QVariant(element.attribute("use", "true").trimmed()).toBool();
            if (!name.isEmpty())
            {
                tempModule->insert(info.name, info);
            }
        }
        else if (name == "item")
        {
            ModuleRelationalInfo info;
            info.depend = element.attribute("depend").trimmed();
            info.on = element.attribute("on").trimmed();
            if (!info.depend.isEmpty() && !info.on.isEmpty())
            {
                relationalList << info;
            }
        }
        moduleConfigXML(element, coreModule, generalModule, relationalList, tempModule);
    }
}

void ModulConfigLoadImpl::moduleContainsAnalysis(QMap<QString, ModuleInfo>& coreModule, QMap<QString, ModuleInfo>& generalModule)
{
    for (auto iter = coreModule.begin(); iter != coreModule.end();)
    {
        if (generalModule.contains(iter.key()))
        {
            iter = coreModule.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

bool ModulConfigLoadImpl::moduleRelationalAnalysis(QMap<QString, ModuleInfo>& generalModule, RelationalList& relationalList,
                                                   QList<ModuleInfo>& result)
{
    /* 创建邻接表 */
    QMap<QString, int> inDegree; /* 每个节点的入度 */
    QMap<QString, std::tuple<QSet<QString>, ModuleInfo>> graph;
    for (auto& item : generalModule)
    {
        graph[item.name] = std::make_tuple(QSet<QString>(), item);
    }
    for (auto& item : relationalList)
    {
        auto find = graph.find(item.on);
        if (find != graph.end())
        {
            std::get<0>(find.value()).insert(item.depend);
            ++inDegree[item.depend];
        }
    }

    /* 开始排序 */
    /* 查找度为0的顶点，作为起始点 */
    QStack<QString> startStack;
    for (auto iter = graph.begin(); iter != graph.end(); ++iter)
    {
        auto& key = iter.key();
        /* 入度为0 */
        if (inDegree[key] == 0)
        {
            startStack.push(key);
        }
    }

    while (!startStack.isEmpty())
    {
        auto key = startStack.pop();
        auto& tupleR = graph[key];
        auto& setR = std::get<0>(tupleR);
        auto& infoR = std::get<1>(tupleR);
        for (auto& item : setR)
        {
            auto& inDegreeR = inDegree[item];
            --inDegreeR;
            if (inDegreeR == 0)
            {
                startStack.push(item);
            }
        }
        result << infoR;
    }

    return result.size() == generalModule.size();
}
ModulConfigLoad::ModulConfigLoad() {}
ModulConfigLoad::~ModulConfigLoad() {}

bool ModulConfigLoad::load()
{
    auto xmlFile = PlatformConfigTools::configPlatform("System/ModuleConfig.xml");
    QFile file(xmlFile);
    if (!file.open(QFile::ReadOnly))
    {
        m_errMsg = file.errorString();
        return false;
    }
    QDomDocument doc;

    QString errMsg;
    int row = 0;
    int col = 0;
    if (!doc.setContent(&file, &errMsg, &row, &col))
    {
        m_errMsg = QString("在%1行%2列出现错误:%3").arg(row).arg(col).arg(errMsg);
        return false;
    }

    QMap<QString, ModuleInfo> coreModule;                   /* 核心模块列表 */
    QMap<QString, ModuleInfo> generalModule;                /* 通用模块列表 */
    QMap<QString, ModuleInfo>* tempModule = &generalModule; /* 临时的变量 */
    RelationalList relationalList;                          /* 模块的关系列表 */
    auto root = doc.documentElement();
    ModulConfigLoadImpl::moduleConfigXML(root, coreModule, generalModule, relationalList, tempModule);

    /*
     * 模块配置解析完成,这里做一个分析
     * 当一个模块同时存在于核心模块和普通模块时此模块会被认为是普通模块
     */
    ModulConfigLoadImpl::moduleContainsAnalysis(coreModule, generalModule);
    /* 排序开始 */
    QList<ModuleInfo> moduleList;
    if (!ModulConfigLoadImpl::moduleRelationalAnalysis(generalModule, relationalList, moduleList))
    {
        m_errMsg = "模块间相互依赖,加载顺序解析失败";
        return false;
    }

    /* 排序完成后,应当加上核心模块的列表 */
    for (auto& item : coreModule)
    {
        m_moduleList << item;
    }
    m_moduleList << moduleList;
    return true;
}
