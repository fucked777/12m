#ifndef KAGSPARAMMACROHANDLER_H
#define KAGSPARAMMACROHANDLER_H

#include "BaseParamMacroHandler.h"

// Ka高速参数宏
class KaGSParamMacroHandler : public BaseParamMacroHandler
{
    Q_OBJECT
public:
    explicit KaGSParamMacroHandler(QObject* parent = nullptr);

    bool handle() override;
    SystemWorkMode getSystemWorkMode() override;

private:
    bool getGSJDUnitTargetParamMap(const TargetInfo& targetInfo, QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap);
};
#endif  // KAGSPARAMMACROHANDLER_H
