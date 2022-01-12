#ifndef SYSTEMWORKMODE_H
#define SYSTEMWORKMODE_H

#include <QList>
#include <QString>

//定义系统中所有的工作模式
// 4413 使用 S标准TTC，S扩2，Ka扩2，X低速，Ka低速，Ka高速

// 4424 使用
// S标准TTC，S扩二，一体化上面级，一体化卫星，一体化高码率，一体化上面级+扩二共载波，一体化上面级+扩二不共载波，一体化卫星+扩二，一体化高码率+扩二，
// 扩跳，X高速，Ka高速

// 4426 使用 S标准TTC，S扩2，Ka扩2，X低速，Ka低速，Ka高速
enum SystemWorkMode
{
    NotDefine = 0,       // 未定义
    STTC = 1,            // S标准TTC
    Skuo2 = 2,           // S扩二
    KaKuo2 = 3,          // Ka扩二
    XDS = 4,             // X低速
    KaDS = 5,            // Ka低速
    KaGS = 6,            // Ka高速
    SYTHSMJ = 7,         // 一体化上面级
    SYTHWX = 8,          // 一体化卫星
    SYTHGML = 9,         // 一体化高码率
    SYTHSMJK2GZB = 10,   // 一体化上面级+扩二共载波
    SYTHSMJK2BGZB = 11,  // 一体化上面级+扩二不共载波
    SYTHWXSK2 = 12,      // 一体化卫星+扩二
    SYTHGMLSK2 = 13,     // 一体化高码率+扩二
    SKT = 14,            // 扩跳
    XGS = 15,            // X高速
};

// 00：S频段 01：C频段 10：X频段 11：Ka频段
enum SystemBandMode
{
    SBand = 0,   // S频段
    CBand = 1,   // C频段
    XBand = 2,   // X频段
    KaBand = 3,  // Ka频段
    NoDefineBand
};

// 00：S频段 01：C频段 10：X频段 11：Ka频段
enum SystemBandLevel
{
    LowLevelBand = 0,   // 低频
    HighLevelBand = 1,  // 高频
    NoDefineLevelBand
};

enum SatelliteTrackingMode
{
    TrackUnknown = 0,         //未知类型
    _4424_S = 1,              // S
    _4424_KaCk = 2,           // ka测控
    _4424_KaDSDT = 3,         // ka低速数传
    _4424_KaGSDT = 4,         // Ka高速数传
    _4424_S_KaCk = 5,         // S+Ka测控
    _4424_S_KaDSDT = 6,       // S+ka低速数传
    _4424_S_KaGSDT = 7,       // S+ka高速数传
    _4424_S_KaCk_KaDSDT = 8,  // S+Ka测控+Ka低速
    _4424_S_KaCK_KaGSDT = 9,  // S+Ka测控+ka高速数传

    _4426_STTC,  //同上
    _4426_SK2,
    _4426_SKT,
    _4426_KaCk,
    _4426_KaDSDT,
    _4426_KaGSDT,

    _4426_STTC_KaCk,
    _4426_STTC_KaDSDT,
    _4426_STTC_KaGSDT,
    _4426_STTC_KaCk_KaDSDT,
    _4426_STTC_KaCK_KaGSDT,

    _4426_SK2_KaCk,
    _4426_SK2_KaDSDT,
    _4426_SK2_KaGSDT,
    _4426_SK2_KaCk_KaDSDT,
    _4426_SK2_KaCK_KaGSDT,

    // 截止20211023 扩跳不能快速校相
    // _4426_SKT_KaCk,
    // _4426_SKT_KaDSDT,
    // _4426_SKT_KaGSDT,
    // _4426_SKT_KaCk_KaDSDT,
    // _4426_SKT_KaCK_KaGSDT,
};

enum class MasterSlave
{
    Unkonwn = 0,
    Master,  // 主
    Slave    // 备
};

// 一体化主用那一个
enum MasterType
{
    NONE = 0,
    YTH_DT = 1,  // 数传
    YTH_KP = 2,  // 扩频
};

// 系统旋向
enum class SystemOrientation
{
    Unkonwn = 0,
    LCircumflex = 1,  //左旋
    RCircumflex = 2,  //右旋
    LRCircumflex = 3  //左右旋同时
};

class SystemWorkModeHelper
{
public:
    // 自定义的SystemWorkMode转为设备实际的模式id
    static int systemWorkModeConverToModeId(SystemWorkMode workMode);
    // 测控基带实际模式ID转换为枚举ID
    static SystemWorkMode modeIDToSystemWorkMode(int modeID);
    // 模式ID是否是有效的
    static bool modeIdIsValid(int modeID);

    // 判断该工作模式是否是测控的模式
    static bool isMeasureContrlWorkMode(SystemWorkMode workMode);

    // 判断该工作模式是否是数传的模式
    static bool isDataTransmissionWorkMode(SystemWorkMode workMode);

    // 判断该工作模式是否是低速数传的模式
    static bool isDsDataTransmissionWorkMode(SystemWorkMode workMode);

    // 判断该工作模式是否是高速数传的模式
    static bool isGsDataTransmissionWorkMode(SystemWorkMode workMode);

    // 判断该工作模式是否是组合工作模式(一体化+XXX)
    static bool isMixWorkMode(SystemWorkMode workMode);

    // 获取工作模式指定目标的模式，主要是对组合工作模式
    static SystemWorkMode getTargetWorkMode(SystemWorkMode workMode, int targetNo);

    // 工作模式转为描述
    static QString systemWorkModeToDesc(SystemWorkMode workMode);

    /* 字符串转为工作模式枚举 */
    static SystemWorkMode systemWorkModeFromString(const QString& raw);

    // 工作模式转为频段
    static SystemBandMode systemWorkModeToSystemBand(SystemWorkMode workMode);

    // 工作模式转为高低频段 Ka为高 S为低
    static SystemBandLevel isSystemWorkModeHighOrLowBand(SystemWorkMode workMode);

    /* 枚举转为字符串 */
    static QString systemWorkModeToString(SystemWorkMode workMode);

    static QString masterSlaveToDesc(MasterSlave masterSlave);

    static QString systemOrientationToDesc(SystemOrientation orientation);

    // 获取全部工作模式
    static QList<SystemWorkMode> getWorkModes();

    // 获取全部测控工作模式
    static QList<SystemWorkMode> getMeasureContrlWorkMode();
    // 获取全部低速
    static QList<SystemWorkMode> getDSContrlWorkMode();
};

#endif  // SYSTEMWORKMODE_H
