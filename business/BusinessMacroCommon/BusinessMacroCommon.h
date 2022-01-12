#ifndef BUSINESSMACROCOMMON_H
#define BUSINESSMACROCOMMON_H

#include "DevProtocol.h"
#include "ParamMacroMessageSerialize.h"
#include "ProtocolXmlTypeDefine.h"
#include "ResourceRestructuringMessageDefine.h"
#include "SystemWorkMode.h"

struct PackCommand
{
    DeviceID deviceID;                          // 设备ID
    int modeId{ -1 };                           // 模式id
    int cmdId{ -1 };                            // 单元或者命令id
    DevMsgType cmdType{ DevMsgType::Unknown };  // 命令类型
    QString operatorInfo;                       // 配置文件配置的描述该条命令执行的内容

    QByteArray data;                            // 组包待发送的数据
    QMap<QString, QVariant> paramDescValueMap;  // 下发的参数名称和对应的值 QMap<参数名称, 参数值>
    QString errorMsg;                           // 在转换为PackCommand的过程中出现的错误

    // 重置数据
    void reset()
    {
        deviceID = DeviceID(-1, -1, -1);
        modeId = -1;
        cmdId = -1;
        cmdType = DevMsgType::Unknown;
        operatorInfo.clear();
        data.clear();
        errorMsg.clear();
        paramDescValueMap.clear();
    }

    // 获取当前下发的参数名称和对应的值
    QString getSetParamDescValue() const
    {
        QStringList list;
        for (auto paramDesc : paramDescValueMap.keys())
        {
            auto paramValueDesc = paramDescValueMap.value(paramDesc);
            list << QString("%1->%2").arg(paramDesc).arg(paramValueDesc.toString());
        }
        return list.isEmpty() ? "" : list.join("，");
    }

    // 获取当前发送命令的设备名称
    QString getCmdDeviceName() const { return GlobalData::getExtensionName(deviceID); }

    // 获取当前发送命令名称
    QString getCmdName() const
    {
        if (cmdType == DevMsgType::UnitParameterSetCmd)
        {
            return GlobalData::getUnitName(deviceID, modeId, cmdId);
        }
        else if (cmdType == DevMsgType::ProcessControlCmd)
        {
            return GlobalData::getCmdName(deviceID, modeId, cmdId);
        }
        else if (cmdType == DevMsgType::GroupParamSetCmd)
        {
            return QString("参数宏下发");
        }
        return QString();
    }

    bool operator==(const PackCommand& other) const
    {
        return this->cmdType == other.cmdType && this->deviceID == other.deviceID && this->modeId == other.modeId && this->cmdId == other.cmdId &&
               this->data == other.data;
    }
};

class MacroCommon
{
public:
    // 从基带配置获取主用、备用测控基带设备ID
    static bool getMasterCKJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, MasterSlave masterSlave = MasterSlave::Master);
    static bool getSlaveCKJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, MasterSlave masterSlave = MasterSlave::Master);
    // 从基带配置获取主用、备用高速基带设备ID

    static bool getMasterGSJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, BBEType gsjdType, int masterNo = 1,
                                      MasterSlave masterSlave = MasterSlave::Master);
    static bool getSlaveGSJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, BBEType gsjdType, int slaveNo = 1,
                                     MasterSlave masterSlave = MasterSlave::Master);

    // 从基带配置获取主用、备用低速基带设备ID
    static bool getMasterDSJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, MasterSlave masterSlave = MasterSlave::Master);
    static bool getSlaveDSJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, MasterSlave masterSlave = MasterSlave::Master);

    // 从配置宏中获取测控前端设备ID
    // 20211028 此方法未使用 且 更改测控前端获取 先注释掉
    // static bool getCKQDDeviceID(SystemWorkMode workMode, DeviceID& deviceID);
    // static bool getCKQDDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID);
    // 从配置宏中获取跟踪前端设备ID
    static bool getGZQDDeviceID(SystemWorkMode workMode, DeviceID& deviceID);
    static bool getGZQDDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID);
    // 从配置宏中获取S高功放设备ID
    static bool getSGGFDeviceID(SystemWorkMode workMode, DeviceID& deviceID);
    static bool getSGGFDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID);
    // 从配置宏中获取Ka高功放设备ID
    static bool getKaGGFDeviceID(SystemWorkMode workMode, DeviceID& deviceID);
    static bool getKaGGFDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID);
    // 从配置宏中获取Ka数传功放设备ID
    static bool getKaDtGGFDeviceID(SystemWorkMode workMode, DeviceID& deviceID);
    static bool getKaDtGGFDeviceID(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID);
    // 从配置宏中获取X1.2GHz下变频器设备ID
    static bool getXXBPQ(SystemWorkMode workMode, DeviceID& deviceID);
    static bool getXXBPQ(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID);
    // 从配置宏中获取Ka1.2GHz下变频器设备ID
    static bool getKaXBPQ(SystemWorkMode workMode, DeviceID& deviceID);
    static bool getKaXBPQ(const QSet<SystemWorkMode>& configMacroWorkMode, SystemWorkMode workMode, DeviceID& deviceID);

    // 从参数宏中获取指定模式下指定点频的上行频率
    static bool getUpFrequency(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, QVariant& value);
    static bool getUpFrequency(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& value);
    // 从参数宏中获取指定模式下指定点频的下行频率
    static bool getDownFrequency(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, QVariant& value);
    static bool getDownFrequency(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& value);
    static bool getDownFrequency(const ParamMacroData& paramMacroData, SystemWorkMode workMode, int pointFreqNo, QVariant& value);
    /* 同时获取上行和下行频率 */
    static bool getFrequency(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, QVariant& up, QVariant& down);
    static bool getFrequency(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& up, QVariant& down);
    // 从参数宏中获取指定模式下指定点频的输出电平
    static bool getOutputLevel(const QString& taskCode, SystemWorkMode workMode, QVariant& value);
    static bool getOutputLevel(const ParamMacroModeData& paramMacroModeData, QVariant& value);
    // 从参数宏中获取指定模式下指定点频的发射极化方式(旋向) 现在都是1左旋 2右旋
    static bool getSendPolarization(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, QVariant& value);
    static bool getSendPolarization(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& value);
    static bool getSendPolarization(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, SystemOrientation& value);
    // 从参数宏中获取指定模式下指定点频的接收极化方式(旋向) 现在都是1左旋 2右旋
    static bool getRecvPolarization(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, QVariant& value);
    static bool getRecvPolarization(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& value);
    static bool getRecvPolarization(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, SystemOrientation& value);
    // 从参数宏中获取指定模式下指定点频的跟踪极化方式(旋向) 现在都是1左旋 2右旋
    static bool getTracePolarization(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, QVariant& value);
    static bool getTracePolarization(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& value);
    static bool getTracePolarization(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, SystemOrientation& value);

    // 获取测角采样率
    static bool getAngularSamplRate(const QString& taskCode, SystemWorkMode workMode, int pointFreqNo, QVariant& value);
    static bool getAngularSamplRate(const ParamMacroModeData& paramMacroModeData, int pointFreqNo, QVariant& value);
};

#endif  // BUSINESSMACROCOMMON_H
