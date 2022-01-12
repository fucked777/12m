#ifndef EPHEMERISDATA_H
#define EPHEMERISDATA_H
#include "EphemerisDataModel.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class EphemerisData;
}
QT_END_NAMESPACE

namespace cppmicroservices
{
    class BundleContext;
}
struct EphemerisDataData;
class EphemerisDataImpl;
class EphemerisData : public QWidget
{
    Q_OBJECT

public:
    EphemerisData(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~EphemerisData();

public:
    void deleteACK(const QByteArray&);

private:
    void showEvent(QShowEvent*) override;
    void init();

    void addItem();
    void deleteItem();
    void queryItem();
    void modifyItem();

signals:
    /* 增删改 */
    void sg_add(const QByteArray&);
    void sg_delete(const QByteArray&);
    void sg_modify(const QByteArray&);

    void sg_addACK(const QByteArray&);
    void sg_modifyACK(const QByteArray&);

private:
    Ui::EphemerisData* ui;
    EphemerisDataImpl* m_impl;
};
#endif  // EPHEMERISDATA_H
