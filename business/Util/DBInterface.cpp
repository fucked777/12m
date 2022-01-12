#include "DBInterface.h"
#include "PlatformInterface.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QMutex>
#include <QSettings>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QUuid>
/* Qt 会动态的加载数据库的plugin,
 * 加载 plugin 的部分，涉及到对本地库文件的管理，这一部分
 * 出现了竞争。于是很自然的想到在初始连接部分设置 Mutex 保护
 * 从 addDatabase / database到 open 的部分，要保证其原子性
 */
Q_GLOBAL_STATIC(QMutex, gDBPoolMutex);

constexpr static auto gTestSql = "select 0;";

class DBInterfacePrivate
{
public:
    static void parseItemConfig(QDomElement& root, QMap<QString, DBConnectInfo>& info, QString& nodeKey)
    {
        auto node = root.firstChild();
        while (!node.isNull())
        {
            auto isComment = node.isComment();
            auto element = node.toElement();
            auto name = node.nodeName();
            node = node.nextSibling();
            /* 跳过注释 */
            if (isComment)
            {
                continue;
            }

            if(name == "Default")
            {
                nodeKey = "Default";
            }
            else if(name == "DeviceStatus")
            {
                nodeKey = "DeviceStatus";
            }
            else if (name == "IP")
            {
                info[nodeKey].ip = element.text().trimmed();
            }
            else if (name == "Port")
            {
                info[nodeKey].port = element.text().trimmed().toInt();
            }
            else if (name == "DBName")
            {
                info[nodeKey].dbName = element.text().trimmed();
            }
            else if (name == "UserName")
            {
                info[nodeKey].username = element.text().trimmed();
            }
            else if (name == "Password")
            {
                info[nodeKey].password = element.text().trimmed();
            }
            else if (name == "Options")
            {
                info[nodeKey].connectOptions = element.text().trimmed();
            }
            parseItemConfig(element, info, nodeKey);
        }
    }
    static QMap<QString, DBConnectInfo> loadDBConnectConfig()
    {
        auto configPath = PlatformConfigTools::configBusiness("DBConnectInfo.xml");
        QFile file(configPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            auto errMsg =QString("%1:%2").arg(file.errorString(), configPath);
            qWarning() << errMsg;
            return {};
        }

        QDomDocument domDoc;
        QString errMsg;
        int errLine{ 0 };
        int errcCol{ 0 };
        if (!domDoc.setContent(&file, &errMsg, &errLine, &errcCol))
        {
            errMsg = QString("设备配置信息解析失败:%1,错误行:%2,错误列:%2").arg(errMsg).arg(errLine).arg(errcCol);
            qWarning() << errMsg;
            return {};
        }
        QMap<QString, DBConnectInfo> info;
        QString nodeKey;
        auto root = domDoc.documentElement();
        parseItemConfig(root, info, nodeKey);
//        for(auto iter = info.begin();iter != info.end();++iter)
//        {
//            qWarning() << iter.key();
//            qWarning() << iter->ip;
//            qWarning() << iter->connectOptions;
//            qWarning() << iter->dbName;
//            qWarning() << iter->password;
//            qWarning() << iter->port;
//            qWarning() << iter->username;
//        }
        return info;
    }
    static void setSqlParameter(QSqlDatabase& database, const QString& connectName, const DBConnectInfo& info)
    {
        database = QSqlDatabase::addDatabase("QODBC", connectName);
        database.setHostName(info.ip);
        database.setDatabaseName(info.dbName);
        database.setUserName(info.username);
        database.setPassword(info.password);
        database.setPort(info.port);
        // database.setConnectOptions("SQL_ATTR_LOGIN_TIMEOUT=3;SQL_ATTR_CONNECTION_TIMEOUT=1");
        database.setConnectOptions(info.connectOptions);
    }

    static DBConnectInfo loadDBConnectConfig(const QString&name)
    {
        return loadDBConnectConfig().value(name);
    }
};

Optional<QSqlDatabase> DBInterface::getConnection(const QString& connectName, const DBConnectInfo& info)
{
    QMutexLocker locker(gDBPoolMutex());
    using ResType = Optional<QSqlDatabase>;
    if (QSqlDatabase::contains(connectName))
    {
        auto database = QSqlDatabase::database(connectName);

        if (!database.isOpen())
        {
            database.open();
        }
        if (database.isOpenError())
        {
            return ResType(ErrorCode::DataBaseOpenFailed, database.lastError().text());
        }

        QSqlQuery tempQuery = database.exec(gTestSql);
        auto lastError = tempQuery.lastError();
        if (lastError.isValid())
        {
            return ResType(ErrorCode::DataBaseOpenFailed, lastError.text());
        }

        return ResType(database);
    }
    /* 设置参数 */
    QSqlDatabase database;
    DBInterfacePrivate::setSqlParameter(database, connectName, info);

    return database.open() ? ResType(database) : ResType(ErrorCode::DataBaseOpenFailed, database.lastError().text());
}
Optional<QSqlDatabase> DBInterface::getNewConnection(const QString& connectName, const DBConnectInfo& info)
{
    QMutexLocker locker(gDBPoolMutex());
    using ResType = Optional<QSqlDatabase>;
    QSqlDatabase database;
    /* 先关闭连接 */
    closeConnection(connectName);
    /* 设置参数 */
    DBInterfacePrivate::setSqlParameter(database, connectName, info);

    return database.open() ? ResType(database) : ResType(ErrorCode::DataBaseOpenFailed, database.lastError().text());
}

Optional<QSqlDatabase> DBInterface::getDefaultConnect(const QString& connectName)
{
    static auto info = DBInterfacePrivate::loadDBConnectConfig("Default");
    return getConnection(connectName, info);
}
Optional<QSqlDatabase> DBInterface::getShortConnect(const QString& connectName)
{
    static auto info = DBInterfacePrivate::loadDBConnectConfig("Default");
    return getNewConnection(connectName, info);
}
Optional<QSqlDatabase> DBInterface::getDeviceStatusConnect(const QString& connectName)
{
    static auto info = DBInterfacePrivate::loadDBConnectConfig("DeviceStatus");
    return getConnection(connectName, info);
}

// QString DBInterface::createConnectName()
//{
//    auto uuid = QUuid::createUuid().toString();
//    return uuid;
//}

QString DBInterface::createConnectNameMacro(const char* func)
{
    QString qfunc(func);
    qfunc.replace("::", "");
    auto uuid = QUuid::createUuid().toString();
    auto value = QString("%1-%2").arg(qfunc, uuid);
    return value;
}

void DBInterface::closeConnection(const QString& connectionName)
{
    if (QSqlDatabase::contains(connectionName))
    {
        {
            QMutexLocker locker(gDBPoolMutex());
            QSqlDatabase tempDBNew = QSqlDatabase::database(connectionName);
            if (tempDBNew.isOpen())
            {
                tempDBNew.close();
            }
        }
        QSqlDatabase::removeDatabase(connectionName);
    }
}
void DBInterface::closeAllConnection()
{
    auto names = QSqlDatabase::connectionNames();
    for (auto& item : names)
    {
        closeConnection(item);
    }
}
