#include "RedisHelper.h"
#include "RedisConfig.h"
#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QTcpSocket>
#include <list>
#include <string>
#include <chrono>
#include "SubscriberHelper.h"
#include "HeartbeatMessage.h"
#include "GlobalData.h"

static constexpr int timeOut = 1000;
namespace redis_helper_impl
{
    thread_local static RedisConfigInfo redisConnectInfo;
}
RedisHelper& RedisHelper::getInstance() noexcept(std::is_nothrow_constructible<RedisHelper>::value)
{
    //! High availability requires at least 2 io service workers
    cpp_redis::network::set_default_nb_workers(2);
    thread_local static RedisHelper instance;

    return instance;
}
bool RedisHelper::getSubscriber(subscriber& sub)
{
    try
    {
        sub.connect(redis_helper_impl::redisConnectInfo.host.toStdString(), redis_helper_impl::redisConnectInfo.port);
        if (sub.is_connected())
        {
            sub.auth(redis_helper_impl::redisConnectInfo.auth.toStdString());
            qWarning() << QString("sub连接成功--%1").arg(redis_helper_impl::redisConnectInfo.host) ;
        }
        else
        {
            return false;
        }
    }
    catch (const std::exception& e)
    {
        qWarning() << "Redis getSubscriber failed:" << e.what();
        return false;
    }
    catch (...)
    {
        return false;
    }
    return true;
}
RedisHelper::RedisHelper(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<HeartbeatMessage>("HeartbeatMessage");
    qRegisterMetaType<HeartbeatMessage>("const HeartbeatMessage&");
    qRegisterMetaType<HeartbeatMessage>("HeartbeatMessage&");

    redis_helper_impl::redisConnectInfo = RedisConfig::loadConfig();
    auto info = GlobalData::masterSlaveInfo();
    if(!info.currentMasterIP.isEmpty())
    {
        redis_helper_impl::redisConnectInfo.host = info.currentMasterIP;
    }
    connect(SubscriberHelper::getInstance(), &SubscriberHelper::sg_statusChange, this, &RedisHelper::disconnectRedis);
}
void RedisHelper::reconnect()
{
    try
    {
        if (!isConnect())
        {
            client.connect(redis_helper_impl::redisConnectInfo.host.toStdString(), redis_helper_impl::redisConnectInfo.port, nullptr, timeOut);
            if (isConnect())
            {
                client.auth(redis_helper_impl::redisConnectInfo.auth.toStdString());
                qWarning() << QString("redis连接成功--%1").arg(redis_helper_impl::redisConnectInfo.host) ;
            }
        }
    }
    catch (const std::exception& e)
    {
        qWarning() << "redis连接错误:" << e.what();
    }
    catch (...)
    {
        qWarning() << "redis连接错误";
    }
}

void RedisHelper::disconnectRedisByHost(const QString&host)
{
    qWarning() << "Redis链接断开";
    if(isConnect())
    {
        client.disconnect();
    }
    if(!host.isEmpty())
    {
        redis_helper_impl::redisConnectInfo.host = host;
    }
    else
    {
        qWarning() << "当前主机ip为空";
    }
}

void RedisHelper::disconnectRedis(const HeartbeatMessage&msg)
{
    disconnectRedisByHost(msg.currentMasterIP);
}
void RedisHelper::resetConnectHost(const QString&host)
{
    if(host.isEmpty() || host == redis_helper_impl::redisConnectInfo.host)
    {
        return;
    }
    disconnectRedisByHost(host);
}

bool RedisHelper::isConnect() { return client.is_connected(); }

bool RedisHelper::toString(const reply& reply, QString& value)
{
    switch (reply.get_type())
    {
    case cpp_redis::reply::type::error:
        value = QString::fromStdString(reply.error());
        return false;
        break;
    case cpp_redis::reply::type::bulk_string:
        value = QString::fromStdString(reply.as_string());
        return true;
        break;
    case cpp_redis::reply::type::simple_string:
        value = QString::fromStdString(reply.as_string());
        return true;
        break;
    case cpp_redis::reply::type::null:
        value = "";
        return false;
        break;
    case cpp_redis::reply::type::integer:
        value = QString::number(reply.as_integer());
        return false;
        break;

    default: value = ""; return false;
    }
}

bool RedisHelper::toString(const reply& reply, QList<QString>& values)
{
    bool flag = true;
    switch (reply.get_type())
    {
    case cpp_redis::reply::type::array:

        for (const auto& item : reply.as_array())
        {
            QString value;
            switch (item.get_type())
            {
            case cpp_redis::reply::type::bulk_string: value = QString::fromStdString(item.as_string()); break;
            case cpp_redis::reply::type::simple_string: value = QString::fromStdString(item.as_string()); break;
            default:
                value = "";
                flag = false;
                break;
            }
            values.append(value);
        }
        break;

    default:
        values = {};
        flag = false;
        break;
    }
    return flag;
}
void RedisHelper::commit()
{
    try
    {
        client.sync_commit(std::chrono::milliseconds(timeOut));
    }
    catch (const std::exception& e)
    {
        qWarning() << "redis commit 错误:" << e.what();
    }
    catch (...)
    {
        qWarning() << "redis commit 错误";
    }
}

void RedisHelper::setData(QString key, QString value, int ttl)
{
    reconnect();
    if (isConnect())
    {
        if (ttl != -1)
        {
            //设置超时时间
            client.setex(key.toStdString(), ttl, value.toStdString());
        }
        else
        {
            client.set(key.toStdString(), value.toStdString());
        }

        commit();
        return;
    }
}

bool RedisHelper::getData(QString key, QString& value)
{
    reconnect();
    if (isConnect())
    {
        std::future<reply> get = client.get(key.toStdString());

        commit();
        if(get.valid())
        {
            return toString(get.get(), value);
        }
    }
    return false;
}

void RedisHelper::hsetData(QString key, QString field, QString value)
{
    reconnect();
    if (isConnect())
    {
        client.hset(key.toStdString(), field.toStdString(), value.toStdString());
        commit();
        return;
    }
}

bool RedisHelper::hgetData(QString key, QString field, QString& value)
{
    reconnect();
    if (isConnect())
    {
        std::future<reply> get = client.hget(key.toStdString(), field.toStdString());

        commit();

        if(get.valid())
        {
            return toString(get.get(), value);
        }
    }
    return false;
}

void RedisHelper::hsetData(QString key, QList<QPair<QString, QString>> values)
{
    reconnect();
    if (isConnect())
    {
        std::vector<std::pair<std::string, std::string>> _fields;
        for (auto item : values)
        {
            std::pair<std::string, std::string> _field;
            _field.first = item.first.toStdString();
            _field.second = item.second.toStdString();
            _fields.push_back(_field);
        }
        client.hmset(key.toStdString(), _fields);
        commit();
        return;
    }
}

void RedisHelper::hsetData(QString key, QMap<QString, QString> values)
{
    reconnect();
    if (isConnect())
    {
        std::vector<std::pair<std::string, std::string>> _fields;
        for (auto itemKey : values.keys())
        {
            std::pair<std::string, std::string> _field;
            _field.first = itemKey.toStdString();
            _field.second = values.value(itemKey).toStdString();
            _fields.push_back(_field);
        }
        client.hmset(key.toStdString(), _fields);
        commit();
        return;
    }
}

bool RedisHelper::hgetData(QString key, QList<QString> fields, QMap<QString, QString>& valueMaps)
{
    reconnect();
    if (isConnect())
    {
        std::vector<std::string> _fileds;
        for (auto item : fields)
        {
            _fileds.push_back(item.toStdString());
        }
        std::future<reply> get = client.hmget(key.toStdString(), _fileds);

        commit();

        if(get.valid())
        {
            QList<QString> list;
            auto flag = toString(get.get(), list);
            for (auto i = 0; i < list.size() && i < fields.size(); i++)
            {
                valueMaps[fields[i]] = list[i];
            }

            return flag;
        }
    }
    return false;
}

bool RedisHelper::hgetData(QString key, QMap<QString, QString>& valueMaps)
{
    reconnect();
    if (isConnect())
    {
        std::future<reply> get = client.hgetall(key.toStdString());
        commit();
        if(get.valid())
        {
            QList<QString> list;
            auto flag = toString(get.get(), list);
            if (flag)
            {
                QString tempKey;
                QString value;
                for (int i = 0; i < list.size(); i++)
                {
                    if (i % 2 == 0)
                    {
                        tempKey = list[i];
                    }
                    else
                    {
                        value = list[i];
                        valueMaps[tempKey] = value;
                    }
                }
            }
            return flag;
        }
    }
    return false;
}

bool RedisHelper::hdelKey(QString key, QString field)
{
    reconnect();
    if (isConnect())
    {
        std::vector<std::string> _fields;
        _fields.push_back(field.toStdString());
        client.hdel(key.toStdString(), _fields);

        commit();
        return true;
    }
    return false;
}

bool RedisHelper::hdelKeys(QString key, QList<QString> fields)
{
    reconnect();
    if (isConnect())
    {
        std::vector<std::string> _fields;
        for (auto field : fields)
        {
            _fields.push_back(field.toStdString());
        }
        client.hdel(key.toStdString(), _fields);

        commit();
        return true;
    }
    return false;
}

bool RedisHelper::delKey(QString key)
{
    reconnect();
    if (isConnect())
    {
        std::vector<std::string> keys;

        keys.push_back(key.toStdString());

        client.del(keys);

        commit();
        return true;
    }
    return false;
}

bool RedisHelper::publish(QString channel, QString message)
{
    reconnect();
    if (isConnect())
    {
        client.publish(channel.toStdString(), message.toStdString());
        commit();
        return true;
    }
    return false;
}

void RedisHelper::publish(QMap<QString, QString> msgChannl)
{
    reconnect();
    if (isConnect())
    {
        for (auto key : msgChannl.keys())
        {
            client.publish(key.toStdString(), msgChannl[key].toStdString());
        }
        commit();
        return;
    }
}

void RedisHelper::publistMusic(const QString& msg, const QString& channel) { publish(channel, msg); }
