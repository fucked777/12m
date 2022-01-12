#include "Wizard.h"
#include "CppMicroServicesUtility.h"
#include "PlatformInterface.h"
#include "Utility.h"
#include <QButtonGroup>
#include <QCoreApplication>
#include <QDebug>
#include <QDomComment>
#include <QFile>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

struct WizardInfo
{
    QString name;
    QVariantMap configMap;
    // PlatformConfigInfo info;
};

class WizardImpl
{
public:
    cppmicroservices::BundleContext context;
    QString configPath;
    QButtonGroup* buttonGroup{ nullptr };
    PlatformConfigInfo info;
    /* 20210306 暂时向导只设计更改业务相关的数据目录
     * 理论上此界面可更改除instanceID 向导界面本身的所有配置
     *
     * 先把配置写全预留一下
     */
    QList<WizardInfo> configList;

    /* 配置文件查找 */
    bool findConfig(PlatformConfigInfo& info);

    OptionalNotValue parseConfigXml();
    Optional<QDomDocument> openXml();
    static void loadGroupConfig(QDomElement& element, WizardInfo& info);
};

bool WizardImpl::findConfig(PlatformConfigInfo& info)
{
    /*
     * 此插件的配置文件是Wizard.xml
     * 查找路径依次是业务目录的上一级-->主程序运行目录-->程序运行目录下的Resources目录如果这两个目录都未找到则认为配置文件不存在
     * 当前界面不会弹出,会直接跳过
     */

    /* 业务目录的上一级 */
    auto tempDir = Utility::dirStitchingQt(info.mainInfo.businessConfigDir, "../Wizard.xml");
    if (QFile::exists(tempDir))
    {
        configPath = tempDir;
        return true;
    }

    /* 主程序运行目录 */
    tempDir = Utility::dirStitchingQt(QCoreApplication::applicationDirPath(), "Wizard.xml");
    if (QFile::exists(tempDir))
    {
        configPath = tempDir;
        return true;
    }

    /* 程序运行目录下的Resources目录 */
    tempDir = Utility::dirStitchingQt(QCoreApplication::applicationDirPath(), "Resources/Wizard.xml");
    if (QFile::exists(tempDir))
    {
        configPath = tempDir;
        return true;
    }
    return false;
}
void WizardImpl::loadGroupConfig(QDomElement& element, WizardInfo& info)
{
    auto configNode = element.firstChild();
    info.name = element.attribute("name").trimmed();
    while (!configNode.isNull())
    {
        auto name = configNode.nodeName();
        auto configElement = configNode.toElement();
        configNode = configNode.nextSibling();

        info.configMap.insert(name, configElement.text());
    }
}

Optional<QDomDocument> WizardImpl::openXml()
{
    using ResType = Optional<QDomDocument>;

    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        auto errMsg = QString("向导配置打开失败:%1:%2").arg(file.errorString(), configPath);
        return ResType(ErrorCode::OpenFileError, errMsg);
    }

    QDomDocument domDoc;
    QString errStr;
    int errLine{ 0 };
    int errcCol{ 0 };
    if (!domDoc.setContent(&file, &errStr, &errLine, &errcCol))
    {
        errStr = QString("向导配置信息解析失败:%1,错误行:%2,错误列:%2").arg(errStr).arg(errLine).arg(errcCol);
        return ResType(ErrorCode::XmlParseError, errStr);
    }
    return ResType(domDoc);
}

OptionalNotValue WizardImpl::parseConfigXml()
{
    QList<WizardInfo> tempList;

    auto docOption = openXml();

    if (!docOption.success())
    {
        return OptionalNotValue(docOption.errorCode(), docOption.msg());
    }

    auto root = docOption->documentElement();

    auto node = root.firstChild();
    while (!node.isNull())
    {
        auto isComment = node.isComment();
        auto name = node.nodeName();
        auto element = node.toElement();
        node = node.nextSibling();
        /* 跳过注释 */
        if (isComment)
        {
            continue;
        }
        if (name == "Group")
        {
            WizardInfo wizardInfo;
            loadGroupConfig(element, wizardInfo);
            if (wizardInfo.name.isEmpty())
            {
                continue;
            }
            tempList << wizardInfo;
        }
    }

    configList = tempList;
    return OptionalNotValue();
}
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
Wizard::Wizard(cppmicroservices::BundleContext context, QWidget* parent)
    : IWizard(parent)
    , m_impl(new WizardImpl)
{
    m_impl->context = context;
}

Wizard::~Wizard() { delete m_impl; }
bool Wizard::init(const PlatformConfigInfo& info)
{
    m_impl->info = info;
    /* 配置文件未找到 */
    if (!m_impl->findConfig(m_impl->info))
    {
        m_errMsg = "向导配置未找到";
        return false;
    }

    /* 解析配置文件 */
    auto parseResult = m_impl->parseConfigXml();
    if (!parseResult)
    {
        m_errMsg = parseResult.msg();
        return false;
    }
    if (m_impl->configList.isEmpty())
    {
        m_errMsg = "向导配置内容为空";
        return false;
    }
    /* 初始化UI */

    this->setMinimumSize(100, 200);
    m_impl->buttonGroup = new QButtonGroup(this);

    auto radioLayout = new QVBoxLayout;
    radioLayout->setContentsMargins(0, 0, 0, 0);
    radioLayout->setSpacing(15);

    int index = 0;
    for (auto& item : m_impl->configList)
    {
        auto button = new QRadioButton(item.name, this);
        m_impl->buttonGroup->addButton(button, index++);

        radioLayout->addWidget(button);
    }
    m_impl->buttonGroup->button(0)->setChecked(true);

    auto radioHLayout = new QHBoxLayout;
    radioHLayout->setContentsMargins(0, 10, 0, 0);
    radioHLayout->setSpacing(0);

    radioHLayout->addStretch();
    radioHLayout->addLayout(radioLayout);
    radioHLayout->addStretch();

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(10);

    auto button = new QPushButton(this);
    button->setText("确认");
    buttonLayout->addStretch();
    buttonLayout->addWidget(button);
    connect(button, &QPushButton::clicked, this, &Wizard::ok);

    button = new QPushButton(this);
    button->setText("取消");
    buttonLayout->addWidget(button);
    connect(button, &QPushButton::clicked, this, &Wizard::cancel);

    auto mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    mainLayout->addLayout(radioHLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    this->setLayout(mainLayout);
    this->setWindowFlags(Qt::WindowCloseButtonHint);
    setWindowTitle("服务器");

    return true;
}
PlatformConfigInfo Wizard::platformConfigInfo() const { return m_impl->info; }
void Wizard::ok()
{
    auto index = m_impl->buttonGroup->checkedId();
    auto info = m_impl->configList.at(index);
    PlatformConfigTools::platformConfigFromMap(m_impl->info, info.configMap);
    this->accept();
}
void Wizard::cancel()
{
    /* 什么也不更改,直接关闭窗口就行 */
    this->reject();
}
