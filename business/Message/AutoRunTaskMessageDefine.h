#ifndef AUTORUNTASKMESSAGEDEFINE_H
#define AUTORUNTASKMESSAGEDEFINE_H

#include <QString>

#include "GlobalDefine.h"
#include "PhaseCalibrationDefine.h"

//校相过程的数据
struct CalibResult
{
    ACUPhaseCalibrationFreqBand freqBand{ ACUPhaseCalibrationFreqBand::S };  //频段
    PhaseCalibrationStatus calibPhase{ PhaseCalibrationStatus::Error };      //校相状态
    SelfTrackCheckResult selfTrackResult{ SelfTrackCheckResult::NoCheck };   //自跟踪检查结果
    int azimTrackZero{ 0 };                                                  //方位跟踪零点
    int pitchTrackZero{ 0 };                                                 //俯仰跟踪零点
    int azCorrDiffer{ 0 };                                                   //方位光电差修正值
    int pitchCorrDiff{ 0 };                                                  //俯仰光电差修正值
    int directResult{ 0 };                                                   //定向灵敏检查结果
    int azimOrient{ 0 };                                                     //方位定向灵敏度
    int pitchOrient{ 0 };                                                    //俯仰定向灵敏度
    int standDireSensit{ 0 };                                                //定向灵敏度标准值
    int crossExamResu{ 0 };                                                  //交叉耦合检查结果
    int azimCrossCoup{ 0 };                                                  //方位交叉耦合
    int pitchCrossCoup{ 0 };                                                 //俯仰交叉耦合
    int crossLowerLim{ 0 };                                                  //交叉耦合下限值(分母)
    double azpr{ 0 };                                                        //方位校相结果
    double elpr{ 0 };                                                        //俯仰校相结果
    double azge{ 0 };                                                        //方位移相值
    double elge{ 0 };                                                        //俯仰移相值
};

struct CalibResultInfo
{
    SystemWorkMode workMode{ SystemWorkMode::NotDefine };  //工作模式
    QString taskCode;                                      //任务代号
    quint64 equipComb{ 0 };                                //设备组合号
    int dotDrequency{ 0 };                                 //点频号
    double downFreq{ 0.0 };                               //下行频率
    double azpr{ 0.0 };                                    //方位移相值
    double elpr{ 0.0 };                                    //俯仰移相值
    double azge{ 0.0 };                                    //斜率
    double elge{ 0.0 };                                    //斜率
};

//定义全局的ACU,DTE,SAT的设备号
const int ACU_A = 0x1011;
const int ACU_B = 0x1012;
const int SAT_A = 0x8101;
const int SAT_B = 0x8102;
const int SAT_C = 0x8103;

#endif  // AUTORUNTASKMESSAGEDEFINE_H
