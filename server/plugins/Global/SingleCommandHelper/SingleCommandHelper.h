#ifndef SINGLECOMMANDHELPER_H
#define SINGLECOMMANDHELPER_H

#include "BusinessMacroCommon.h"
#include "CustomPacketMessageDefine.h"
#include "SingleCommandXmlReader.h"

class SingleCommandHelper
{
public:
    SingleCommandHelper();

    // 组单命令  replaceDeviceID为需要替换配置文件的设备ID
    bool packSingleCommand(const QString& singleCmdId, PackCommand& packCommand, DeviceID replaceDeviceID = {},
                           QMap<QString, QVariant> replaceParamMap = {}) const;

    // 替参数
    bool packSingleCommand(const QString& singleCmdId, PackCommand& packCommand, QMap<QString, QVariant> replaceParamMap) const;

    // 需要替换模式的
    bool packSingleCommand(const QString& singleCmdId, PackCommand& packCommand, DeviceID replaceDeviceID, int replaceModeId,
                           QMap<QString, QVariant> replaceParamMap = {}) const;

    // 需要替换模式的
    bool packSingleCommand(const QString& singleCmdId, PackCommand& packCommand, DeviceID replaceDeviceID, SystemWorkMode replaceSystemMode,
                           QMap<QString, QVariant> replaceParamMap = {}) const;

    // 主要用于单命令 比如DTE任务加载
    bool packSingleCommand(const QString& singleCmdId, PackCommand& packCommand, const QMap<int, QList<QPair<QString, QVariant>>>& multiParamMap,
                           DeviceID replaceDeviceID = {}, const QVariantMap& replaceParamMap = {}) const;

    /* 反向获取ini文件里面的参数 */
    QVariant getSingleCmdValue(const QString& stepKey, const QString& filedKey, const QVariant& defaultValue = QVariant());
    DeviceID getSingleCmdDeviceID(const QString& stepKey, const QString& defaultValue = QString());
    int getSingleCmdUnitOrCmd(const QString& stepKey, int defaultValue = -1);

private:
    // 组包单命令，组包消息转为PackCommand
    bool packSingleCommand(const SingleCommand& singleCmd, PackCommand& packCommand) const;

    // 获取下发的参数名称和对应的值描述
    void getProcessControlCmdSettingParamDesc(const MessageAddress& msgAddr, const ProcessControlCmdMessage& processCtrlCmdMsg,
                                              QMap<QString, QVariant>& paramDescValueMap) const;
    // 获取下发的参数名称和对应的值描述
    void getUnitParamSetCmdParamDesc(const MessageAddress& msgAddr, const UnitParamSetMessage& unitSetMsg,
                                     QMap<QString, QVariant>& paramDescValueMap) const;
    void getUnitParamSetCmdMultiParamDesc(const MessageAddress& msgAddr, const UnitParamSetMessage& unitSetMsg, const QMap<QString, QVariant>& src,
                                          QMap<QString, QVariant>& paramDescValueMap, int targetNo, bool fixed) const;
    bool createMultiPackMsg(const MessageAddress& targetAddr, const SingleCommand& singleCmd, PackCommand& packCommand, PackMessage& packMsg) const;
    bool createPackMsg(const MessageAddress& targetAddr, const SingleCommand& singleCmd, PackCommand& packCommand, PackMessage& packMsg) const;

private:
    QMap<QString, SingleCommand> mSingleCmdMap;  // 任务计划运行需要的单命令
};

#endif  // SINGLECOMMANDHELPER_H
