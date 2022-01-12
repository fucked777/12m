#ifndef DBINTERFACE_H
#define DBINTERFACE_H

#include "Utility.h"
#include <QDateTime>
#include <QTimer>

struct DBConnectInfo
{
    int port{ 2003 };
    QString ip{ "localhost" };
    QString dbName{ "db" };
    QString username{ "SYSDBA" };
    QString password{ "szoscar55" };
    QString connectOptions{ "SQL_ATTR_LOGIN_TIMEOUT=3;SQL_ATTR_CONNECTION_TIMEOUT=1" };
};

#define createConnectName() createConnectNameMacro(__FUNCTION__)

class QSqlDatabase;
class DBInterface
{
public:
    static QString createConnectNameMacro(const char* func);
    // static QString createConnectName();
    static int getMaxCount();

    /* 长连接重复使用 */
    static Optional<QSqlDatabase> getDefaultConnect(const QString& connectName);
    static Optional<QSqlDatabase> getConnection(const QString& connectName, const DBConnectInfo& info);

    /* 短链接每次都是新的,需要在使用后手动销毁 */
    static Optional<QSqlDatabase> getShortConnect(const QString& connectName);
    static Optional<QSqlDatabase> getNewConnection(const QString& connectName, const DBConnectInfo& info);
    static void closeConnection(const QString& connectionName);

    /* 状态入库 */
    static Optional<QSqlDatabase> getDeviceStatusConnect(const QString& connectName);

    /* 关闭当前的所有数据库连接  */
    static void closeAllConnection();
};

class AutoCloseDBHelper
{
public:
    explicit AutoCloseDBHelper(const QString& name)
        : m_connectName(name)
    {
    }
    AutoCloseDBHelper()
        : m_connectName(DBInterface::createConnectName())
    {
    }
    ~AutoCloseDBHelper() { DBInterface::closeConnection(m_connectName); }
    QString operator()() const { return m_connectName; }

private:
    QString m_connectName;
};

#endif  // DBINTERFACE_H
