#include "MenuConfigLoad.h"
#include "PlatformInterface.h"
#include <QDebug>
#include <QFile>
#include <QMessageBox>

MenuConfigLoad::MenuConfigLoad() {}

bool MenuConfigLoad::load()
{
    QFile file(PlatformConfigTools::configPlatform("System/MenuConfig.xml"));
    if (!file.open(QFile::ReadOnly))
    {
        mErrMsg = QString("打开菜单配置文件失败：%1:%2").arg(file.fileName()).arg(file.errorString());
        return false;
    }

    QDomDocument doc;
    QString error;
    int errorRow = -1;
    int errorCol = -1;
    if (!doc.setContent(&file, &error, &errorRow, &errorCol))
    {
        file.close();
        mErrMsg = QString("解析菜单文件错误：%1,在%2第%3行第%4列").arg(error).arg(file.fileName()).arg(errorRow).arg(errorCol);
        return false;
    }
    file.close();

    QDomElement root = doc.documentElement();
    QString defaultLoadNodeName = root.attribute("defaultLoad");
    auto defaultLoadEle = root.firstChildElement(defaultLoadNodeName);
    if (defaultLoadEle.isNull())
    {
        mErrMsg = QString("解析菜单配置文件失败，默认加载菜单配置节点不存在：%1").arg(defaultLoadNodeName);
        return false;
    }

    parseMenuConfigNode(defaultLoadEle, mMenuInfos);

    return true;
}

QString MenuConfigLoad::errMsg() { return mErrMsg; }

QList<MenuInfo> MenuConfigLoad::menuList() const { return mMenuInfos; }

void MenuConfigLoad::parseMenuConfigNode(const QDomElement& domEle, QList<MenuInfo>& menuInfos) const
{
    auto menuNode = domEle.firstChild();
    while (!menuNode.isNull())
    {
        auto menuEle = menuNode.toElement();
        if (!menuEle.isNull())
        {
            if (menuEle.tagName() == "Menu")
            {
                MenuInfo menuInfo;
                menuInfo.name = menuEle.attribute("name");
                menuInfo.moduleName = menuEle.attribute("moduleName");
                menuInfo.subWidgetName = menuEle.attribute("subWidgetName");
                menuInfo.pngSrc = menuEle.attribute("pngSrc",":/image/resource/") + menuEle.attribute("icon", "");
                menuInfo.iconName = menuEle.attribute("icon", "");
                menuInfo.widgetType = MenuInfoHelper::fromStr(menuEle.attribute("type"));
                menuInfo.isDefaultTriggered = menuEle.attribute("defaultTriggered", "0") == "1" ? true : false;

                QStringList list = menuEle.attribute("size").split(",");
                if (list.size() == 2)
                {
                    menuInfo.size = QSize(list[0].toUInt(), list[1].toUInt());
                }

                parseMenuConfigNode(menuEle, menuInfo.subMenus);

                menuInfos.append(menuInfo);
            }
        }

        menuNode = menuNode.nextSibling();
    }
}
