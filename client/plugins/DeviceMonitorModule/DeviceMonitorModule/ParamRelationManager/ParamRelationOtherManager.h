#ifndef PARAMRELATIONOTHERMANAGER_H
#define PARAMRELATIONOTHERMANAGER_H

#include "CustomPacketMessageDefine.h"
#include "ProtocolXmlTypeDefine.h"
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>

class ParamRelationOtherManager : public QObject
{
    Q_OBJECT
public:
    explicit ParamRelationOtherManager(QObject* parent = nullptr){};

    // 测控基带标准TTC 遥测模拟源单元
    bool checkParamRelationCkTtcTAS(const QMap<QString, QVariant>& setParamDataMap, QString& errorMsg);
    // 测控基带标准TTC 遥测解调单元
    bool checkParamRelationCkTtcTU(const QMap<QString, QVariant>& setParamData, QString& errorMsg);
    // 测控基带标准TTC 遥控单元
    bool checkParamRelationCkTtcRCU(const QMap<QString, QVariant>& setParamData, QString& errorMsg);
    // 高速基带 I/Q路译码单元
    bool checkParamRelationGSRDU(const QMap<QString, QVariant>& setParamData, QString& errorMsg);
    // 高速基带 模拟源单元
    bool checkParamRelationGSMNY(const QMap<QString, QVariant>& setParamData, QString& errorMsg);
    // 低速基带 解调单元
    bool checkParamRelationDSJT(QMap<QString, QVariant>& setParamData, QString& errorMsg);
    // 低速基带 模拟源单元
    bool checkParamRelationDSMNY(const QMap<QString, QVariant>& setParamData, QString& errorMsg);
    //跟踪基带 标准+低速 扩频+低速  解调单元
    bool checkParamRelationGZJT(const QMap<QString, QVariant>& setParamData, QString& errorMsg);
    // 计算码速率最大值及最小值
    void calcuCodeRateMinMaxValue(int initMinValue, int initMaxValue, double bValue, QString mark, int& minValue, int& maxValue);

    // 测控基带扩频 遥测模拟源单元
    bool checkParamRelationCkSKpTAS(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg);
    // 测控基带扩频 遥测解调单元
    bool checkParamRelationCkSKpTU(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg);
    // 测控基带扩频 遥控单元
    bool checkParamRelationCkSKpRCU(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg);

    // 测控基带 扩跳频 接收单元或者调制单元
    bool checkParamRelationKTJSAndTZ(const QMap<QString, QVariant>& setParamData, QString& errorMsg);

    // 测控基带 一体化上面级+扩二不共载波、一体化上面级+扩二共载波、上行码率+扩二、卫星+扩二 遥测解调 遥测模拟源
    bool checkParamRelationYTHSKpTUAndTAS(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg);

    // 测控基带 一体化卫星/卫星+扩二 遥控单元
    bool checkParamRelationYTHWXRCU(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg);
    // 测控基带 一体化上面级+扩二不共载波、一体化上面级+扩二共载波、上行码率+扩二 遥控单元
    bool checkParamRelationYTHRCU(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg);

    // 联试应答机标准TTC 遥测模拟源单元
    bool checkParamRelationLSTtcTAS(const QMap<QString, QVariant>& setParamData, QString& errorMsg);
    // 联试应答机 一体化上面级+扩二共载波\扩二不共载波、卫星+扩2 遥测模拟源单元
    bool checkParamRelationYTHSMJK2GTtcTAS(const QMap<QString, QVariant>& setParamData, QString& errorMsg);
    //联试应答机终端扩频 模拟源
    bool checkParamRelationLSKuoTAS(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg);
    //前端测试开关网络
    bool checkParamRelationBeforeTest(const QMap<QString, QVariant>& setParamData, QString& errorMsg);

    //判断存储转发存储单元日期的参数问题
    bool checkParamCCZFRelationSU(const QMap<QString, QVariant>& setParamData, QString& errorMsg);

    //判断高速基带时间关系
    bool checkCmdParamGSRelation(const QMap<QString, QVariant>& setParamData, QString& errorMsg);
    //判断存储转发时间关系
    bool checkCmdParamCCZFRelation(const QMap<QString, QVariant>& setParamData, QString& errorMsg, int cmdID, DeviceID& deviceID);

    //时频终端
    bool checkParamSPZDRelationTFTU(const QMap<QString, QVariant>& setParamData, QString& errorMsg);

    //这里是为了处理跟踪基带里X  Ka两个频段的范围的函数，主要是更改命令里面的控件的范围
    //  void setControlNumRage(UnitReportMessage& unitData, QMap<int, QWidget*>& cmdWidgetMap, int modeID, DeviceID& deviceID);

private:
    const QString SET_TITLE = QString("设置");
    int currentStatus = 1;
signals:
};

#endif  // PARAMRELATIONOTHERMANAGER_H
