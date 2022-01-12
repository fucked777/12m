#ifndef DMEDITDIALOG_H
#define DMEDITDIALOG_H
#include <QDialog>

struct DMDataItem;
struct UIDataParameter;
class DMParamInputOrDisplay;
class DMWidget;
class DMEditDialog : public QDialog
{
public:
    DMEditDialog(const UIDataParameter& parameter, DMWidget* parent = nullptr);
    ~DMEditDialog();
    void setUIValue(const DMDataItem& value); /* 设置当前ui交互的数据 */

private:
    void ok();
    void addACK(const QByteArray& value);
    void modifyACK(const QByteArray& value);

private:
    DMWidget* m_parent;
    DMParamInputOrDisplay* m_dmParamInputOrDisplay;
    QString m_devName;
    QString m_id;
    bool m_isAdd{ true };
};

#endif  // DMEDITDIALOG_H
