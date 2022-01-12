#ifndef MACROSERVICEMANAGERDIALOG_H
#define MACROSERVICEMANAGERDIALOG_H
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MacroServiceManagerDialog;
}
QT_END_NAMESPACE

namespace cppmicroservices
{
    class BundleContext;
}

class TableView;
class MacroServiceManagerDialogImpl;
class MacroServiceManagerDialog : public QWidget
{
    Q_OBJECT

public:
    MacroServiceManagerDialog(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~MacroServiceManagerDialog();

    void initTableView();

    void timerEvent(QTimerEvent* event);

private:
    Ui::MacroServiceManagerDialog* ui;
    MacroServiceManagerDialogImpl* m_impl;
};
#endif  // MACROSERVICEMANAGERDIALOG_H
