#ifndef DISTANCECALIBRATION_H
#define DISTANCECALIBRATION_H
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class DistanceCalibration;
}
QT_END_NAMESPACE

namespace cppmicroservices
{
    class BundleContext;
}
class DistanceCalibrationImpl;
class DistanceCalibration : public QWidget
{
    Q_OBJECT

public:
    DistanceCalibration(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~DistanceCalibration();

    void startDistanceCalibrationACK(const QByteArray& data);        /* 开始校零 */
    void stopDistanceCalibrationACK(const QByteArray& data);         /* 结束校零 */
    void deleteDistanceCalibrationResultACK(const QByteArray& data); /* 删除校零结果 */
    void clearHistryACK(const QByteArray& data);                     /* 清理历史数据 */
    void queryHistryACK(const QByteArray& data);                     /* 查询历史数据 */

private:
    void init();
    /* 重载ui数据 */
    void reloadUIData();
    /* 任务代号切换 */
    void taskCodeChange();
    /* 工作模式切换 */
    void workModeChange();
    /* 点频切换 */
    void dpChange();
    void showEvent(QShowEvent* event) override;
    void timerEvent(QTimerEvent* event) override;
    void controlButtonEnable(bool enable);

    void startDistanceCalibration();        /* 开始校零 */
    void stopDistanceCalibration();         /* 结束校零 */
    void deleteDistanceCalibrationResult(); /* 删除校零结果 */
    void clearHistry();                     /* 清理历史数据 */
    void queryHistry();                     /* 查询历史数据 */

    void start();
    void stop();
    bool isStart();
    void addLog(const QString&);

    void masterStatusChange();

signals:
    void sg_startDistanceCalibration(const QByteArray& data);        /* 开始校零 */
    void sg_stopDistanceCalibration(const QByteArray& data);         /* 结束校零 */
    void sg_deleteDistanceCalibrationResult(const QByteArray& data); /* 删除校零结果 */
    void sg_clearHistry(const QByteArray& data);                     /* 清理历史数据 */
    void sg_queryHistry(const QByteArray& data);                     /* 查询历史数据 */

    /* redis推送的日志是异步多线程的,通过信号槽搞到主线程,否则会崩溃 */
    void sg_addLog(const QString&);

private:
    Ui::DistanceCalibration* ui;
    DistanceCalibrationImpl* m_impl;
};
#endif  // DISTANCECALIBRATION_H
