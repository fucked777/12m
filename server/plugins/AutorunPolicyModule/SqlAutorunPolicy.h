#ifndef SQLAUTORUNPOLICY_H
#define SQLAUTORUNPOLICY_H

#include "AutorunPolicyMessageDefine.h"
#include "Utility.h"
#include <QList>
#include <QObject>
#include <QSqlDatabase>
#include <QVariantMap>

class SqlAutorunPolicy
{
public:
    static OptionalNotValue initDB();
    static Optional<AutorunPolicyData> updateItem(const AutorunPolicyData& item);
    static Optional<AutorunPolicyData> selectItem();
};

#endif  // SQLAUTORUNPOLICY_H
