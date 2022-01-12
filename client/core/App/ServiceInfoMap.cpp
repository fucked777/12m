#include "ServiceInfoMap.h"

ServiceInfoMap::ServiceInfoMap() {}
ServiceInfoMap::~ServiceInfoMap()
{
    for (auto& info : m_serviceInfoMap)
    {
        for (auto& item : info.menuNameMapWidget)
        {
            item->deleteLater();
        }
    }
    m_serviceInfoMap.clear();
}

QMap<QString, QWidget*>& ServiceInfoMap::getMenuNameMapWidget(void* key)
{
    auto serviceFind = m_serviceInfoMap.find(key);
    if (serviceFind == m_serviceInfoMap.end())
    {
        return m_serviceInfoMap[key].menuNameMapWidget;
    }
    return serviceFind->menuNameMapWidget;
}
QWidget* ServiceInfoMap::getWidget(void* key, const QString& name)
{
    auto serviceFind = m_serviceInfoMap.find(key);
    if (serviceFind == m_serviceInfoMap.end())
    {
        return nullptr;
    }
    auto& info = serviceFind->menuNameMapWidget;
    auto widgetFind = info.find(name);
    if (widgetFind == info.end())
    {
        return nullptr;
    }
    return widgetFind.value();
}
void ServiceInfoMap::insertWidget(void* key, const QString& name, QWidget* widget)
{
    Q_ASSERT(widget != nullptr);

    auto& info = m_serviceInfoMap[key].menuNameMapWidget;
    auto widgetFind = info.find(name);
    if (widgetFind != info.end())
    {
        if (widgetFind.value() != widget)
        {
            /* 同一个菜单多个控件,且是不同的,销毁就行 */
            widgetFind.value()->deleteLater();
        }
    }
    info[name] = widget;
}
void ServiceInfoMap::removeWidget(void* key, const QString& name)
{
    auto widget = removeGetWidget(key, name);
    if (widget != nullptr)
    {
        widget->deleteLater();
    }
}
QWidget* ServiceInfoMap::removeGetWidget(void* key, const QString& name)
{
    auto serviceFind = m_serviceInfoMap.find(key);
    if (serviceFind == m_serviceInfoMap.end())
    {
        return nullptr;
    }
    QWidget* ret = nullptr;
    auto& info = serviceFind->menuNameMapWidget;
    auto widgetFind = info.find(name);
    if (widgetFind != info.end())
    {
        ret = widgetFind.value();
        info.erase(widgetFind);
    }
    return ret;
}
void ServiceInfoMap::unstallService(void* key)
{
    auto serviceFind = m_serviceInfoMap.find(key);
    if (serviceFind == m_serviceInfoMap.end())
    {
        return;
    }
    auto& info = serviceFind->menuNameMapWidget;
    for (auto& item : info)
    {
        if (item != nullptr)
        {
            item->deleteLater();
        }
    }
    m_serviceInfoMap.erase(serviceFind);
}
// void ServiceInfoMap::insert(void*service, const QString&name, QWidget*)
//{
//    auto &info = m_serviceInfoMap[service];
//    info.
//}
