#ifndef ORBITPRODICTIONUI_H
#define ORBITPRODICTIONUI_H
#include <QDialog>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class OrbitProdictionUI;
}
QT_END_NAMESPACE

namespace cppmicroservices
{
    class BundleContext;
}

class OrbitProdictionUI : public QWidget
{
    Q_OBJECT

public:
    OrbitProdictionUI(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~OrbitProdictionUI();

private:
    Ui::OrbitProdictionUI* ui;
};
#endif  // ORBITPRODICTIONUI_H
