#ifndef NUMBERSTATUSFREQLABEL_H
#define NUMBERSTATUSFREQLABEL_H

#include "ProtocolXmlTypeDefine.h"
#include "StatusLabel.h"

#include <QVariant>

/**
 * @brief The NumberStatusFreqLabel class
 * 控件说明:
 * 这个控件主要解决的问题是动态显示26测控信道的接收频率或发射频率
 * 1.测控信道分别S测控、Ka测控和Ka低速
 * 2.S测控发射频率从测控基带遍历判断出第一个主用基带
 * 然后判断工作模式,如果是S扩二或Ka扩二，那么取主用目标的值
 * 3.取对应的下行频率或上行频率
 *
 * 刷新逻辑
 * 1.在MainScreenControlFactory类中添加类型名为NumberStatusFreqLabel的控件
 * 2.appendDynamicParamWidget把控件添加进需要动态刷新的控件成员变量mDynamicParamWidget里面
 * 3.appendDyanamicDevids,原本的刷新都是挨个遍历QMap<int, QMap<int, QMap<int, QMap<int, QMap<QString, QWidget*>>>>> mWidgetMap这个对象的key
 * 现在我自定义控件需要向多个设备取值,因此,我这边会把所有需要用到的devids添加进成员变量mDynamicDevids中
 * 4.从redis中把所有的需要动态刷新的devid存进DeviceParamWidget.cpp文件的成员变量currentDataMap中
 * 5.把配置文件中的刷新信息存入成员变量QMap<QString, QList<NumberStatusFreqLabel::SumInfo>> d_freqInfos;中
 * 6.按照显示逻辑刷新该控件
 *
 * 属性介绍
 * deviceId、modeId、unitId、targetId、paramId通用属性不做介绍
 * devids该控件中所有用得到的设备,使用连接符;连接,例如:devids="0x4001;0x4002"
 * green里面存储的是一个数值,如果从缓存中取出的值等于green里面的值,那么就表示灯锁定状态
 * red同green
 * 标签名为ParamInfo表示刷值信息;标签名为MainInfo表示主备信息;标签为TargetInfo表示主跟踪目标信息
 */

class NumberStatusFreqLabel : public StatusLabel
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)
    Q_PROPERTY(QString dealType READ getDealType WRITE setDealType)
    Q_PROPERTY(QString uniqueKey READ getUniqueKey WRITE setUniqueKey)
    Q_PROPERTY(QVariantList devids READ getDevids WRITE setDevids)
    Q_PROPERTY(bool isMultiTarget READ getIsMultiTarget WRITE setIsMultiTarget)
public:
    enum TargetType
    {
        K2_Type,
        Multi_Type,
        Single_Type
    };

    struct ParamInfo
    {
        DeviceID deviceId;
        int modeId{ 0 };
        int unitId{ 0 };
        int targetId{ 0 };
        QString paramId;
    };

    struct MainInfo
    {
        DeviceID deviceId;
        int modeId{ 0 };
        int unitId{ 0 };
        int targetId{ 0 };
        QString paramId;
        QString value;
    };

    struct TargetInfo
    {
        DeviceID deviceId;
        int modeId{ 0 };
        int unitId{ 0 };
        int targetId{ 0 };
        QString paramId;
        TargetType type{ Single_Type };
    };

    struct SumInfo
    {
        ParamInfo paramInfo;
        MainInfo mainInfo;
    };

    struct MultiTargetSumInfo
    {
        ParamInfo paramInfo;
        MainInfo mainInfo;
        TargetInfo targetInfo;
    };

    explicit NumberStatusFreqLabel(const QDomElement& domEle, QWidget* parent = nullptr);

    void setValue(const QVariant& value);
    QVariant getValue() const;

    void setDealType(const QString& dealType);
    QString getDealType() const;

    void setUniqueKey(const QString& uniqueKey);
    QString getUniqueKey() const;

    void setDevids(QVariantList& list);
    QVariantList getDevids();

    void setIsMultiTarget(const bool isMultiTarget);
    bool getIsMultiTarget() const;

    QList<SumInfo> getParamInfos();
    QMap<int, QList<MultiTargetSumInfo>> getMultiTargetParamInfos();

private:
    void parseNode(const QDomElement& domEle);
    void parseModeNode(const QDomElement& domEle, int& modeId, QList<MultiTargetSumInfo>& infos);
    void parseItemNode(const QDomElement& domEle, SumInfo& info);
    void parseItemNode(const QDomElement& domEle, MultiTargetSumInfo& info);
    void parseParamInfoNode(const QDomElement& domEle, ParamInfo& info);
    void parseMainInfoNode(const QDomElement& domEle, MainInfo& info);
    void parseTargetInfoNode(const QDomElement& domEle, TargetInfo& info);

private:
    QList<SumInfo> d_SumInfos;
    QMap<int, QList<MultiTargetSumInfo>> d_MultiTargetSumInfos;
    QString d_dealType;
    QString d_uniqueKey;
    QVariantList d_deviceIds;
    bool d_isMultiTarget{ false };
};

#endif  // NUMBERSTATUSFREQLABEL_H
