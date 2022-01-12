#ifndef PARAMETERBINDINGSELECT_H
#define PARAMETERBINDINGSELECT_H

#include <QDialog>

namespace Ui
{
    class ParameterBindingSelect;
}
struct AutomateTestItemInfo;
class ParameterBindingSelect : public QDialog
{
    Q_OBJECT

public:
    explicit ParameterBindingSelect(const AutomateTestItemInfo&, QWidget* parent = nullptr);
    ~ParameterBindingSelect();
    QString text();
    QString id();

private:
    Ui::ParameterBindingSelect* ui;
};

#endif  // PARAMETERBINDINGSELECT_H
