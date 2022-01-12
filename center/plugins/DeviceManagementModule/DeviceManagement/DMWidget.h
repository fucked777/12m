#ifndef DMWIDGET_H
#define DMWIDGET_H

#include "Utility.h"
#include <QWidget>
namespace Ui
{
    class DMWidget;
}
namespace cppmicroservices
{
    class BundleContext;
}

class IPacketManager;
class IGlobalData;
class DMWidgetImpl;
class DMWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DMWidget(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~DMWidget();

    void deleteACK(const QByteArray&);

signals:
    /* 增删查改 */
    void sg_add(const QByteArray&);
    void sg_delete(const QByteArray&);
    // void sg_query(const QByteArray&);
    void sg_modify(const QByteArray&);

    void sg_addACK(const QByteArray&);
    void sg_modifyACK(const QByteArray&);

private:
    void showEvent(QShowEvent* e) override;
    void init();
    void parseConfig();
    void addItem();
    void deleteItem();
    void tabWidgetChanged();
    OptionalNotValue queryItem();
    void modifyItem();

private:
    Ui::DMWidget* ui;
    DMWidgetImpl* m_impl;
};

#endif  // DMWIDGET_H
