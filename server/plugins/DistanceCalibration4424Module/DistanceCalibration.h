#ifndef DISTANCECALIBRATION_H
#define DISTANCECALIBRATION_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class DistanceCalibrationImpl;
class DistanceCalibration : public INetMsg
{
    Q_OBJECT
public:
    DistanceCalibration(cppmicroservices::BundleContext context);
    ~DistanceCalibration();

private:
    /* 开始校零
     * data DataTransmissionCenterData
     * ack OptionalNotValue
     * 这里只是返回结束所有的信息都由redis推送
     */
    void startDistanceCalibration(const ProtocolPack& pack);
    /* 结束校零
     * data 被忽略
     * ack OptionalNotValue
     */
    void stopDistanceCalibration(const ProtocolPack& pack);

    /* 删除校零结果
     * data DataTransmissionCenterData
     * ack Optional<DataTransmissionCenterData>
     */
    void deleteDistanceCalibrationResult(const ProtocolPack& pack);
    /* 清理历史数据
     * data ClearDistanceCalibrationHistry
     * ack OptionalNotValue
     */
    void clearHistry(const ProtocolPack& pack);
    /* 查询历史数据
     * data QueryDistanceCalibrationHistry
     * ack Optional<DistanceCalibrationItemList>
     */
    void queryHistry(const ProtocolPack& pack);
public slots:

    void sendByteArrayToDevice(const QByteArray& data);

    // 推送提示消息
    void slotPublishInfoMsg(const QString& msg);
    // 推送警告消息
    void slotPublishWarningMsg(const QString& msg);
    // 推送错误消息
    void slotPublishErrorMsg(const QString& msg);

signals:
    /*
     * 接收命令结果数据
     * data CmdResult
     * 这个就是ACK
     */
    void sg_deviceCMDResult(const ProtocolPack& pack);

private:
    DistanceCalibrationImpl* m_impl;
};
#endif  // DATATRANSMISSIONCENTER_H
