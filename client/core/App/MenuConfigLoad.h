#ifndef MENUCONFIGLOAD_H
#define MENUCONFIGLOAD_H
#include "CoreUtility.h"
#include <QString>

#include <QDomElement>

class MenuConfigLoad
{
public:
    MenuConfigLoad();

    bool load();
    QString errMsg();

    QList<MenuInfo> menuList() const;

private:
    void parseMenuConfigNode(const QDomElement& domEle, QList<MenuInfo>& menuInfos) const;

private:
    QString mErrMsg;
    QList<MenuInfo> mMenuInfos;
};

#endif  // MENUCONFIGLOAD_H
