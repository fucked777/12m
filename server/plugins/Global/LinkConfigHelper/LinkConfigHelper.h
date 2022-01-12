#ifndef LINKCONFIGHELPER_H
#define LINKCONFIGHELPER_H

#include <QSet>

#include "BusinessMacroCommon.h"
#include "ConfigMacroMessageDefine.h"
#include "CustomPacketMessageDefine.h"
#include "LinkConfigMessageDefine.h"
#include "PlanRunMessageDefine.h"
#include "SatelliteManagementDefine.h"

class LinkConfigHelper
{
public:
    static LinkConfigHelper& getInstance()
    {
        static LinkConfigHelper linkConfigHelper;
        return linkConfigHelper;
    }

    // 根据选择的设备信息进行链路组包
    bool packLinkConfigCmd(LinkType linkType, SystemWorkMode workMode, const QSet<QString> keyValuePairSet, QList<PackCommand>& packCmdList,
                           QString& errorMsg);

    bool packLinkConfigCmd(LinkType linkType,const  QSet<SystemWorkMode>& workMode, const QSet<QString> keyValuePairSet, QList<PackCommand>& packCmdList,
                           QString& errorMsg);

public:
    // 相同设备同一种单元命令或者过程控制命令参数合并到一起发下去
    static void mergeCommands(const QList<Command>& commandList, QList<Command>& mergedCommandList);
    // 把链路配置文件中的命令，组包消息转为PackCommand
    static bool packCommand(const Command& command, PackCommand& packCommand);

private:
    explicit LinkConfigHelper();
    // 从链路配置文件中查找指定链路下的模式
    bool getLinkConfigMode(LinkType linkType, int modeId, LinkMode& linkMode, QString& errorMsg);

    // 获取下发的参数名称和对应的值描述
    static void getProcessControlCmdSettingParamDesc(const MessageAddress& msgAddr, const ProcessControlCmdMessage& processCtrlCmdMsg,
                                                     QMap<QString, QVariant>& paramDescValueMap);
    // 获取下发的参数名称和对应的值描述
    static void getUnitParamSetCmdParamDesc(const MessageAddress& msgAddr, const UnitParamSetMessage& unitSetMsg,
                                            QMap<QString, QVariant>& paramDescValueMap);
};

#endif  // LINKCONFIGHELPER_H
