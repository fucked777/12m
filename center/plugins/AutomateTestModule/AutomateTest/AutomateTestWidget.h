#ifndef AUTOMATETESTWIDGET_H
#define AUTOMATETESTWIDGET_H

#include "AutomateTestDefine.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class AutomateTestWidget;
}
namespace cppmicroservices
{
    class BundleContext;
}
QT_END_NAMESPACE

class AutomateTestWidgetImpl;
class AutomateTestWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AutomateTestWidget(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~AutomateTestWidget() override;

    void deleteTestItemACK(const QByteArray& data); /* 删除测试项 */
    void startTestACK(const QByteArray& data);      /* 开始测试 */
    void stopTestACK(const QByteArray& data);       /* 终止测试 */
    /* 查询测试项 现在是redis推送 认为数据量不大全推redis上去了
     * 接口保留
     * 有问题改成这种查询
     */
    void queryTestItemACK(const QByteArray& data);

signals:
    void sg_addTestItemACK(const QByteArray& data);   /* 添加测试项 */
    void sg_addTestBindItemACK(const QByteArray&);    /* 添加参数绑定 */
    void sg_deleteTestBindItemACK(const QByteArray&); /* 删除参数绑定 */

    void sg_clearHistryACK(const QByteArray& data); /* 清理测试结果历史数据 */
    void sg_deleteTestResultACK(const QByteArray&); /* 删除测试结果 */
    void sg_queryTestResultACK(const QByteArray&);  /* 查询测试结果 */

    void sg_addTestItem(const QByteArray&);        /* 添加测试项 */
    void sg_addTestBindItem(const QByteArray&);    /* 添加参数绑定 */
    void sg_deleteTestBindItem(const QByteArray&); /* 删除参数绑定 */
    void sg_deleteTestItem(const QByteArray&);     /* 删除测试项 */
    void sg_deleteTestResult(const QByteArray&);   /* 删除测试结果 */
    void sg_clearHistry(const QByteArray& data);   /* 清理测试结果历史数据 */
    void sg_startTest(const QByteArray&);          /* 开始测试 */
    void sg_stopTest(const QByteArray&);           /* 终止测试 */
    void sg_queryTestItem(const QByteArray&);      /* 查询测试项 */
    void sg_queryTestResult(const QByteArray&);    /* 查询测试结果 */

    void sg_recvLogs(const QString& syslog);

private:
    /* 刷新数据 */
    void refreshData();
    /* 显示添加的界面 */
    void showAddTestItemWidget();
    void showTreeItem(const QModelIndex& index);
    /* 显示can数绑定的界面 */
    void showParameterBindingWidget();
    /* 显示测试测结果的页面 */
    void showTestResultWidget();

    /* 删除一条记录 */
    void deleteTestItem();

    /* 开始测试 */
    void startTest();

    /* 结束测试 */
    void stopTest();

    void showEvent(QShowEvent* event) override;
    /* 过程信息 */
    void appendMessage(const QString&);
    void clearMessage();

    // void progressBar(FlowTipsType, bool);
    void resetProgressBar(); /* 重置进度条为初始状态 */
    void recvLogs(const QString& syslog);
    void unregisterSuber();
    void registerSuber();

private slots:
    /* 测试使用 刷新配置文件 */
    void testRefreshConfig();

private:
    Ui::AutomateTestWidget* ui;
    AutomateTestWidgetImpl* m_impl;
};

#endif  // AUTOMATETESTWIDGET_H
