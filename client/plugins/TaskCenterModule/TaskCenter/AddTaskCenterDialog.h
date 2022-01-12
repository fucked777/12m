#ifndef ADDTASKCENTERDIALOG_H
#define ADDTASKCENTERDIALOG_H

#include "TaskCenterDefine.h"
#include <QDialog>

namespace Ui
{
    class AddTaskCenterDialog;
}

class TaskCenter;
class TaskCenterModel;
class AddTaskCenterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTaskCenterDialog(TaskCenter* parent);
    ~AddTaskCenterDialog();

    TaskCenterData data() const;
    void setData(const TaskCenterData& data);

private:
    void ok();
    void addACK(const QByteArray& data);
    void modifyACK(const QByteArray& data);

private:
    Ui::AddTaskCenterDialog* ui;
    TaskCenter* m_parent;
    TaskCenterData m_taskCenter;
    bool m_isAdd{ true };
};

#endif  // ADDTASKCENTERDIALOG_H
