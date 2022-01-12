#include "DataTransmissionCenter.h"
#include "CppMicroServicesUtility.h"
#include "DataTransmissionCenterSerialize.h"
#include "GlobalData.h"
#include "MacroHelper.h"
#include "MessagePublish.h"
#include "RedisHelper.h"
#include "SqlDataTransmissionCenter.h"
#include <QString>

class DataTransmissionCenterImpl
{
public:
    cppmicroservices::BundleContext context;
    SqlDataTransmissionCenter sql;
    int timerID{ -1 };
    static void pushData(const DataTransmissionCenterMap&);
    Optional<DataTransmissionCenterMap> reload(bool push = true);

    void silenceReload(bool push = true);
};
void DataTransmissionCenterImpl::pushData(const DataTransmissionCenterMap& data) { GlobalData::setDataTransmissionCenterInfo(data); }
Optional<DataTransmissionCenterMap> DataTransmissionCenterImpl::reload(bool push)
{
    auto itemMap = sql.selectItem();
    if (itemMap.success() & push)
    {
        DataTransmissionCenterImpl::pushData(itemMap.value());
    }

    return itemMap;
}
void DataTransmissionCenterImpl::silenceReload(bool push)
{
    auto itemMap = reload(push);
    if (!itemMap)
    {
        auto msg = QString("查询数传中心数据错误:%1").arg(itemMap.msg());
        //        Error(itemMap.errorCode(), msg);
        return;
    }
}
DataTransmissionCenter::DataTransmissionCenter(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new DataTransmissionCenterImpl)
{
    m_impl->context = context;

    auto initDBRes = m_impl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("任务中心初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("reload", &DataTransmissionCenter::reload, this);
    registerSubscribe("refresh", &DataTransmissionCenter::refresh, this);
    registerSubscribe("addItem", &DataTransmissionCenter::addItem, this);
    registerSubscribe("deleteItem", &DataTransmissionCenter::deleteItem, this);
    registerSubscribe("queryItem", &DataTransmissionCenter::queryItem, this);
    registerSubscribe("modifyItem", &DataTransmissionCenter::modifyItem, this);

    m_impl->silenceReload();
    start();
}

DataTransmissionCenter::~DataTransmissionCenter() { delete m_impl; }
void DataTransmissionCenter::start()
{
    if (m_impl->timerID == -1)
    {
        m_impl->timerID = startTimer(65000);
    }
}
void DataTransmissionCenter::stop()
{
    if (m_impl->timerID != -1)
    {
        killTimer(m_impl->timerID);
        m_impl->timerID = -1;
    }
}
void DataTransmissionCenter::timerEvent(QTimerEvent* /*event*/) { m_impl->silenceReload(); }
void DataTransmissionCenter::reload(const ProtocolPack& pack)
{
    auto itemMap = m_impl->reload();
    auto bak = pack;
    bak.data.clear();
    bak.data << itemMap;
    silenceSendACK(bak);
}
void DataTransmissionCenter::refresh(const ProtocolPack& pack)
{
    auto itemMap = m_impl->reload(false);
    auto bak = pack;
    bak.data.clear();
    bak.data << OptionalNotValue(itemMap.errorCode(), itemMap.msg());
    silenceSendACK(bak);
}
void DataTransmissionCenter::addItem(const ProtocolPack& pack)
{
    auto bak = pack;
    DataTransmissionCenterData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.insertItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("添加数传中心:%1成功").arg(item.centerName), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("添加数传中心:%1失败").arg(item.centerName), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;

    m_impl->silenceReload();
    silenceSendACK(bak);
}

void DataTransmissionCenter::deleteItem(const ProtocolPack& pack)
{
    auto bak = pack;
    DataTransmissionCenterData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.deleteItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除数传中心:%1成功").arg(item.centerName), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除数传中心:%1失败").arg(item.centerName), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;

    m_impl->silenceReload();
    silenceSendACK(bak);
}
void DataTransmissionCenter::queryItem(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data.clear();

    bak.data << m_impl->sql.selectItem();
    silenceSendACK(bak);
}
void DataTransmissionCenter::modifyItem(const ProtocolPack& pack)
{
    auto bak = pack;
    DataTransmissionCenterData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = m_impl->sql.updateItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("修改数传中心:%1成功").arg(item.centerName), pack.userID, MACRSTR(MODULE_NAME));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("修改数传中心:%1失败").arg(item.centerName), pack.userID, MACRSTR(MODULE_NAME));
    }
    bak.data << ans;
    m_impl->silenceReload();
    silenceSendACK(bak);
}
