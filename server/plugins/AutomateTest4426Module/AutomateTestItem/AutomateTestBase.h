#ifndef AUTOMATETESTBASE_H
#define AUTOMATETESTBASE_H
#include "AutomateTestDefine.h"

#include "ConfigMacroMessageDefine.h"
#include "Utility.h"
#include <QList>
#include <QObject>
#include <QScopedPointer>
#include <QThread>

struct ManualMessage;
struct ConfigMacroData;
struct CmdResult;
struct DeviceID;
struct PackCommand;
struct ProtocolPack;
enum class AutomateTestStatus;
class AutomateTestHelper;
class AutomateTest;
class AutomateTestBaseImpl;
class SingleCommandHelper;
class AutomateTestBase : public QThread
{
    friend AutomateTestHelper;
    Q_OBJECT
    Q_DISABLE_COPY(AutomateTestBase)
public:
    enum class ExecStatus
    {
        Success,
        Timeout,
        Failed,
        Exit,
    };
    AutomateTestBase(AutomateTest* parent);
    ~AutomateTestBase();
    void startTest(const ProtocolPack& pack);
    void stopTest();
    bool isRunning() const;
    bool isExit() const;
    inline QString curtestTypeName() { return m_testPlanItem.testTypeName; }

    const AutomateTestPlanItem& testPlanItem() const;
    const ConfigMacroData& configMacroData() const;
    double upFreq() const;
    double downFreq() const;
    /* 当前旋向 */
    bool isLeftSpin() const;
    bool isRightSpin() const;

signals:
    void sg_end();

public slots:
    // 提示信息
    void signalInfoMsg(const QString& msg);
    // 警告信息
    void signalWarningMsg(const QString& msg);
    // 错误信息
    void signalErrorMsg(const QString& msg);

    void signalSendToDevice(const QByteArray& data); /* 发送命令到设备 */
private:
    void pushStatus(AutomateTestStatus status); /* 推送界面状态 */

protected:
    void pushLog(const QString& msg, bool error = false);          /* 推送日志 */
    void acceptDeviceCMDResultMessage(const ProtocolPack& pack);   /* 命令结果 */
    void acceptDeviceCMDResponceMessage(const ProtocolPack& pack); /* 命令响应 */
    void sendByteArrayToDevice(const QByteArray& data);            /* 发送命令到设备 */

    /* 等待指令执行完成 */
    ExecStatus waitExecSuccess(const PackCommand& packCommand, qint32 ttl = 40);
    /* 等待状态到位 单个参数 */
    ExecStatus waitSingleStatusChange(const PackCommand& packCommand, const DeviceID& devID, qint32 unitID, const QString& paramID,
                                      const QVariant& value, qint32 ttl = 40);
    /* 等待有结果上报的数据 */
    ExecStatus waitExecResponceData(CmdResult& cmdResult, const PackCommand& packCommand, qint32 count,
                                    qint32 ttl); /* count重试次数 timeout超时时间 */
    /* 等待有结果上报的数据 条件判断 */
    enum class ConditionResult
    {
        Error, /* 错误 */
        OK,    /* 条件成立 */
        Wait,  /* 条件不成立继续等待 */
    };

    using ConditionFunc = ConditionResult (*)(CmdResult&, QString&, const QVariant&);
    ExecStatus waitExecResponceCondition(CmdResult& cmdResult, const PackCommand& packCommand, qint32 count, qint32 ttl, const QVariant& arg,
                                         ConditionFunc func); /* count重试次数 timeout超时时间 */
    /* 等待有结果上报的数据 */
    ExecStatus waitExecResponceMultiData(QList<CmdResult>& cmdResultList, const PackCommand& packCommand, qint32 count,
                                         qint32 ttl); /* count重试次数 timeout超时时间 */

    /* 发送指令不等待结果 */
    void notWaitExecSuccess(const PackCommand& packCommand);
    /* 发送指令不等待结果 这里的安静指的是不弹窗 但是有日志 */
    void notWaitExecSuccessBeQuiet(const PackCommand& packCommand);
    /* 设置配置宏和参数宏 */
    bool setConfigMacroAndParamMacro(const LinkType& linkType);

protected:
    void run() override;
    virtual bool pretreatment();                                  /* 参数预处理 */
    virtual bool linkConfiguration() = 0;                         /* 设置环路 */
    virtual bool parameterSetting() = 0;                          /* 参数设置 */
    virtual bool testing(CmdResult& cmdResult) = 0;               /* 开始测试 */
    virtual bool resultOperation(const CmdResult& cmdResult) = 0; /* 保存结果 */
    virtual void endOfTest(); /* 结束测试(清理) 这个不能太耗时 不用等结果直接发指令就行 */

    ExecStatus waitExecSuccessImpl(const PackCommand& packCommand, qint32 ttl = 40, bool timeoutError = true);

protected:
    QScopedPointer<SingleCommandHelper> m_singleCommandHelper;
    AutomateTestPlanItem m_testPlanItem;
    ConfigMacroData m_configMacroData;
    double m_upFreq{ 0.0 };
    double m_downFreq{ 0.0 };

private:
    QScopedPointer<AutomateTestBaseImpl> m_impl;
};

#endif  // AUTOMATETESTBASE_H
