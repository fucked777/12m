#ifndef AUTORUNPOLICYMESSAGEDEFINE_H
#define AUTORUNPOLICYMESSAGEDEFINE_H
#include <QVariantMap>

/* 自动化运行策略的数据就是QVariantMap */
struct AutorunPolicyData
{
    QVariantMap policy;
};

Q_DECLARE_METATYPE(AutorunPolicyData);
#endif  // AUTORUNPOLICYMESSAGEDEFINE_H
