#ifndef PARAMRELATIONMANAGER_H
#define PARAMRELATIONMANAGER_H

#include "WWidget.h"
#include <QFileInfoList>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QObject>
#include <QSet>
#include <QVariant>

class QScrollArea;
class QDomElement;
class DeviceMonitor;
// 引用参数
struct ReferenceParam
{
    int deviceID;
    int modeID;
    int unitID;
    QString refParamId;  // 引用参数的id
    int targetIdx;       // 如果引用是多目标的，对应的第几个目标

    bool isValid() const { return deviceID != 0x0000 && unitID != 0 && refParamId != ""; }

    ReferenceParam()
    {
        deviceID = 0x0000;
        modeID = 0xFFFF;
        unitID = 0;
        refParamId = "";
        targetIdx = 0;
    }
};

enum ControlEnable
{
    Unusable = 0,
    Usable,
    Default
};

struct Receiver
{
    QString id;
    QString desc;
    ControlEnable isEnable{ Default };
    QMap<QString, QVariant> enums;  // 如果控件是下拉框对应
    QVariant value;                 // 其它输入框
    QVariant minValue;
    QVariant maxValue;

    ReferenceParam refVal;  // 可以是引用指定配置的控件值

    WWidget* control{ nullptr };
};

struct AssociationTriggered
{
    /* 需要获取值的控件ID */
    QList<std::tuple<QString, WWidget*>> controlList;
    // QMap<> controlMap;
    /* Key是多个控件的Value组合出来的
     * 比如有3个控件的值决定其他的控件的值
     * 值分别为 1 "abc"  22.3 那Key就是
     * "1 abc 22.300000"
     *
     * 对应的Value是对应的控件的数据
     */
    QMap<QString, QList<Receiver>> receiverAssociationMap;
};

struct TriggerValue
{
    QString value;
    QString desc;

    QMap<QString, Receiver> receiverMap;

    /* 多对多
     * 一个控件的数据触发
     * 然后会对比其他控件的值
     * 最后根据上述控件的值改变其他控件的状态以及数值
     */
    QMap<QString, AssociationTriggered> associationMap;
};

struct Trigger
{
    QString id;
    QString desc;

    QMap<QString, TriggerValue> trigValMap;

    WWidget* control{ nullptr };
};

// 储存单元或者命令参数的关联关系
struct Relation
{
    QString desc;

    int sysID{ -1 };
    int devID{ -1 };
    int extenID{ -1 };
    int modelID{ 0xFFFF };

    // 如果是单元需要设置下面两个
    int unitID{ -1 };
    QString unitType;

    // 如果是命令需要设置命令号
    int cmdID{ -1 };
    QMap<QString, Trigger> triggerIdMap;
};

class ParamRelationManager : public QObject
{
    Q_OBJECT
public:
    explicit ParamRelationManager(QObject* parent = nullptr);

    void addUnit(DeviceMonitor* device);
    void addCmd(DeviceMonitor* device);
    //添加存储转发的命令关联关系
    // void addCmdSaveAndForWardExtent(SaveAndForwardExtent* save);
private:
    void onTrigger(QMap<QString, Relation>* relationMap, const QString& relationKey, const QString& triggerID, const QVariant& value,
                   DeviceMonitor* device);

private:
    void loadXML(const QString& relationXmlPath);
    void scanXmlFile(QFileInfoList& infoList, const QString& filePath);
    void parseXML(const QDomElement& docElement, QString& triggerId, QString& trigValue, QString& associationID, QString& associationItemID,
                  Relation& relation);
    // 查找该单元是否存在关联关系
    bool findUnitRelation(int sysId, int deviceId, int modeId, const QString& unitType, Relation& relation);
    // 查找该命令是否存在关联关系
    bool findCmdRelation(int sysId, int deviceId, int modeId, int cmdId, Relation& relation);
    void initUnitRelationControl(QScrollArea* scrollArea, Relation& relation);
    void initCmdRelationControl(QGroupBox* cmdGroupBox, Relation& relation);
    // 添加控件关联关系信号槽
    void addParamRelation(QMap<QString, Relation>* relationMap, const Relation& relation, const QString& relationKey, DeviceMonitor* device);
    // 初始化控件最开始时的关联状态
    void initControlStatus(const Relation& relation);
    // 从上报的信息获取指定的值
    //    bool getReportValue(quint64 deviceID, const int modeID, const int unitID, const QString& paramId, QVariant& paramVal, const int targetIdx =
    //    0);

private:
    const QString STATUS_TITLE = QString("状态");
    const QString SET_TITLE = QString("设置");

    QList<Relation> tempUnitParamRelationList;  // 保存从配置文件解析出来的单元参数关系
    QList<Relation> tempCmdParamRelationList;   // 保存从配置文件解析出来的命令参数关系

    // 通过从配置文件中解析出来的关系判断，如果存在保存单控件关联关系到这里
    QMap<QString, Relation> mUnitParamRelationMap; /* List改成Map只是为了索引方便 */
    QMap<QString, Relation> cmdParamRelationMap;
};

#endif  // PARAMRELATIONMANAGER_H
