#ifndef AUTOMATETESTRESULTDIALOG_H
#define AUTOMATETESTRESULTDIALOG_H

#include "AutomateTestDefine.h"
#include "Utility.h"
#include <QDialog>

namespace Ui
{
    class AutomateTestResultDialog;
}
struct AutomateTestItemInfo;
class AutomateTestWidget;
class AutomateTestResultDialogImpl;
class AutomateTestResultDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AutomateTestResultDialog(AutomateTestWidget* parent);
    ~AutomateTestResultDialog();
    void intiUIData(const AutomateTestItemInfo& info);

private:
    /* 查询所有的记录 */
    void selectItem();
    void selectItemACK(const QByteArray&);

    /* 删除一条记录 */
    void deleteItem();
    void deleteItemACK(const QByteArray&);

    /* 清理数据 */
    void finishingItem();
    void finishingItemACK(const QByteArray&);

    /* 表格单击 */
    void tableViewClicked(const QModelIndex& index);
    void recvResult(const QByteArray&);

signals:
    void signal_sendCmdAutoTest(uint, uint, const QByteArray&);

private:
    Ui::AutomateTestResultDialog* ui;
    AutomateTestResultDialogImpl* m_impl;
};

/* 测试结果的展示页面内部使用 */
class AutomateTestResultWidgetImpl : public QWidget
{
    Q_OBJECT

public:
    AutomateTestResultWidgetImpl(QWidget* parent = nullptr);
    ~AutomateTestResultWidgetImpl();
    void initUI(const AutomateTestResultGroupList& resultList);
    void initEmptyUI(const QString&);
};

#endif  // AUTOMATETESTRESULTDIALOG_H
