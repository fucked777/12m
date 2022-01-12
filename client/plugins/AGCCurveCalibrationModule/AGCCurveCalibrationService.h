#ifndef AGCCURVECALIBRATIONSERVICE_H
#define AGCCURVECALIBRATIONSERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class AGCCurveCalibrationServiceImpl;
class AGCCurveCalibrationService : public INetMsg
{
    Q_OBJECT
public:
    explicit AGCCurveCalibrationService(cppmicroservices::BundleContext context);
    ~AGCCurveCalibrationService();

signals:
    void sg_selectAGCDataACK(const QByteArray& pack);
    void sg_saveAGCDataACK(const QByteArray& pack);
    void sg_startAGCDataACK(const QByteArray& pack);
    void sg_endAGCDataACK(const QByteArray& pack);
    void sg_deleteAGCDataACK(const QByteArray& pack);
    void sg_finishingItemACK(const QByteArray& pack);
    void sg_sendToMACBACK(const QByteArray& pack);
    void sg_sendToACUACK(const QByteArray& pack);

public:
    void selectAGCData(const QByteArray& data);
    void saveAGCData(const QByteArray& data);
    void startAGCData(const QByteArray& data);
    void endAGCData(const QByteArray& data);
    void deleteAGCData(const QByteArray& data);
    void finishingItem(const QByteArray& data);
    void sendToMACB(const QByteArray& data);
    void sendToACU(const QByteArray& data);

    void selectAGCDataACK(const ProtocolPack& pack);
    void saveAGCDataACK(const ProtocolPack& pack);
    void startAGCDataACK(const ProtocolPack& pack);
    void endAGCDataACK(const ProtocolPack& pack);
    void deleteAGCDataACK(const ProtocolPack& pack);
    void finishingItemACK(const ProtocolPack& pack);
    void sendToMACBACK(const ProtocolPack& pack);
    void sendToACUACK(const ProtocolPack& pack);

private:
    AGCCurveCalibrationServiceImpl* m_impl;
};
#endif  // AGCCURVECALIBRATIONSERVICE_H
