#ifndef PHASECALIBRATION_H
#define PHASECALIBRATION_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class PhaseCalibrationImpl;
class PhaseCalibration : public INetMsg
{
    Q_OBJECT
public:
    PhaseCalibration(cppmicroservices::BundleContext context);
    ~PhaseCalibration();

private:
    /* 开始校零
     * data DataTransmissionCenterData
     * ack OptionalNotValue
     * 这里只是返回结束所有的信息都由redis推送
     */
    void startPhaseCalibration(const ProtocolPack& pack);
    /* 结束校零
     * data 被忽略
     * ack OptionalNotValue
     */
    void stopPhaseCalibration(const ProtocolPack& pack);
    void endPhaseCalibration();

    /* 删除校零结果
     * data DataTransmissionCenterData
     * ack Optional<DataTransmissionCenterData>
     */
    void deletePhaseCalibrationResult(const ProtocolPack& pack);
    /* 清理历史数据
     * data ClearPhaseCalibrationHistry
     * ack OptionalNotValue
     */
    void clearHistry(const ProtocolPack& pack);
    /* 查询历史数据
     * data QueryPhaseCalibrationHistry
     * ack Optional<PhaseCalibrationItemList>
     */
    void queryHistry(const ProtocolPack& pack);


private:
    PhaseCalibrationImpl* m_impl;
};
#endif  // PHASECALIBRATION_H
