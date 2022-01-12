#ifndef PHASECALIBRATIONSERVICE_H
#define PHASECALIBRATIONSERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class PhaseCalibrationServiceImpl;
class PhaseCalibrationService : public INetMsg
{
    Q_OBJECT
public:
    PhaseCalibrationService(cppmicroservices::BundleContext context);
    ~PhaseCalibrationService();

signals:
    void sg_startPhaseCalibrationACK(const QByteArray& data);        /* 开始校相 */
    void sg_stopPhaseCalibrationACK(const QByteArray& data);         /* 结束校相 */
    void sg_deletePhaseCalibrationResultACK(const QByteArray& data); /* 删除校零结果 */
    void sg_clearHistryACK(const QByteArray& data);                  /* 清理历史数据 */
    void sg_queryHistryACK(const QByteArray& data);                  /* 查询历史数据 */

public:
    void startPhaseCalibration(const QByteArray& data);        /* 开始校相 */
    void stopPhaseCalibration(const QByteArray& data);         /* 结束校相 */
    void deletePhaseCalibrationResult(const QByteArray& data); /* 删除校零结果 */
    void clearHistry(const QByteArray& data);                  /* 清理历史数据 */
    void queryHistry(const QByteArray& data);                  /* 查询历史数据 */

    void startPhaseCalibrationACK(const ProtocolPack& pack);        /* 开始校相 */
    void stopPhaseCalibrationACK(const ProtocolPack& pack);         /* 结束校相 */
    void deletePhaseCalibrationResultACK(const ProtocolPack& pack); /* 删除校零结果 */
    void clearHistryACK(const ProtocolPack& pack);                  /* 清理历史数据 */
    void queryHistryACK(const ProtocolPack& pack);                  /* 查询历史数据 */

private:
    PhaseCalibrationServiceImpl* m_impl;
};
#endif  // PHASECALIBRATIONSERVICE_H
