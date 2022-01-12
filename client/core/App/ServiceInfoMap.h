#ifndef SERVICEINFOMAP_H
#define SERVICEINFOMAP_H
#include <QMap>
#include <QString>
#include <QWidget>

/* 20210401
 * 服务数据信息
 * 这里的逻辑是,界面是基于模块服务的
 * 且一个模块的服务可能会创建多个界面，比如设备监控根据系统号可以创建8个界面
 * 所以之前的逻辑卸载模块的时候只删除一个界面是不合理的
 * 应当以服务为基准,删除当前模块下的所有界面
 */
struct ServiceSubInfo
{
    QMap<QString, QWidget*> menuNameMapWidget;
};

class ServiceInfoMap
{
public:
    ServiceInfoMap();
    ~ServiceInfoMap();
    QMap<QString, QWidget*>& getMenuNameMapWidget(void* key);
    QWidget* getWidget(void* key, const QString& name);
    void insertWidget(void* key, const QString& name, QWidget*);
    void removeWidget(void* key, const QString& name);
    QWidget* removeGetWidget(void* key, const QString& name);
    void unstallService(void* key);

private:
    /* 将服务做一个映射关系
     * 这里的void*是服务的指针
     * 但是这里仅仅只是作为key使用
     */
    QMap<void*, ServiceSubInfo> m_serviceInfoMap;
};

#endif  // SERVICEINFOMAP_H
