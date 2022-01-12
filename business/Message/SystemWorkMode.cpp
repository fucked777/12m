#include "SystemWorkMode.h"
#include "ExtendedConfig.h"
#include <QList>
#include <QString>

// 自定义的SystemWorkMode转为设备实际的模式id
int SystemWorkModeHelper::systemWorkModeConverToModeId(SystemWorkMode workMode)
{
    switch (workMode)
    {
    case STTC: return 0x1;
    case Skuo2: return 0x4;
    case KaKuo2: return 0x5;
    case SYTHSMJ: return 0x10;
    case SYTHWX: return 0x11;
    case SYTHGML: return 0x12;
    case SYTHSMJK2GZB: return 0x15;
    case SYTHSMJK2BGZB: return 0x13;
    case SYTHWXSK2: return 0x17;
    case SYTHGMLSK2: return 0x19;
    case SKT: return 0x21;
    default: break;
    }

    return 0xFFFF;  // 无模式
}

// 测控基带实际模式ID转换为枚举ID
SystemWorkMode SystemWorkModeHelper::modeIDToSystemWorkMode(int modeID)
{
    switch (modeID)
    {
    case 0x1: return STTC;
    case 0x4: return Skuo2;
    case 0x5: return KaKuo2;
    case 0x10: return SYTHSMJ;
    case 0x11: return SYTHWX;
    case 0x12: return SYTHGML;
    case 0x15: return SYTHSMJK2GZB;
    case 0x13: return SYTHSMJK2BGZB;
    case 0x17: return SYTHWXSK2;
    case 0x19: return SYTHGMLSK2;
    case 0x21: return SKT;
    default: break;
    }

    return NotDefine;  // 无模式
}

bool SystemWorkModeHelper::modeIdIsValid(int modeID)
{
    switch (modeID)
    {
    case 0x01:
    case 0x04:
    case 0x05:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x15:
    case 0x13:
    case 0x17:
    case 0x19:
    case 0x21:
    case 0xFFFF: return true;
    }

    return false;
}

// 判断该工作模式是否是测控的模式
bool SystemWorkModeHelper::isMeasureContrlWorkMode(SystemWorkMode workMode)
{
    switch (workMode)
    {
    case STTC:
    case Skuo2:
    case KaKuo2:
    case SYTHSMJ:
    case SYTHWX:
    case SYTHGML:
    case SYTHSMJK2GZB:
    case SYTHSMJK2BGZB:
    case SYTHWXSK2:
    case SYTHGMLSK2:
    case SKT: return true;
    default: break;
    }
    return false;
}

// 判断该工作模式是否是数传的模式
bool SystemWorkModeHelper::isDataTransmissionWorkMode(SystemWorkMode workMode)
{
    switch (workMode)
    {
    case XDS:
    case KaDS:
    case KaGS:
    case XGS: return true;
    default: break;
    }
    return false;
}

// 判断该工作模式是否是低速数传的模式
bool SystemWorkModeHelper::isDsDataTransmissionWorkMode(SystemWorkMode workMode)
{
    switch (workMode)
    {
    case XDS:
    case KaDS: return true;
    default: break;
    }
    return false;
}

// 判断该工作模式是否是高速数传的模式
bool SystemWorkModeHelper::isGsDataTransmissionWorkMode(SystemWorkMode workMode)
{
    switch (workMode)
    {
    case KaGS:
    case XGS: return true;
    default: break;
    }
    return false;
}

// 判断该工作模式是否是组合工作模式(一体化+XXX)
bool SystemWorkModeHelper::isMixWorkMode(SystemWorkMode workMode)
{
    switch (workMode)
    {
    case SYTHSMJK2GZB:
    case SYTHSMJK2BGZB:
    case SYTHWXSK2:
    case SYTHGMLSK2: return true;
    default: break;
    }
    return false;
}

// 获取工作模式指定目标的模式，主要是对组合工作模式
SystemWorkMode SystemWorkModeHelper::getTargetWorkMode(SystemWorkMode workMode, int targetNo)
{
    switch (workMode)
    {
    case STTC:
    case Skuo2:
    case KaKuo2:
    case XDS:
    case KaDS:
    case KaGS:
    case SYTHSMJ:
    case SYTHWX:
    case SYTHGML:
    case SKT:
    case XGS: return workMode;
    case SYTHSMJK2GZB:
    {
        if (targetNo == 1)  // 第一个目标为一体化上面级
        {
            return SYTHSMJ;
        }
        else if (targetNo == 2)  // 第二目标为S扩二
        {
            return Skuo2;
        }
    }
    break;
    case SYTHSMJK2BGZB:
    {
        if (targetNo == 1)
        {
            return SYTHSMJ;
        }
        else if (targetNo == 2)
        {
            return Skuo2;
        }
    }
    break;
    case SYTHWXSK2:
    {
        if (targetNo == 1)
        {
            return SYTHWX;
        }
        else if (targetNo == 2)
        {
            return Skuo2;
        }
    }
    break;
    case SYTHGMLSK2:
    {
        if (targetNo == 1)
        {
            return SYTHGML;
        }
        else if (targetNo == 2)
        {
            return Skuo2;
        }
    }
    break;
    case NotDefine: return NotDefine;
    default: break;
    }

    return NotDefine;
}

// 工作模式转为描述
QString SystemWorkModeHelper::systemWorkModeToDesc(SystemWorkMode workMode)
{
    switch (workMode)
    {
    case STTC: return QString("标准TTC");
    case Skuo2: return QString("S扩二");
    case KaKuo2: return QString("Ka扩二");
    case XDS: return QString("X低速");
    case KaDS: return QString("Ka低速数传");
    case KaGS: return QString("Ka高速数传");
    case SYTHSMJ: return QString("一体化上面级");
    case SYTHWX: return QString("一体化卫星");
    case SYTHGML: return QString("一体化高码率");
    case SYTHSMJK2GZB: return QString("一体化上面级+扩二共载波");
    case SYTHSMJK2BGZB: return QString("一体化上面级+扩二不共载波");
    case SYTHWXSK2: return QString("一体化卫星+扩二");
    case SYTHGMLSK2: return QString("一体化高码率+扩二");
    case SKT: return QString("扩跳频");
    case XGS: return QString("X高速数传");
    case NotDefine: break;
    default: break;
    }
    return QString("未定义");
}
/* 字符串转为工作模式枚举 */
SystemWorkMode SystemWorkModeHelper::systemWorkModeFromString(const QString& raw)
{
    auto strMode = raw.toUpper();

    if (strMode == "STTC")
    {
        return STTC;
    }
    else if (strMode == "SKUO2")
    {
        return Skuo2;
    }
    else if (strMode == "KAKUO2")
    {
        return KaKuo2;
    }
    else if (strMode == "XDS")
    {
        return XDS;
    }
    else if (strMode == "KADS")
    {
        return KaDS;
    }
    else if (strMode == "KAGS")
    {
        return KaGS;
    }
    else if (strMode == "SYTHSMJ")
    {
        return SYTHSMJ;
    }
    else if (strMode == "SYTHWX")
    {
        return SYTHWX;
    }
    else if (strMode == "SYTHGML")
    {
        return SYTHGML;
    }
    else if (strMode == "SYTHSMJK2GZB")
    {
        return SYTHSMJK2GZB;
    }
    else if (strMode == "SYTHSMJK2BGZB")
    {
        return SYTHSMJK2BGZB;
    }
    else if (strMode == "SYTHWXSK2")
    {
        return SYTHWXSK2;
    }
    else if (strMode == "SYTHGMLSK2")
    {
        return SYTHGMLSK2;
    }
    else if (strMode == "SKT")
    {
        return SKT;
    }
    else if (strMode == "XGS")
    {
        return XGS;
    }

    return NotDefine;
}

// 工作模式转为频段
SystemBandMode SystemWorkModeHelper::systemWorkModeToSystemBand(SystemWorkMode workMode)
{
    switch (workMode)
    {
    case STTC: return SystemBandMode::SBand;
    case Skuo2: return SystemBandMode::SBand;
    case KaKuo2: return SystemBandMode::KaBand;
    case XDS: return SystemBandMode::XBand;
    case KaDS: return SystemBandMode::KaBand;
    case KaGS: return SystemBandMode::KaBand;
    case SYTHSMJ: return SystemBandMode::SBand;
    case SYTHWX: return SystemBandMode::SBand;
    case SYTHGML: return SystemBandMode::SBand;
    case SYTHSMJK2GZB: return SystemBandMode::SBand;
    case SYTHSMJK2BGZB: return SystemBandMode::SBand;
    case SYTHWXSK2: return SystemBandMode::SBand;
    case SYTHGMLSK2: return SystemBandMode::SBand;
    case SKT: return SystemBandMode::SBand;
    case XGS: return SystemBandMode::XBand;
    case NotDefine: break;
    default: break;
    }
    return NoDefineBand;
}

// 工作模式转为高低频段 Ka为高 S为低
SystemBandLevel SystemWorkModeHelper::isSystemWorkModeHighOrLowBand(SystemWorkMode workMode)
{
    switch (workMode)
    {
    case STTC: return SystemBandLevel::LowLevelBand;
    case Skuo2: return SystemBandLevel::LowLevelBand;
    case KaKuo2: return SystemBandLevel::HighLevelBand;
    case SYTHSMJ: return SystemBandLevel::LowLevelBand;
    case SYTHWX: return SystemBandLevel::LowLevelBand;
    case SYTHGML: return SystemBandLevel::LowLevelBand;
    case SYTHSMJK2GZB: return SystemBandLevel::LowLevelBand;
    case SYTHSMJK2BGZB: return SystemBandLevel::LowLevelBand;
    case SYTHWXSK2: return SystemBandLevel::LowLevelBand;
    case SYTHGMLSK2: return SystemBandLevel::LowLevelBand;
    case SKT: return SystemBandLevel::LowLevelBand;
    default: break;
    }
    return SystemBandLevel::NoDefineLevelBand;
}

/* 枚举转为字符串 */
QString SystemWorkModeHelper::systemWorkModeToString(SystemWorkMode workMode)
{
    switch (workMode)
    {
    case STTC: return QString("STTC");
    case Skuo2: return QString("Skuo2");
    case KaKuo2: return QString("KaKuo2");
    case XDS: return QString("XDS");
    case KaDS: return QString("KaDS");
    case KaGS: return QString("KaGS");
    case SYTHSMJ: return QString("SYTHSMJ");
    case SYTHWX: return QString("SYTHWX");
    case SYTHGML: return QString("SYTHGML");
    case SYTHSMJK2GZB: return QString("SYTHSMJK2GZB");
    case SYTHSMJK2BGZB: return QString("SYTHSMJK2BGZB");
    case SYTHWXSK2: return QString("SYTHWXSK2");
    case SYTHGMLSK2: return QString("SYTHGMLSK2");
    case SKT: return QString("SKT");
    case XGS: return QString("XGS");
    case NotDefine: break;
    default: break;
    }
    return QString();
}

QString SystemWorkModeHelper::masterSlaveToDesc(MasterSlave masterSlave)
{
    switch (masterSlave)
    {
    case MasterSlave::Master: return QString("主");
    case MasterSlave::Slave: return QString("备");
    default: return QString("?"); ;
    }
    return QString("?");
}

QString SystemWorkModeHelper::systemOrientationToDesc(SystemOrientation orientation)
{
    switch (orientation)
    {
    case SystemOrientation::LCircumflex: return QString("左旋");
    case SystemOrientation::RCircumflex: return QString("右旋");
    case SystemOrientation::LRCircumflex: return QString("左右旋同时");
    default: return QString("未知");
    }
    return QString("未知");
}

QList<SystemWorkMode> SystemWorkModeHelper::getWorkModes()
{
    auto currProject = ExtendedConfig::curProjectID();
    if (currProject == "4424")
    {
        return { STTC, Skuo2, SYTHSMJ, SYTHWX, SYTHGML, SYTHSMJK2GZB, SYTHSMJK2BGZB, SYTHWXSK2, SYTHGMLSK2, SKT, XGS, KaGS };
    }
    else if (currProject == "4426")
    {
        return { STTC, Skuo2, KaKuo2, SKT, XDS, KaDS, KaGS };
    }
    return {};
}

// 获取全部测控工作模式
QList<SystemWorkMode> SystemWorkModeHelper::getMeasureContrlWorkMode()
{
    auto currProject = ExtendedConfig::curProjectID();
    if (currProject == "4424")
    {
        return { STTC, Skuo2, SYTHSMJ, SYTHWX, SYTHGML, SYTHSMJK2GZB, SYTHSMJK2BGZB, SYTHWXSK2, SYTHGMLSK2, SKT };
    }
    else if (currProject == "4426")
    {
        return { STTC, Skuo2, KaKuo2, SKT };
    }
    return {};
}
QList<SystemWorkMode> SystemWorkModeHelper::getDSContrlWorkMode()
{
    auto currProject = ExtendedConfig::curProjectID();
    if (currProject == "4426")
    {
        return { XDS, KaDS };
    }

    return {};
}
