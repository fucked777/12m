#ifndef PROTOCOLXMLYPEDEFINE_H
#define PROTOCOLXMLYPEDEFINE_H

#include <QMap>
#include <QVariant>

#include "GlobalData.h"

const int gNoModeType = 0xFFFF;

enum AttributeType
{
    AttributeType_Char = 1,
    AttributeType_Int8,
    AttributeType_UInt8,
    AttributeType_Short,
    AttributeType_UShort,
    AttributeType_Int,
    AttributeType_UInt,
    AttributeType_Float,
    AttributeType_Double,
    AttributeType_Long,
    AttributeType_ULong,
    AttributeType_Bit,

    AttributeType_Invalid
};

enum DisplayFormat
{
    DisplayFormat_Enum,
    DisplayFormat_Int8,
    DisplayFormat_UInt8,
    DisplayFormat_Int16,
    DisplayFormat_UInt16,
    DisplayFormat_Int32,
    DisplayFormat_UInt32,
    DisplayFormat_Float,
    DisplayFormat_Double,
    DisplayFormat_Int64,
    DisplayFormat_UInt64,

    DisplayFormat_String,

    DisplayFormat_LED,        // 状态灯
    DisplayFormat_Hex,        // 十六进制
    DisplayFormat_HexString,  // 十六进制 UInt[6]
    DisplayFormat_Bit,
    DisplayFormat_IP,     // IP地址 传输以十六进制发送
    DisplayFormat_StrIP,  // IP地址 传输以字符串发送：192.20.1.20

    DisplayFormat_AccumDate,    // 积日
    DisplayFormat_BCDDate,      // BCD日期
    DisplayFormat_BCDTime,      // BCD时间
    DisplayFormat_StrDate,      // 字符串日期
    DisplayFormat_StrTime,      // 字符串时间
    DisplayFormat_StrDateTime,  // 字符串日期时间

    DisplayFormat_WdBm,          // 功率(W)和分贝毫瓦(dBm) 转换
    DisplayFormat_TracksNumber,  // 轨道根数下发命令 特殊处理需要用公式

    DisplayFormat_Invalid
};

struct ControlAlignment
{
    QString currentProjectID;          //配置里的当前项目标识
    QString nameControlAlignmentType;  //名字控件的对齐值  AlignLeft  0x01 AlignRight 0x02   暂时只用到这两种
};

// 多目标数据
struct UnitMultiTarget
{
    int id;                    // 多目标的ID
    QString recycleNum{ -1 };  // 多目标的循环次数，如果能够转为int，表示具体循环的次数，如果为字符串，表示循环次数为该单元的另一个参数的值
    QString desc;
    bool isShow{ true };  // 此recycle里面的数据是否显示
};

struct DevEnumEntry
{
    QString sValue;  // 原始值
    QString uValue;  // 使用值
    QString desc;    // 描述值
};

// 枚举的通用格式
struct DevEnum
{
    QString name;                       // 枚举ID
    QString desc;                       // 枚举描述
    QList<DevEnumEntry> emumEntryList;  // 具体枚举值列表
};

struct UnitLayout
{
    QString id;
    QString desc;
    int row = 0;
    int col = 0;
    int rowspan = 0;
    int colspan = 0;
};

// 参数设置基础属性
struct ParameterAttribute
{
    int pid{ -1 };                                                        // 序号
    QString id;                                                           // 参数ID
    QString desc;                                                         // 中文描述
    AttributeType attributeType{ AttributeType::AttributeType_Invalid };  // 参数类型
    DisplayFormat displayFormat{ DisplayFormat::DisplayFormat_Invalid };  // 显示类型
    AttributeType transferType{ AttributeType::AttributeType_Invalid };   // 传输类型，主要用于bit bit:uint|1 表示的传输类型为uint
    int length{ 0 };                                                      // 长度
    QString minValue;                                                     // 最小值
    QString maxValue;                                                     // 最大值
    QString step;                                                         // 步长
    QString unit;                                                         // 单位
    QString enumType;                                                     // 关联enum.xml
    QVariant initValue;                                                   // 初始化默认值
    bool isShow{ true };                                                  // 是否显示
    bool isEnable{ true };                                                // 是否可被编辑
    QString groupId;                                                      // 分组ID
    int targetCount{ 1 };
    QVariant fixValue;  //这个是固定值，如果有参数配置了这个属性，那么在下发参数的时候固定使用这个值
    QString unitId;     // 扩展用于参数宏点频参数使用
    QString formula;    // 公式 轨道根数下发命令需要
};

// 查询结果基础属性
// 对于UI来说就是需要显示属性
struct CmdAttribute
{
    int cmdNumb;                                          // 命令序号
    QString desc;                                         // 描述
    QString extenType;                                    // 分机类型
    QList<ParameterAttribute> requestList;                // 设置参数
    QList<ParameterAttribute> resultList;                 // 状态参数
    QMap<int, UnitMultiTarget> targetData;                // 多目标的数据类型
    QMap<int, QList<ParameterAttribute>> requestRecycle;  // QMap<多目标ID，QList<设置参数>>
    QMap<int, QList<ParameterAttribute>> resultRecycle;   // QMap<多目标ID，QList<状态参数>>

    //处理布局和分组
    QMap<QString, QList<ParameterAttribute>> statusGroupMap;                   //普通状态分组
    QMap<int, QMap<QString, QList<ParameterAttribute>>> multiStatusGroupMap;   //多目标状态分组
    QMap<QString, QList<ParameterAttribute>> settingGroupMap;                  //普通设置参数分组
    QMap<int, QMap<QString, QList<ParameterAttribute>>> multiSettingGroupMap;  //多目标设置参数分组
    QMap<QString, UnitLayout> unitLayoutMap;                                   // 布局信息

    bool isShow{ true };  // 此命令是否显示

    inline bool isValid()
    {
        if (cmdNumb < 0)
        {
            return false;
        }

        if (extenType.isEmpty())
        {
            return false;
        }
        return true;
    }

    // 通过参数id获取引用该参数上报值的循环目标参数
    inline void getResultRecycleTargetParams(const QString& paramId, int& targetNo, QList<ParameterAttribute>& targetParams) const
    {
        for (auto tarNo : targetData.keys())
        {
            const auto& cmdMultiTarget = targetData[tarNo];
            if (cmdMultiTarget.recycleNum == paramId)
            {
                targetNo = tarNo;
                targetParams = resultRecycle.value(tarNo);
                return;
            }
        }
    }
    // 通过参数id获取引用该参数上报值的循环目标参数
    inline void getRequestRecycleTargetParams(const QString& paramId, int& targetNo, QList<ParameterAttribute>& targetParams) const
    {
        for (auto tarNo : targetData.keys())
        {
            const auto& cmdMultiTarget = targetData[tarNo];
            if (cmdMultiTarget.recycleNum == paramId)
            {
                targetNo = tarNo;
                targetParams = requestRecycle.value(tarNo);
                return;
            }
        }
    }
};

// 控制结果响应
struct ControlResponse
{
    AttributeType dataType{ AttributeType::AttributeType_Invalid };
    QList<QString> enumList;
};

// 以下为四个层级的类型
// 单元级
struct Unit
{
    int unitCode{ -1 };           // 单元编码
    QString unitType;             // 单元类型
    QString desc;                 // 描述
    int modeId{ 0xFFFF };         // 模式ID
    QString kpValid{ "" };        //用于扩频4目标的有效标识
    bool isAutoMap{ false };      //  此单元是否是自动映射单元
    bool isShow{ true };          // 此单元是否显示
    bool isModifySetBtn{ true };  // 此单元是否需要修改设置按钮     有些单元只有状态参数,故不需要这些按钮

    QMap<QString, DevEnum> enumMap;         // 该单元命令对应的枚举列表 key为具体的字段
    QMap<int, UnitMultiTarget> targetData;  // 多目标的数据类型

    QList<ParameterAttribute> queryResultList;             // 该单元对应的状态上报命令列表（所有状态和设置参数的集合）
    QMap<int, QList<ParameterAttribute>> queryRecycleMap;  // 该单元对应的状态上报命令列表（限所有多目标）

    QList<ParameterAttribute> parameterSetList;     // 该单元对应的参数设置命令列表
    QList<ParameterAttribute> parameterStatusList;  // 该单元对应的状态参数设置命令列表

    QMap<int, QList<ParameterAttribute>> multiSettingMap;  // 该单元对应的多目标设置参数map,key是多目标ID
    QMap<int, QList<ParameterAttribute>> multiStatusMap;   // 该单元对应的多目标状态参数map,key是多目标ID

    //处理布局和分组
    QMap<QString, QList<ParameterAttribute>> statusGroupMap;                   //普通状态分组
    QMap<int, QMap<QString, QList<ParameterAttribute>>> multiStatusGroupMap;   //多目标状态分组
    QMap<QString, QList<ParameterAttribute>> settingGroupMap;                  //普通设置参数分组
    QMap<int, QMap<QString, QList<ParameterAttribute>>> multiSettingGroupMap;  //多目标设置参数分组
    QMap<QString, UnitLayout> unitLayoutMap;                                   //布局信息

    // 判断该单元是否是有效的单元
    inline bool isValid()
    {
        if (unitType.isEmpty())
        {
            return false;
        }

        if (modeId != gNoModeType)
        {
            if (queryRecycleMap.size() < 0)
            {
                if (queryResultList.size() < 0)
                {
                    return false;
                }
            }
        }
        else
        {
            if (queryResultList.size() < 0)
            {
                return false;
            }
        }

        return true;
    }

    // 判断公共参数是否含有某参数id
    inline bool paramSetListContains(const QString& paramId) const
    {
        for (const auto& attr : parameterSetList)
        {
            if (attr.id == paramId)
            {
                return true;
            }
        }
        return false;
    }

    // 判断对目标参数是否包含

    // 获取参数的分组名
    inline QString getParamGroupName(const QString& paramId) const
    {
        // 目前只写了获取单目标设置参数的分组名
        for (auto groupName : settingGroupMap.keys())
        {
            for (auto param : settingGroupMap.value(groupName))
            {
                if (param.id == paramId)
                {
                    return unitLayoutMap[groupName].desc;
                }
            }
        }
        return QString();
    }
    inline QString getMultiParamGroupName(int targetNo, bool fixed) const
    {
        if (fixed)
        {
            return QString("目标%1").arg(targetNo);
        }
        return targetData.value(targetNo).desc;
    }

    // 通过参数id获取引用该参数上报值的循环目标参数
    inline void getRecycleTargetParams(const QString& paramId, int& targetNo, QList<ParameterAttribute>& targetParams) const
    {
        for (auto tarNo : targetData.keys())
        {
            const auto& unitMultiTarget = targetData[tarNo];
            if (unitMultiTarget.recycleNum == paramId)
            {
                targetNo = tarNo;
                targetParams = queryRecycleMap.value(tarNo);
                return;
            }
        }
    }

    // 判断该单元的长度是否是动态长度
    inline bool isDynamicLength() const
    {
        if (!queryRecycleMap.isEmpty())
        {
            for (auto targetNo : queryRecycleMap.keys())
            {
                const auto& unitMultiTarget = targetData[targetNo];

                bool isOk = false;
                unitMultiTarget.recycleNum.toInt(&isOk);  // 不能转为int，表示该目标的循环次数是根据该单元的另一个参数值动态改变的
                if (!isOk)
                {
                    return true;
                }
            }
        }
        return false;
    }

    inline int unitLength() const
    {
        // 动态单元长度需要根据上报值来确定
        if (isDynamicLength())
            return -1;

        int length = 0;
        int bitLength = 0;
        // 获取单目标字节长度
        for (auto i = 0; i < queryResultList.size(); i++)
        {
            if (queryResultList[i].attributeType == AttributeType_Bit)
            {
                bitLength += queryResultList[i].length;
                if (i + 1 < queryResultList.size() && queryResultList[i + 1].attributeType != AttributeType_Bit)
                {
                    length += bitLength / 8;
                }
            }
            else
            {
                bitLength = 0;
                length += queryResultList[i].length;
            }
        }

        // 获取多目标字节长度
        auto allRecycle = 0;
        if (queryRecycleMap.size() > 0)
        {
            for (auto targetNo : queryRecycleMap.keys())
            {
                auto recycle = 0;
                auto params = queryRecycleMap[targetNo];
                for (auto param : params)
                {
                    recycle += param.length;
                }

                auto recycleNum = targetData[targetNo].recycleNum.toInt();
                if (recycleNum > 0)
                {
                    recycle *= recycleNum;
                }

                allRecycle += recycle;
            }
        }

        return allRecycle + length;
    }
};

// 模式级
struct ModeCtrl
{
    int ID{ -1 };
    QString desc;
    QMap<int, CmdAttribute> cmdMap;     // 过程控制命令Map key为命令序号
    QMap<QString, DevEnum> cmdEnumMap;  // 过程控制对应的枚举Map key为字段名
    QMap<int, Unit> unitMap;            // 单元Map key为单元编码
};

// 分机级
struct Extension
{
    int ID{ -1 };                  // 分机ID
    QString extenType;             // 分机类型
    QString desc;                  // 描述
    bool haveMode{ false };        // 模式标志 0表示无 1表示有
    QString relationDevice{ "" };  //关联设备
    bool isShow{ true };           // 此分机设备是否显示

    int modeAddr{ -1 };           // 模式在数据报文体中的位置 //add
    int modeLength{ -1 };         // 模式值在数据报文体中的长度 //add
    int currentModeID{ 0xFFFF };  // 当前分机模式ID //add
    int typeID{ 0 };              // 当前分机类型ID

    QMap<int, ModeCtrl> modeCtrlMap;  // 模式单元
};

// 设备级
struct Device
{
    int ID{ -1 };                       // 设备ID
    QString name;                       // 设备名字
    QString desc;                       // 描述
    QMap<int, Extension> extensionMap;  // 分机Map，key为分机ID
};

// 设备ID
struct DeviceID
{
    int sysID = -1;
    int devID = -1;
    int extenID = -1;

    DeviceID() {}

    DeviceID(int systemId, int deviceId, int extenId)
    {
        sysID = systemId;
        devID = deviceId;
        extenID = extenId;
    }

    DeviceID(int deviceId)
    {
        DeviceID tempDevID;
        tempDevID << deviceId;

        sysID = tempDevID.sysID;
        devID = tempDevID.devID;
        extenID = tempDevID.extenID;
    }

    bool operator==(const DeviceID& deviceID) const { return sysID == deviceID.sysID && devID == deviceID.devID && extenID == deviceID.extenID; }
    bool operator!=(const DeviceID& deviceID) const { return !(*this == deviceID); }

    bool operator<(const DeviceID& deviceID) const
    {
        if (deviceID.sysID == sysID)
        {
            if (deviceID.devID == devID)
            {
                return extenID < deviceID.extenID;
            }

            return devID < deviceID.devID;
        }

        return sysID < deviceID.sysID;
    }
    bool operator<<(const int& deviceID)
    {
        sysID = (deviceID & 0xF000) >> 12;
        devID = (deviceID & 0x0F00) >> 8;
        extenID = deviceID & 0xFF;
        return true;
    }

    bool operator>>(int& deviceId) const
    {
        deviceId = 0;
        deviceId = deviceId | ((sysID & 0xF) << 12);
        deviceId = deviceId | ((devID & 0xF) << 8);
        deviceId = deviceId | ((extenID & 0xFF));
        return true;
    }

    inline int toInt() const
    {
        int deviceId;
        *this >> deviceId;

        return deviceId;
    }
    inline quint32 toUInt() const
    {
        int deviceId;
        *this >> deviceId;

        return static_cast<quint32>(deviceId);
    }

    uint toDeviceInfo()
    {
        uint deviceID = 0;
        deviceID = deviceID | quint32((sysID & 0xF) << 12);
        deviceID = deviceID | quint32((devID & 0xF) << 8);
        deviceID = deviceID | quint32((extenID & 0xFF) << 0);
        return deviceID;
    }

    static DeviceID fromHex(const QString& hexDeviceID) { return DeviceID(hexDeviceID.toInt(nullptr, 16)); }

    inline QString toHex() const { return QString::number(toInt(), 16); }

    inline bool isValid() const { return GlobalData::isExistsExtension(*this); }

    void resz()
    {
        sysID = -1;
        devID = -1;
        extenID = -1;
    }
};

// 系统级
struct System
{
    int ID{ -1 };  // 系统ID
    QString name;  // 系统名称
    QString desc;  // 描述

    QMap<int, Device> deviceMap;  // 设备Map key为设备ID
};

Q_DECLARE_METATYPE(ParameterAttribute)
Q_DECLARE_METATYPE(UnitLayout)
Q_DECLARE_METATYPE(DevEnum)
Q_DECLARE_METATYPE(UnitMultiTarget)
Q_DECLARE_METATYPE(Unit)
Q_DECLARE_METATYPE(ModeCtrl)
Q_DECLARE_METATYPE(Extension)
Q_DECLARE_METATYPE(Device)
Q_DECLARE_METATYPE(System)
Q_DECLARE_METATYPE(ControlAlignment)

///////////////////////////////////////////////
/// 以下类型为读取协议相关的配置文件融合使用 ///////

using UnitXmlDataMap = QMap<int, QMap<QString, Unit>>;  //第一个key为模式类型
                                                        //第二个key为单元类型

using CmdAttrMap = QMap<QString, QMap<int, QMap<int, CmdAttribute>>>;  //第一个key为分机类型
                                                                       //第二个key为模式类型
                                                                       //第三个key为命令序号

using CmdEnumMap = QMap<QString, QMap<int, QMap<QString, DevEnum>>>;  //第一个key为分机类型
                                                                      //第二个key为模式类型
                                                                      //第三个key具体字段名称
struct CmdXMLData
{
    CmdAttrMap cmdAttrMap;
    CmdEnumMap cmdEnumMap;
};

#endif  // PROTOCOLXMLYPEDEFINE_H
