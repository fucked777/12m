#ifndef DEVGENERALSERIALIZE_H
#define DEVGENERALSERIALIZE_H
#include "DevGeneralDefine.h"
#include "Utility.h"

class ModeConvert
{
public:
    static QString macbToStr(MACBMode);
    static MACBMode macbFromStr(const QString&);
    static QString macbToDesStr(MACBMode);
    static QList<MACBMode> macbModeList(bool hasUnknown = false);                // 参数是是否包含未知
    static QList<EnumInfo<MACBMode>> macbModeListInfo(bool hasUnknown = false);  // 参数是是否包含未知
    static int macbToDevValue(MACBMode);

    static QString tbToStr(TBMode);
    static TBMode tbFromStr(const QString&);
    static QString tbToDesStr(TBMode);
    static QList<TBMode> tbModeList(bool hasUnknown = false);                 // 参数是是否包含未知
    static QList<EnumInfo<TBMode>> tbbModeListInfo(bool hasUnknown = false);  // 参数是是否包含未知
    static int tbbToDevValue(TBMode);

    static QString dtbToStr(DTBMode);
    static DTBMode dtbFromStr(const QString&);
    static QString dtbToDesStr(DTBMode);
    static QList<DTBMode> dtbModeList(bool hasUnknown = false);                // 参数是是否包含未知
    static QList<EnumInfo<DTBMode>> dtbModeListInfo(bool hasUnknown = false);  // 参数是是否包含未知
    static int dtbToDevValue(DTBMode);
};

class FBConvert
{
public:
    static QString fbToStr(FrequencyBand);
    static FrequencyBand fbFromStr(const QString&);
    static QString fbToDesStr(FrequencyBand);
    static QList<FrequencyBand> fbList(bool hasUnknown = false);                // 参数是是否包含未知
    static QList<EnumInfo<FrequencyBand>> fbListInfo(bool hasUnknown = false);  // 参数是是否包含未知
};
/* 上下行 */
// class UDConvert
//{
// public:
//    static QString fbToStr(UpAndDown);
//    static UpAndDown fbFromStr(const QString&);
//    static QString fbToDesStr(UpAndDown);
//    static QList<UpAndDown> fbList(bool hasUnknown = false);                // 参数是是否包含未知
//    static QList<EnumInfo<UpAndDown>> fbListInfo(bool hasUnknown = false);  // 参数是是否包含未知
//};

#endif  // DEVICEMODESERIALIZE_H
