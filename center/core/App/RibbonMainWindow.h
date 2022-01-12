#ifndef RIBBONMAINWINDOW_H
#define RIBBONMAINWINDOW_H
#include "SARibbonBar/SARibbonMainWindow.h"

#include "ServiceInfoMap.h"

#include "CoreUtility.h"
#include "CppMicroServicesUtility.h"
#include <QMainWindow>
#include <QMap>

class QDomElement;
class QAction;
class QMenu;
class QMenuBar;
class QMdiArea;
class SARibbonCategory;
class SARibbonContextCategory;
class QTextEdit;
class SARibbonPannel;

using namespace cppmicroservices;

class QMenu;

struct ModuleInfo;
struct MenuInfo;

enum class MenuWidgetType;

class RibbonMainWindow : public SARibbonMainWindow
{
    Q_OBJECT
public:
    explicit RibbonMainWindow(cppmicroservices::BundleContext context, QWidget* parent = 0);
    ~RibbonMainWindow();
    void closeApplication();

private:
private slots:
    void onShowContextCategory(bool on);

private:
private slots:
    void onMenuButtonPopupCheckableTest(bool b);
    void onInstantPopupCheckableTest(bool b);
    void onDelayedPopupCheckableTest(bool b);

    void onMenuButtonPopupCheckabletriggered(bool b);
    void onInstantPopupCheckabletriggered(bool b);
    void onDelayedPopupCheckabletriggered(bool b);

public:
    // 设置模块信息
    void setModuleInfo(const QList<ModuleInfo>&);
    // 设置菜单信息
    void setMenuInfo(const QList<MenuInfo>& menuInfos);
    // 初始化
    void init();
    // 登录
    void login();

protected:
    void closeEvent(QCloseEvent* event);
    void closeMenu(MenuInfo menuInfo);

private:
    // 创建菜单
    void createMenu(const QList<MenuInfo>& menuInfos, SARibbonPannel* pannel);

    // void installGuiServices(ServiceReferenceU serviceRef);
    void uninstallGuiServices(ServiceReferenceU serviceRef);

signals:
    // void signalInstallGuiServices(ServiceReferenceU service_ref);
    void signalUninstallGuiServices(ServiceReferenceU service_ref);

private slots:
    // 菜单点击
    void slotMenuActionClicked(const MenuInfo& menuInfo);
    void showEvent(QShowEvent* event);

private:
    BundleContext mContext;

    ListenerToken mListenerToken;

    QList<MenuInfo> mMenuInfos;      // 菜单信息
    QList<ModuleInfo> mModuleInfos;  // 模块信息

    QMap<QString, QWidget*> mWidgetMap;

    QString m_ribbonIconPath;

    ServiceInfoMap serviceInfoMap;
    MenuInfo defaultMenuInfo;
    MenuInfo* m_pLoginMenuInfo = nullptr;
    QDialog* m_pLoginDialog = nullptr;
    QMdiArea* m_pMdiArea = nullptr;

private:
    SARibbonContextCategory* m_contextCategory;
};

#endif  // RIBBONMAINWINDOW_H
