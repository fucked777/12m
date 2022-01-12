#ifndef CONFIGMACROMESSAGEDEFINE_H
#define CONFIGMACROMESSAGEDEFINE_H

#include <QMap>
#include <QSet>
#include <QVariant>

#include "DevProtocol.h"
#include "ExtendedConfig.h"
#include "ProtocolXmlTypeDefine.h"
#include "SystemWorkMode.h"

// 配置宏命令
struct ConfigMacroCmd
{
    ConfigMacroCmd() {}

    ConfigMacroCmd(const QString& id, const QVariant& value, const QString& desc = "")
    {
        this->id = id;
        this->value = value;
        this->desc = desc;
    }

    ConfigMacroCmd(const QString& id, const QVariant& value, const DeviceID& deviceID, const QString& desc = "")
    {
        this->id = id;
        this->value = value;
        this->deviceID = deviceID;
        this->desc = desc;
    }

    QString id;          // 参数id
    QString desc;        // 参数描述
    QVariant value;      // 参数值
    int startBit{ -1 };  // 设备组合号里面的开始位
    int endBit{ -1 };    // 设备组合号里面的结束位
    int offset{ 0 };     // 设备组合号里面的偏移量
    int linkType{ 0 };   // 设备组合号类设备上下行区分

    DeviceID deviceID{ 0 };  // 该选项对应的设备ID，比如测控前端、跟踪前端、下变频器 （该属性可以没有）
};

struct ConfigMacroModeItemList
{
    SystemWorkMode workMode{ SystemWorkMode::NotDefine };
    QList<ConfigMacroCmd> configMacroCmdList;
};
struct ConfigMacroModeItemMap
{
    SystemWorkMode workMode{ SystemWorkMode::NotDefine };
    QMap<QString, ConfigMacroCmd> configMacroCmdMap;
};

// 配置宏数据
struct ConfigMacroData
{
    QString configMacroID;                        // 用于标识每个配置宏唯一ID
    QString configMacroName;                      // 配置宏名称
    QString workModeDesc;                         // 工作模式   STTC + SK2 + KaK2 使用+分隔
    QString stationID;                            // 站ID     //工作模式和站ID 2个参数联合作为主键
    MasterSlave isMaster{ MasterSlave::Master };  // 主备模式
    QString desc;                                 // 配置宏描述
    /*
     * 20211029 wp 更改为Map
     * 比如获取一个值 假设是跟踪基带吧  GZJD
     * 然后在不同模式比较处理之后再设置
     * 因为是List 则会出现多个跟踪基带然后在实际读取的时候就会出现读取最开始的内个值 不是想要的
     *
     * 后面测控前端 下变频器都要更改为主备的形式所以先改这个
     *
     * 这里改名字的原因是直接让之前的都报错 那样就知道使用范围了可以一次性改完
     */
    QMap<SystemWorkMode, ConfigMacroModeItemMap> configMacroCmdModeMap;  // 该配置宏需要执行的命令 QMap<模式id, QList<命令信息>>

    inline bool isValid() const { return !configMacroCmdModeMap.isEmpty(); }
    inline bool existWorkMode(SystemWorkMode workMode) const { return configMacroCmdModeMap.contains(workMode); }
    // 不能凭空增加工作模式
    // 设置和获取值
    inline void setValue(SystemWorkMode workMode, const QString& key, const QVariant& value, const QString& desc = "")
    {
        if (configMacroCmdModeMap.contains(workMode))
        {
            auto& configMacroCmdData = configMacroCmdModeMap[workMode];
            ConfigMacroCmd configMacroCmd(key, value, desc);
            configMacroCmdData.configMacroCmdMap.insert(key, configMacroCmd);
            configMacroCmdData.workMode = workMode;
        }
    }
    inline bool getValue(SystemWorkMode workMode, const QString& key, QVariant& value) const
    {
        auto configMacroCmdModeMapIter = configMacroCmdModeMap.find(workMode);
        if (configMacroCmdModeMapIter == configMacroCmdModeMap.end())
        {
            return false;
        }
        auto& valueMap = configMacroCmdModeMapIter.value();
        for (auto& configMacroCmd : valueMap.configMacroCmdMap)
        {
            if (configMacroCmd.id.endsWith(key))
            {
                value = configMacroCmd.value;
                return true;
            }
        }
        value = QVariant();
        return false;
    }
    inline void removeModeValue(SystemWorkMode workMode, const QString& key)
    {
        auto configMacroCmdModeMapIter = configMacroCmdModeMap.find(workMode);
        if (configMacroCmdModeMapIter == configMacroCmdModeMap.end())
        {
            return;
        }
        auto& valueMap = configMacroCmdModeMapIter.value();
        for (auto iter = valueMap.configMacroCmdMap.begin(); iter != valueMap.configMacroCmdMap.end(); ++iter)
        {
            if (iter->id.endsWith(key))
            {
                valueMap.configMacroCmdMap.erase(iter);
                return;
            }
        }
    }
    inline void removeModeValueExclude(const QSet<SystemWorkMode>& exclude, const QString& key)
    {
        for (auto iter = configMacroCmdModeMap.begin(); iter != configMacroCmdModeMap.end(); ++iter)
        {
            auto mode = iter.key();
            if (exclude.contains(mode))
            {
                continue;
            }
            auto& valueMap = iter.value();
            for (auto valueIter = valueMap.configMacroCmdMap.begin(); valueIter != valueMap.configMacroCmdMap.end(); ++valueIter)
            {
                if (valueIter->id.endsWith(key))
                {
                    valueMap.configMacroCmdMap.erase(valueIter);
                    break;
                }
            }
        }
    }

    // 获取旋向
    inline bool getSystemOrientation(SystemWorkMode workMode, const QString& key, SystemOrientation& orientation)
    {
        QVariant value;
        if (!getValue(workMode, key, value))
        {
            return false;
        }
        orientation = (SystemOrientation)(value.toInt());
        return true;
    }
    // 设置工作模式
    inline void setLinkWorkMode(SystemWorkMode workMode) { setValue(workMode, "LinkWorkMode", (int)workMode, "链路工作模式"); }

    // 设置点频数
    inline void setPointFreqNum(SystemWorkMode workMode, int pointFreqNum) { setValue(workMode, "PointFreqNum", pointFreqNum, "点频数量"); }

    // 设置和获取主用类型 区分主用数传还是扩频 一体化XXX+扩频模式是使用
    inline void setMasterType(SystemWorkMode workMode, MasterType masterType) { setValue(workMode, "MasterType", (int)masterType, "主用类型"); }
    inline bool getMasterType(SystemWorkMode workMode, MasterType& masterType)
    {
        QVariant var;
        if (getValue(workMode, "MasterType", var))
        {
            masterType = (MasterType)(var.toInt());
            return true;
        }
        return false;
    }

    // 设置和获取主跟目标发射极化旋向
    inline void setMasterLaunchPolar(SystemWorkMode workMode, SystemOrientation orientation)
    {
        setValue(workMode, "MasterLaunchPolar", (int)orientation, "主跟目标发射极化");
    }
    inline bool getMasterLaunchPolar(SystemWorkMode workMode, SystemOrientation& orientation)
    {
        return getSystemOrientation(workMode, "MasterLaunchPolar", orientation);
    }

    // 设置和获取非主跟目标发射极化旋向
    inline void setSlaveLaunchPolar(SystemWorkMode workMode, SystemOrientation orientation)
    {
        setValue(workMode, "SlaveLaunchPolar", (int)orientation, "非主跟目标发射极化");
    }
    inline bool getSlaveLaunchPolar(SystemWorkMode workMode, SystemOrientation& orientation)
    {
        return getSystemOrientation(workMode, "SlaveLaunchPolar", orientation);
    }

    // 设置和获取主跟目标接收极化旋向
    inline void setMasterReceivePolar(SystemWorkMode workMode, SystemOrientation orientation)
    {
        setValue(workMode, "MasterReceivePolar", (int)orientation, "主跟目标接收极化");
    }
    inline bool getMasterReceivePolar(SystemWorkMode workMode, SystemOrientation& orientation)
    {
        return getSystemOrientation(workMode, "MasterReceivePolar", orientation);
    }

    // 设置和获取非主跟目标接收极化旋向
    inline void setSlaveReceivePolar(SystemWorkMode workMode, SystemOrientation orientation)
    {
        setValue(workMode, "SlaveReceivePolar", (int)orientation, "非主跟目标接收极化");
    }
    inline bool getSlaveReceivePolar(SystemWorkMode workMode, SystemOrientation& orientation)
    {
        return getSystemOrientation(workMode, "SlaveReceivePolar", orientation);
    }

    // 设置和获取主跟目标跟踪极化旋向
    inline void setMasterTrackPolar(SystemWorkMode workMode, SystemOrientation orientation)
    {
        setValue(workMode, "MasterTrackPolar", (int)orientation, "主跟目标跟踪极化");
    }
    inline bool getMasterTrackPolar(SystemWorkMode workMode, SystemOrientation& orientation)
    {
        return getSystemOrientation(workMode, "MasterTrackPolar", orientation);
    }

    // 设置和获取非主跟目标跟踪极化旋向
    inline void setSlaveTrackPolar(SystemWorkMode workMode, SystemOrientation orientation)
    {
        setValue(workMode, "SlaveTrackPolar", (int)orientation, "非主跟目标跟踪极化");
    }
    inline bool getSlaveTrackPolar(SystemWorkMode workMode, SystemOrientation& orientation)
    {
        return getSystemOrientation(workMode, "SlaveTrackPolar", orientation);
    }

    // 设置和获取发射极化旋向  pointFreqNo代表设置第几个点频的发射极化  只有一个点频时默认为1
    inline void setLaunchPolar(SystemWorkMode workMode, SystemOrientation orientation, int pointFreqNo = 1)
    {
        setValue(workMode, QString("LaunchPolar%1").arg(pointFreqNo), (int)orientation, "发射极化");
    }
    inline bool getLaunchPolar(SystemWorkMode workMode, SystemOrientation& orientation, int pointFreqNo = 1)
    {
        return getSystemOrientation(workMode, QString("LaunchPolar%1").arg(pointFreqNo), orientation);
    }

    // 设置和获取接收极化旋向  pointFreqNo代表设置第几个点频的接收极化  只有一个点频时默认为1
    inline void setReceivePolar(SystemWorkMode workMode, SystemOrientation orientation, int pointFreqNo = 1)
    {
        setValue(workMode, QString("ReceivePolar%1").arg(pointFreqNo), (int)orientation, "接收极化");
    }
    inline bool getReceivePolar(SystemWorkMode workMode, SystemOrientation& orientation, int pointFreqNo = 1)
    {
        return getSystemOrientation(workMode, QString("ReceivePolar%1").arg(pointFreqNo), orientation);
    }

    // 设置和获取跟踪极化旋向 pointFreqNo代表设置第几个点频的跟踪极化  只有一个点频时默认为1
    inline void setTrackPolar(SystemWorkMode workMode, SystemOrientation orientation, int pointFreqNo = 1)
    {
        setValue(workMode, QString("TrackPolar%1").arg(pointFreqNo), (int)orientation, "跟踪极化");
    }
    inline bool getTrackPolar(SystemWorkMode workMode, SystemOrientation& orientation, int pointFreqNo = 1)
    {
        return getSystemOrientation(workMode, QString("TrackPolar%1").arg(pointFreqNo), orientation);
    }

    // 双模式判定 对测控的链路有影响
    inline void setDoubleMode(SystemWorkMode workMode, bool isDouble) { setValue(workMode, "DoubleMode", static_cast<int>(isDouble), "双模式"); }
    inline bool getDoubleMode(SystemWorkMode workMode)
    {
        QVariant isDouble;
        if (!getValue(workMode, "DoubleMode", isDouble))
        {
            return false;
        }
        return isDouble.toBool();
    }

    inline bool getConfigMacroCmd(SystemWorkMode workMode, const QString& key, ConfigMacroCmd& configCmd) const
    {
        auto configMacroCmdModeMapIter = configMacroCmdModeMap.find(workMode);
        if (configMacroCmdModeMapIter == configMacroCmdModeMap.end())
        {
            return false;
        }
        auto& valueMap = configMacroCmdModeMapIter.value();
        for (auto& configMacroCmd : valueMap.configMacroCmdMap)
        {
            if (configMacroCmd.id.endsWith(key))
            {
                configCmd = configMacroCmd;
                return true;
            }
        }
        return false;
    }

    // 设置和获取测控基带主用数量
    inline void setCKJDMasterNumber(SystemWorkMode workMode, int number) { setValue(workMode, "CKJDMasterNumber", number, "主用测控基带数量"); }
    inline bool getCKJDMasterNumber(SystemWorkMode workMode, int& number) const
    {
        QVariant var;
        if (getValue(workMode, "CKJDMasterNumber", var))
        {
            number = var.toInt();
            return true;
        }
        return false;
    }

    // 设置和获取测控目标数量
    inline void setCKTargetNumber(SystemWorkMode workMode, int number) { setValue(workMode, "CKTargetNumber", number, "测控目标数量"); }
    inline bool getCKTargetNumber(SystemWorkMode workMode, int& number) const
    {
        QVariant var;
        if (getValue(workMode, "CKTargetNumber", var))
        {
            number = var.toInt();
            return true;
        }
        return false;
    }

    // 设置和获取低速基带主用数量
    inline void setDSJDMasterNumber(SystemWorkMode workMode, int number) { setValue(workMode, "DSJDMasterNumber", number, "主用低速基带数量"); }
    inline bool getDSJDMasterNumber(SystemWorkMode workMode, int& number) const
    {
        QVariant var;
        if (getValue(workMode, "DSJDMasterNumber", var))
        {
            number = var.toInt();
            return true;
        }
        return false;
    }

    // 设置和获取高速基带主用数量
    inline void setGSJDMasterNumber(SystemWorkMode workMode, int number) { setValue(workMode, "GSJDMasterNumber", number, "主用高速基带数量"); }
    inline bool getGSJDMasterNumber(SystemWorkMode workMode, int& number) const
    {
        QVariant var;
        if (getValue(workMode, "GSJDMasterNumber", var))
        {
            number = var.toInt();
            return true;
        }
        return false;
    }

    // 设置和获取高速高频箱编号 // dcNo下变频器编号 1234 4426 Ka高速
    inline bool getKaGSXBBQNo(SystemWorkMode workMode, int& dcNo, int no = 1) const
    {
        QVariant var;
        if (getValue(workMode, QString("KAGS_DC%1").arg(no), var))
        {
            dcNo = var.toInt();
            return true;
        }
        return false;
    }

    // 4426 高速低速不能同时跟踪 加一个变量来防止打架 0未定义 1是使用高速 2使用低速
    inline void setDTGSOrDS(SystemWorkMode workMode, int number) { setValue(workMode, "DTGSOrDS", number, "数传使用高速跟踪还是低速跟踪"); }
    inline bool getDTGSOrDS(SystemWorkMode workMode, int& number) const
    {
        QVariant var;
        if (getValue(workMode, "DTGSOrDS", var))
        {
            number = var.toInt();
            return true;
        }
        return false;
    }

    // 获取当前的跟踪模式 获取到的一定是处理过后的可以直接用的
    inline void setTrackMode(SystemWorkMode workMode, SatelliteTrackingMode trackingMode)
    {
        setValue(workMode, "TrackMode", static_cast<int>(trackingMode), "当前的跟踪模式");
    }
    inline bool getTrackMode(SystemWorkMode workMode, SatelliteTrackingMode& trackingMode) const
    {
        trackingMode = SatelliteTrackingMode::TrackUnknown;
        QVariant var;
        if (getValue(workMode, "TrackMode", var))
        {
            trackingMode = SatelliteTrackingMode(var.toInt());
            return true;
        }
        return false;
    }
    inline bool getTrackMode(SatelliteTrackingMode& trackingMode) const
    {
        trackingMode = SatelliteTrackingMode::TrackUnknown;
        if (configMacroCmdModeMap.isEmpty())
        {
            return false;
        }

        QVariant var;
        if (getValue(configMacroCmdModeMap.firstKey(), "TrackMode", var))
        {
            trackingMode = SatelliteTrackingMode(var.toInt());
            return true;
        }
        return false;
    }

    // 设置设备ID
    inline void setDeviceID(SystemWorkMode workMode, const DeviceID& deviceID, const QString& deviceKey, int no = 0)
    {
        QString key;
        QString extenName;

        key = QString("%1%2").arg(deviceKey, (no == 0 ? "" : QString::number(no)));
        // key = QString("%1%2").arg(deviceKey).arg(no);
        extenName = GlobalData::getExtensionName(deviceID);

        if (configMacroCmdModeMap.contains(workMode))
        {
            auto& configMacroCmdData = configMacroCmdModeMap[workMode];

            auto value = deviceID.extenID;  // 以分机Id作为值
            ConfigMacroCmd configMacroCmd(key, value, deviceID, extenName);
            configMacroCmdData.configMacroCmdMap.insert(key, configMacroCmd);
            configMacroCmdData.workMode = workMode;
        }
    }

    // 获取设备ID
    inline bool getDeviceID(SystemWorkMode workMode, DeviceID& deviceID, const QString& deviceKey, int no = 0) const
    {
        auto key = QString("%1%2").arg(deviceKey, (no == 0 ? "" : QString::number(no)));
        auto configMacroCmdModeMapIter = configMacroCmdModeMap.find(workMode);
        if (configMacroCmdModeMapIter == configMacroCmdModeMap.end())
        {
            return false;
        }

        auto& valueMap = configMacroCmdModeMapIter.value();
        for (auto& configMacroCmd : valueMap.configMacroCmdMap)
        {
            if (configMacroCmd.id.endsWith(key))
            {
                deviceID = configMacroCmd.deviceID;
                return true;
            }
        }
        deviceID = DeviceID();
        return false;
    }

    // 设置所有数据
    inline void setAllData(SystemWorkMode workMode, const DeviceID& deviceID, const QVariant& value, const QString& deviceKey, int no = 0)
    {
        QString key;
        QString desc;

        key = QString("%1%2").arg(deviceKey, (no == 0 ? "" : QString::number(no)));
        // key = QString("%1%2").arg(deviceKey).arg(no);
        desc = GlobalData::getExtensionName(deviceID);

        if (configMacroCmdModeMap.contains(workMode))
        {
            auto& configMacroCmdData = configMacroCmdModeMap[workMode];
            ConfigMacroCmd configMacroCmd(key, value, deviceID, desc);
            configMacroCmdData.configMacroCmdMap.insert(key, configMacroCmd);
            configMacroCmdData.workMode = workMode;
        }
    }

    // 设置所有数据
    inline bool getAllData(SystemWorkMode workMode, DeviceID& deviceID, QVariant& value, const QString& deviceKey, int no = 0) const
    {
        auto key = QString("%1%2").arg(deviceKey, (no == 0 ? "" : QString::number(no)));
        auto configMacroCmdModeMapIter = configMacroCmdModeMap.find(workMode);
        if (configMacroCmdModeMapIter == configMacroCmdModeMap.end())
        {
            return false;
        }

        auto& valueMap = configMacroCmdModeMapIter.value();
        for (auto& configMacroCmd : valueMap.configMacroCmdMap)
        {
            if (configMacroCmd.id.endsWith(key))
            {
                deviceID = configMacroCmd.deviceID;
                value = configMacroCmd.value;
                return true;
            }
        }
        return false;
    }

    // 设置和获取主用或者备用第几台 测控基带设备ID
    inline bool getCKJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, int no = 1) const { return getDeviceID(workMode, deviceID, "CKJD", no); }
    inline void setCKJDDeviceID(SystemWorkMode workMode, const DeviceID& deviceID, int no = 1) { setDeviceID(workMode, deviceID, "CKJD", no); }

    // 设置和获取主用或者备用第几台 高速基带设备ID
    inline bool getGSJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, int no = 1) const { return getDeviceID(workMode, deviceID, "GSJD", no); }
    inline void setGSJDDeviceID(SystemWorkMode workMode, const DeviceID& deviceID, int no = 1) { setDeviceID(workMode, deviceID, "GSJD", no); }

    // 设置和获取主用或者备用第几台 高速基带设备ID
    inline bool getDSJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, int no = 1) const { return getDeviceID(workMode, deviceID, "DSJD", no); }
    inline void setDSJDDeviceID(SystemWorkMode workMode, const DeviceID& deviceID, int no = 1) { setDeviceID(workMode, deviceID, "DSJD", no); }

    // 设置和获取测控前端设备ID
    inline bool getCKQDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, int no = 1) const
    {
        auto curProject = ExtendedConfig::curProjectID();
        if (curProject != "4426")
        {
            no = 0;
        }
        return getDeviceID(workMode, deviceID, "CKQD", no);
    }
    inline void setCKQDDeviceID(SystemWorkMode workMode, const DeviceID& deviceID, int no = 1)
    {
        auto curProject = ExtendedConfig::curProjectID();
        if (curProject != "4426")
        {
            no = 0;
        }
        setDeviceID(workMode, deviceID, "CKQD", no);
    }

    // 获取该配置宏的指定模式的跟踪前端设备ID

    inline void removeRedundantGZQD(const QSet<SystemWorkMode>& exclude) { removeModeValueExclude(exclude, "GZQD1"); }
    inline bool getGZQDDeviceID(SystemWorkMode workMode, DeviceID& deviceID, int no = 1) const { return getDeviceID(workMode, deviceID, "GZQD", no); }
    inline void setGZQDDeviceID(SystemWorkMode workMode, const DeviceID& deviceID, int no = 1) { setDeviceID(workMode, deviceID, "GZQD", no); }
    inline bool getGZJDDeviceID(SystemWorkMode workMode, DeviceID& deviceID) const { return getDeviceID(workMode, deviceID, "GZJD"); }
    inline void setGZJDDeviceID(SystemWorkMode workMode, const DeviceID& deviceID) { setDeviceID(workMode, deviceID, "GZJD"); }
    /* 存储转发 */
    inline bool getCCZFDeviceID(SystemWorkMode workMode, DeviceID& deviceID, int no = 1) const { return getDeviceID(workMode, deviceID, "CCZF", no); }
    inline void setCCZFDeviceID(SystemWorkMode workMode, const DeviceID& deviceID, int no = 1) { setDeviceID(workMode, deviceID, "CCZF", no); }

    // 获取该配置宏的指定模式的S高功放设备ID
    inline bool getSGGFDeviceID(SystemWorkMode workMode, DeviceID& deviceID) const { return getDeviceID(workMode, deviceID, "SHPA"); }

    // 获取该配置宏的指定模式的Ka高功放设备ID
    inline bool getKaGGFDeviceID(SystemWorkMode workMode, DeviceID& deviceID) const { return getDeviceID(workMode, deviceID, "KAHPA"); }

    // 获取该配置宏的指定模式的Ka高功放设备ID
    inline bool getKaDtGGFDeviceID(SystemWorkMode workMode, DeviceID& deviceID) const { return getDeviceID(workMode, deviceID, "KADTHPA"); }

    // 获取该配置宏的指定模式的X1.2GHz下变频器设备ID  4424 X高速
    inline bool getXXBPQDeviceID(SystemWorkMode workMode, DeviceID& deviceID, int no = 1) const
    {
        return getDeviceID(workMode, deviceID, QString("XDC%1").arg(no));
    }
    // 获取该配置宏的指定模式的Ka下变频器设备ID      4424 Ka高速
    inline bool getKaXBPQDeviceID(SystemWorkMode workMode, DeviceID& deviceID, int no = 1) const
    {
        return getDeviceID(workMode, deviceID, QString("KADC%1").arg(no));
    }
    // 获取Ka高速下变频器设备ID  4426 Ka高速
    inline bool getKaGSXBBQDeviceID(SystemWorkMode workMode, DeviceID& deviceID, int no = 1) const
    {
        return getDeviceID(workMode, deviceID, "KAGS_DC", no);
    }
    // dcNo下变频器编号 1234 4426 Ka高速
    inline void setKaGSXBBQInfo(SystemWorkMode workMode, const DeviceID& deviceID, int dcNo, int no = 1)
    {
        setAllData(workMode, deviceID, dcNo, "KAGS_DC", no);
    }
    inline bool getKaGSXBBQInfo(SystemWorkMode workMode, DeviceID& deviceID, int& dcNo, int no = 1) const
    {
        QVariant var;
        if (!getAllData(workMode, deviceID, var, "KAGS_DC", no))
        {
            return false;
        }
        dcNo = var.toInt();
        return true;
    }
};

Q_DECLARE_METATYPE(ConfigMacroCmd);
Q_DECLARE_METATYPE(ConfigMacroModeItemList);
Q_DECLARE_METATYPE(ConfigMacroModeItemMap);
Q_DECLARE_METATYPE(ConfigMacroData);

// 命令组合
using ConfigMacroCmdMap = QMap<QString, ConfigMacroCmd>;
using ConfigMacroCmdList = QList<ConfigMacroCmd>;

// 命令模式组合
using ConfigMacroCmdModeList = QList<ConfigMacroModeItemList>;
using ConfigMacroCmdModeMap = QMap<SystemWorkMode, ConfigMacroModeItemMap>;

// 所有的组合
using ConfigMacroDataMap = QMap<QString, ConfigMacroData>;
using ConfigMacroDataList = QList<ConfigMacroData>;

#endif  // CONFIGMACROMESSAGEDEFINE_H
