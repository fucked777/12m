#ifndef SATELLITESERVICE_H
#define SATELLITESERVICE_H

#include "INetMsg.h"
struct ProtocolPack;
namespace cppmicroservices
{
    class BundleContext;
}

#include "SatelliteManagementDefine.h"
class SatelliteServiceImpl;
class SatelliteService : public INetMsg
{
    Q_OBJECT
public:
    SatelliteService(cppmicroservices::BundleContext context);
    ~SatelliteService();

signals:

    void signalAddSatelliteResponce(bool flag, SatelliteManagementData request);

    void signalEditSatelliteResponce(bool flag, SatelliteManagementData request);

    void signalDelSatelliteResponce(bool flag, SatelliteManagementData request);

    void signalAllSatelliteResponce(bool flag, QList<SatelliteManagementData> requestList);

public slots:
    /* 添加配置宏 */
    void slotAddSatellite(QString json);

    /* 编辑配置宏 */
    void slotEditSatellite(QString json);

    /* 删除配置宏 */
    void slotDelSatellite(QString json);

    /* 获取所有的配置宏接口 */
    void slotGetAllSatellite(QString json);

public:
private:
    void AddSatelliteResponce(const ProtocolPack& pack);
    void EditSatelliteResponce(const ProtocolPack& pack);
    void DelSatelliteResponce(const ProtocolPack& pack);
    void GetSatelliteResponce(const ProtocolPack& pack);

private:
    SatelliteServiceImpl* m_impl;
};
#endif  // SatelliteSERVICE_H
