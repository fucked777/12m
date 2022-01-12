#ifndef PHASECALIBRATION_H
#define PHASECALIBRATION_H
#include "Utility.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class PhaseCalibration;
}
QT_END_NAMESPACE

namespace cppmicroservices
{
    class BundleContext;
}

struct StartPhaseCalibration;
struct ExtensionStatusReportMessage;
class PhaseCalibrationImpl;
class PhaseCalibration : public QWidget
{
    Q_OBJECT

public:
    PhaseCalibration(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~PhaseCalibration();

    void startPhaseCalibrationACK(const QByteArray& data);        /* 开始校零 */
    void stopPhaseCalibrationACK(const QByteArray& data);         /* 结束校零 */
    void deletePhaseCalibrationResultACK(const QByteArray& data); /* 删除校零结果 */
    void clearHistryACK(const QByteArray& data);                  /* 清理历史数据 */
    void queryHistryACK(const QByteArray& data);                  /* 查询历史数据 */

private:
    void startPhaseCalibration();        /* 开始校零 */
    void stopPhaseCalibration();         /* 结束校零 */
    void deletePhaseCalibrationResult(); /* 删除校零结果 */
    void clearHistry();                  /* 清理历史数据 */
    void queryHistry();                  /* 查询历史数据 */

    void start();
    void stop();
    bool isStart();
    void addLog(const QString&);

    /* 重载ui数据 */
    void reloadUIData();
    /* 任务代号切换 */
    void taskCodeChange();
    /* 工作模式切换 */
    void sWorkModeChanged();
    void ycWorkModeChanged();
    void scWorkModeChanged();

    /* 工作模式切换 */
    void sDotDrequencyChanged();
    void ycDotDrequencyChanged();
    void scDotDrequencyChanged();

    /* 重置界面显示 */
    void resetValue();
    /* ACU的状态实时显示 */
    void setTFSData(bool isUpdate, const ExtensionStatusReportMessage& message);

signals:
    void sg_startPhaseCalibration(const QByteArray& data);        /* 开始校零 */
    void sg_stopPhaseCalibration(const QByteArray& data);         /* 结束校零 */
    void sg_deletePhaseCalibrationResult(const QByteArray& data); /* 删除校零结果 */
    void sg_clearHistry(const QByteArray& data);                  /* 清理历史数据 */
    void sg_queryHistry(const QByteArray& data);                  /* 查询历史数据 */

    /* redis推送的日志是异步多线程的,通过信号槽搞到主线程,否则会崩溃 */
    void sg_addLog(const QString&);

private:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void timerEvent(QTimerEvent* event) override;

private:
    Ui::PhaseCalibration* ui;
    PhaseCalibrationImpl* m_impl;
};
#endif  // PHASECALIBRATION_H
