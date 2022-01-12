#ifndef DMITEMWIDGET_H
#define DMITEMWIDGET_H
#include "DMMessageDefine.h"
#include "Utility.h"
#include <QWidget>

namespace Ui
{
    class DMItemWidget;
}

struct UIDataParameter;
class DMItemWidgetImpl;
class DMItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DMItemWidget(const UIDataParameter& parameter, QWidget* parent = nullptr);
    ~DMItemWidget();
    Optional<DMDataItem> getUIValue();        /* 获取当前ui交互的数据 */
    void setUIValue(const DMDataItem& value); /* 设置当前ui交互的数据 */

    Optional<DMDataItem> getSelectItem();          /* 获取当前表格中选中的数据 */
    void addItem(const DMDataItem& value);         /* 表格-添加 */
    void deleteItem(const DMDataItem& value);      /* 表格-删除 */
    void updateItem(const DMDataItem& value);      /* 表格-更新 */
    void setItems(const QList<DMDataItem>& value); /* 表格-设置 */
    bool dataExist(const QString& devID) const;
    QString type() const;
    UIDataParameter uiDataParameter() const;
    void clearInput();

private:
    void init();
    void viewItem(const QModelIndex& index);

private:
    Ui::DMItemWidget* ui;
    DMItemWidgetImpl* m_impl;
};

#endif  // DMITEMWIDGET_H
