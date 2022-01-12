#include "ParamMacroService.h"
#include "CppMicroServicesUtility.h"

#include "MessagePublish.h"
#include "ParamMacroMessageSerialize.h"
#include "RedisHelper.h"
#include "SqlParamMacroManager.h"
#include "GlobalData.h"

#define MODULENAME "ParamMacroModule"

class ParamMacroServiceImpl
{
public:
    cppmicroservices::BundleContext context;

    SqlParamMacroManager sql;
    int timerID{ -1 };

    static void pushData(const ParamMacroDataList& data);
    Optional<ParamMacroDataList> reload(bool push = true);
    void silenceReload(bool push = true);
};
ParamMacroService::ParamMacroService(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , mImpl(new ParamMacroServiceImpl)
{
    mImpl->context = context;

    // 初始化数据库
    auto initDBRes = mImpl->sql.initDB();
    if (!initDBRes)
    {
        auto msg = QString("参数宏初始化数据库失败:%1").arg(initDBRes.msg());
        qWarning() << msg;
        return;
    }

    registerSubscribe("addItem", std::bind(&ParamMacroService::addItem, this, std::placeholders::_1));
    registerSubscribe("modifyItem", std::bind(&ParamMacroService::modifyItem, this, std::placeholders::_1));
    registerSubscribe("deleteItem", std::bind(&ParamMacroService::deleteItem, this, std::placeholders::_1));
    registerSubscribe("queryItem", std::bind(&ParamMacroService::queryItem, this, std::placeholders::_1));

    mImpl->silenceReload();
    start();
}

ParamMacroService::~ParamMacroService()
{
    stop();
    delete mImpl;
}

void ParamMacroService::timerEvent(QTimerEvent* /*event*/) { mImpl->silenceReload(); }

void ParamMacroService::start()
{
    if (mImpl->timerID == -1)
    {
        mImpl->timerID = startTimer(70000);
    }
}

void ParamMacroService::stop()
{
    if (mImpl->timerID != -1)
    {
        killTimer(mImpl->timerID);
        mImpl->timerID = -1;
    }
}

void ParamMacroService::addItem(const ProtocolPack& pack)
{
    auto bak = pack;
    ParamMacroData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = mImpl->sql.insertItem(item);

    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("添加参数宏:%1-%2成功").arg(item.taskCode).arg(item.name), pack.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("添加参数宏:%1-%2失败").arg(item.taskCode).arg(item.name), pack.userID, MODULENAME);
    }

    bak.data << ans;
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void ParamMacroService::modifyItem(const ProtocolPack& pack)
{
    auto bak = pack;
    ParamMacroData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = mImpl->sql.updateItem(item);
    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("修改参数宏:%1-%2成功").arg(item.taskCode).arg(item.name), pack.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("修改参数宏:%1-%2失败").arg(item.taskCode).arg(item.name), pack.userID, MODULENAME);
    }

    bak.data << ans;
    silenceSendACK(bak);

    mImpl->silenceReload();
}

void ParamMacroService::deleteItem(const ProtocolPack& pack)
{
    auto bak = pack;
    ParamMacroData item;
    bak.data >> item;
    bak.data.clear();

    auto ans = mImpl->sql.deleteItem(item);

    if (ans.success())
    {
        SystemLogPublish::infoMsg(QString("删除参数宏:%1-%2成功").arg(item.taskCode).arg(item.name), pack.userID, MODULENAME);
    }
    else
    {
        SystemLogPublish::errorMsg(QString("删除参数宏:%1-%2失败").arg(item.taskCode).arg(item.name), pack.userID, MODULENAME);
    }

    bak.data << ans;
    silenceSendACK(bak);

    GlobalData::delParamMacroData(item.taskCode);
    mImpl->silenceReload();
}

void ParamMacroService::queryItem(const ProtocolPack& pack)
{
    auto bak = pack;
    bak.data.clear();

    bak.data << mImpl->sql.selectItem();
    silenceSendACK(bak);
}

void ParamMacroServiceImpl::pushData(const ParamMacroDataList& data) { GlobalData::setParamMacroData(data); }

Optional<ParamMacroDataList> ParamMacroServiceImpl::reload(bool push)
{
    auto items = sql.selectItem();
    if (items.success() & push)
    {
        pushData(items.value());
    }

    return items;
}

void ParamMacroServiceImpl::silenceReload(bool push)
{
    auto items = reload(push);
    if (!items)
    {
        auto msg = QString("查询参数宏数据错误:%1").arg(items.msg());
        SystemLogPublish::errorMsg(msg);
        return;
    }
}
