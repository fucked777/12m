#ifndef AUTORUNPOLICYWIDGET_H
#define AUTORUNPOLICYWIDGET_H

#include <QWidget>

namespace cppmicroservices
{
    class BundleContext;
}

class AutorunPolicyWidgetImpl;
class AutorunPolicyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AutorunPolicyWidget(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~AutorunPolicyWidget() override;

    /* 只有更新 */
    void modifyACK(const QByteArray&);
signals:
    void sg_modify(const QByteArray&);

private:
    void refreshUI();
    void reloadConfig();
    void updateAutorunPolicy();

    void showEvent(QShowEvent* event) override;

private:
    AutorunPolicyWidgetImpl* m_impl = nullptr;
};

#endif  // AUTORUNPOLICYWIDGET_H
