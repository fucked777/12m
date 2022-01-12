#ifndef STATIONNETCENTERREMOTECMD_H
#define STATIONNETCENTERREMOTECMD_H
#include "StationNetCenterMessageDefine.h"
#include "StationNetCenterType.h"
#include "Utility.h"
#include <QMap>
#include <memory>

struct TaskTimeList;
struct DeviceID;
struct PackCommand;
class StationNetCenterRemoteCmdImpl;
class StationNetCenterRemoteCmd : public QObject
{
    Q_OBJECT
public:
    StationNetCenterRemoteCmd(const SNCCmdMap& sncCmdMap, QObject* parent = nullptr);
    ~StationNetCenterRemoteCmd();

    /* 所有进入这里的命令都不需要再发送应答了
     * remoteCmd 远控命令的数据
     * errMsg 错误信息
     *
     */
    void execCmd(const ZwzxRemoteControlCMD& remoteCmd);

signals:
    // 发送数据到设备
    void signalSendToDevice(const QByteArray& data);
    void signalRemoteControlCmdResult(const ZwzxRemoteControlCMD& remoteCmd, ZwzxRemoteControlCmdReplyResult result);

private:
    void notifyInfoLog(const QString& logStr);
    void notifyWarningLog(const QString& logStr);
    void notifyErrorLog(const QString& logStr);
    void notifySpecificTipsLog(const QString& logStr);
    bool waitExecSuccess(const PackCommand& packCommand, uint ttl = 10);
    bool waitExecSuccess(const QString& key, const DeviceID&, uint ttl = 10);
    bool execLinkCommand(const SNCCmdNode& sncCmdNode, const ConfigMacroModeItemMap& configMacroCmdList);

    //    // 通信测试 00H
    //    ZwzxRemoteControlCmdReplyResult handleCommunicationTest(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    // 测控上行载波开关（S频段） 01H
    //    ZwzxRemoteControlCmdReplyResult handleMeasureControlUpWaveSwitch01(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleMeasureControlUpWaveSwitch02(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    // 测控上行遥控开关（S频段） 02H
    //    ZwzxRemoteControlCmdReplyResult handleMeasureControlUpRemoteCtrSwitch01(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleMeasureControlUpRemoteCtrSwitch02(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);

    //    // 上行输出载波开关（量子专用）10H
    //    // void handleUpDataTransWaveSwitch01(const ZwzxRemoteControlCMD& remoteCmd);
    //    // void handleUpDataTransWaveSwitch02(const ZwzxRemoteControlCMD& remoteCmd);
    //    // void handleUpDataTransWaveSwitch03(const ZwzxRemoteControlCMD& remoteCmd);
    //    // void handleUpDataTransWaveSwitch04(const ZwzxRemoteControlCMD& remoteCmd);
    //    // void handleUpDataTransWaveSwitch05(const ZwzxRemoteControlCMD& remoteCmd);
    //    // void handleUpDataTransWaveSwitch06(const ZwzxRemoteControlCMD& remoteCmd);
    //    // void handleUpDataTransWaveSwitch11(const ZwzxRemoteControlCMD& remoteCmd);
    //    // void handleUpDataTransWaveSwitch12(const ZwzxRemoteControlCMD& remoteCmd);
    //    // void handleUpDataTransWaveSwitch13(const ZwzxRemoteControlCMD& remoteCmd);

    //    // X 测控上行通道开关 03H 保留
    //    // void handleXUpDataTransWaveSwitch(const ZwzxRemoteControlCMD& remoteCmd);

    //    // Ka 测控上行通道开关 04H
    //    ZwzxRemoteControlCmdReplyResult handleKaUpDataTransWaveSwitch01(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaUpDataTransWaveSwitch02(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaUpDataTransWaveSwitch03(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);

    //    // S  数传上行通道开关 11H 保留
    //    // void handleSDataTransWaveSwitch(const ZwzxRemoteControlCMD& remoteCmd);
    //    // X  数传上行通道开关 12H 保留
    //    // void handleXDataTransWaveSwitch(const ZwzxRemoteControlCMD& remoteCmd);

    //    // Ka 数传上行通道开关 13H
    //    ZwzxRemoteControlCmdReplyResult handleKaDataTransWaveSwitch01(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDataTransWaveSwitch02(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);

    //    // S  下行数传开关 21H
    //    // ZwzxRemoteControlCmdReplyResult handleSDownDataTransWaveSwitch(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);

    //    // X  下行数传开关 22H
    //    // 0x01-0x10 是高速的 X只有低速
    //    ZwzxRemoteControlCmdReplyResult handleXDownDataTransWaveSwitch11(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleXDownDataTransWaveSwitch12(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleXDownDataTransWaveSwitch13(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleXDownDataTransWaveSwitch14(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleXDownDataTransWaveSwitch15(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleXDownDataTransWaveSwitch16(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);

    //    // Ka 下行数传开关 23H
    //    // 0x01-0x10 是Ka高速
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch01(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch02(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch03(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch04(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch05(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch06(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch07(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch08(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch09(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch10(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    // 0x01-0x10 是Ka低速
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch11(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch12(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch13(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch14(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch15(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitch16(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);

    //    // 测控通道中心 AAH
    //    ZwzxRemoteControlCmdReplyResult handleCKCenterDownDataTransWaveSwitch01(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleCKCenterDownDataTransWaveSwitch02(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleCKCenterDownDataTransWaveSwitch03(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleCKCenterDownDataTransWaveSwitch04(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);

    //    // 数传通道中心 55H
    //    ZwzxRemoteControlCmdReplyResult handleDTCenterDownDataTransWaveSwitch01(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleDTCenterDownDataTransWaveSwitch02(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleDTCenterDownDataTransWaveSwitch03(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleDTCenterDownDataTransWaveSwitch04(const ZwzxRemoteControlCMD& remoteCmd, QString& errMsg);

    // private:
    //    ZwzxRemoteControlCmdReplyResult handleMeasureControlUpWaveSwitch(const ZwzxRemoteControlCMD& remoteCmd, bool hpPower, bool carrierOutput,
    //                                                                     bool addRemove, QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaUpDataTransWaveSwitch(const ZwzxRemoteControlCMD& remoteCmd, bool hpPower, bool carrierOutput,
    //                                                                  bool addRemove, QString& errMsg);
    //    /* 下行数传开关 */
    //    /* 低速的通道1是Ka低速 通道2 是X低速 */
    //    ZwzxRemoteControlCmdReplyResult handleDSDownDataTransWaveSwitchAll(const ZwzxRemoteControlCMD& /* remoteCmd */, bool isKa, bool start,
    //                                                                       QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitchAll(const ZwzxRemoteControlCMD& /* remoteCmd */, bool start, QString&
    //    errMsg);

    //    /* 低速的通道1是Ka低速 通道2 是X低速 */
    //    ZwzxRemoteControlCmdReplyResult handleDSDownDataTransWaveSwitchItem(const ZwzxRemoteControlCMD& /* remoteCmd */, int dp, bool isKa, bool
    //    start,
    //                                                                        QString& errMsg);
    //    ZwzxRemoteControlCmdReplyResult handleKaDownDataTransWaveSwitchItem(const ZwzxRemoteControlCMD& /* remoteCmd */, int dp, bool start,
    //                                                                        QString& errMsg);

    //    ZwzxRemoteControlCmdReplyResult handleCenterDownDataTransWaveSwitch(const ZwzxRemoteControlCMD& remoteCmd, const QString& indexStr,
    //                                                                        quint32 number, QString& errMsg);

private:
    StationNetCenterRemoteCmdImpl* m_impl;
};

#endif  // STATIONNETCENTERREMOTECMD_H
