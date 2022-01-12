#ifndef REDISHELPER_H
#define REDISHELPER_H
#include "Utility.h"
#include <QString>

#include <cpp_redis/cpp_redis>
using namespace cpp_redis;

struct HeartbeatMessage;
class RedisHelper : public QObject
{
    Q_OBJECT
public:
    static RedisHelper& getInstance() noexcept(std::is_nothrow_constructible<RedisHelper>::value);

    bool getSubscriber(cpp_redis::subscriber& sub);
    virtual ~RedisHelper() noexcept {}

    RedisHelper(const RedisHelper&) = delete;
    RedisHelper& operator=(const RedisHelper&) = delete;

    //连接redis 或者重新连接redis
    void reconnect();
    void disconnectRedis(const HeartbeatMessage&msg);
    void disconnectRedisByHost(const QString&host);
    void resetConnectHost(const QString&host);

    void setData(QString key, QString value, int ttl = -1);
    bool getData(QString key, QString& value);

    void hsetData(QString key, QString field, QString value);
    bool hgetData(QString key, QString field, QString& value);

    void hsetData(QString key, QList<QPair<QString, QString>> values);
    void hsetData(QString key, QMap<QString, QString> values);

    bool hgetData(QString key, QList<QString> fields, QMap<QString, QString>& valueMaps);
    bool hgetData(QString key, QMap<QString, QString>& valueMaps);

    bool hdelKey(QString key, QString field);
    bool hdelKeys(QString key, QList<QString> fields);

    bool delKey(QString key);

    bool publish(QString channel, QString message);
    void publish(QMap<QString, QString> msgChannl);
    void publistMusic(const QString& msg, const QString& channel = "Music");

    bool isConnect();

    bool toString(const cpp_redis::reply& reply, QString& value);
    bool toString(const cpp_redis::reply& reply, QList<QString>& value);

protected:
    RedisHelper(QObject *parent = nullptr);
    void commit();

    cpp_redis::client client;

};

#endif  // JSONHELPER_H
