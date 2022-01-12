#ifndef REDISCONFIG_H
#define REDISCONFIG_H
#include "Utility.h"

struct RedisConfigInfo
{
    QString auth;
    QString host{ "127.0.0.1" };
    quint16 port{ 6379 };
};

class RedisConfig
{
public:
    /* 读取错误会直接返回默认配置数据 */
    static RedisConfigInfo loadConfig();
};

#endif  // REDISCONFIG_H
