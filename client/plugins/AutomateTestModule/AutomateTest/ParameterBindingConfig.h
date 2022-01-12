#ifndef PARAMETERBINDINGCONFIG_H
#define PARAMETERBINDINGCONFIG_H

#include "AutomateTestUtility.h"
#include "Utility.h"
#include <QDialog>
#include <QList>

namespace Ui
{
    class ParameterBindingConfig;
}

class AutomateTestWidget;
class ParameterBindingConfigImpl;
class ParameterBindingConfig : public QDialog
{
    Q_OBJECT

public:
    explicit ParameterBindingConfig(AutomateTestWidget* parent);
    ~ParameterBindingConfig();
    OptionalNotValue init(const AutomateTestItem& testItem);

    void addTestBindItemACK(const QByteArray&);    /* 添加参数绑定 */
    void deleteTestBindItemACK(const QByteArray&); /* 删除参数绑定 */

private:
    void add();
    void del();
    void bindItemClicket(const QModelIndex& index);
    void showEvent(QShowEvent* event) override;
    void resetUI(const QString&);

private:
    Ui::ParameterBindingConfig* ui;
    ParameterBindingConfigImpl* m_impl;
};

#endif  // PARAMETERBINDINGCONFIG_H
