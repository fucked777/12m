#ifndef TASKCENTERWIDGET_H
#define TASKCENTERWIDGET_H

#include "TaskCenterModel.h"
#include <QMenu>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class TaskCenter;
}
QT_END_NAMESPACE
namespace cppmicroservices
{
    class BundleContext;
}

struct TaskCenterData;
class TaskCenterImpl;
class TaskCenter : public QWidget
{
    Q_OBJECT

public:
    explicit TaskCenter(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~TaskCenter();

public:
    void deleteACK(const QByteArray&);

private:
    void showEvent(QShowEvent*) override;
    void init();

    void addItem();
    void deleteItem();
    void queryItem();
    void modifyItem();

signals:
    /* 增删改 */
    void sg_add(const QByteArray&);
    void sg_delete(const QByteArray&);
    void sg_modify(const QByteArray&);

    void sg_addACK(const QByteArray&);
    void sg_modifyACK(const QByteArray&);

private:
    Ui::TaskCenter* ui;
    TaskCenterImpl* m_impl;
};

#endif  // TASKCENTERWIDGET_H
