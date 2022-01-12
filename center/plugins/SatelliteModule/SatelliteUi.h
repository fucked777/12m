#ifndef SATELLITEUI_H
#define SATELLITEUI_H
#include "SatelliteManagementDefine.h"
#include <QWidget>
#include <SatelliteManagementSerialize.h>
QT_BEGIN_NAMESPACE
namespace Ui
{
    class SatelliteUi;
}
QT_END_NAMESPACE

namespace cppmicroservices
{
    class BundleContext;
}

class SatelliteConfigImpl;
class SatelliteUi : public QWidget
{
    Q_OBJECT

public:
    SatelliteUi(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~SatelliteUi();
    /*初始化加载UI*/
    void loadUI();
signals:
    /* 添加卫星 */
    void signalAddSatellite(QString json);

    /* 编辑卫星 */
    void signalEditSatellite(QString json);

    /* 删除卫星 */
    void signalDelSatellite(QString json);

    /* 获取所有的卫星接口 */
    void signalGetAllSatellite(QString json);

    void signalAddSatelliteResponce(bool flag, SatelliteManagementData request);

    void signalEditSatelliteResponce(bool flag, SatelliteManagementData request);

public slots:
    void slotDelSatelliteResponce(bool flag, SatelliteManagementData request);

    void slotAllSatelliteResponce(bool flag, QList<SatelliteManagementData> requestList);

private slots:

    void on_pushButton_add_clicked();

    void on_pushButton_refresh_clicked();

    void on_pushButton_edit_clicked();

    void on_pushButton_delete_clicked();

private:
    void initStyleQss();

    void showEvent(QShowEvent*) override;

private:
    Ui::SatelliteUi* ui;

    SatelliteConfigImpl* m_impl = nullptr;
};
#endif  // SATELLITEUI_H
