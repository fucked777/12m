#include "SqlAutorunPolicy.h"
#include "AutorunPolicyMessageSerialize.h"
#include "DBInterface.h"
#include "SqlGlobalData.h"
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

static char sqlGlobalDataAutorunPolicyName[] = "AutorunPolicy";  // 自动化运行策略的数据索引

OptionalNotValue SqlAutorunPolicy::initDB()
{
    AutoCloseDBHelper closeHelper;
    {
        auto db = DBInterface::getShortConnect(closeHelper());
        if (!db)
        {
            return OptionalNotValue(db.errorCode(), db.msg());
        }

        auto result = SqlGlobalData::initDB(db.value());
        if (!result)
        {
            return result;
        }

        SqlGlobalData::updateItemNotExist(db.value(), sqlGlobalDataAutorunPolicyName);
        return OptionalNotValue();
    }
}

Optional<AutorunPolicyData> SqlAutorunPolicy::updateItem(const AutorunPolicyData& item)
{
    using ResType = Optional<AutorunPolicyData>;

    QByteArray array;
    array << item;

    auto result = SqlGlobalData::updateItem(sqlGlobalDataAutorunPolicyName, array);
    return (!result) ? ResType(result.errorCode(), result.msg()) : ResType(item);
}
Optional<AutorunPolicyData> SqlAutorunPolicy::selectItem()
{
    using ResType = Optional<AutorunPolicyData>;
    auto result = SqlGlobalData::selectItem(sqlGlobalDataAutorunPolicyName);
    if (!result)
    {
        return ResType(result.errorCode(), result.msg());  //"未找到自动化运行策略的数据"
    }
    AutorunPolicyData tempData;
    result.value() >> tempData;
    return ResType(tempData);
}
