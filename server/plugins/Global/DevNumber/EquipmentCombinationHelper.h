#ifndef EQUIPMENTCOMBINATIONHELPER_H
#define EQUIPMENTCOMBINATIONHELPER_H

//#include "ProtocolXmlTypeDefine.h"
#include "SystemWorkMode.h"
#include <QtCore>

struct DeviceID;
struct ConfigMacroData;
class EquipmentCombinationHelper
{
public:
    /*获取测控的设备组合号*/
    static bool getCKDevNumber(SystemWorkMode systemWorkMode, const ConfigMacroData& configMacroData, const DeviceID& ckDeviceID,
                               const DeviceID& gzDeviceID, quint64& devNum);
    static bool getCKDevNumber(SystemWorkMode systemWorkMode, const ConfigMacroData& configMacroData, quint64& devNum);

    /* 获取数传的设备组合号*/
    static bool getDTDevNumber(SystemWorkMode systemWorkMode, ConfigMacroData configMacroData, DeviceID gzDeviceID, quint64& devNum);
    static bool getDTDevNumber(SystemWorkMode systemWorkMode, ConfigMacroData configMacroData, quint64& devNum);

private:
    /************************************************************/
    /* 第一个字节共用的 */
    /* 天线号 */
    //    0000：表示非资源重组系统
    //    0001：表示天线1
    //    0010：表示天线2
    //    0011：表示天线3
    //    1111：表示天线15
    static quint64 setAntennaNumber(quint64& tar, quint8 sour);
    static quint8 getAntennaNumber(quint64 sour);
    /* 系统跟踪频段 */
    //    00：S频段
    //    01：C频段
    //    10：X频段
    //    11：Ka频段
    static quint64 setSystemTrackingFrequencyBand(quint64& tar, quint8 sour);
    static quint8 getSystemTrackingFrequencyBand(quint64 sour);
    /* 设备组合号类型 */
    //    00：无此参数
    //    01：测控设备组合号
    //    10：数传设备组合号
    static quint64 setEquipmentCombinationType(quint64& tar, quint8 sour);
    static quint8 getEquipmentCombinationType(quint64 sour);

    /* 低频段测控频段 */
    //    00：S频段
    //    01：C频段
    //    10：X频段
    //    11：Ka频段
    static quint64 setLowFrequencyBandCK(quint64& tar, quint8 sour);
    static quint8 getLowFrequencyBandCK(quint64 sour);

    /* 高频段测控频段 */
    //    00：S频段
    //    01：C频段
    //    10：X频段
    //    11：Ka频段
    static quint64 setHighFrequencyBandCK(quint64& tar, quint8 sour);
    static quint8 getHighFrequencyBandCK(quint64 sour);

    /* 上行频段 */
    //    00：S频段
    //    01：C频段
    //    10：X频段
    //    11：Ka频段
    static quint64 setUplinkBand(quint64& tar, quint8 sour);
    static quint8 getUplinkBand(quint64 sour);

    /* 测控基带 */
    // 0000：无意义
    // 0001：表示基带1
    // 1111：表示基带15
    static quint64 setCkDeviceNum(quint64& tar, quint8 sour);
    static quint8 getCkDeviceNum(quint64 sour);

    /* 跟踪基带 */
    // 0000：无意义
    // 0001：表示基带1
    // 1111：表示基带15
    static quint64 setGzDeviceNum(quint64& tar, quint8 sour);
    static quint8 getGzDeviceNum(quint64 sour);

    /* 跟踪基带 */
    // 0000：无意义
    // 0001：表示基带1
    // 1111：表示基带15
    static quint64 setDtGzDeviceNum(quint64& tar, quint8 sour);
    static quint8 getDtGzDeviceNum(quint64 sour);
    /**
     * 功能 getBit 获取位数据
     * 参数 sour 源数据
     * 参数 indexL 最低位索引
     * 参数 indexH 最高位索引
     * 返回值
     */
    static quint8 getBit(quint64 sour, int indexL, int indexH);

    /**
     * 功能 setBit 设置位数据
     * 参数 tar 目标数据
     * 参数 index 最低位索引
     * 参数 flag true=1，false=0
     * 返回值
     */
    static quint64 setBit(quint64 tar, int index, bool flag);

    /**
     * 功能 resetBit
     * 参数 tar
     * 参数 indexL
     */
    static void resetBit(quint64& tar, int indexL, int indexH);

    /**
     * 功能 setBit
     * 参数 tar
     * 参数 sour
     * 参数 indexL
     * 参数 indexH
     * 返回值
     */
    static quint64 setBit(quint64& tar, quint8 sour, int indexL, int indexH);
};

#endif  // EQUIPMENTCOMBINATIONHELPER_H
