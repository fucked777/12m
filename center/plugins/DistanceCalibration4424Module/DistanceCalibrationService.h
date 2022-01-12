#ifndef DISTANCECALIBRATIONSERVICE_H
#define DISTANCECALIBRATIONSERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class DistanceCalibrationServiceImpl;
class DistanceCalibrationService : public INetMsg
{
    Q_OBJECT
public:
    DistanceCalibrationService(cppmicroservices::BundleContext context);
    ~DistanceCalibrationService();

signals:
    void sg_startDistanceCalibrationACK(const QByteArray& data);        /* 开始校零 */
    void sg_stopDistanceCalibrationACK(const QByteArray& data);         /* 结束校零 */
    void sg_deleteDistanceCalibrationResultACK(const QByteArray& data); /* 删除校零结果 */
    void sg_clearHistryACK(const QByteArray& data);                     /* 清理历史数据 */
    void sg_queryHistryACK(const QByteArray& data);                     /* 查询历史数据 */

public:
    void startDistanceCalibration(const QByteArray& data);        /* 开始校零 */
    void stopDistanceCalibration(const QByteArray& data);         /* 结束校零 */
    void deleteDistanceCalibrationResult(const QByteArray& data); /* 删除校零结果 */
    void clearHistry(const QByteArray& data);                     /* 清理历史数据 */
    void queryHistry(const QByteArray& data);                     /* 查询历史数据 */

    void startDistanceCalibrationACK(const ProtocolPack& pack);        /* 开始校零 */
    void stopDistanceCalibrationACK(const ProtocolPack& pack);         /* 结束校零 */
    void deleteDistanceCalibrationResultACK(const ProtocolPack& pack); /* 删除校零结果 */
    void clearHistryACK(const ProtocolPack& pack);                     /* 清理历史数据 */
    void queryHistryACK(const ProtocolPack& pack);                     /* 查询历史数据 */

private:
    DistanceCalibrationServiceImpl* m_impl;
};
#endif  // DISTANCECALIBRATIONSERVICE_H
