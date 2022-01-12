#ifndef STTCPARAMMACROHANDLER_H
#define STTCPARAMMACROHANDLER_H

#include "BaseParamMacroHandler.h"

// S频段参数宏
class STTCParamMacroHandler : public BaseParamMacroHandler
{
    Q_OBJECT
public:
    explicit STTCParamMacroHandler(QObject* parent = nullptr);

    bool handle() override;
    SystemWorkMode getSystemWorkMode() override;

private:
    // 测控基带
    bool ckjdParam(const QSet<SystemWorkMode>& workModeSet);
    // Skuo2+kakuo2
    bool doubleMode(const QSet<SystemWorkMode>& workModeSet);
    // 双主机的模式
    bool doubleMasterMode(const QSet<SystemWorkMode>& workModeSet);
    bool singleMode(const QSet<SystemWorkMode>& workModeSet);
    // 送数开关
    void ckTaskSS(const QSet<SystemWorkMode>& workModeSet);
    // 测控前端
    bool ckqgParam(const QSet<SystemWorkMode>& workModeSet, const ParamMacroModeData& paramMacroModeItem);
    // 其他设备
    bool otherDeviceParam(const ParamMacroModeData& paramMacroModeItem);
    // 站址装订
    void zzzz(const QSet<SystemWorkMode>& workModeSet);

private:
    CKJDControlInfo m_ckjdInfo;
    /* 给前端的 */
    QVariant m_qdUpFrequency;
    QVariant m_qdDownFrequency;
    /* 当前的 */
    QVariant m_upFrequency;
    QVariant m_downFrequency;
};
#endif  // STTCPARAMMACROHANDLER_H
