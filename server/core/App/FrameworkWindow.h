#ifndef FRAMEWORKWINDOW_H
#define FRAMEWORKWINDOW_H

#include "CppMicroServicesUtility.h"
#include <QMainWindow>
#include <QMap>

struct Menu
{
    QString name;
    QString module;
    QString type;

    QList<Menu> subMenus;
};

using namespace cppmicroservices;
class QMenu;
struct ModuleInfo;
struct MenuInfo;

enum class MenuWidgetType;
class FrameworkWindowImpl;
class FrameworkWindow : public QMainWindow
{
    Q_OBJECT

public:
    FrameworkWindow(cppmicroservices::BundleContext context, QWidget* parent = 0);
    ~FrameworkWindow();
    // 设置模块信息
    void setModuleInfo(const QList<ModuleInfo>&);
    // 设置菜单信息
    void setMenuInfo(const QList<MenuInfo>& menuInfos);
    // 初始化
    void init();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void install();
    // 创建菜单
    void createMenu(const QList<MenuInfo>& menuInfos, QMenu* parentMenu);

    void uninstallGuiServices(ServiceReferenceU serviceRef);

signals:
    void signalInstallGuiServices(ServiceReferenceU service_ref);
    void signalUninstallGuiServices(ServiceReferenceU service_ref);

private slots:
    // 菜单点击
    void slotMenuActionClicked(const MenuInfo& menuInfo);

private:
    FrameworkWindowImpl* m_impl;
};

#endif  // FRAMEWORKWINDOW_H
