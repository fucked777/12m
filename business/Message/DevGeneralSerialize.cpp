
#include "DevGeneralSerialize.h"
#include <QMap>

#define MODE_UNKNOWN_STR           "UNKNOWN"
#define MACBMODE_STTC_STR          "CTRL_STTC"
#define MACBMODE_SK2TCC_STR        "CTRL_SK2TCC"
#define MACBMODE_SMCDTI_AL_STR     "CTRL_SMCDTI_AL"
#define MACBMODE_SMCDTI_SATE_STR   "CTRL_SMCDTI_SATE"
#define MACBMODE_SMCDTI_UCR_STR    "CTRL_SMCDTI_UCR"
#define MACBMODE_SMCDTI_ALNC_STR   "CTRL_SMCDTI_ALNC"
#define MACBMODE_SMCDTI_ALC_STR    "CTRL_SMCDTI_ALC"
#define MACBMODE_SMCDTI_SATEK2_STR "CTRL_SMCDTI_SATEK2"
#define MACBMODE_SMCDTI_UCRK2_STR  "CTRL_SMCDTI_UCRK2"

#define MODE_UNKNOWN_DESC_STR           "未知"
#define MACBMODE_STTC_DESC_STR          "S频段标准TTC"
#define MACBMODE_SK2TCC_DESC_STR        "S频段扩频TTC"
#define MACBMODE_SMCDTI_AL_DESC_STR     "S频段测控数传一体化(上面级)"
#define MACBMODE_SMCDTI_SATE_DESC_STR   "S频段测控数传一体化(卫星)"
#define MACBMODE_SMCDTI_UCR_DESC_STR    "S频段测控数传一体化(上行高码率)"
#define MACBMODE_SMCDTI_ALNC_DESC_STR   "S频段测控数传一体化(上面级+扩频不共载波)"
#define MACBMODE_SMCDTI_ALC_DESC_STR    "S频段测控数传一体化(上面级+扩频共载波)"
#define MACBMODE_SMCDTI_SATEK2_DESC_STR "S频段测控数传一体化(卫星+扩频)"
#define MACBMODE_SMCDTI_UCRK2_DESC_STR  "S频段测控数传一体化(上行高码率+扩频)"

#define TBMODE_STTC_STR         "TRACK_STTC"
#define TBMODE_SKTCC_STR        "TRACK_SKTCC"
#define TBMODE_SMCDTI_STR       "TRACK_SMCDTI"
#define TBMODE_XKADT_STR        "TRACK_XKADT"
#define TBMODE_STTC_XKADT_STR   "TRACK_STTC_XKADT"
#define TBMODE_SKTTC_XKADT_STR  "TRACK_SKTTC_XKADT"
#define TBMODE_SMCDTI_XKADT_STR "TRACK_SMCDTI_XKADT"
#define TBMODE_SMCDTI_SKTTC_STR "TRACK_SMCDTI_SKTTC"
#define TBMODE_SEXPJUMP_STR     "TRACK_SEXPJUMP"

#define TBMODE_STTC_DESC_STR         "S频段标准TTC模式"
#define TBMODE_SKTCC_DESC_STR        "S频段扩频TTC模式"
#define TBMODE_SMCDTI_DESC_STR       "S测控数传一体化"
#define TBMODE_XKADT_DESC_STR        "X/Ka宽带跟踪"
#define TBMODE_STTC_XKADT_DESC_STR   "S标准TTC+X/Ka高速数传"
#define TBMODE_SKTTC_XKADT_DESC_STR  "S扩频TTC+X/Ka高速数传"
#define TBMODE_SMCDTI_XKADT_DESC_STR "S测控数传一体化+X/Ka高速数传"
#define TBMODE_SMCDTI_SKTTC_DESC_STR "S频段扩频TTC+测控数传一体化"
#define TBMODE_SEXPJUMP_DESC_STR     "S频段扩跳频"

#define DTBMODE_HDT_STR      "DT_HDT"
#define DTBMODE_HDT_DESC_STR "高速数传"

#define FB_S_STR  "S"
#define FB_X_STR  "X"
#define FB_KA_STR "Ka"

#define FB_S_DESC_STR  "S频段"
#define FB_X_DESC_STR  "X频段"
#define FB_KA_DESC_STR "Ka频段"

//#define UD_UP_STR   "UP"
//#define UD_DOWN_STR "Down"

//#define UD_UP_DESC_STR   "上行"
//#define UD_DOWN_DESC_STR "下行"

QString ModeConvert::macbToStr(MACBMode mode)
{
    switch (mode)
    {
    case MACBMode::Unknown:
    {
        break;
    }
    case MACBMode::STTC:
    {
        return MACBMODE_STTC_STR;
    }
    case MACBMode::SK2TCC:
    {
        return MACBMODE_SK2TCC_STR;
    }
    case MACBMode::SMCDTI_AL:
    {
        return MACBMODE_SMCDTI_AL_STR;
    }
    case MACBMode::SMCDTI_Sate:
    {
        return MACBMODE_SMCDTI_SATE_STR;
    }
    case MACBMode::SMCDTI_UCR:
    {
        return MACBMODE_SMCDTI_UCR_STR;
    }
    case MACBMode::SMCDTI_ALNC:
    {
        return MACBMODE_SMCDTI_ALNC_STR;
    }
    case MACBMode::SMCDTI_ALC:
    {
        return MACBMODE_SMCDTI_ALC_STR;
    }
    case MACBMode::SMCDTI_SateK2:
    {
        return MACBMODE_SMCDTI_SATEK2_STR;
    }
    case MACBMode::SMCDTI_UCRK2:
    {
        return MACBMODE_SMCDTI_UCRK2_STR;
    }
    }
    return MODE_UNKNOWN_STR;
}
MACBMode ModeConvert::macbFromStr(const QString& raw)
{
    auto mode = raw.toUpper();
    if (mode == MACBMODE_STTC_STR)
    {
        return MACBMode::STTC;
    }
    else if (mode == MACBMODE_SK2TCC_STR)
    {
        return MACBMode::SK2TCC;
    }
    else if (mode == MACBMODE_SMCDTI_AL_STR)
    {
        return MACBMode::SMCDTI_AL;
    }
    else if (mode == MACBMODE_SMCDTI_SATE_STR)
    {
        return MACBMode::SMCDTI_Sate;
    }
    else if (mode == MACBMODE_SMCDTI_UCR_STR)
    {
        return MACBMode::SMCDTI_UCR;
    }
    else if (mode == MACBMODE_SMCDTI_ALNC_STR)
    {
        return MACBMode::SMCDTI_ALNC;
    }
    else if (mode == MACBMODE_SMCDTI_ALC_STR)
    {
        return MACBMode::SMCDTI_ALC;
    }
    else if (mode == MACBMODE_SMCDTI_SATEK2_STR)
    {
        return MACBMode::SMCDTI_SateK2;
    }
    else if (mode == MACBMODE_SMCDTI_UCRK2_STR)
    {
        return MACBMode::SMCDTI_UCRK2;
    }

    return MACBMode::Unknown;
}
QString ModeConvert::macbToDesStr(MACBMode mode)
{
    switch (mode)
    {
    case MACBMode::Unknown:
    {
        break;
    }
    case MACBMode::STTC:
    {
        return MACBMODE_STTC_DESC_STR;
    }
    case MACBMode::SK2TCC:
    {
        return MACBMODE_SK2TCC_DESC_STR;
    }
    case MACBMode::SMCDTI_AL:
    {
        return MACBMODE_SMCDTI_AL_DESC_STR;
    }
    case MACBMode::SMCDTI_Sate:
    {
        return MACBMODE_SMCDTI_SATE_DESC_STR;
    }
    case MACBMode::SMCDTI_UCR:
    {
        return MACBMODE_SMCDTI_UCR_DESC_STR;
    }
    case MACBMode::SMCDTI_ALNC:
    {
        return MACBMODE_SMCDTI_ALNC_DESC_STR;
    }
    case MACBMode::SMCDTI_ALC:
    {
        return MACBMODE_SMCDTI_ALC_DESC_STR;
    }
    case MACBMode::SMCDTI_SateK2:
    {
        return MACBMODE_SMCDTI_SATEK2_DESC_STR;
    }
    case MACBMode::SMCDTI_UCRK2:
    {
        return MACBMODE_SMCDTI_UCRK2_DESC_STR;
    }
    }
    return MODE_UNKNOWN_DESC_STR;
}

QString ModeConvert::tbToStr(TBMode mode)
{
    switch (mode)
    {
    case TBMode::Unknown:
    {
        break;
    }
    case TBMode::STTC:
    {
        return TBMODE_STTC_STR;
    }
    case TBMode::SKTCC:
    {
        return TBMODE_SKTCC_STR;
    }
    case TBMode::SMCDTI:
    {
        return TBMODE_SMCDTI_STR;
    }
    case TBMode::XKaDT:
    {
        return TBMODE_XKADT_STR;
    }
    case TBMode::STTC_XKaDT:
    {
        return TBMODE_STTC_XKADT_STR;
    }
    case TBMode::SKTTC_XKaDT:
    {
        return TBMODE_SKTTC_XKADT_STR;
    }
    case TBMode::SMCDTI_XKaDT:
    {
        return TBMODE_SMCDTI_XKADT_STR;
    }
    case TBMode::SMCDTI_SKTTC:
    {
        return TBMODE_SMCDTI_SKTTC_STR;
    }
    case TBMode::SExpJump:
    {
        return TBMODE_SEXPJUMP_STR;
    }
    }
    return MODE_UNKNOWN_STR;
}
TBMode ModeConvert::tbFromStr(const QString& raw)
{
    auto mode = raw.toUpper();
    if (mode == TBMODE_STTC_STR)
    {
        return TBMode::STTC;
    }
    else if (mode == TBMODE_SKTCC_STR)
    {
        return TBMode::SKTCC;
    }
    else if (mode == TBMODE_SMCDTI_STR)
    {
        return TBMode::SMCDTI;
    }
    else if (mode == TBMODE_XKADT_STR)
    {
        return TBMode::XKaDT;
    }
    else if (mode == TBMODE_STTC_XKADT_STR)
    {
        return TBMode::STTC_XKaDT;
    }
    else if (mode == TBMODE_SKTTC_XKADT_STR)
    {
        return TBMode::SKTTC_XKaDT;
    }
    else if (mode == TBMODE_SMCDTI_XKADT_STR)
    {
        return TBMode::SMCDTI_XKaDT;
    }
    else if (mode == TBMODE_SMCDTI_SKTTC_STR)
    {
        return TBMode::SMCDTI_SKTTC;
    }
    else if (mode == TBMODE_SEXPJUMP_STR)
    {
        return TBMode::SExpJump;
    }

    return TBMode::Unknown;
}
QString ModeConvert::tbToDesStr(TBMode mode)
{
    switch (mode)
    {
    case TBMode::Unknown:
    {
        break;
    }
    case TBMode::STTC:
    {
        return TBMODE_STTC_DESC_STR;
    }
    case TBMode::SKTCC:
    {
        return TBMODE_SKTCC_DESC_STR;
    }
    case TBMode::SMCDTI:
    {
        return TBMODE_SMCDTI_DESC_STR;
    }
    case TBMode::XKaDT:
    {
        return TBMODE_XKADT_DESC_STR;
    }
    case TBMode::STTC_XKaDT:
    {
        return TBMODE_STTC_XKADT_DESC_STR;
    }
    case TBMode::SKTTC_XKaDT:
    {
        return TBMODE_SKTTC_XKADT_DESC_STR;
    }
    case TBMode::SMCDTI_XKaDT:
    {
        return TBMODE_SMCDTI_XKADT_DESC_STR;
    }
    case TBMode::SMCDTI_SKTTC:
    {
        return TBMODE_SMCDTI_SKTTC_DESC_STR;
    }
    case TBMode::SExpJump:
    {
        return TBMODE_SEXPJUMP_DESC_STR;
    }
    }
    return MODE_UNKNOWN_DESC_STR;
}
QList<MACBMode> ModeConvert::macbModeList(bool hasUnknown)
{
    if (hasUnknown)
    {
        static QList<MACBMode> modeList{
            MACBMode::Unknown,    MACBMode::STTC,        MACBMode::SK2TCC,     MACBMode::SMCDTI_AL,     MACBMode::SMCDTI_Sate,
            MACBMode::SMCDTI_UCR, MACBMode::SMCDTI_ALNC, MACBMode::SMCDTI_ALC, MACBMode::SMCDTI_SateK2, MACBMode::SMCDTI_UCRK2,
        };
        return modeList;
    }

    static QList<MACBMode> modeList{
        MACBMode::STTC,        MACBMode::SK2TCC,     MACBMode::SMCDTI_AL,     MACBMode::SMCDTI_Sate,  MACBMode::SMCDTI_UCR,
        MACBMode::SMCDTI_ALNC, MACBMode::SMCDTI_ALC, MACBMode::SMCDTI_SateK2, MACBMode::SMCDTI_UCRK2,
    };
    return modeList;
}
QList<TBMode> ModeConvert::tbModeList(bool hasUnknown)
{
    if (hasUnknown)
    {
        static QList<TBMode> modeList{
            TBMode::Unknown,    TBMode::STTC,        TBMode::SKTCC,        TBMode::SMCDTI,       TBMode::XKaDT,
            TBMode::STTC_XKaDT, TBMode::SKTTC_XKaDT, TBMode::SMCDTI_XKaDT, TBMode::SMCDTI_SKTTC, TBMode::SExpJump,
        };
        return modeList;
    }
    static QList<TBMode> modeList{
        TBMode::STTC,        TBMode::SKTCC,        TBMode::SMCDTI,       TBMode::XKaDT,    TBMode::STTC_XKaDT,
        TBMode::SKTTC_XKaDT, TBMode::SMCDTI_XKaDT, TBMode::SMCDTI_SKTTC, TBMode::SExpJump,
    };
    return modeList;
}
static QList<EnumInfo<MACBMode>> macbModeListInfoImpl()
{
    using IT = EnumInfo<MACBMode>;
    QList<EnumInfo<MACBMode>> ret;

    ret.append(IT{ MACBMode::STTC, MACBMODE_STTC_STR, MACBMODE_STTC_DESC_STR });
    ret.append(IT{ MACBMode::SK2TCC, MACBMODE_SK2TCC_STR, MACBMODE_SK2TCC_DESC_STR });
    ret.append(IT{ MACBMode::SMCDTI_AL, MACBMODE_SMCDTI_AL_STR, MACBMODE_SMCDTI_AL_DESC_STR });
    ret.append(IT{ MACBMode::SMCDTI_Sate, MACBMODE_SMCDTI_SATE_STR, MACBMODE_SMCDTI_SATE_DESC_STR });
    ret.append(IT{ MACBMode::SMCDTI_UCR, MACBMODE_SMCDTI_UCR_STR, MACBMODE_SMCDTI_UCR_DESC_STR });
    ret.append(IT{ MACBMode::SMCDTI_ALNC, MACBMODE_SMCDTI_ALNC_STR, MACBMODE_SMCDTI_ALNC_DESC_STR });
    ret.append(IT{ MACBMode::SMCDTI_ALC, MACBMODE_SMCDTI_ALC_STR, MACBMODE_SMCDTI_ALC_DESC_STR });
    ret.append(IT{ MACBMode::SMCDTI_SateK2, MACBMODE_SMCDTI_SATEK2_STR, MACBMODE_SMCDTI_SATEK2_DESC_STR });
    ret.append(IT{ MACBMode::SMCDTI_UCRK2, MACBMODE_SMCDTI_UCRK2_STR, MACBMODE_SMCDTI_UCRK2_DESC_STR });
    return ret;
}
static QList<EnumInfo<MACBMode>> macbModeListInfoImplUnknown()
{
    using IT = EnumInfo<MACBMode>;
    QList<EnumInfo<MACBMode>> ret;

    ret.append(IT{ MACBMode::Unknown, MODE_UNKNOWN_STR, MODE_UNKNOWN_DESC_STR });
    ret.append(macbModeListInfoImpl());
    return ret;
}

QList<EnumInfo<MACBMode>> ModeConvert::macbModeListInfo(bool hasUnknown)
{
    if (hasUnknown)
    {
        static auto tempList = macbModeListInfoImplUnknown();
        return tempList;
    }

    static auto tempList = macbModeListInfoImpl();
    return tempList;
}
static QList<EnumInfo<TBMode>> tbbModeListInfoImpl()
{
    using IT = EnumInfo<TBMode>;
    QList<EnumInfo<TBMode>> ret;
    ret.append(IT{ TBMode::STTC, TBMODE_STTC_STR, TBMODE_STTC_DESC_STR });
    ret.append(IT{ TBMode::SKTCC, TBMODE_SKTCC_STR, TBMODE_SKTCC_DESC_STR });
    ret.append(IT{ TBMode::SMCDTI, TBMODE_SMCDTI_STR, TBMODE_SMCDTI_DESC_STR });
    ret.append(IT{ TBMode::XKaDT, TBMODE_XKADT_STR, TBMODE_XKADT_DESC_STR });
    ret.append(IT{ TBMode::STTC_XKaDT, TBMODE_STTC_XKADT_STR, TBMODE_STTC_XKADT_DESC_STR });
    ret.append(IT{ TBMode::SKTTC_XKaDT, TBMODE_SKTTC_XKADT_STR, TBMODE_SKTTC_XKADT_DESC_STR });
    ret.append(IT{ TBMode::SMCDTI_XKaDT, TBMODE_SMCDTI_XKADT_STR, TBMODE_SMCDTI_XKADT_DESC_STR });
    ret.append(IT{ TBMode::SMCDTI_SKTTC, TBMODE_SMCDTI_SKTTC_STR, TBMODE_SMCDTI_SKTTC_DESC_STR });
    ret.append(IT{ TBMode::SExpJump, TBMODE_SEXPJUMP_STR, TBMODE_SEXPJUMP_DESC_STR });
    return ret;
}

static QList<EnumInfo<TBMode>> tbbModeListInfoImplUnknown()
{
    using IT = EnumInfo<TBMode>;
    QList<EnumInfo<TBMode>> ret;

    ret.append(IT{ TBMode::Unknown, MODE_UNKNOWN_STR, MODE_UNKNOWN_DESC_STR });
    ret.append(tbbModeListInfoImpl());
    return ret;
}

QList<EnumInfo<TBMode>> ModeConvert::tbbModeListInfo(bool hasUnknown)
{
    if (hasUnknown)
    {
        static auto tempList = tbbModeListInfoImplUnknown();
        return tempList;
    }

    static auto tempList = tbbModeListInfoImpl();
    return tempList;
}
QString ModeConvert::dtbToStr(DTBMode mode)
{
    switch (mode)
    {
    case DTBMode::Unknown:
    {
        break;
    }
    case DTBMode::HDT:
    {
        return DTBMODE_HDT_STR;
    }
    }
    return MODE_UNKNOWN_STR;
}
DTBMode ModeConvert::dtbFromStr(const QString& raw)
{
    auto mode = raw.toUpper();
    if (mode == DTBMODE_HDT_STR)
    {
        return DTBMode::HDT;
    }
    return DTBMode::Unknown;
}
QString ModeConvert::dtbToDesStr(DTBMode mode)
{
    switch (mode)
    {
    case DTBMode::Unknown:
    {
        break;
    }
    case DTBMode::HDT:
    {
        return DTBMODE_HDT_DESC_STR;
    }
    }
    return MODE_UNKNOWN_DESC_STR;
}
QList<DTBMode> ModeConvert::dtbModeList(bool hasUnknown)
{
    if (hasUnknown)
    {
        static QList<DTBMode> modeList{
            DTBMode::Unknown,
            DTBMode::HDT,
        };
        return modeList;
    }
    static QList<DTBMode> modeList{
        DTBMode::HDT,
    };
    return modeList;
}
QList<EnumInfo<DTBMode>> ModeConvert::dtbModeListInfo(bool hasUnknown)
{
    if (hasUnknown)
    {
        static QList<EnumInfo<DTBMode>> modeList{
            { DTBMode::Unknown, MODE_UNKNOWN_STR, MODE_UNKNOWN_DESC_STR },
            { DTBMode::HDT, DTBMODE_HDT_STR, DTBMODE_HDT_DESC_STR },
        };
        return modeList;
    }
    static QList<EnumInfo<DTBMode>> modeList{
        { DTBMode::HDT, DTBMODE_HDT_STR, DTBMODE_HDT_DESC_STR },
    };
    return QList<EnumInfo<DTBMode>>();
}

QString FBConvert::fbToStr(FrequencyBand fb)
{
    switch (fb)
    {
    case FrequencyBand::Unknown:
    {
        break;
    }
    case FrequencyBand::S:
    {
        return FB_S_STR;
    }
    case FrequencyBand::X:
    {
        return FB_X_STR;
    }
    case FrequencyBand::Ka:
    {
        return FB_KA_STR;
    }
    }
    return MODE_UNKNOWN_STR;
}
FrequencyBand FBConvert::fbFromStr(const QString& fb)
{
    if (fb == FB_S_STR)
    {
        return FrequencyBand::S;
    }
    else if (fb == FB_X_STR)
    {
        return FrequencyBand::X;
    }
    else if (fb == FB_KA_STR)
    {
        return FrequencyBand::Ka;
    }
    return FrequencyBand::Unknown;
}
QString FBConvert::fbToDesStr(FrequencyBand fb)
{
    switch (fb)
    {
    case FrequencyBand::Unknown:
    {
        break;
    }
    case FrequencyBand::S:
    {
        return FB_S_DESC_STR;
    }
    case FrequencyBand::X:
    {
        return FB_X_DESC_STR;
    }
    case FrequencyBand::Ka:
    {
        return FB_KA_DESC_STR;
    }
    }
    return MODE_UNKNOWN_DESC_STR;
}
QList<FrequencyBand> FBConvert::fbList(bool hasUnknown)
{
    if (hasUnknown)
    {
        static QList<FrequencyBand> fbList{
            FrequencyBand::Unknown,
            FrequencyBand::S,
            FrequencyBand::X,
            FrequencyBand::Ka,
        };
        return fbList;
    }

    static QList<FrequencyBand> fbList{
        FrequencyBand::S,
        FrequencyBand::X,
        FrequencyBand::Ka,
    };
    return fbList;
}
static QList<EnumInfo<FrequencyBand>> fbListInfoImpl(bool hasUnknown)
{
    using IT = EnumInfo<FrequencyBand>;
    QList<EnumInfo<FrequencyBand>> ret;
    if (hasUnknown)
    {
        ret.append(IT{ FrequencyBand::Unknown, MODE_UNKNOWN_STR, MODE_UNKNOWN_DESC_STR });
    }

    ret.append(IT{ FrequencyBand::S, FB_S_STR, FB_S_DESC_STR });
    ret.append(IT{ FrequencyBand::X, FB_X_STR, FB_X_DESC_STR });
    ret.append(IT{ FrequencyBand::Ka, FB_KA_STR, FB_KA_DESC_STR });
    return ret;
}
QList<EnumInfo<FrequencyBand>> FBConvert::fbListInfo(bool hasUnknown)
{
    static QList<EnumInfo<FrequencyBand>> fbList = fbListInfoImpl(hasUnknown);
    return fbList;
}

// QString UDConvert::fbToStr(UpAndDown ud)
//{
//    switch (ud)
//    {
//    case UpAndDown::Unknown:
//    {
//        break;
//    }
//    case UpAndDown::Up:
//    {
//        return UD_UP_STR;
//    }
//    case UpAndDown::Down:
//    {
//        return UD_DOWN_STR;
//    }
//    }
//    return MODE_UNKNOWN_STR;
//}
// UpAndDown UDConvert::fbFromStr(const QString& ud)
//{
//    if (ud == UD_UP_STR)
//    {
//        return UpAndDown::Up;
//    }
//    else if (ud == UD_DOWN_STR)
//    {
//        return UpAndDown::Down;
//    }
//    return UpAndDown::Unknown;
//}
// QString UDConvert::fbToDesStr(UpAndDown ud)
//{
//    switch (ud)
//    {
//    case UpAndDown::Unknown:
//    {
//        break;
//    }
//    case UpAndDown::Up:
//    {
//        return UD_UP_DESC_STR;
//    }
//    case UpAndDown::Down:
//    {
//        return UD_DOWN_DESC_STR;
//    }
//    }
//    return MODE_UNKNOWN_STR;
//}

// QList<UpAndDown> UDConvert::fbList(bool hasUnknown)
//{
//    if (hasUnknown)
//    {
//        static QList<UpAndDown> udList{
//            UpAndDown::Unknown,
//            UpAndDown::Up,
//            UpAndDown::Down,
//        };
//        return udList;
//    }
//    static QList<UpAndDown> udList{ UpAndDown::Up, UpAndDown::Down };
//    return udList;
//}
// QList<EnumInfo<UpAndDown>> UDConvert::fbListInfo(bool hasUnknown)
//{
//    if (hasUnknown)
//    {
//        static QList<EnumInfo<UpAndDown>> udList{
//            { UpAndDown::Unknown, MODE_UNKNOWN_STR, MODE_UNKNOWN_DESC_STR },
//            { UpAndDown::Up, UD_UP_STR, UD_UP_DESC_STR },
//            { UpAndDown::Down, UD_DOWN_STR, UD_DOWN_DESC_STR },
//        };
//        return udList;
//    }
//    static QList<EnumInfo<UpAndDown>> udList{
//        { UpAndDown::Up, UD_UP_STR, UD_UP_DESC_STR },
//        { UpAndDown::Down, UD_DOWN_STR, UD_DOWN_DESC_STR },
//    };
//    return udList;
//}
