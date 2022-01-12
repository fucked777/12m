#ifndef DEVGENERALDEFINE_H
#define DEVGENERALDEFINE_H
#include <QObject>

/* Measurement and Control Integration */
/* 测控基带的模式 */
enum class MACBMode : qint32
{
    Unknown = 0x00,       /* 未知模式 */
    STTC = 0x01,          /* S频段标准TTC模式 */
    SK2TCC = 0x04,        /* S频段扩频TTC模式 */
    SMCDTI_AL = 0x10,     /* S频段测控数传一体化(上面级)模式 */
    SMCDTI_Sate = 0x11,   /* S频段测控数传一体化(卫星)模式 */
    SMCDTI_UCR = 0x12,    /* S频段测控数传一体化(上行高码率)模式 */
    SMCDTI_ALNC = 0x13,   /* S频段测控数传一体化(上面级+扩频不共载波)模式 */
    SMCDTI_ALC = 0x15,    /* S频段测控数传一体化(上面级+扩频共载波)模式 */
    SMCDTI_SateK2 = 0x17, /* S频段测控数传一体化(卫星+扩频)模式 */
    SMCDTI_UCRK2 = 0x19,  /* S频段测控数传一体化(上行高码率+扩频)模式 */
    // SExpJump = 0x1A,      /* S频段扩跳频模式 */
};
/* 跟踪基带模式 */
enum class TBMode : qint32
{
    Unknown = 0x00,      /* 未知模式 */
    STTC = 0x01,         /* S频段标准TTC模式 */
    SKTCC = 0x02,        /* S频段扩频TTC模式 */
    SMCDTI = 0x03,       /* S测控数传一体化 */
    XKaDT = 0x04,        /* X/Ka宽带跟踪 */
    STTC_XKaDT = 0x05,   /* S标准TTC+X/Ka高速数传 */
    SKTTC_XKaDT = 0x06,  /* S扩频TTC+X/Ka高速数传 */
    SMCDTI_XKaDT = 0x07, /* S测控数传一体化+X/Ka高速数传 */
    SMCDTI_SKTTC = 0x08, /* S频段扩频TTC+测控数传一体化 */
    SExpJump = 0x09,     /* S频段扩跳频 */
};

/* 数传模式 */
enum class DTBMode : qint32
{
    Unknown = 0x00, /* 未知模式 */
    HDT = 0x01,     /* 高速数传 */
};

/* 联试应答机终端模式 */
enum class JTRMode : qint32
{
    Unknown = 0x00, /* 未知模式 */
    TTC = 0x01,     /* 标准TTC模式 */
    K2TCC = 0x02,   /* 扩频TTC模式 */
    TI_AL = 0x3,    /* 一体化(上面级)模式 */
    TI_ALC = 0x5,   /* 一体化(上面级+扩频共载波)模式 */
    TI_ALNC = 0x6,  /* 一体化(上面级+扩频不共载波)模式 */
};

/* 主备机 */
enum class MainStandby
{
    // Unknown = 0, /* 未知 */
    NotUsed = 1, /* 不使用 */
    Main = 2,    /* 主机 */
    Standby = 3, /* 备机 */
};

enum class FrequencyBand : qint32
{
    Unknown = 0x00,
    S = 0x01,
    X = 0x02,
    Ka = 0x03,
};

/* 上下行 */
// enum class UpAndDown : qint32
//{
//    Unknown = 0x00,
//    Up = 0x01,
//    Down = 0x02,
//};
#endif  // DEVGENERALDEFINE_H
