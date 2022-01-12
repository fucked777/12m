#ifndef PLANRUNMESSAGE_H
#define PLANRUNMESSAGE_H
#include <QMap>
#include <QString>

#include "ConfigMacroMessageDefine.h"
#include "ParamMacroMessage.h"
#include "PhaseCalibrationDefine.h"
#include "SystemWorkMode.h"

// 链路类型
enum class LinkType
{
    Unknown = 0,    // 未知
    RWFS = 1,       // 任务方式链路
    LSYDJYXBH = 2,  // 联试应答机有线闭环
    PKXLBPQBH = 3,  // 偏馈校零变频器闭环
    SPMNYWXBH = 4,  // 射频模拟源无线闭环
    SPMNYYXBH = 5,  // 射频模拟源有线闭环
    ZPBH = 6,       // 中频闭环
    SZHBH = 7,      // 数字化闭环
    LinkReset = 8,  // 链路重置
};

// 手动控制类型
enum class ManualType
{
    Unknown = 0,               // 未知
    ConfigMacro = 1,           // 配置宏
    ParamMacro,                // 参数宏
    ConfigMacroAndParamMacro,  // 配置宏和参数宏
    ResourceRelease,           // 资源释放
    OnekeyXL,                  // 一键校零
    OnekeyXX,                  // 一键校相
};

struct TargetInfo
{
    int targetNo{ 0 };                                     // 目标号
    SystemWorkMode workMode{ SystemWorkMode::NotDefine };  // 该目标的工作模式
    QString taskCode;                                      // 该目标对应的任务代号
    int pointFreqNo{ 0 };                                  // 该目标对应的点频号
};

struct LinkLine
{
    LinkType linkType{ LinkType::Unknown };                // 链路类型
    SystemWorkMode workMode{ SystemWorkMode::NotDefine };  // 工作模式
    QMap<int, TargetInfo> targetMap;                       // 目标信息
    int masterTargetNo{ -1 };                              // 主用目标号 不是主用目标设置为 -1
    MasterType masterType = NONE;                          // 0:表示不区分扩频还是数传，1：扩频 2：数传
};

// 流程运行的命令
struct ManualMessage
{
    ManualType manualType{ ManualType::Unknown };        // 当前执行的控制类型
    MasterSlave linkMasterSlave{ MasterSlave::Master };  // 使用主用还是备用链路

    bool isManualContrl{ false };     // 是否是手动控制界面点击下发的命令
    ConfigMacroData configMacroData;  // 当前流程的设备、旋向等信息

    bool isExternalParamMacro{ false }; /* 是否使用外部参数宏数据 */
    ParamMacroData paramMacroData;      /* 外部传入的参数宏 */

    // 是否给ACU下参数宏
    bool acuParam{ true };
    /*
     * 20211114 将DTE的计划移动到参数宏下发里面
     * 当是长期任务时 时间无效 当前DTE没有这个任务类型 直接下的是3年的计划
     * 否则时间有效
     */
    bool ltDTETask{ true };
    QDateTime dteTraceStartDateTime;
    QDateTime dteTaskEndDateTime;
    /* 资源释放删除DTE任务 */
    bool resourceReleaseDeleteDTETask{ true };
    /* 资源释放删除存转任务 */
    bool resourceReleaseDeleteCZTask{ true };
    /* 资源释放删除ACU任务 */
    bool resourceReleaseDeleteACUTask{ true };

    /* 是否是测试 */
    bool isTest{ false };
    /* 校零模式 默认代表所有模式都校零 否则只校单个
     * 因为测控混合模式会出问题
     */
    SystemWorkMode xlMode{ SystemWorkMode::NotDefine };
    /* 校相模式 默认是根据当前卫星配置校相
     * 否则根据此字段来配置
     */
    ACUPhaseCalibrationFreqBand xxMode{ ACUPhaseCalibrationFreqBand::Unknown };

    QMap<SystemWorkMode, LinkLine> linkLineMap;  // QMap<链路模式, 链路信息>

    // 设备组合号
    quint64 devNumber{ 0 };

    // 主跟目标代号
    QString masterTaskCode;  //主跟任务代号
    QString masterTaskBz;    //主跟任务标识

    // 已经处理好了16进制
    QString txUACAddr; /* 天线UAC地址/站UAC地址 */

    // 添加链路，如果存在相同模式的链路，就合并链路下的目标
    inline void appendLine(const LinkLine& newLinkLine)
    {
        // 如果不包含新链路的模式，直接添加
        if (!linkLineMap.contains(newLinkLine.workMode))
        {
            linkLineMap[newLinkLine.workMode] = newLinkLine;
            return;
        }

        // 再原始链路上，添加新目标
        auto& originLink = linkLineMap[newLinkLine.workMode];
        auto lastTargetNo = originLink.targetMap.lastKey();

        for (auto targetInfo : newLinkLine.targetMap)
        {
            ++lastTargetNo;
            targetInfo.targetNo = lastTargetNo;
            originLink.targetMap[lastTargetNo] = targetInfo;
        }
    }
    inline bool isDoubleMode() const { return (linkLineMap.contains(Skuo2) && linkLineMap.contains(KaKuo2)); }
    inline bool isContainsCkMode() const
    {
        if (linkLineMap.contains(STTC) || linkLineMap.contains(Skuo2) || linkLineMap.contains(KaKuo2) || linkLineMap.contains(SKT))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

class PlanRunHelper
{
public:
    static QString getLinkTypeDesc(LinkType linkType)
    {
        switch (linkType)
        {
        case LinkType::RWFS: return QString("任务方式");
        case LinkType::LSYDJYXBH: return QString("联试应答机有线闭环");
        case LinkType::PKXLBPQBH: return QString("偏馈校零变频器闭环");
        case LinkType::SPMNYWXBH: return QString("射频模拟源无线闭环");
        case LinkType::SPMNYYXBH: return QString("射频模拟源有线闭环");
        case LinkType::ZPBH: return QString("中频闭环");
        case LinkType::SZHBH: return QString("数字化闭环");
        case LinkType::LinkReset: return QString("链路重置");
        case LinkType::Unknown: break;
        }

        return QString("未知");
    }

    static QString getLinkMasterSlaveDesc(MasterSlave type)
    {
        switch (type)
        {
        case MasterSlave::Master: return QString("主用链路");
        case MasterSlave::Slave: return QString("备用链路");
        case MasterSlave::Unkonwn: break;
        }
        return QString("未知");
    }
};

#endif  // PLANRUNMESSAGE_H
