#ifndef AUTOMATETESTMODULESERVICE_H
#define AUTOMATETESTMODULESERVICE_H

#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class AutomateTestModuleServiceImpl;
class AutomateTestModuleService : public INetMsg
{
    Q_OBJECT
public:
    AutomateTestModuleService(cppmicroservices::BundleContext context);
    ~AutomateTestModuleService();

signals:
    void sg_addTestItemACK(const QByteArray& data);      /* 添加测试项 */
    void sg_addTestBindItemACK(const QByteArray&);       /* 添加参数绑定 */
    void sg_deleteTestBindItemACK(const QByteArray&);    /* 删除参数绑定 */
    void sg_deleteTestItemACK(const QByteArray& data);   /* 删除测试项 */
    void sg_deleteTestResultACK(const QByteArray& data); /* 删除测试结果 */
    void sg_clearHistryACK(const QByteArray& data);      /* 清理测试结果历史数据 */
    void sg_startTestACK(const QByteArray& data);        /* 开始测试 */
    void sg_stopTestACK(const QByteArray& data);         /* 终止测试 */
    void sg_queryTestItemACK(const QByteArray& data);    /* 查询测试项 */
    void sg_queryTestResultACK(const QByteArray& data);  /* 查询测试结果 */

public:
    void addTestItem(const QByteArray& data);      /* 添加测试项 */
    void addTestBindItem(const QByteArray&);       /* 添加参数绑定 */
    void deleteTestBindItem(const QByteArray&);    /* 删除参数绑定 */
    void deleteTestItem(const QByteArray& data);   /* 删除测试项 */
    void deleteTestResult(const QByteArray& data); /* 删除测试结果 */
    void clearHistry(const QByteArray& data);      /* 清理测试结果历史数据 */
    void startTest(const QByteArray& data);        /* 开始测试 */
    void stopTest(const QByteArray& data);         /* 终止测试 */
    void queryTestItem(const QByteArray& data);    /* 查询测试项 */
    void queryTestResult(const QByteArray& data);  /* 查询测试结果 */

    void addTestItemACK(const ProtocolPack& pack);        /* 添加测试项 */
    void addTestBindItemACK(const ProtocolPack& pack);    /* 添加参数绑定 */
    void deleteTestBindItemACK(const ProtocolPack& pack); /* 删除参数绑定 */
    void deleteTestItemACK(const ProtocolPack& pack);     /* 删除测试项 */
    void deleteTestResultACK(const ProtocolPack& pack);   /* 删除测试结果 */
    void clearHistryACK(const ProtocolPack& data);        /* 清理测试结果历史数据 */
    void startTestACK(const ProtocolPack& pack);          /* 开始测试 */
    void stopTestACK(const ProtocolPack& pack);           /* 终止测试 */
    void queryTestItemACK(const ProtocolPack& pack);      /* 查询测试项 */
    void queryTestResultACK(const ProtocolPack& pack);    /* 查询测试结果 */

private:
    AutomateTestModuleServiceImpl* m_impl;
};
#endif  // AUTOMATETESTMODULESERVICE_H
