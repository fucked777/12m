#ifndef TRKBBEPARAMMACROHANDLER_H
#define TRKBBEPARAMMACROHANDLER_H

#include "BaseParamMacroHandler.h"
#include "BusinessMacroCommon.h"
#include "PlanRunMessageDefine.h"
#include <QDomElement>
#include <QObject>

namespace TrkBBE
{
    enum BBETYPE
    {
        CK,  //测控基带
        GS,  //高速基带
        DS,  //低速基带
        TRK  //跟踪基带
    };

    struct ParamInfo
    {
        //映射到跟踪基带
        int unitId;    //跟踪基带对应的单元id
        int targetId;  //目标id
        QString id;    //参数id
        QString desc;  //具体参数描述

        BBETYPE bbe;  //基带类型
        //从哪里取值，可能是测控，高速，低速
        int src_unitId;       //对应的单元id
        int src_targetId;     //是否是多目标  这个值大于0就表示是多目标
        QString src_paramId;  //参数id

        QString rule;  //配置文件中的rule，为特殊处理。比如标准ttc的下行频率
    };

}  // namespace TrkBBE

struct CalibResultInfo;
class TrkBBEParamMacroHandlerImpl;
class TrkBBEParamMacroHandler : public BaseParamMacroHandler
{
    Q_OBJECT
public:
    explicit TrkBBEParamMacroHandler(QObject* parent = nullptr);
    ~TrkBBEParamMacroHandler();

    bool handle() override;

    SystemWorkMode getSystemWorkMode() override;

private:
    bool ckWorkDataSet(const int modeID, SystemWorkMode ckMode, const DeviceID deviceId_CkBBE,
                       QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap, MasterSlave masterSlave = MasterSlave::Master);
    bool scWorkDataSet(const int modeID, SystemWorkMode scMode, const DeviceID deviceId_ScBBE,
                       QMap<int, QMap<int, QMap<QString, QVariant>>>& unitTargetParamMap, MasterSlave masterSlave = MasterSlave::Master);

    //单模式处理  单测控或者单数传
    bool singleModeDeal(DeviceID gzDeviceID);
    //双模式处理  一种测控+一种数传
    bool doubleModeDeal(DeviceID gzDeviceID);
    // 角误差切换
    void configGZDeviceJWC();
    // 相位装订
    void preBinding();
    void bindingToGZJD(const CalibResultInfo& pcInfo, const DeviceID& deviceID);

private:
    TrkBBEParamMacroHandlerImpl* m_impl;
};

class TrkBBEXmlParser : public QObject
{
    Q_OBJECT
public:
    explicit TrkBBEXmlParser(const QString& fileName);

public:
    QList<TrkBBE::ParamInfo> getParamInfos(int modeId) const;
    QMap<int, QList<TrkBBE::ParamInfo>> getAllModesParamInfos() const;

private:
    void parseMainNode(const QDomElement& domEle, int& modeID, QMap<int, QList<TrkBBE::ParamInfo>>& modeParamInfos);
    void parseFieldNode(const QDomElement& domEle, TrkBBE::ParamInfo& paramInfo);

private:
    QMap<int, QList<TrkBBE::ParamInfo>> d_modeParamInfos;  //跟踪基带模式id，具体参数
};

#endif  // TRKBBEPARAMMACROHANDLER_H
