#ifndef AGCCURVECALIBRATION_H
#define AGCCURVECALIBRATION_H
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class AGCCurveCalibration;
}
namespace cppmicroservices
{
    class BundleContext;
}
QT_END_NAMESPACE

struct AGCCalibrationItem;
class AGCCurveCalibrationImpl;
class AGCCurveCalibration : public QWidget
{
    Q_OBJECT

public:
    explicit AGCCurveCalibration(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~AGCCurveCalibration();

    void selectAGCDataACK(const QByteArray&);
    void saveAGCDataACK(const QByteArray&);
    void startAGCDataACK(const QByteArray&);
    void endAGCDataACK(const QByteArray&);
    void deleteAGCDataACK(const QByteArray&);
    void finishingItemACK(const QByteArray&);
    void sendToMACBACK(const QByteArray&);
    void sendToACUACK(const QByteArray&);

private:
    void taskCodeChanged(); /* 任务代号改变 */
    void workModeChanged(); /* 工作模式改变 */
    // void dotDrequencyChanged();           /* 点频改变 */
    void registerSuber();                 /* 注册日志推送 */
    void unregisterSuber();               /* 中断日志推送 */
    void recvLogs(const QString& syslog); /* 接收日志 */
    void clearAll();
    void clearPoint();

    void selectAGCData();
    void saveAGCData();
    void startAGCData();
    void endAGCData();
    void deleteAGCData();
    void finishingItem();
    void sendToMACB();
    void sendToACU();
    void manualCreateAGC();

    void setEnableEditAGC();
    void setEnableEditAGCImpl(int index);
    void addPoint();
    void removePoint();
    void historyShowPoint(const QModelIndex& index);

signals:
    void sg_selectAGCData(const QByteArray& data);
    void sg_saveAGCData(const QByteArray& data);
    void sg_startAGCData(const QByteArray& data);
    void sg_endAGCData(const QByteArray& data);
    void sg_deleteAGCData(const QByteArray& data);
    void sg_finishingItem(const QByteArray& data);
    void sg_sendToMACB(const QByteArray& data);
    void sg_sendToACU(const QByteArray& data);

    void sg_recvLogs(const QString& syslog);

private:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void reloadUIParam(); /* 重新加载ui的参数 */

private:
    Ui::AGCCurveCalibration* ui;
    AGCCurveCalibrationImpl* m_impl;
};
#endif  // AGCCURVECALIBRATION_H
