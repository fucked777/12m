#include "SatelliteService.h"
#include "CppMicroServicesUtility.h"
#include "SatelliteManagementDefine.h"
#include "SatelliteManagementSerialize.h"
#include "Utility.h"
#include <QMessageBox>

class SatelliteServiceImpl
{
    DIS_COPY_MOVE(SatelliteServiceImpl)
public:
    inline SatelliteServiceImpl() {}
    inline ~SatelliteServiceImpl() {}
    cppmicroservices::BundleContext context;
};
SatelliteService::SatelliteService(cppmicroservices::BundleContext context)
    : INetMsg()
    , m_impl(new SatelliteServiceImpl)
{
    m_impl->context = context;

    qRegisterMetaType<SatelliteManagementData>("SatelliteManagementData");

    registerSubscribe("GetSatelliteResponce", &SatelliteService::GetSatelliteResponce, this);

    registerSubscribe("AddSatelliteResponce", &SatelliteService::AddSatelliteResponce, this);
    registerSubscribe("EditSatelliteResponce", &SatelliteService::EditSatelliteResponce, this);
    registerSubscribe("DelSatelliteResponce", &SatelliteService::DelSatelliteResponce, this);
}

SatelliteService::~SatelliteService() { delete m_impl; }

void SatelliteService::slotAddSatellite(QString json)
{
    ProtocolPack pack;
    pack.srcID = "";
    pack.desID = "ProcessClientSatelliteRequest";
    pack.module = false;
    pack.operationACK = "AddSatelliteResponce";
    pack.operation = "AddSatellite";
    pack.data = json.toUtf8();
    this->send(pack);
}

void SatelliteService::slotEditSatellite(QString json)
{
    ProtocolPack pack;
    pack.srcID = "";
    pack.desID = "ProcessClientSatelliteRequest";
    pack.module = false;
    pack.operation = "EditSatellite";
    pack.operationACK = "EditSatelliteResponce";
    pack.data = json.toUtf8();
    this->send(pack);
}

void SatelliteService::slotDelSatellite(QString json)
{
    ProtocolPack pack;
    pack.srcID = "";
    pack.desID = "ProcessClientSatelliteRequest";
    pack.module = false;
    pack.operation = "DelSatellite";
    pack.operationACK = "DelSatelliteResponce";
    pack.data = json.toUtf8();
    this->send(pack);
}

void SatelliteService::slotGetAllSatellite(QString json)
{
    ProtocolPack pack;
    pack.srcID = "";
    pack.desID = "ProcessClientSatelliteRequest";
    pack.module = false;
    pack.operationACK = "GetSatelliteResponce";
    pack.operation = "GetSatellite";
    pack.data = json.toUtf8();
    this->send(pack);
}

void SatelliteService::AddSatelliteResponce(const ProtocolPack& pack)
{
    auto responce = Optional<SatelliteManagementData>::emptyOptional();

    pack.data >> responce;

    SatelliteManagementData value;
    if (responce.success())
    {
        value = responce.value();
    }
    emit signalAddSatelliteResponce(responce.success(), value);
}

void SatelliteService::EditSatelliteResponce(const ProtocolPack& pack)
{
    auto responce = Optional<SatelliteManagementData>::emptyOptional();

    pack.data >> responce;

    SatelliteManagementData value;
    if (responce.success())
    {
        value = responce.value();
    }
    emit signalEditSatelliteResponce(responce.success(), value);
}

void SatelliteService::DelSatelliteResponce(const ProtocolPack& pack)
{
    auto responce = Optional<SatelliteManagementData>::emptyOptional();

    pack.data >> responce;
    emit signalDelSatelliteResponce(responce.success(), responce.value());
}

void SatelliteService::GetSatelliteResponce(const ProtocolPack& pack)
{
    auto responce = Optional<QList<SatelliteManagementData>>::emptyOptional();

    pack.data >> responce;
    if (responce.success())
    {
        auto value = responce.value();
        emit signalAllSatelliteResponce(responce.success(), value);
    }
    else
    {
    }
}
