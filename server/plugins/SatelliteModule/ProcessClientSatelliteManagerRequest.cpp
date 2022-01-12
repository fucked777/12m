#include "ProcessClientSatelliteManagerRequest.h"
#include "CppMicroServicesUtility.h"

#include "GlobalData.h"
#include "MessagePublish.h"
#include "SatelliteManagementSerialize.h"
#include "SqlSatelliteManager.h"
#define MODULENAME "SatelliteModule"

class ProcessClientSatelliteRequestImpl
{
public:
    cppmicroservices::BundleContext context;
    SqlSatelliteManager sql;
    int timerID{ -1 };

    static void pushData(const SatelliteManagementDataList& data);
    Optional<SatelliteManagementDataList> reload(bool push = true);
    void silenceReload(bool push = true);
};

ProcessClientSatelliteRequest::ProcessClientSatelliteRequest(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , mImpl(new ProcessClientSatelliteRequestImpl)
{
    mImpl->context = context;

    qRegisterMetaType<SatelliteManagementData>("SatelliteManagementData");

    //初始化数据库
    auto initDBRes = mImpl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("卫星管理初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("AddSatellite", std::bind(&ProcessClientSatelliteRequest::AddSatellite, this, std::placeholders::_1));
    registerSubscribe("EditSatellite", std::bind(&ProcessClientSatelliteRequest::EditSatellite, this, std::placeholders::_1));
    registerSubscribe("DelSatellite", std::bind(&ProcessClientSatelliteRequest::DelSatellite, this, std::placeholders::_1));
    registerSubscribe("GetSatellite", std::bind(&ProcessClientSatelliteRequest::GetSatellite, this, std::placeholders::_1));

    mImpl->silenceReload();
    start();
}

void ProcessClientSatelliteRequest::start()
{
    if (mImpl->timerID == -1)
    {
        mImpl->timerID = startTimer(17000);
    }
}
void ProcessClientSatelliteRequest::stop()
{
    if (mImpl->timerID != -1)
    {
        killTimer(mImpl->timerID);
        mImpl->timerID = -1;
    }
}

ProcessClientSatelliteRequest::~ProcessClientSatelliteRequest()
{
    stop();
    delete mImpl;
}

void ProcessClientSatelliteRequest::timerEvent(QTimerEvent* /*event*/) { mImpl->silenceReload(); }

void ProcessClientSatelliteRequest::AddSatellite(const ProtocolPack& pack)
{
    auto bak = pack;
    SatelliteManagementData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = mImpl->sql.insertItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("添加卫星:%1成功").arg(item.m_satelliteCode), pack.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("添加卫星:%1失败").arg(item.m_satelliteCode), pack.userID, MODULENAME);
    }

    mImpl->silenceReload();

    bak.data << ans;
    silenceSendACK(bak);
}

void ProcessClientSatelliteRequest::EditSatellite(const ProtocolPack& pack)
{
    auto bak = pack;
    SatelliteManagementData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = mImpl->sql.updateItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("修改卫星:%1信息成功").arg(item.m_satelliteCode), pack.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("修改卫星:%1信息失败").arg(item.m_satelliteCode), pack.userID, MODULENAME);
    }

    mImpl->silenceReload();

    bak.data << ans;
    silenceSendACK(bak);
}

void ProcessClientSatelliteRequest::DelSatellite(const ProtocolPack& pack)
{
    auto bak = pack;
    SatelliteManagementData item;
    bak.data >> item;
    bak.data.clear();
    auto ans = mImpl->sql.deleteItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除卫星:%1成功").arg(item.m_satelliteCode), pack.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除卫星:%1失败").arg(item.m_satelliteCode), pack.userID, MODULENAME);
    }

    mImpl->silenceReload();

    bak.data << ans;
    silenceSendACK(bak);
}

void ProcessClientSatelliteRequest::GetSatellite(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data.clear();

    bak.data << mImpl->sql.selectItem();
    silenceSendACK(bak);
}

void ProcessClientSatelliteRequestImpl::pushData(const SatelliteManagementDataList& data) { GlobalData::setSatelliteManagementData(data); }

Optional<SatelliteManagementDataList> ProcessClientSatelliteRequestImpl::reload(bool push)
{
    auto items = sql.selectItem();
    if (items.success() & push)
    {
        pushData(items.value());
    }

    return items;
}

void ProcessClientSatelliteRequestImpl::silenceReload(bool push)
{
    auto items = reload(push);
    if (!items)
    {
        auto msg = QString("查询卫星管理数据错误:%1").arg(items.msg());
        // Error(items.errorCode(), msg);
        return;
    }
}
