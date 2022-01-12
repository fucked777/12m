#ifndef XGSPARAMMACROHANDLER_H
#define XGSPARAMMACROHANDLER_H

#include "BaseParamMacroHandler.h"

// X高速参数宏
class XGSParamMacroHandler : public BaseParamMacroHandler
{
    Q_OBJECT
public:
    explicit XGSParamMacroHandler(QObject* parent = nullptr);

    bool handle() override;
    SystemWorkMode getSystemWorkMode() override;

private:
    bool getGSJDUnitTargetParamMap(const TargetInfo& targetInfo, QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap);
};
#endif  // XGSPARAMMACROHANDLER_H
