#ifndef AGCMANAGE_H
#define AGCMANAGE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class AGCManageImpl;
class AGCManage : public INetMsg
{
    Q_OBJECT
public:
    AGCManage(cppmicroservices::BundleContext context);
    ~AGCManage();

private:
    /*
     * 查询数据
     * data AGCResultQuery
     * ack Optional<AGCCalibrationItemList>
     */
    void selectAGCData(const ProtocolPack& pack);
    /*
     * 添加数据
     * data AGCCalibrationItem
     * ack Optional<AGCCalibrationItem>
     */
    void saveAGCData(const ProtocolPack& pack);
    /*
     * 删除数据
     * data AGCCalibrationItem
     * ack Optional<AGCCalibrationItem>
     */
    void deleteAGCData(const ProtocolPack& pack);
    /*
     * 清理数据
     * data ClearAGCCalibrationHistry
     * ack OptionalNotValue
     */
    void finishingItem(const ProtocolPack& pack);

    /*
     * 装订数据给基带
     * data AGCCalibrationItem
     * ack OptionalNotValue
     */
    void sendToMACB(const ProtocolPack& pack);
    /*
     * 装订数据给ACU
     * data AGCCalibrationItem
     * ack OptionalNotValue
     */
    void sendToACU(const ProtocolPack& pack);
    /*
     * 装订数据给ACU
     * data AGCCalibrationItem
     * ack 连续发送 AGCCalibrationProcessACK
     */
    void startAGCData(const ProtocolPack& pack);
    void endAGCData(const ProtocolPack& pack);

private:
    void endCalibrationServer(); /* 停止标校服务 */

private:
    AGCManageImpl* m_impl;
};
#endif  // AUTOMATETEST_H
