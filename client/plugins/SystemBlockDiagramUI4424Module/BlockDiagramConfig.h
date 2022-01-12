#ifndef BLOCKDIAGRAMCONFIG_H
#define BLOCKDIAGRAMCONFIG_H
#include "BlockDiagramDefine.h"
#include "Utility.h"
#include <QString>

class QDomElement;
class BlockDiagramConfig
{
public:
    BlockDiagramConfig();
    ~BlockDiagramConfig();
    /* 重新加载配置 */
    OptionalNotValue reload();

    SystemBlockInfo getParseResult() { return m_info; }

private:
    OptionalNotValue parseXML(const QDomElement& element);
    Optional<SystemTypeInfo> parseSystemTypeInfo(const QDomElement& element);
    Optional<TwoValueSwitchTypeInfo> parseTwoValueSwitchTypeInfo(const QDomElement& element);
    Optional<DeviceStatusTypeInfo> parseDeviceStatusTypeInfo(const QDomElement& element);

private:
    SystemBlockInfo m_info;
};

#endif  // BLOCKDIAGRAMCONFIG_H
