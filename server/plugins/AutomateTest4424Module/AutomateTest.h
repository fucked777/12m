#ifndef AUTOMATETEST_H
#define AUTOMATETEST_H

#include "ControlFlowHandler.h"
#include "INetMsg.h"
namespace cppmicroservices
{
    class BundleContext;
}

class AutomateTestImpl;
class AutomateTest : public INetMsg
{
    Q_OBJECT
public:
    AutomateTest(cppmicroservices::BundleContext context);
    ~AutomateTest();

private:
    void start();
    void stop();
    void timerEvent(QTimerEvent* /*event*/) override;
    void stopTestPlan();
    /*
     * 添加测试项
     * data AutomateTestPlanItemList
     * ack Optional<AutomateTestPlanItemList>
     */
    void addTestItem(const ProtocolPack& pack);
    /*
     * 删除测试项
     * data AutomateTestPlanItem
     * ack Optional<AutomateTestPlanItem>
     */
    void deleteTestItem(const ProtocolPack& pack); /* 删除测试项 */

    void clearHistry(const ProtocolPack& pack);
    /*
     * 添加测试绑定
     * data AutomateTestParameterBind
     * ack Optional<AutomateTestParameterBind>
     */
    void addTestBindItem(const ProtocolPack& pack);
    /*
     * 删除测试绑定
     * data AutomateTestParameterBind
     * ack Optional<AutomateTestParameterBind>
     */
    void deleteTestBindItem(const ProtocolPack& pack);
    /*
     * 开始测试
     * data AutomateTestPlanItem
     * ack TestProcessACK
     *
     * 过程信息全部走的redis日志
     */
    void startTest(const ProtocolPack& pack); /* 开始测试 */
    /*
     * 停止测试
     * data 空
     * ack 空
     */
    void stopTest(const ProtocolPack& pack); /* 终止测试 */
    /*
     * 删除测试绑定
     * data AutomateTestResult
     * ack Optional<AutomateTestResult>
     */
    void deleteTestResult(const ProtocolPack& pack); /* 删除测试结果 */
    /*
     * 删除测试绑定
     * data AutomateTestResultQuery
     * ack Optional<AutomateTestResultList>
     */
    void queryTestResult(const ProtocolPack& pack); /* 查询测试结果 */

    /* 走的redis暂时未使用
     * 现在认为数据量不会太多
     * 直接全部推上去了
     */
    void queryTestItem(const ProtocolPack& pack); /* 查询测试项 */

signals:
    /*
     * 接收命令结果数据
     * data CmdResult
     * 这个就是ACK
     */
    void sg_deviceCMDResult(const ProtocolPack& pack);
    void sg_deviceCMDResponce(const ProtocolPack& pack); /* 命令响应 */

private:
    AutomateTestImpl* m_impl;
};
#endif  // AUTOMATETEST_H
