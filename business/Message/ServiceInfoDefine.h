#ifndef SERVICEINFODEFINE_H
#define SERVICEINFODEFINE_H

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QObject>

/* 服务器的信息,拆成两部分
 * 一部分是服务器在线状态 就一个值true代表在线 其他的值都是不在线
 * 另一部分是服务器的其他信息,只会在程序启动后推送一次
 */
struct ServiceInfo
{
    QDateTime startTime; /* 服务器的启动时间 */
};
Q_DECLARE_METATYPE(ServiceInfo)

#endif  // SERVICEINFODEFINE_H
