#ifndef PACKDATABUILDER_H
#define PACKDATABUILDER_H
#include "JsonHelper.h"
#include <Utility.h>
struct CmdRequest;
struct UnitParamRequest;
class PackDataBuilder
{
public:
    /*
     * 此函数是将命令直接转换成Map
     * 全部是默认的值
     * 此函数只考虑key-value对应的情况 其他的什么多目标啥的不考虑
     */
    static Optional<QMap<QString, QVariant>> cmdToVarMap(int sysNum, int devNum, int extNum, int modeNum, int cmdID);

    static Optional<QMap<QString, QVariant>> unitToVarMap(int sysNum, int devNum, int extNum, int modeNum, int unitID);

    //查看了引用,虽然现在只有系统框图使用到这个接口，但是我还是人为做区分
    //cmdToCmdRequest只打包过程控制命令 unitToCmdRequest只打包单元参数控制命令
    static Optional<CmdRequest> cmdToCmdRequest(int sysNum, int devNum, int extNum, int modeNum, int cmdID,
                                                const QMap<QString, QVariant>& replace = {});
    static Optional<UnitParamRequest> unitToCmdRequest(int sysNum, int devNum, int extNum, int modeNum, int unitID,
                                                const QMap<QString, QVariant>& replace = {});
};

#endif  // PACKDATABUILDER_H
