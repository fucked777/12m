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
};
#endif  // STTCPARAMMACROHANDLER_H
